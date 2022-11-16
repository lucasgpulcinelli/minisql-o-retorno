#ifndef __BTREE_H__
#define __BTREE_H__

#include <inttypes.h>
#include <stdio.h>

#include "file_control.h"

#define INDICES_PAGE_SIZE 65
#define INDICES_HEADER_SIZE 17

#define BRANCHES 5
#define SEARCH_KEYS (BRANCHES-1)
#define BRANCH_METADATA_SIZE 3

#define EMPTY_RRN -1
#define EMPTY_VALUE -1

#define FULL_NODE -2
#define INSERTION_SUCCESS 0

#define LEAF '1'
#define NOT_LEAF '0'
#define IS_NOT_LEAF(in) (in->leaf == NOT_LEAF)

enum dataIndices {
    branch_rrn = 0,
    data_value,
    data_rrn
};

typedef struct {
    char leaf;
    int32_t keys;
    int32_t height;
    int32_t node_rrn;

    int32_t data[BRANCHES][BRANCH_METADATA_SIZE];
} indexNode;

typedef struct {
    FILE* fp;

    char status;
    int32_t root_node_rrn;
    int32_t total_keys;
    int32_t height;
    int32_t next_node_rrn;
} indexTree;


typedef struct {
    indexTree* tree;
    table* table;
} bTree;


bTree* openBTree(char* data_filename, char* indices_filename, 
                 const char* mode);

void closeBTree(bTree* bt);

void rewindBTree(bTree* bt);

entry* bTreeReadNextEntry(bTree* bt);

bool bTreeHasNextEntry(bTree* bt);

indexNode* createIndexNode(int height, int node_rrn);

indexTree* openIndexTree(char* filename, const char* mode);

void closeIndexTree(indexTree* it);

indexNode* readIndexNode(indexTree* it);

void freeIndexNode(indexNode* in);

int32_t indexNodeSearch(indexTree* it, int32_t curr_rrn, int32_t value);

entry* bTreeSearch(bTree* bt, int32_t value);

void writeIndexTreeHeader(indexTree* it);

indexTree* createIndexTree(char* indices_filename);

void insertEntryInIndexTree(indexTree* it, entry* es, int32_t data_rrn);

#endif
