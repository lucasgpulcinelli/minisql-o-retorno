#include <stdio.h>

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
    rewind(fp_out);
    fwrite(&(head->status), sizeof(char), 1, fp_out);
    fwrite(&(head->stack), sizeof(int32_t), 1, fp_out);
    fwrite(&(head->nextRRN), sizeof(uint32_t), 1, fp_out);
    fwrite(&(head->entries_removed), sizeof(int32_t), 1, fp_out);
    fwrite(&(head->pages), sizeof(uint32_t), 1, fp_out);
    fwrite(&(head->times_compacted), sizeof(uint32_t), 1, fp_out);
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

    t->entries = createEntry(ENTRIES_PER_PAGE);
    t->index = 0;
    t->size = 0;
    t->fp = fp;

    readNextPage(t);

    return t;
}

void rewindTable(table* t){
    fseek(t->fp, HEADER_SIZE, SEEK_SET);
}

int readNextPage(table* t){
    /*
     * feof does not work here because we read exactaly the size of the file
     * in the last iteration, we still need to check if the next byte is EOF
     * (feof would only signal the end of file after an EOF was read)
     */
    t->size = 0;
    while(t->size < ENTRIES_PER_PAGE){
        clearEntry(t->entries+t->size);
        int ret = readEntry(t->fp, t->entries+t->size);
        if(ret < 0){
            return t->size;
        }
 
        if(t->entries[t->size].fields[removed].value.integer == 1){
            continue;
        }
        t->size++;
    }
    return t->size;
}

entry* readNextEntry(table* t){
    if(t->index >= t->size){
        if(readNextPage(t) <= 0){
            return NULL;
        }
        t->index = 0;
    }
    return t->entries + t->index++;
}

void deleteTable(table* t){
    deleteEntry(t->entries, ENTRIES_PER_PAGE);
    deleteHeader(t->header);
    free(t);
}


void deleteHeader(header* h){
    free(h);
}
