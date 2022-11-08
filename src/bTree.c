#include <stdlib.h>

#include "bTree.h"
#include "file_control.h"
#include "utils.h"

bTree* openBTree(char* data_filename, char* indices_filename,
                   const char* mode){
    
    bTree* bt = malloc(sizeof(bTree));
    bt->table = openTable(data_filename, mode);
    bt->tree = openIndexTree(indices_filename, mode);

    return bt;
}

indexTree* openIndexTree(char* filename, const char* mode){
    indexTree* it = malloc(sizeof(indexTree));
    OPEN_FILE(it->fp, filename, mode);

    it->status = fgetc(it->fp);
    if(it->status != OK_HEADER){
        EXIT_ERROR();
    }

    fread(&(it->root_node_rrn), sizeof(int32_t), 1, it->fp);
    fread(&(it->total_keys), sizeof(int32_t), 1, it->fp);
    fread(&(it->height), sizeof(int32_t), 1, it->fp);
    fread(&(it->next_node_rrn), sizeof(int32_t), 1, it->fp);

    fseek(it->fp, it->root_node_rrn*INDICES_PAGE_SIZE, SEEK_SET);

    return it;
}

void closeIndexTree(indexTree* it){
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
    indexNode* in = malloc(sizeof(indexNode));

    in->leaf = getc(it->fp);
    fread(&(in->keys), sizeof(int32_t), 1, it->fp);
    fread(&(in->height), sizeof(int32_t), 1, it->fp);
    fread(&(in->node_rrn), sizeof(int32_t), 1, it->fp);

    for(int i = 0; i < SEARCH_KEYS; i++){
        for(int j = 0; j < 3; j++){
            fread(&(in->data[i][j]), sizeof(int32_t), 1, it->fp);
        }
    }

    fread(&(in->data[BRANCHES-1][branch_rnn]), sizeof(int32_t), 1, it->fp);

    return in;
}

void freeIndexNode(indexNode* in){
    free(in);
}

entry* bTreeSearch(bTree* bt, int32_t value){
    return NULL;
}
