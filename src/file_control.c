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

table* readTableBinary(FILE* fp){
    table* t;
    XALLOC(table, t, 1);

    t->header = readHeader(fp);
    if(!t->header->status){
        deleteHeader(t->header);
        free(t);
        return NULL;
    }

    //PAGE_SIZE/MAX_SIZE_ENTRY is the amount of entries a single page has.
    //This is the absolute maximum size the table can have for this file
    t->entries = createEntry(t->header->pages*PAGE_SIZE/MAX_SIZE_ENTRY);

    int size = 0;
    int c;
    /*
     * feof does not work here because we read exactaly the size of the file
     * in the last iteration, we still need to check if the next byte is EOF
     * (feof would only signal the end of file after an EOF was read)
     */
    while((c = getc(fp)) != EOF){
        ungetc(c, fp);

        readEntry(fp, t->entries+size);
 
        if(t->entries[size].fields[removed].value.integer == 1){
            clearEntry(t->entries+size);
            continue;
        }

        size++;
    }

    t->size = size;
    return t;
}

void deleteTable(table* t){
    deleteEntry(t->entries, t->header->pages*PAGE_SIZE/MAX_SIZE_ENTRY);
    deleteHeader(t->header);
    free(t);
}


void deleteHeader(header* h){
    free(h);
}
