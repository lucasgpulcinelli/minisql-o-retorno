#include <cstdio>
#include <cstring>
#include <cerrno>
#include <system_error>

#include "table.hpp"
#include "Graph.hpp"
#include "NetworkGraph.hpp"

extern "C" {
#include "entries.h"
#include "utils.h"
}

void Table::readHeader(){
    IS_TABLE_OPENED(this, "couldn't read table metadata");

    std::rewind(fp);
    std::fread(&(metadata->status), sizeof(char), 1, fp);
    std::fread(&(metadata->stack), sizeof(int32_t), 1, fp);
    std::fread(&(metadata->nextRRN), sizeof(int32_t), 1, fp);
    std::fread(&(metadata->entries_removed), sizeof(int32_t), 1, fp);
    std::fread(&(metadata->pages), sizeof(int32_t), 1, fp);
    std::fread(&(metadata->times_compacted), sizeof(int32_t), 1, fp);

    std::fseek(fp, PAGE_SIZE, SEEK_SET);
}

void Table::writeHeader(){
    IS_TABLE_OPENED(this, "couldn't update table metadata");
    if(read_only){
        throw std::system_error(EACCES, std::generic_category(),
            "cannot write header in read-only table");
    }

    std::rewind(fp);
    std::fwrite(&(metadata->status), sizeof(char), 1, fp);
    std::fwrite(&(metadata->stack), sizeof(int32_t), 1, fp);
    std::fwrite(&(metadata->nextRRN), sizeof(uint32_t), 1, fp);
    std::fwrite(&(metadata->entries_removed), sizeof(int32_t), 1, fp);
    std::fwrite(&(metadata->pages), sizeof(uint32_t), 1, fp);
    std::fwrite(&(metadata->times_compacted), sizeof(uint32_t), 1, fp);

    for(uint32_t i = 0; i < PAGE_SIZE-HEADER_SIZE; i++){
        std::putc('$', fp);
    }
}

Table::Table(char* table_name, const char* mode) {
    XALLOC(header, metadata, 1);
    OPEN_FILE(fp, table_name, mode);
    readHeader();

    if(metadata->status == ERR_HEADER){
        //if the header is not OK, the whole file is invalid and an error is 
        //generated
        EXIT_ERROR();
    }

    if(std::strchr(mode, 'w') != NULL || std::strchr(mode, '+') != NULL){
        //if we are opening the file for writing, write that the file is 
        //invalid (the status will be restored at closeTable)
        metadata->status = ERR_HEADER;
        writeHeader();
        read_only = false;
    }else{
        read_only = true;
    }
}

void Table::seek(size_t entry_number){
    IS_TABLE_OPENED(this, "couldn't seek entry on table");
    std::fseek(fp, entry_number * MAX_SIZE_ENTRY + PAGE_SIZE, SEEK_SET);
}

void Table::rewind(){
    IS_TABLE_OPENED(this, "couldn't rewind table");
    std::fseek(fp, PAGE_SIZE, SEEK_SET);
}

bool Table::hasNextEntry(){
    IS_TABLE_OPENED(this, "couldn't check if there is a next entry");
    int c;
    if((c = std::getc(fp)) == EOF){
        return false;
    }

    std::ungetc(c, fp);
    return true;
}

entry* Table::readNextEntry(){
    IS_TABLE_OPENED(this, "couldn't read next entry");
    if(!hasNextEntry()){
        return NULL;
    }

    entry* new_entry = createEntry(1);
    readEntry(fp, new_entry);
    return new_entry;
}

int32_t Table::appendEntry(entry* es){
    IS_TABLE_OPENED(this, "couldn't append entry");
    if(read_only){
        throw std::system_error(EACCES, std::generic_category(),
            "cannot append entry in read-only table");
    }

    if(metadata->stack != EMPTY_STACK){
        std::fseek(fp, PAGE_SIZE + metadata->stack*MAX_SIZE_ENTRY, SEEK_SET);
        entry* erased = createEntry(1);
        readEntry(fp, erased);

        std::fseek(fp, -MAX_SIZE_ENTRY, SEEK_CUR);
        int32_t new_entry_rrn = metadata->stack;
        metadata->stack = GET_NEXT_STACK_RRN(es);
        metadata->entries_removed--;
        deleteEntry(erased, 1);

        writeEntry(fp, es);
        return new_entry_rrn;

    }else if(!std::feof(fp)){
        std::fseek(fp, 0, SEEK_END);
    }

    writeEntry(fp, es);
    metadata->nextRRN++;
    return (metadata->nextRRN - 1); //new entry's rrn.
}

void Table::removeEntry(size_t rrn){
    IS_TABLE_OPENED(this, "couldn't remove entry from table");
    if(read_only){
        throw std::system_error(EACCES, std::generic_category(),
            "cannot append entry in read-only table");
    }

    seek(rrn);
    writeEmptyEntry();
    metadata->entries_removed++;
    metadata->stack = rrn;
}

void Table::writeEmptyEntry(){
    int32_t i = 0;

    std::putc(REMOVED, fp);
    i++;

    std::fwrite(&(metadata->stack), sizeof(int32_t), 1, fp);
    i += 4;

    for(; i < MAX_SIZE_ENTRY; i++){
        std::putc('$', fp);
    }
}

uint32_t Table::getTimesCompacted(){
    IS_TABLE_OPENED(this, "couldn't retrieve times compacted");
    return metadata->times_compacted;
}

void Table::setTimesCompacted(uint32_t num_times_compacted){
    IS_TABLE_OPENED(this, "couldn't set times compacted");
    metadata->times_compacted = num_times_compacted;
}

Table::~Table(){
    metadata->pages = NUM_PAGES_FORMULA(metadata->nextRRN);
    metadata->status = OK_HEADER;

    if(!read_only){
        //if the file is not read only, restore the header status as OK.
        //If writing operations were made, update table metadata.
        writeHeader();
    }
    
    std::fclose(fp);
    std::free(metadata);
}