#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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

void writeHeader(FILE* fp, header* head){
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

table* createEmptyTable(char* table_name){
    table* new_table;
    XALLOC(table, new_table, 1);
    XALLOC(header, new_table->header, 1);
    new_table->read_only = false;

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

table* openTable(char* table_name, const char* mode){
    table* t;
    XALLOC(table, t, 1);

    OPEN_FILE(t->fp, table_name, mode);
    t->header = readHeader(t->fp);
    if(t->header->status == ERR_HEADER){
        EXIT_ERROR();
    }

    if(strchr(mode, 'w') != NULL || strchr(mode, '+') != NULL){
        t->header->status = ERR_HEADER;
        writeHeader(t->fp, t->header);
        t->read_only = false;
    }else{
        t->read_only = true;
    }

    return t;
}

void seekTable(table* t, size_t entry_number){
    fseek(t->fp, entry_number * MAX_SIZE_ENTRY + PAGE_SIZE, SEEK_SET);
}

void rewindTable(table* t){
    fseek(t->fp, PAGE_SIZE, SEEK_SET);
}

bool tableHasNextEntry(table* t){
    int c;
    if((c = getc(t->fp)) == EOF){
        return false;
    }
    ungetc(c, t->fp);
    return true;
}


entry* tableReadNextEntry(table* t){
    if(!tableHasNextEntry(t)){
        return NULL;
    }
    entry* new_entry = createEntry(1);
    readEntry(t->fp, new_entry);
    return new_entry;
}

void appendEntryOnTable(table* t, entry* es){
    if(t->header->stack != EMPTY_STACK){
        fseek(t->fp, PAGE_SIZE + t->header->stack*MAX_SIZE_ENTRY, SEEK_SET);
        entry* erased = createEntry(1);
        readEntry(t->fp, erased);

        fseek(t->fp, -MAX_SIZE_ENTRY, SEEK_CUR);
        t->header->stack = erased->fields[linking].value.integer;
        t->header->entries_removed--;
        deleteEntry(erased, 1);

        writeEntry(t->fp, es);
        return;

    }else if(!feof(t->fp)){
        fseek(t->fp, 0, SEEK_END);
    }

    writeEntry(t->fp, es);
    t->header->nextRRN++;
}

void removeEntryFromTable(table* t, size_t rrn){
    seekTable(t, rrn);
    writeEmptyEntry(t->fp, t->header->stack);
    t->header->entries_removed++;
    t->header->stack = rrn;
}

void closeTable(table *t){
    t->header->pages = NUM_PAGES_FORMULA(t->header->nextRRN);
    t->header->status = OK_HEADER;

    if(!t->read_only){
        writeHeader(t->fp, t->header);
        tableHashOnScreen(t);
    }
    fclose(t->fp);
    free(t->header);
    free(t);
}

void tableHashOnScreen(table* t){
    rewind(t->fp);
    uint32_t sum = 0;
	for(int c = getc(t->fp); c != EOF; c = getc(t->fp)){
		sum += c;
	}

	printf("%lf\n", sum / (double) 100);
}

uint32_t getTimesCompacted(table* t){
    return t->header->times_compacted;
}

void setTimesCompacted(table* t, uint32_t num_times_compacted){
    t->header->times_compacted = num_times_compacted;
}