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
//Empty keys and empty rrns are stored as -1 in the indices file.
#define EMPTY_RRN -1
#define EMPTY_VALUE -1
//The boolean field that tells if indexNode is leaf or not is stored as
//char '0' or char '1'.
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
 * the actual data table and the other for indices. 
 */
typedef struct {
    indexTree* tree;
    table* table;
} bTree;

/*
 * key_struct implements the pair (Ci, Pri) that is stored in the indexTree.
 * Ci is the ith key, and Pri is the pointer to its entry in the binary table.
 */
typedef struct {
    int32_t idConnect;
    int32_t rrn;
} key_struct;

/*
 * treeEntry is an ADT that implements the quadruple (Pi, (Ci, Pri), P(i+1)),
 * in which (Ci, Pri) is a key_struct as defined above, and Pi is the ith
 * pointer to a subtree (i.e. the subtree's root rrn). This ADT is used
 * in the implementation of insertEntryInIndexTree, and its purpose is to
 * encapsulate the information that is to be inserted in an indexNode: if
 * it contains the key that the user passed, both subtree fields are left
 * empty (i.e. memsetted to EMPTY_RRN), otherwise if it is the result of
 * a node split, both subtree fields point to the newly created subtrees.
 */
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

/*
 * createBTreeFromTable creates an empty bTree and inserts each entry in
 * the binary table in the BTree. It returns a pointer to that tree, 
 * opened in "w+b" mode.
 */
bTree* createBTreeFromTable(char* table_name, char* indices_filename);

/*
 * rewindBTree is equivalent to rewindTable(bt->table).
 */
void rewindBTree(bTree* bt);

/*
 * bTreeReadNextEntry is equivalent to tableReadNextEntry(bt->table).
 */
entry* bTreeReadNextEntry(bTree* bt);

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

/*
 * readCurNode reads the node in the current position of the indices file, 
 * and returns a pointer to a copy of it in main memory. If the current 
 * position points to the indices' file header or another invalid position,
 * the return value will be trash.
 */
indexNode* readCurNode(indexTree* it);

/*
 * readIndexNode reads the node with relative register numer rrn, and 
 * returns a poiter to a copy of it in main memory. If rrn is set to
 * EMPTY_RRN, readIndexNode returns NULL;
 */
indexNode* readIndexNode(indexTree* it, int32_t rrn);

/*
 * printIndexNode recursively prints the subtree that has root in indexNode in.
 */
void printIndexNode(indexTree* it, indexNode* in);

/*
 * printIndexTree prints all the nodes of the tree in the order that they
 * appear in the indices binary file.
 */
void printIndexTree(indexTree* it);

/*
 * writeIndexNode writes the indexNode instance "in" in the indices binary
 * file of indexTree it.
 */
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

/*
 * writeIndexTreeHeader updates the header of the indices binary file. It
 * should only be called if there were any updates to the tree, and when
 * one is closing the file (in order to minimize dis accesses).
 */
void writeIndexTreeHeader(indexTree* it);

/*
 * createIndexTree creates an empty indexTree, and returns a pointer to
 * it opened in "w+b" mode.
 */
indexTree* createIndexTree(char* indices_filename);

/*
 * createTreeEntry creates and initializes a treeEntry (that can be
 * inserted in an indexTree) with Ci set to the idConnect field of
 * es, and Pri set to rrn.
 */
treeEntry* createTreeEntry(entry* es, int32_t rrn);

/*
 * freeTreeEntry frees the memory allocated to treeEntry te.
 */
void freeTreeEntry(treeEntry* te);

/*
 * insertEntryInBtree inserts entry es in bTree bt.
 */
void insertEntryInBTree(bTree* bt, entry* es);

/*
 * insertEntryInIndexTree inserts entry te in indexTree it.
 */
void insertEntryInIndexTree(indexTree* it, treeEntry* te);

/*
 * setIndexNode copies contents of treeEntry "te" to the ith
 * key (the position "i" here is "index") in indexNode "in". It also
 * copies the subtree pointers in "te" to their respective equivalents
 * in indexNode "in".
 */
void setIndexNode(indexNode* in, treeEntry* te, size_t index);

/*
 * recursiveNodeInsert recursively inserts treeEntry "te" in the
 * subtree with root node "rrn" (rrn here, as the name sugests, is
 * the relative register number of the node in the indices binary file).
 * If there was a split in the subtree's root node, it returns a treeEntry
 * containing the key to be promoted, and two pointers to
 * the newly created subtrees. Otherwise, it returns a null pointer.
 */
treeEntry* recursiveNodeInsert(indexTree* it, int32_t rrn, treeEntry* te);

/*
 * insertEntryInNode inserts treeEntry "te" in node "in" (the insertion
 * preserves key order, of course). If the node is full, it calls the
 * function splitAndInsert and returns its return value. Otherwise, it
 * simply does an ordered insertion of "te" and return null.
 */
treeEntry* insertEntryInNode(indexTree* it, indexNode* in, treeEntry* te);

/*
 * translateToTheRight copies a triple (Pi, Ci, Pri) (as defined in the
 * documentation of treeEntry) from position index to position index + 1
 * in the data field of an indexNode (here passed as an integer matrix).
 * It overwrites the data at position (index + 1), and its sole purpose
 * is to implement the ordered insertion used in insertEntryInNode.
 */
void translateToTheRight(int32_t data[BRANCHES][BRANCH_METADATA_SIZE], 
                         ssize_t index);

/*
 * splitAndInsert splits the node "in", inserts treeEntry "te" and returns 
 * a treeEntry to be promoted, as described in the documentation of
 * recursiveNodeInsert.
 */
treeEntry* splitAndInsert(indexTree* it, indexNode* in, treeEntry* te);

/*
 * orderEntries receives a full node and a treeEntry as input and returns
 * an array of (SEARCH_KEYS + 1) entries. This array simply contais the entries
 * stored in "te" and "in", but ordered. The sole purpose of this procedure
 * is to sort these entries, which in turn is necessary to find their median
 * and to split the subtree "in" into two valid subtrees. orderEntries also
 * takes a pointer to an integer, and sets it to the position of treeEntry
 * "te" in the array.
 */
treeEntry* sortEntries(indexNode* in, treeEntry* te, int32_t* te_index);

/*
 * setTreeEntry copies the contents of treeEntry "set" to treeEntry "te",
 * overriding the contents of treeEntry "te". It is an auxliary function in
 * the implementation of splitAndInsert and sortEntries.
 */
void setTreeEntry(treeEntry* te, treeEntry* set);

#endif
