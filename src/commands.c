#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "entries.h"
#include "file_control.h"
#include "inputs.h"
#include "utils.h"


void commandCreate(void){
    char* csv_input_name;
    char* bin_output_name;
    scanf("%ms %ms", &csv_input_name, &bin_output_name);

    FILE* fp_in;
    FILE* fp_out;
    OPEN_FILE(fp_in, csv_input_name, "rb");
    OPEN_FILE(fp_out, bin_output_name, "wb");
    free(csv_input_name);
    free(bin_output_name);

    header *head;
    XALLOC(header, head, 1);
    INIT_FILE_HEADER(head, false, EMPTY_STACK, 0, NO_ENTRIES_REMOVED, 1, NOT_COMPACTED)
    writeHeader(fp_out, head);

    char *first_page_trash;
    MEMSET_ALLOC(char, first_page_trash, PAGE_SIZE - HEADER_SIZE);
    fwrite(first_page_trash, sizeof(char), PAGE_SIZE - HEADER_SIZE, fp_out);
    free(first_page_trash);

    entry *es = createEntry(1);
    while(!feof(fp_in)) {
        char *line;
        readFirstLine(&line, fp_in);
        readEntryFromCSV(line, es);
        free(line);

        writeEntry(fp_out, es);
        head->nextRRN++;
    }

    deleteEntry(es, 1);
    fclose(fp_in);

    head->pages = (head->nextRRN)*sizeof(entry)/PAGE_SIZE + 1;
    writeHeader(fp_out, head);
    free(head);
    fclose(fp_out);
}

void commandFrom(void){
    char* bin_filename;
    scanf("%ms", &bin_filename);
    
    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rb");

    table* t = readTableBinary(fp);
    if(t == NULL){
        printf("Falha no processamento do arquivo.\n");
        exit(0);
    }
    if(t->size == 0){
        printf("Registro inexistente.\n");
        exit(0);
    }

    entry* e;
    while((e = readNextEntry(t)) != NULL){
        printEntry(e);
        printf("\n");
    }

    printf("Numero de páginas de disco: %d\n\n", t->header->pages);

    deleteTable(t);
    fclose(fp);
    free(bin_filename);
}

void commandWhere(void){
    char* bin_filename;
    int n;
    scanf("%ms %d", &bin_filename, &n);
    field* where = readTuples(n);

    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rb");

    table* t = readTableBinary(fp);
    if(t == NULL){
        printf("Falha no processamento do arquivo.\n");
        exit(0);
    }
    if(t->size == 0){
        printf("Registro inexistente.\n");
        exit(0);
    }

    for(int i = 0; i < n; i++, rewindTable(t)){
        printf("Busca %d\n", i);
        
        entry* e;
        while((e = readNextEntry(t)) != NULL){
            field f_cmp = e->fields[where[i].field_type];

            if(fieldCmp(where[i], f_cmp) != 0){
                continue;
            }

            printEntry(e);
            printf("\n");
        }

        printf("Numero de páginas de disco: %d\n\n", t->header->pages);
    }

    deleteTable(t);
    fclose(fp);
    freeTuples(where, n);
    free(bin_filename);
}

void commandDelete(void){
    char* bin_filename;
    int n;
    scanf("%ms %d", &bin_filename, &n);
    //readTuples(n);
    
    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rwb");

    fclose(fp);
    free(bin_filename);
}

void commandInsert(void){
    char* bin_filename;
    int32_t n;
    scanf("%ms %d", &bin_filename, &n);
    
    FILE* fp;
    OPEN_FILE(fp, bin_filename, "r+b");
    header *head = readHeader(fp);

    entry* es;
    entry* erased;
    XALLOC(entry, es, 1);
    XALLOC(entry, erased, 1);
    
    ssize_t stack = head->stack;
    while(stack != EMPTY_STACK && n > 0) {
        fseek(fp, PAGE_SIZE + MAX_SIZE_ENTRY*stack, SEEK_SET);
        readEntry(fp, erased);
        readEntryFromStdin(es);

        stack = erased->fields[linking].value.integer;
        fseek(fp, PAGE_SIZE + MAX_SIZE_ENTRY*stack, SEEK_SET);
        writeEntry(fp, es);
        head->entries_removed--;
        n--;
    }

    head->stack = stack;
    free(erased);
    fseek(fp, 0, SEEK_END);

    while(n > 0) {
        readEntryFromStdin(es);
        writeEntry(fp, es);
        (head->nextRRN)++;
        n--;
    }

    deleteEntry(es, 1);

    head->pages = 1 + head->nextRRN/PAGE_SIZE;
    writeHeader(fp, head);

    free(head);
    fclose(fp);
    free(bin_filename);
}

void commandCompact(void){
    char* bin_filename;
    scanf("%ms", &bin_filename);
    
    FILE* fp;
    OPEN_FILE(fp, bin_filename, "r+b");

    

    fclose(fp);
    free(bin_filename);
}
