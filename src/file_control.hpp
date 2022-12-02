#ifndef __FILE_CONTROL_H__
#define __FILE_CONTROL_H__

#include <cinttypes>
#include <cstdio>
#include <stdexcept>
#include <string>

extern "C"{
#include "entries.h"
}


#define PAGE_SIZE 960
#define ENTRIES_PER_PAGE (PAGE_SIZE/MAX_SIZE_ENTRY)
#define HEADER_SIZE 21

//Macros that define header encoding. 
#define OK_HEADER '1'
#define ERR_HEADER '0'
#define EMPTY_STACK -1

/* The number of disk pages occupied by a table is the ceiling
 * of table_size/PAGE_SIZE. Its gruesome formula (that maximizes
 * efficiency) is given by the expression below:
 */
#define NUM_PAGES_FORMULA(nextRRN)                                   \
    nextRRN/ENTRIES_PER_PAGE +                                       \
    ((nextRRN/ENTRIES_PER_PAGE)*ENTRIES_PER_PAGE != nextRRN) + 1     \

#define IS_TABLE_OPENED(this, error_msg)                                       \
    if(this->fp == NULL){                                                      \
        throw std::runtime_error(error_msg +                                   \
                                 std::string(": no table has been opened"));   \
                                                                               \
    } else if (this->metadata == NULL){                                        \
        throw std::runtime_error(error_msg +                                   \
                                 std::string(": couldn't read file metadata"));\
    }                                                                          \



//struct header contains the header for a table binary file
struct header {
    char status;
    int32_t stack;
    uint32_t nextRRN;
    uint32_t entries_removed;
    uint32_t pages;
    uint32_t times_compacted;
};

class Table {
    private:

    header* metadata;
    FILE* fp;
    bool read_only;

    /*
     * readheader reads the header from the table binary file.
     */
    void readHeader();
    /*
     * writeheader writes the updated header in the table binary file.
     */
    void writeHeader();

    /*
     * writeEmptyEntry writes a single entry as removed with a stack index.
     */
    void writeEmptyEntry();

    public:

    /*
     * openTable opens the table in the file named table_name on a
     * mode specified by the user. It supports "rb", read-only mode,
     * "r+b", read and write mode, and "wb", write-only mode. These
     * modes work in the same way as file descriptors in the C
     * standart library.
     */
    void openTable(char* table_name, const char* mode);
    void close();


    /*
     * seekTable seeks to the index of the entry provided, such that the 
     * readNextEntry() will return the entry with the RRN provided.
     */
    void seekTable(size_t entry_number);
    /*
     * rewindTable returns the table to the begginig, being equivalent to 
     * seekTable(rrn = 0).
     */
    void rewindTable();
    /*
     * hasNextEntry returns true if the table can read another entry,
     * meaning if readNextEntry() will not be NULL in the next call.
     */
    bool hasNextEntry();


    /*
     * tableReadNextEntry reads the next entry from the table, the
     * returned entry must be deleted afterwards.
     */
    entry* readNextEntry();
    /*
     * appendEntry writes entry es on the table. If the stack of
     * deleted entries is empty, it appends the entry at the end of the table.
     * Otherwise, it overwrites the first deleted entry and updates the stack.
     * It returns the RRN of the newly appended entry.
     */
    int32_t appendEntry(entry* es);
    /*
     * removeEntryFromTable deletes the entry of number rrn from the table.
     */
    void removeEntry(size_t rrn);

    /*
     * getTimesCompacted returns the number of times the table has been 
     * compacted.
     */
    uint32_t getTimesCompacted();
    /*
     * setTimesCompacted sets the number of times the table has been compacted
     * to uint32_t num_times_compacted. This is not a dangerous operations, 
     * since this number is unimportant to all other table operations
     */
    void setTimesCompacted(uint32_t num_times_compacted);

    Table();
    ~Table();
};

#endif
