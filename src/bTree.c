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

indexNode* createIndexNode(int height, int node_rrn, char leaf){
    indexNode* in;
    MEMSET_ALLOC(indexNode, in, 1, EMPTY_VALUE);

    in->leaf = leaf;
    in->keys = 0;
    in->height = height;
    in->node_rrn = node_rrn;

    return in;
}

void swapValues(int32_t (*data)[BRANCH_METADATA_SIZE], ssize_t index) {
    int32_t temp_data_value = data[index - 1][data_value];
    int32_t temp_data_rrn = data[index - 1][data_rrn];

    data[index - 1][data_value] = data[index][data_value];
    data[index - 1][data_rrn] = data[index][data_rrn];

    data[index][data_value] = temp_data_value;
    data[index][data_rrn] = temp_data_rrn;
}

void insertEntryInLeaf(indexNode* in, treeEntry* te){
    if(IS_NOT_LEAF(in)){
        errno = EINVAL;
        ABORT_PROGRAM("cannot insert in non-leaf node\n");
    }

    if(in->keys == SEARCH_KEYS){
        errno = EINVAL;
        ABORT_PROGRAM("cannot insert in a leaf node that is full\n");
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

void freeIndexNode(indexNode* in){
    free(in);
}

int32_t indexNodeSearch(indexTree* it, int32_t curr_rrn, int32_t value){
    if(curr_rrn == -1){
        return -1;
    }

    indexNode* node = readIndexNode(it, curr_rrn);
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
    rewind(it->fp);
    char dollar_sign = '$';

    fwrite(&(it->status), sizeof(char), 1, it->fp);
    fwrite(&(it->root_node_rrn), sizeof(int32_t), 1, it->fp);
    fwrite(&(it->total_keys), sizeof(int32_t), 1, it->fp);
    fwrite(&(it->height), sizeof(int32_t), 1, it->fp);
    fwrite(&(it->next_node_rrn), sizeof(int32_t), 1, it->fp);
    fwrite(&dollar_sign, sizeof(char), INDICES_PAGE_SIZE - INDICES_HEADER_SIZE,
        it->fp);
}

indexTree* createIndexTree(char* indices_filename){
    indexTree* it;
    XALLOC(indexTree, it, 1);

    OPEN_FILE(it->fp, indices_filename, "w+b");
    it->status = ERR_HEADER;
    it->root_node_rrn = EMPTY_RRN;
    it->total_keys = 0;
    it->height = 0;
    it->next_node_rrn = 0;

    writeIndexTreeHeader(it);
    return it;
}

treeEntry* createTreeEntry(entry* es, int32_t rrn) {
    treeEntry* te;
    XALLOC(treeEntry, te, 1);

    te->idConnect = es->fields[idConnect].value.integer;
    te->rrn = rrn;

    return te;
}

void freeTreeEntry(treeEntry* te) {
    free(te);
}

void insertEntryInIndexTree(indexTree* it, entry* es){
    if(it->root_node_rrn == EMPTY_RRN) {
        indexNode* root_node = createIndexNode(1, 0);
        root_node->leaf = '0';
    }
}
