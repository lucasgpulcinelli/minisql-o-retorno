#include <stdlib.h>

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

    return it;
}

void closeIndexTree(indexTree* it){
    writeIndexTreeHeader(it);
    fclose(it->fp);
    free(it);
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

indexNode* readIndexNode(indexTree* it){
    indexNode* in;
    XALLOC(indexNode, in, 1);

    //read node metadata
    in->leaf = getc(it->fp);
    fread(&(in->keys), sizeof(int32_t), 1, it->fp);
    fread(&(in->height), sizeof(int32_t), 1, it->fp);
    fread(&(in->node_rrn), sizeof(int32_t), 1, it->fp);

    //read all search keys (all an not just in->key ensures in->data is not RAM
    //trash data but well defined -1 and '$'). Because this is a single disk 
    //page there is no practical difference in time.
    for(int i = 0; i < SEARCH_KEYS; i++){
        for(int j = 0; j < 3; j++){
            fread(&(in->data[i][j]), sizeof(int32_t), 1, it->fp);
        }
    }

    //last array index does not have an entry associated.
    fread(&(in->data[BRANCHES-1][branch_rrn]), sizeof(int32_t), 1, it->fp);

    return in;
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
    fseek(it->fp, (curr_rrn+1) * INDICES_PAGE_SIZE, SEEK_SET);
    indexNode* node = readIndexNode(it);
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
    rewind(it->fp);

    fwrite(&(it->status), sizeof(char), 1, it->fp);
    fwrite(&(it->root_node_rrn), sizeof(int32_t), 1, it->fp);
    fwrite(&(it->total_keys), sizeof(int32_t), 1, it->fp);
    fwrite(&(it->height), sizeof(int32_t), 1, it->fp);
    fwrite(&(it->next_node_rrn), sizeof(int32_t), 1, it->fp);
    fwrite("$", sizeof(char), INDICES_PAGE_SIZE - INDICES_HEADER_SIZE, 
        it->fp);
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
