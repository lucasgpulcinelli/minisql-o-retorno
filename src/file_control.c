#include <stdio.h>
#include <stdbool.h>

#include "file_control.h"
#include "entries.h"
#include "utils.h"


header* readHeader(FILE* fp){
    header* h;
    XALLOC(header, h, 1);

    rewind(fp);

    fread(&(h->status), 1, 1, fp);
    fread(&(h->stack), 4, 1, fp);
    fread(&(h->nextRRN), 4, 1, fp);
    fread(&(h->entries_removed), 4, 1, fp);
    fread(&(h->pages), 4, 1, fp);
    fread(&(h->times_compacted), 4, 1, fp);

    return h;
}

void writeHeader(FILE *fp_out, header *head) {
    int position = ftell(fp_out);

    rewind(fp_out);
    fwrite(&(head->status), sizeof(char), 1, fp_out);
    fwrite(&(head->stack), sizeof(int32_t), 1, fp_out);
    fwrite(&(head->nextRRN), sizeof(uint32_t), 1, fp_out);
    fwrite(&(head->entries_removed), sizeof(int32_t), 1, fp_out);
    fwrite(&(head->pages), sizeof(uint32_t), 1, fp_out);
    fwrite(&(head->times_compacted), sizeof(uint32_t), 1, fp_out);

    fseek(fp_out, position, SEEK_SET);
}

table* readTableBinary(FILE* fp){
    table* t;
    XALLOC(table, t, 1);

    t->header = readHeader(fp);
    if(!t->header->status){
        deleteHeader(t->header);
        free(t);
        return NULL;
    }

    t->fp = fp;

    return t;
}

void seekTable(table* t, size_t entry_number){
    fseek(t->fp, entry_number * MAX_SIZE_ENTRY + HEADER_SIZE, SEEK_SET);
}

void rewindTable(table* t){
    fseek(t->fp, HEADER_SIZE, SEEK_SET);
}

bool hasNextEntry(table* t){
    int c;
    if((c = getc(t->fp)) == EOF){
        return false;
    }
    ungetc(c, t->fp);
    return true;
}


entry* readNextEntry(table* t){
    if(!hasNextEntry(t)){
        return NULL;
    }
    entry* new_entry = createEntry(1);
    readEntry(t->fp, new_entry);
    return new_entry;
}

void deleteTable(table* t){
    deleteHeader(t->header);
    free(t);
}


void deleteHeader(header* h){
    free(h);
}
