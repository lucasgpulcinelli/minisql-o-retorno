#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "bTree.h"
#include "file_control.h"
#include "utils.h"

bTree* openBTree(char* data_filename, char* indices_filename,
                   const char* mode){
    bTree* bt;
    XALLOC(bTree, bt, 1);

    bt->table = openTable(data_filename, mode);
    bt->tree = openIndexTree(indices_filename, mode);

    return bt;
}

indexTree* openIndexTree(char* filename, const char* mode){
    indexTree* it;
    XALLOC(indexTree, it, 1);
    OPEN_FILE(it->fp, filename, mode);

    it->status = fgetc(it->fp);
    if(it->status != OK_HEADER){
        //if the header is not right, nothing can be considered valid, abort 
        //the program with an error message.
        EXIT_ERROR();
    }

    fread(&(it->root_node_rrn), sizeof(int32_t), 1, it->fp);
    fread(&(it->total_keys), sizeof(int32_t), 1, it->fp);
    fread(&(it->height), sizeof(int32_t), 1, it->fp);
    fread(&(it->next_node_rrn), sizeof(int32_t), 1, it->fp);
    it->nodes_read = 0;

    if(strchr(mode, 'w') != NULL || strchr(mode, '+') != NULL){
        it->status = ERR_HEADER;
        it->read_only = false;
        writeIndexTreeHeader(it);

    } else {
        it->read_only = true;
    }

    fseek(it->fp, INDICES_PAGE_SIZE, SEEK_SET);
    return it;
}

bTree* createBTreeFromTable(char* table_name, char* indices_filename){
    bTree* new_tree;
    XALLOC(bTree, new_tree, 1);

    new_tree->table = openTable(table_name, "r+b");
    new_tree->tree = createIndexTree(indices_filename);

    for(uint32_t table_rrn = 0;tableHasNextEntry(new_tree->table); table_rrn++){
        entry* es = tableReadNextEntry(new_tree->table);

        if(!ENTRY_REMOVED(es)){
            treeEntry* te = createTreeEntry(es, table_rrn);
            insertEntryInIndexTree(new_tree->tree, te);
            freeTreeEntry(te);
        }

        deleteEntry(es, 1);
    }

    return new_tree;
}

void closeIndexTree(indexTree* it){
    if(!(it->read_only)){
        it->status = OK_HEADER;
        writeIndexTreeHeader(it);
    }

    fclose(it->fp);
    free(it);
}

void bTreeTablehashOnScreen(bTree* bt){
    tableHashOnScreen(bt->table);
}

void bTreeIndexTreehashOnScreen(bTree* bt){
    rewind(bt->tree->fp);
    uint32_t sum = 0;
	for(int c = getc(bt->tree->fp); c != EOF; c = getc(bt->tree->fp)){
		sum += c;
	}

	printf("%lf\n", sum / (double) 100);
}

void closeBTree(bTree* bt){
    closeTable(bt->table);
    closeIndexTree(bt->tree);
    free(bt);
}

void rewindBTree(bTree* bt){
    rewindTable(bt->table);
}

entry* bTreeReadNextEntry(bTree* bt){
    return tableReadNextEntry(bt->table);
}

bool bTreeHasNextEntry(bTree* bt){
    return tableHasNextEntry(bt->table);
}

indexNode* createIndexNode(int height, int node_rrn, char leaf){
    indexNode* in;
    MEMSET_ALLOC(indexNode, in, 1, EMPTY_VALUE);

    in->leaf = leaf;
    in->keys = 0;
    in->height = height;
    in->node_rrn = node_rrn;

    return in;
}

void printIndexNode(indexTree* it, indexNode* in){
    if(in == NULL){
        return;
    }

    int i;
    for(i = 0; i < in->keys; i++){
        indexNode* next = readIndexNode(it, in->data[i][branch_rrn]);
        printIndexNode(it, next);
        if(next != NULL){
            free(next);
        }

        if(in->data[i][data_value] == -1){
            continue;
        }
        printf("C%d_%d: %d\n", i+1, in->node_rrn, in->data[i][data_value]);
    }

    indexNode* next = readIndexNode(it, in->data[i][branch_rrn]);

    printIndexNode(it, next);

    if(next != NULL){
        free(next);
    }
}

void printIndexTree(indexTree* it){
    printf("header: \n");
    printf("status: %d, root_node_rrn: %d, total_keys: %d\n", 
           it->status, it->root_node_rrn, it->total_keys);
    printf("height: %d, next_node_rnn: %d\n\n", it->height, it->next_node_rrn);

    indexNode* in = readIndexNode(it, it->root_node_rrn);
    printIndexNode(it, in);
}

indexNode* readCurNode(indexTree* it){
    indexNode* in;
    MEMSET_ALLOC(indexNode, in, 1, -1);

    int read = getc(it->fp);
    if(read == EOF){
        //if we can't read a char, we are at the end of the file, so return NULL
        free(in);
        return NULL;
    }

    //read node metadata
    in->leaf = read;
    fread(&(in->keys), sizeof(int32_t), 1, it->fp);
    fread(&(in->height), sizeof(int32_t), 1, it->fp);
    fread(&(in->node_rrn), sizeof(int32_t), 1, it->fp);

    //read all search keys (all an not just in->key ensures in->data is not RAM
    //trash data but well defined -1 and '$'). Because this is a single disk 
    //page there is no practical difference in time.
    for(int i = 0; i < SEARCH_KEYS; i++){
        for(int j = 0; j < BRANCH_METADATA_SIZE; j++){
            fread(&(in->data[i][j]), sizeof(int32_t), 1, it->fp);
        }
    }

    //last array index does not have an entry associated.
    fread(&(in->data[BRANCHES-1][branch_rrn]), sizeof(int32_t), 1, it->fp);
    return in;
}

indexNode* readIndexNode(indexTree* it, int32_t rrn){
    if(rrn == EMPTY_RRN){
        return NULL;
    }else{
        fseek(it->fp, INDICES_PAGE_SIZE*(rrn + 1), SEEK_SET);
        return readCurNode(it);
    }
}

void writeIndexNode(indexTree* it, indexNode* in){
    if(it->read_only){
        errno = EACCES;
        ABORT_PROGRAM("index tree opened in read only mode");
    }

    fseek(it->fp, INDICES_PAGE_SIZE*(in->node_rrn + 1), SEEK_SET);
    fwrite(&(in->leaf), sizeof(char), 1, it->fp);
    fwrite(&(in->keys), sizeof(int32_t), 1, it->fp);
    fwrite(&(in->height), sizeof(int32_t), 1, it->fp);
    fwrite(&(in->node_rrn), sizeof(int32_t), 1, it->fp);

    for(size_t index = 0; index < SEARCH_KEYS; index++){
        fwrite(&(in->data[index][branch_rrn]), sizeof(int32_t), 1, it->fp);
        fwrite(&(in->data[index][data_value]), sizeof(int32_t), 1, it->fp);
        fwrite(&(in->data[index][data_rrn]), sizeof(int32_t), 1, it->fp);
    }   

    fwrite(&(in->data[SEARCH_KEYS][branch_rrn]), sizeof(int32_t), 1, it->fp);
}

void freeIndexNode(indexNode* in){
    free(in);
}

int32_t indexNodeSearch(indexTree* it, int32_t curr_rrn, int32_t value){
    //if we are in an invalid node, the value does not exist in the tree
    if(curr_rrn == -1){
        return -1;
    }

    //go to the index provided and read the node
    indexNode* node = readIndexNode(it, curr_rrn);
    it->nodes_read++;

    for(int i = 0; i < SEARCH_KEYS; i++){
        if(node->data[i][data_value] == value){
            //if the entry has the primary key value, we found the answer
            int32_t ret = node->data[i][data_rrn];
            freeIndexNode(node);
            return ret;
        }

        if(node->data[i][data_value] > value 
           || node->data[i][data_value] == -1){
            //if the entry key is greater than what we need, the value could be 
            //on the left (in our case the same index in the array)
            int rrn = node->data[i][branch_rrn];
            freeIndexNode(node);
            return indexNodeSearch(it, rrn, value);
        }
    }

    //if the value is greater than all entries, it could only be in the last
    //branch
    int rrn = node->data[SEARCH_KEYS][branch_rrn];
    freeIndexNode(node);
    return indexNodeSearch(it, rrn, value);
}

entry* bTreeSearch(bTree* bt, int32_t value){
    //search starts at the root node
    int32_t rrn = indexNodeSearch(bt->tree, bt->tree->root_node_rrn, value);
    if(rrn == -1){
        //if it is not found, return NULL
        return NULL;
    }

    //return the entry at the given rrn
    seekTable(bt->table, rrn);
    return tableReadNextEntry(bt->table);
}

void writeIndexTreeHeader(indexTree* it){
    if(it->read_only){
        errno = EACCES;
        ABORT_PROGRAM("index tree opened in read only mode");
    }

    rewind(it->fp);
    fwrite(&(it->status), sizeof(char), 1, it->fp);
    fwrite(&(it->root_node_rrn), sizeof(int32_t), 1, it->fp);
    fwrite(&(it->total_keys), sizeof(int32_t), 1, it->fp);
    fwrite(&(it->height), sizeof(int32_t), 1, it->fp);
    fwrite(&(it->next_node_rrn), sizeof(int32_t), 1, it->fp);

    for(uint32_t i = 0; i < INDICES_PAGE_SIZE - INDICES_HEADER_SIZE; i++){
        putc('$', it->fp);
    }
}

indexTree* createIndexTree(char* indices_filename){
    indexTree* it;
    XALLOC(indexTree, it, 1);

    OPEN_FILE(it->fp, indices_filename, "w+b");
    it->read_only = false;
    it->status = ERR_HEADER;
    it->root_node_rrn = EMPTY_RRN;
    it->total_keys = 0;
    it->height = 0;
    it->next_node_rrn = 0;

    writeIndexTreeHeader(it);
    return it;
}

treeEntry* createTreeEntry(entry* es, int32_t rrn){
    treeEntry* te;
    MEMSET_ALLOC(treeEntry, te, 1, -1);

    te->key.idConnect = es->fields[idConnect].value.integer;
    te->key.rrn = rrn;

    return te;
}

void freeTreeEntry(treeEntry* te){
    free(te);
}

void insertEntryInBTree(bTree* bt, entry* es){
    if(bt->tree->read_only || bt->table->read_only){
        errno = EACCES;
        ABORT_PROGRAM("index tree opened in read only mode");
    }

    int32_t rrn = appendEntryOnTable(bt->table, es);

    treeEntry* te = createTreeEntry(es, rrn);
    insertEntryInIndexTree(bt->tree, te);
    free(te);
}

void insertEntryInIndexTree(indexTree* it, treeEntry* te){
    if(it == NULL){
        errno = EINVAL;
        ABORT_PROGRAM("cannot insert in null index tree\n");

    } else if (te == NULL){
        errno = EINVAL;
        ABORT_PROGRAM("cannot insert null entry\n");
    }

    treeEntry* carry_on = recursiveNodeInsert(it, it->root_node_rrn, te);
    it->total_keys++;

    if(carry_on != NULL){
        indexNode* root = createIndexNode(it->height + 1, it->next_node_rrn,
            (it->root_node_rrn == EMPTY_RRN)? LEAF : NOT_LEAF);
        setIndexNode(root, carry_on, 0);
        root->keys++;

        if(it->root_node_rrn != EMPTY_RRN){
            free(carry_on);
        }

        it->height++;
        it->root_node_rrn = it->next_node_rrn;
        it->next_node_rrn++;

        writeIndexNode(it, root);
        free(root);
    }
}

void setIndexNode(indexNode* in, treeEntry* te, size_t index){
    in->data[index][branch_rrn] = te->lower_subtree;
    in->data[index][data_value] = te->key.idConnect;
    in->data[index][data_rrn] = te->key.rrn;
    in->data[index + 1][branch_rrn] = te->upper_subtree;
}

treeEntry* recursiveNodeInsert(indexTree* it, int32_t rrn, treeEntry* te){
    indexNode* in = readIndexNode(it, rrn);
    if(in == NULL){
        return te;

    } else if(IS_LEAF(in)){
        treeEntry* carry = insertEntryInNode(it, in, te);
        free(in);
        return carry;
    }

    int32_t branch = 0;
    while(in->data[branch][data_value] != EMPTY_VALUE &&
          in->data[branch][data_value] < te->key.idConnect &&
          branch < SEARCH_KEYS){
        branch++;
    }

    treeEntry* carry_on = recursiveNodeInsert(it, in->data[branch][branch_rrn], te);
    if(carry_on != NULL){
        treeEntry* next_carry = insertEntryInNode(it, in, carry_on);
        free(carry_on);
        carry_on = next_carry;
    }

    free(in);
    return carry_on;
}

treeEntry* insertEntryInNode(indexTree* it, indexNode* in, treeEntry* te){
    if(in->keys == SEARCH_KEYS){
        return splitAndInsert(it, in, te);
    }

    ssize_t key_index = in->keys - 1;
    while(in->data[key_index][data_value] > te->key.idConnect){
        translateToTheRight(in->data, key_index);
        key_index--;
    }

    key_index++;
    setIndexNode(in, te, key_index);
    in->keys++;

    writeIndexNode(it, in);
    return NULL;
}

void translateToTheRight(int32_t data[BRANCHES][BRANCH_METADATA_SIZE], 
                         ssize_t index){
    data[index + 2][branch_rrn] = data[index + 1][branch_rrn];
    data[index + 1][branch_rrn] = data[index][branch_rrn];
    data[index + 1][data_value] = data[index][data_value];
    data[index + 1][data_rrn] = data[index][data_rrn];
}

treeEntry* splitAndInsert(indexTree* it, indexNode* in, treeEntry* te){
    if(in->keys < SEARCH_KEYS){
        errno = ENOTSUP;
        ABORT_PROGRAM("Cannot split a node that is not full");
    }

    treeEntry* entries;
    XALLOC(treeEntry, entries, SEARCH_KEYS + 1);
    for(size_t index = 0; index < SEARCH_KEYS; index++){
        entries[index].lower_subtree = in->data[index][branch_rrn];
        entries[index].key.idConnect = in->data[index][data_value];
        entries[index].key.rrn = in->data[index][data_rrn];
        entries[index].upper_subtree = in->data[index + 1][branch_rrn];
    }

    setTreeEntry(entries + SEARCH_KEYS, te);

    treeEntry *tmp;
    ssize_t te_index = SEARCH_KEYS; 
    XALLOC(treeEntry, tmp, 1);
    while(te_index > 0 && 
          entries[te_index - 1].key.idConnect > entries[te_index].key.idConnect){
        setTreeEntry(tmp, entries + te_index);
        setTreeEntry(entries + te_index, entries + te_index - 1);
        setTreeEntry(entries + te_index - 1, tmp);
        te_index--;
    }

    free(tmp);
    indexNode* split = createIndexNode(in->height, it->next_node_rrn, in->leaf);
    it->next_node_rrn++;

    memset(&(in->data), -1, BRANCHES*BRANCH_METADATA_SIZE*sizeof(int32_t));
    for(size_t i = 0; i < MEDIAN; i++){
        setIndexNode(in, entries + i, i);
        setIndexNode(split, entries + i + MEDIAN + 1, i);
    }

    if(te_index < MEDIAN){
        setIndexNode(in, entries + te_index, te_index);

    } else if(te_index > MEDIAN){
        setIndexNode(split, entries + te_index, te_index - MEDIAN - 1);

    } else {
        in->data[MEDIAN][branch_rrn] = entries[MEDIAN].lower_subtree;
        split->data[0][branch_rrn] = entries[MEDIAN].upper_subtree;
    }

    in->keys = SEARCH_KEYS/2;
    split->keys = SEARCH_KEYS/2;
    
    treeEntry* carry;
    XALLOC(treeEntry, carry, 1);
    setTreeEntry(carry, entries + MEDIAN);
    carry->lower_subtree = in->node_rrn;
    carry->upper_subtree = split->node_rrn;

    writeIndexNode(it, split);
    writeIndexNode(it, in);
    free(split);
    free(entries);

    return carry;
}

void setTreeEntry(treeEntry* te, treeEntry* set){
    te->lower_subtree = set->lower_subtree;
    te->key.idConnect = set->key.idConnect;
    te->key.rrn = set->key.rrn;
    te->upper_subtree = set->upper_subtree;
}
