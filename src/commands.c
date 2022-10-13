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
    XALLOC(header, head, ONE_ELEMENT);
    INIT_FILE_HEADER(head, false, EMPTY_STACK, 0, NO_ENTRIES_REMOVED, 1, NOT_COMPACTED)
    writeHeader(fp_out, head);

    char *first_page_trash;
    MEMSET_ALLOC(char, first_page_trash, PAGE_SIZE - HEADER_SIZE);
    fwrite(first_page_trash, sizeof(char), PAGE_SIZE - HEADER_SIZE, fp_out);
    free(first_page_trash);

    entry *es = createEntry(ONE_ELEMENT);
    while(!feof(fp_in)) {
        char *line;
        readFirstLine(&line, fp_in);
        readEntryFromCSV(line, es);
        free(line);

        writeEntry(fp_out, es);
        head->nextRRN++;
    }

    free(es);
    fclose(fp_in);

    head->pages = (head->nextRRN)*sizeof(entry)/PAGE_SIZE + 1;
    writeHeader(fp_out, head);
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
    int c;
    if((c = getc(fp)) == EOF){
        printf("Registro inexistente.\n");
        exit(0);
    }
    ungetc(c, fp);

    entry* e;
    while((e = readNextEntry(t)) != NULL){
        if(e->fields[removed].value.integer == 1){
            deleteEntry(e, 1);
            continue;
        }
        printEntry(e);
        printf("\n");

        deleteEntry(e, 1);
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
    int c;
    if((c = getc(fp)) == EOF){
        printf("Registro inexistente.\n");
        exit(0);
    }
    ungetc(c, fp);


    for(int i = 0; i < n; i++, rewindTable(t)){
        printf("Busca %d\n", i);

        entry* e;
        while((e = readNextEntry(t)) != NULL){
            if(e->fields[removed].value.integer == 1){
                deleteEntry(e, 1);
                continue;
            }

            field f_cmp = e->fields[where[i].field_type];

            if(fieldCmp(where[i], f_cmp) != 0){
                deleteEntry(e, 1);
                continue;
            }

            printEntry(e);
            printf("\n");

            deleteEntry(e, 1);
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
    field* where = readTuples(n);

    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rb+");

    table* t = readTableBinary(fp);
    if(t == NULL){
        printf("Falha no processamento do arquivo.\n");
        exit(0);
    }
    int c;
    if((c = getc(fp)) == EOF){
        printf("Registro inexistente.\n");
        exit(0);
    }
    ungetc(c, fp);


    t->header->status = false;
    writeHeader(t->fp, t->header);

    for(int i = 0; i < n; i++, rewindTable(t)){
        entry* e;
        int rnn = 0;
        while((e = readNextEntry(t)) != NULL){
            if(e->fields[removed].value.integer == 1){
                deleteEntry(e, 1);
                rnn++;
                continue;
            }

            field f_cmp = e->fields[where[i].field_type];

            if(fieldCmp(where[i], f_cmp) != 0){
                deleteEntry(e, 1);
                rnn++;
                continue;
            }

            entry* new_e = createEntry(1);

            new_e->fields[removed].value.integer = 1;
            new_e->fields[linking].value.integer = t->header->stack;
            t->header->stack = rnn;
            seekTable(t, rnn);
            writeEntry(t->fp, new_e);

            deleteEntry(new_e, 1);
            deleteEntry(e, 1);
            rnn++;
        }

        t->header->status = true;
        writeHeader(t->fp, t->header);

        printf("Numero de páginas de disco: %d\n\n", t->header->pages);
    }

    deleteTable(t);
    fclose(fp);
    freeTuples(where, n);
    free(bin_filename);
}

void commandInsert(void){
    char* bin_filename;
    uint32_t n;
    scanf("%ms %d", &bin_filename, &n);
    //readTable(n);

    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rwb");

    fclose(fp);
    free(bin_filename);
}

void commandCompact(void){
    char* bin_filename;
    scanf("%ms", &bin_filename);

    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rb+");
    rewind(fp);


    table* t = readTableBinary(fp);
    if(t == NULL){
        printf("Falha no processamento do arquivo.\n");
        exit(0);
    }

    //escreve inconsistente no header

    int read_end = 0;
    int write_end = 0;

    entry* e_read;

    while((e_read = readNextEntry(t)) != NULL){
        printf("%d\n", e_read->fields[removed].value.integer);
        if(e_read->fields[removed].value.integer == 1){
            //registro removido
            read_end++; //somente pula esse registro e não escreve nada
            deleteEntry(e_read, 1);
            continue;
        }
        //registro não removido

        if(read_end == write_end){
            //não precisa escrever no disco, as informações seriam iguais
            read_end++;
            write_end++;
            deleteEntry(e_read, 1);
            continue;
        }

        fseek(fp, write_end*MAX_SIZE_ENTRY, SEEK_SET);
        seekTable(t, read_end);

        read_end++;
        write_end++;
        deleteEntry(e_read, 1);
    }

    deleteTable(t);
    fclose(fp);
    free(bin_filename);
}
