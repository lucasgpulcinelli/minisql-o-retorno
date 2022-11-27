#ifndef __BTREE_H__
#define __BTREE_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "file_control.h"

#define INDICES_PAGE_SIZE 65
#define INDICES_HEADER_SIZE 17

//number of branches each node can have
#define BRANCHES 5
//number of rrn and primary key pairs each node can have
#define SEARCH_KEYS (BRANCHES-1)
//number of int32_t's each entry has (the meaning is defined in the 
//dataIndices enum)
#define BRANCH_METADATA_SIZE 3
#define MEDIAN SEARCH_KEYS/2

#define EMPTY_RRN -1
#define EMPTY_VALUE -1
#define EMPTY_TREE_ROOT_RRN -1

#define LEAF '1'
#define NOT_LEAF '0'

#define IS_NOT_LEAF(in) (in->leaf == NOT_LEAF)
#define IS_LEAF(in) (in->leaf == LEAF)
#define IS_FULL(in) (in->keys == SEARCH_KEYS)

//enum dataIndices defines the meaning of each branch entry in a node.
enum dataIndices {
    branch_rrn = 0, //rrn inside the index table of the child node
    data_value, //value stored for the entry
    data_rrn //rrn of the entry in the data table
};

/*
 * indexNode defines a single node in an indexTree, with some metadata fields 
 * about the node and the primary data array.
 * 
 * leaf is a character that can be '0' or '1', defining if the node is a leaf
 * keys is the number of keys the node currently is storing
 * height is the height of the node
 * node_rrn is the rrn of the node in the indexTable file
 * data is the actual values and rrns of entries and child node rrns
 */
typedef struct {
    char leaf;
    int32_t keys;
    int32_t height;
    int32_t node_rrn;

    int32_t data[BRANCHES][BRANCH_METADATA_SIZE];
} indexNode;

/*
 * struct indexTree defines a full index tree in a file, note that it only 
 * contains data for the index file, not having information on any entries
 * data except of it's rrn and primary key.
 */
typedef struct {
    FILE* fp;
    bool read_only;
    int nodes_read;

    char status;
    int32_t root_node_rrn;
    int32_t total_keys;
    int32_t height;
    int32_t next_node_rrn;
} indexTree;

/*
 * struct bTree implements a full B tree with information on two files: one for
 * the actual data table and other for indices. 
 */
typedef struct {
    indexTree* tree;
    table* table;
} bTree;

typedef struct {
    int32_t idConnect;
    int32_t rrn;
} key_struct;

typedef struct {
    int32_t lower_subtree;
    int32_t upper_subtree;
    key_struct key;
} treeEntry;


/*
 * openBTree opens both files related to a BTree and creates the struct to
 * represent them. If the files are opened for writing (with 'w' or '+' in the
 * mode) an error header status is written until closeBTree is called.
 */
bTree* openBTree(char* data_filename, char* indices_filename, 
                 const char* mode);

/*
 * closeBTree closes a bTree and frees all resources associated. If the files
 * were opened for writing, ok header status are written in both.
 */
void closeBTree(bTree* bt);

bTree* createBTreeFromTable(char* table_name, char* indices_filename);

/*
 * rewindBTree is equivalent to rewindTable(bt->table).
 */
void rewindBTree(bTree* bt);

/*
 * bTreeReadNextEntry is equivalent to tableReadNextEntry(bt->table).
 */
entry* bTreeReadNextEntry(bTree* bt);

void bTreeTablehashOnScreen(bTree* bt);

void bTreeIndexTreehashOnScreen(bTree* bt);

/*
 * bTreeHasNextEntry is equivalent to tableHasNextEntry(bt->table).
 */
bool bTreeHasNextEntry(bTree* bt);

/*
 * openIndexTree opens an index tree in the specified mode, writing an error 
 * headers status if the file is opened with writing permissions.
 */
indexTree* openIndexTree(char* filename, const char* mode);

/*
 * closeIndexTree closes a bTree and frees all resources associated. If the file
 * was opened for writing, an ok header status is written.
 */
void closeIndexTree(indexTree* it);

indexNode* readCurNode(indexTree* it);

indexNode* readIndexNode(indexTree* it, int32_t rrn);

void printIndexNode(indexTree* it, indexNode* in);

void printIndexTree(indexTree* it);

void writeIndexNode(indexTree* it, indexNode* in);

/*
 * freeIndexNode frees all resources used in an indexNode.
 */
void freeIndexNode(indexNode* in);

/*
 * indexNodeSearch recurses to find the rnn in the data file that contains a
 * certain value, using the index tree and a current node rrn.
 */
int32_t indexNodeSearch(indexTree* it, int32_t curr_rrn, int32_t value);

/*
 * bTreeSearch searches for the entry with the primary key value given using
 * the indexTable for the fewest disk reads possible.
 */
entry* bTreeSearch(bTree* bt, int32_t value);

void writeIndexTreeHeader(indexTree* it);

indexTree* createIndexTree(char* indices_filename);

treeEntry* createTreeEntry(entry* es, int32_t rrn);

void freeTreeEntry(treeEntry* te);

void insertEntryInBTree(bTree* bt, entry* es);

void insertEntryInIndexTree(indexTree* it, treeEntry* te);

void setIndexNode(indexNode* in, treeEntry* te, size_t index);

treeEntry* recursiveNodeInsert(indexTree* it, int32_t rrn, treeEntry* te);

treeEntry* insertEntryInNode(indexTree* it, indexNode* in, treeEntry* te);

void translateToTheRight(int32_t data[BRANCHES][BRANCH_METADATA_SIZE], 
                         ssize_t index);

treeEntry* splitAndInsert(indexTree* it, indexNode* in, treeEntry* te);

void setTreeEntry(treeEntry* te, treeEntry* set);

#endif
