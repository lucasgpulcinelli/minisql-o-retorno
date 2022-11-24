#ifndef __BTREE_H__
#define __BTREE_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "file_control.h"

#define INDICES_PAGE_SIZE 65
#define INDICES_HEADER_SIZE 17

#define BRANCHES 5
#define SEARCH_KEYS (BRANCHES-1)
#define BRANCH_METADATA_SIZE 3
#define MEDIAN SEARCH_KEYS/2

#define EMPTY_RRN -1
#define EMPTY_VALUE -1

#define LEAF '1'
#define NOT_LEAF '0'

#define IS_NOT_LEAF(in) (in->leaf == NOT_LEAF)
#define IS_LEAF(in) (in->leaf == LEAF)
#define IS_FULL(in) (in->keys == SEARCH_KEYS)

enum dataIndices {
    branch_rrn = 0,
    data_value,
    data_rrn
};

typedef struct {
    int32_t idConnect;
    int32_t rrn;
} treeEntry;

typedef struct {
    char leaf;
    int32_t keys;
    int32_t height;
    int32_t node_rrn;

    int32_t data[BRANCHES][BRANCH_METADATA_SIZE];
} indexNode;

typedef struct {
    FILE* fp;
    bool read_only;

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

typedef struct {
    int32_t lower_subtree;
    int32_t upper_subtree;
    treeEntry key;
} treeCarryOn;


bTree* openBTree(char* data_filename, char* indices_filename, 
                 const char* mode);

void closeBTree(bTree* bt);

bTree* createBTreeFromTable(char* table_name, char* indices_filename);

void rewindBTree(bTree* bt);

entry* bTreeReadNextEntry(bTree* bt);

void bTreeTablehashOnScreen(bTree* bt);

void bTreeIndexTreehashOnScreen(bTree* bt);

bool bTreeHasNextEntry(bTree* bt);

indexNode* createIndexNode(int height, int node_rrn, char leaf);

indexTree* openIndexTree(char* filename, const char* mode);

void closeIndexTree(indexTree* it);

indexNode* readCurNode(indexTree* it);

indexNode* readIndexNode(indexTree* it, int32_t rrn);

void writeIndexNode(indexTree* it, indexNode* in);

void freeIndexNode(indexNode* in);

int32_t indexNodeSearch(indexTree* it, int32_t curr_rrn, int32_t value);

entry* bTreeSearch(bTree* bt, int32_t value);

void writeIndexTreeHeader(indexTree* it);

indexTree* createEmptyIndexTree(char* indices_filename);

treeEntry* createTreeEntry(entry* es, int32_t rrn);

void freeTreeEntry(treeEntry* te);

void insertEntryInIndexTree(indexTree* it, treeEntry* te);

treeCarryOn* recursiveNodeInsert(indexTree* it, int32_t node_rrn, treeEntry* te);

treeCarryOn* insertEntryInLeaf(indexTree* it, indexNode* in, treeEntry* te);

treeCarryOn* insertCarryOn(indexTree* it, treeCarryOn* carry_on, int32_t branch, 
                           indexNode* in);

treeCarryOn* splitNode(indexTree* it, indexNode* in);

#endif
