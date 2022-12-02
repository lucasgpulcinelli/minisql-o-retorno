#include <cstdio>
#include <cstring>
#include <cerrno>
#include <system_error>

#include "file_control.hpp"

extern "C" {
#include "entries.h"
#include "utils.h"
}

void Table::readHeader(){
    IS_TABLE_OPENED(this, "couldn't read table metadata");

    rewind(fp);
    fread(&(metadata->status), sizeof(char), 1, fp);
    fread(&(metadata->stack), sizeof(int32_t), 1, fp);
    fread(&(metadata->nextRRN), sizeof(int32_t), 1, fp);
    fread(&(metadata->entries_removed), sizeof(int32_t), 1, fp);
    fread(&(metadata->pages), sizeof(int32_t), 1, fp);
    fread(&(metadata->times_compacted), sizeof(int32_t), 1, fp);

    fseek(fp, PAGE_SIZE, SEEK_SET);
}

void Table::writeHeader(){
    IS_TABLE_OPENED(this, "couldn't update table metadata");
    if(read_only){
        throw std::system_error(EACCES, std::generic_category(),
                                "cannot write header in read-only table");
    }

    rewind(fp);
    fwrite(&(metadata->status), sizeof(char), 1, fp);
    fwrite(&(metadata->stack), sizeof(int32_t), 1, fp);
    fwrite(&(metadata->nextRRN), sizeof(uint32_t), 1, fp);
    fwrite(&(metadata->entries_removed), sizeof(int32_t), 1, fp);
    fwrite(&(metadata->pages), sizeof(uint32_t), 1, fp);
    fwrite(&(metadata->times_compacted), sizeof(uint32_t), 1, fp);

    for(uint32_t i = 0; i < PAGE_SIZE-HEADER_SIZE; i++){
        putc('$', fp);
    }
}

void Table::openTable(char* table_name, const char* mode) {
    OPEN_FILE(fp, table_name, mode);
    readHeader();

    if(metadata->status == ERR_HEADER){
        //if the header is not OK, the whole file is invalid and an error is 
        //generated
        EXIT_ERROR();
    }

    if(strchr(mode, 'w') != NULL || strchr(mode, '+') != NULL){
        //if we are opening the file for writing, write that the file is 
        //invalid (the status will be restored at closeTable)
        metadata->status = ERR_HEADER;
        writeHeader();
        read_only = false;
    }else{
        read_only = true;
    }
}

void Table::seekTable(size_t entry_number){
    IS_TABLE_OPENED(this, "couldn't seek entry on table");
    fseek(fp, entry_number * MAX_SIZE_ENTRY + PAGE_SIZE, SEEK_SET);
}

void Table::rewindTable(){
    IS_TABLE_OPENED(this, "couldn't rewind table");
    fseek(fp, PAGE_SIZE, SEEK_SET);
}

bool Table::hasNextEntry(){
    IS_TABLE_OPENED(this, "couldn't check if there is a next entry");
    int c;
    if((c = getc(fp)) == EOF){
        return false;
    }

    ungetc(c, fp);
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
        fseek(fp, PAGE_SIZE + metadata->stack*MAX_SIZE_ENTRY, SEEK_SET);
        entry* erased = createEntry(1);
        readEntry(fp, erased);

        fseek(fp, -MAX_SIZE_ENTRY, SEEK_CUR);
        int32_t new_entry_rrn = metadata->stack;
        metadata->stack = erased->fields[linking].value.integer;
        metadata->entries_removed--;
        deleteEntry(erased, 1);

        writeEntry(fp, es);
        return new_entry_rrn;

    }else if(!feof(fp)){
        fseek(fp, 0, SEEK_END);
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

    seekTable(rrn);
    writeEmptyEntry();
    metadata->entries_removed++;
    metadata->stack = rrn;
}

void Table::writeEmptyEntry(){
    int32_t i = 0;

    putc(REMOVED, fp);
    i++;

    fwrite(&(metadata->stack), sizeof(int32_t), 1, fp);
    i += 4;

    for(; i < MAX_SIZE_ENTRY; i++){
        putc('$', fp);
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