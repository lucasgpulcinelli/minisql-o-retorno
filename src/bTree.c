#include <stdlib.h>
#include <errno.h>

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

    return it;
}

bTree* createBTreeFromTable(char* table_name, char* indices_filename){
    bTree* new_tree;
    XALLOC(bTree, new_tree, 1);

    new_tree->table = openTable(table_name, "r+b");
    new_tree->tree = createIndexTree(indices_filename);

    while(tableHasNextEntry(new_tree->table)){
        entry* es = tableReadNextEntry(new_tree->table);

        if(!ENTRY_REMOVED(es)){
            treeEntry* te = createTreeEntry(es, new_tree->tree->next_node_rrn);
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

void swapValues(int32_t (*data)[BRANCH_METADATA_SIZE], ssize_t index){
    int32_t temp_data_value = data[index - 1][data_value];
    int32_t temp_data_rrn = data[index - 1][data_rrn];

    data[index - 1][data_value] = data[index][data_value];
    data[index - 1][data_rrn] = data[index][data_rrn];

    data[index][data_value] = temp_data_value;
    data[index][data_rrn] = temp_data_rrn;
}

treeCarryOn* insertEntryInLeaf(indexTree* it, indexNode* in, treeEntry* te){
    if(IS_NOT_LEAF(in)){
        errno = EINVAL;
        ABORT_PROGRAM("cannot insert in non-leaf node\n");
    }

    if(in->keys == SEARCH_KEYS){
        treeCarryOn* carry = splitNode(it, in);

        if(carry->key.idConnect < te->idConnect){
            indexNode* upper = readIndexNode(it, carry->upper_subtree);
            insertEntryInLeaf(it, upper, te);
            free(upper);
        } else {
            insertEntryInLeaf(it, in, te);
        }

        return carry;
    }

    in->data[SEARCH_KEYS - 1][data_value] = te->idConnect;
    in->data[SEARCH_KEYS - 1][data_rrn] = te->rrn;
    in->keys++;

    for(ssize_t swap = SEARCH_KEYS - 1; swap > 0; swap--){
        if(in->data[swap - 1][data_value] == EMPTY_VALUE ||
           in->data[swap - 1][data_value] > in->data[swap][data_value]){
            swapValues(in->data, swap);
        }
    }

    writeIndexNode(it, in);
    return NULL;
}

indexNode* readCurNode(indexTree* it){
    indexNode* in;
    XALLOC(indexNode, in, 1);

    in->leaf = getc(it->fp);
    fread(&(in->keys), sizeof(int32_t), 1, it->fp);
    fread(&(in->height), sizeof(int32_t), 1, it->fp);
    fread(&(in->node_rrn), sizeof(int32_t), 1, it->fp);

    for(int i = 0; i < SEARCH_KEYS; i++){
        for(int j = 0; j < 3; j++){
            fread(&(in->data[i][j]), sizeof(int32_t), 1, it->fp);
        }
    }

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

    fwrite(&(in->data[BRANCHES][branch_rrn]), sizeof(int32_t), 1, it->fp);
}

void freeIndexNode(indexNode* in){
    free(in);
}

int32_t indexNodeSearch(indexTree* it, int32_t curr_rrn, int32_t value){
    if(curr_rrn == -1){
        return -1;
    }

    fseek(it->fp, (curr_rrn+1) * INDICES_PAGE_SIZE, SEEK_SET);
    indexNode* node = readCurNode(it);
    it->nodes_read++;

    for(int i = 0; i < SEARCH_KEYS; i++){
        if(node->data[i][data_value] == value){
            int32_t ret = node->data[i][data_rrn];
            freeIndexNode(node);
            return ret;
        }

        if(node->data[i][data_value] > value 
           || node->data[i][data_value] == -1){

            int rrn = node->data[i][branch_rrn];
            freeIndexNode(node);
            return indexNodeSearch(it, rrn, value);
        }
    }

    int rrn = node->data[SEARCH_KEYS][branch_rrn];
    freeIndexNode(node);
    return indexNodeSearch(it, rrn, value);
}

entry* bTreeSearch(bTree* bt, int32_t value){
    int32_t rrn = indexNodeSearch(bt->tree, bt->tree->root_node_rrn, value);
    if(rrn == -1){
        return NULL;
    }

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

    it->fp = fopen(indices_filename, "w+b");
    it->status = ERR_HEADER;
    it->root_node_rrn = EMPTY_TREE_ROOT_RRN;
    it->total_keys = 0;
    it->height = 0;
    it->next_node_rrn = 0;

    writeIndexTreeHeader(it);
    return it;
}

treeEntry* createTreeEntry(entry* es, int32_t rrn){
    treeEntry* te;
    XALLOC(treeEntry, te, 1);

    te->idConnect = es->fields[idConnect].value.integer;
    te->rrn = rrn;

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

    es++;
}

void insertEntryInIndexTree(indexTree* it, treeEntry* te){
    if(it == NULL){
        errno = EINVAL;
        ABORT_PROGRAM("pointer to index tree is null\n");

    } else if (te == NULL){
        errno = EINVAL;
        ABORT_PROGRAM("pointer to tree entry is null\n");
    }

    treeCarryOn* carry_on = recursiveNodeInsert(it, it->root_node_rrn, te);

    if(it->root_node_rrn == EMPTY_RRN || carry_on != NULL){
        it->height++;
        indexNode* root = createIndexNode(it->height, it->next_node_rrn,
                                         (carry_on == NULL)? LEAF : NOT_LEAF);
        it->root_node_rrn = root->node_rrn;
        it->next_node_rrn++;
        it->total_keys++;

        if(carry_on != NULL){
            root->data[0][branch_rrn] = carry_on->lower_subtree;
            root->data[0][data_value] = carry_on->key.idConnect;
            root->data[0][data_rrn] = carry_on->key.rrn;
            root->data[1][branch_rrn] = carry_on->upper_subtree;
            free(carry_on);
        }

        writeIndexNode(it, root);
        free(root);
    }
}

treeCarryOn* recursiveNodeInsert(indexTree* it, int32_t rrn, treeEntry* te){
    indexNode* in = readIndexNode(it, rrn);
    if(in == NULL){
        return NULL;

    } else if(IS_LEAF(in)){
        return insertEntryInLeaf(it, in, te);
    }

    int32_t branch = 0;
    while(in->data[branch][data_value] != EMPTY_VALUE &&
          in->data[branch][data_value] < te->idConnect && branch++);

    treeCarryOn* carry_on = recursiveNodeInsert(it, in->data[branch][branch_rrn], te);
    if(carry_on != NULL){
        treeCarryOn* next_carry = insertCarryOn(it, carry_on, branch, in);
        free(carry_on);
        free(in);

        return next_carry;
    }

    free(in);
    return NULL;
}

treeCarryOn* insertCarryOn(indexTree* it, treeCarryOn* carry_on, int32_t branch, 
                           indexNode* in){
    if(in->keys == SEARCH_KEYS){
        treeCarryOn* carry = splitNode(it, in);

        if(branch > MEDIAN){
            indexNode* upper = readIndexNode(it, carry->upper_subtree);
            insertCarryOn(it, carry_on, branch - MEDIAN - 1, in);
            free(upper);
        } else {
            insertCarryOn(it, carry_on, branch, in);
        }

        return carry;
    }

    for(int32_t key = SEARCH_KEYS; key > branch; key--){
        in->data[key][branch_rrn] = in->data[key - 1][branch_rrn];
        in->data[key][data_value] = in->data[key - 1][data_value];
        in->data[key][data_rrn] = in->data[key - 1][data_rrn];
    }

    in->data[branch][branch_rrn] = carry_on->lower_subtree;
    in->data[branch][data_value] = carry_on->key.idConnect;
    in->data[branch][data_rrn] = carry_on->key.rrn;
    in->data[branch + 1][branch_rrn] = carry_on->upper_subtree;

    return NULL;
}

treeCarryOn* splitNode(indexTree* it, indexNode* in){
    if(in->keys < SEARCH_KEYS){
        errno = ENOTSUP;
        ABORT_PROGRAM("Cannot split a node that is not full");
    }

    indexNode* split = createIndexNode(in->height, it->next_node_rrn, in->leaf);
    it->next_node_rrn++;

    for(int32_t key = MEDIAN + 1; key < SEARCH_KEYS; key++){
        split->data[key - MEDIAN][branch_rrn] = in->data[key][branch_rrn];
        split->data[key - MEDIAN][data_value] = in->data[key][data_value];
        split->data[key - MEDIAN][data_rrn] = in->data[key][data_rrn];
        split->keys++;

        in->data[key][branch_rrn] = EMPTY_RRN;
        in->data[key][data_value] = EMPTY_VALUE;
        in->data[key][data_rrn] = EMPTY_RRN;
    }
    
    treeCarryOn* carry;
    XALLOC(treeCarryOn, carry, 1);
    carry->lower_subtree = in->node_rrn;
    carry->upper_subtree = split->node_rrn;

    in->data[MEDIAN][data_value] = EMPTY_VALUE;
    in->data[MEDIAN][data_rrn] = EMPTY_RRN;

    writeIndexNode(it, in);
    writeIndexNode(it, split);

    return carry;
}

