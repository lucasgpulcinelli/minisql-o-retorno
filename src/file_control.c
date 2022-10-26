#include <stdio.h>
#include <stdbool.h>

#include "file_control.h"
#include "entries.h"
#include "utils.h"


header* readHeader(FILE* fp){
    header* h;
    XALLOC(header, h, 1);

    rewind(fp);

    fread(&(h->status), sizeof(char), 1, fp);
    fread(&(h->stack), sizeof(int32_t), 1, fp);
    fread(&(h->nextRRN), sizeof(int32_t), 1, fp);
    fread(&(h->entries_removed), sizeof(int32_t), 1, fp);
    fread(&(h->pages), sizeof(int32_t), 1, fp);
    fread(&(h->times_compacted), sizeof(int32_t), 1, fp);

    fseek(fp, PAGE_SIZE, SEEK_SET);
    return h;
}

void writeHeader(FILE *fp, header *head) {
    rewind(fp);
    fwrite(&(head->status), sizeof(char), 1, fp);
    fwrite(&(head->stack), sizeof(int32_t), 1, fp);
    fwrite(&(head->nextRRN), sizeof(uint32_t), 1, fp);
    fwrite(&(head->entries_removed), sizeof(int32_t), 1, fp);
    fwrite(&(head->pages), sizeof(uint32_t), 1, fp);
    fwrite(&(head->times_compacted), sizeof(uint32_t), 1, fp);

    for(uint32_t i = 0; i < PAGE_SIZE-HEADER_SIZE; i++){
        putc('$', fp);
    }
}

table* readTableBinary(FILE* fp){
    table* t;
    XALLOC(table, t, 1);

    t->header = readHeader(fp);
    if(t->header->status != '1'){
        deleteHeader(t->header);
        free(t);
        return NULL;
    }

    t->fp = fp;

    return t;
}

table* createEmptyTable(char* table_name) {
    table* new_table;
    XALLOC(table, new_table, 1);
    XALLOC(header, new_table->header, 1);

    OPEN_FILE(new_table->fp, table_name, "w+b");
    new_table->header->status = ERR_HEADER;
    new_table->header->stack = EMPTY_STACK;
    new_table->header->nextRRN = 0;
    new_table->header->entries_removed = 0;
    new_table->header->pages = 1;
    new_table->header->times_compacted = 0;

    writeHeader(new_table->fp, new_table->header);
    return new_table;
}

void seekTable(table* t, size_t entry_number){
    fseek(t->fp, entry_number * MAX_SIZE_ENTRY + PAGE_SIZE, SEEK_SET);
}

void rewindTable(table* t){
    fseek(t->fp, PAGE_SIZE, SEEK_SET);
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

void writeEntryOnTable(table* t, entry* es) {
    writeEntry(t->fp, es);
    t->header->nextRRN += !feof(t->fp);
}

void closeTable(table *t) {
    t->header->pages = NUM_PAGES_FORMULA(t->header->nextRRN);
    t->header->status = OK_HEADER;

    writeHeader(t->fp, t->header);
    tableHashOnScreen(t);
    deleteTable(t);
}

void tableHashOnScreen(table* t) {
    rewind(t->fp);
    uint32_t sum = 0;
	for(int c = getc(t->fp); c != EOF; c = getc(t->fp)) {
		sum += c;
	}

	printf("%lf\n", sum / (double) 100);
}

void deleteTable(table* t){
    deleteHeader(t->header);
    free(t);
}

void deleteHeader(header* h){
    free(h);
}

uint32_t getTimesCompacted(table* t) {
    return t->header->times_compacted;
}
