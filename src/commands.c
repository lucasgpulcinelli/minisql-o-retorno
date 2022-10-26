#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "commands.h"
#include "entries.h"
#include "file_control.h"
#include "inputs.h"
#include "utils.h"


void commandCreate(void){
    char* csv_input_name;
    char* table_output_name;
    READ_INPUT("%ms %ms", &csv_input_name, &table_output_name);

    FILE* fp_in;
    OPEN_FILE(fp_in, csv_input_name, "rb");
    free(csv_input_name);

    char* csv_header;
    readFirstLine(&csv_header, fp_in);
    if(strcmp(csv_header, CSV_HEADER)){
        errno = EINVAL;
        ABORT_PROGRAM("Invalid CSV header");
    }
    free(csv_header);

    table* t = createEmptyTable(table_output_name);
    entry *es = createEntry(1);
    free(table_output_name);

    while(!feof(fp_in)) {
        char* line;
        readFirstLine(&line, fp_in);
        if(!strcmp(line, "")){
            free(line);
            break;
        }

        readEntryFromCSV(line, es);
        free(line);
        writeEntryOnTable(t, es);
        clearEntry(es);
    }

    deleteEntry(es, 1);
    fclose(fp_in);
    closeTable(t);
}

void commandFrom(void){
    char* bin_filename;
    READ_INPUT("%ms", &bin_filename);

    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rb");

    table* t = readTableBinary(fp);
    if(t == NULL){
        printf("Falha no processamento do arquivo.\n");
        exit(EXIT_SUCCESS);
    }

    int printed = 0;
    for(entry* e; (e = readNextEntry(t)) != NULL; deleteEntry(e, 1)){
        if(ENTRY_REMOVED(e)){
            continue;
        }

        printed++;
        printEntry(e);
    }

    if(printed == 0){
        printf("Registro inexistente.\n\n");
    }

    printf("Numero de paginas de disco: %d\n\n", t->header->pages);

    deleteTable(t);
    fclose(fp);
    free(bin_filename);
}

void commandWhere(void){
    char* bin_filename;
    int n;
    READ_INPUT("%ms %d", &bin_filename, &n);
    field* where = readTuples(n);

    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rb");

    table* t = readTableBinary(fp);
    if(t == NULL){
        printf("Falha no processamento do arquivo.\n");
        exit(EXIT_SUCCESS);
    }

    for(int i = 0; i < n; i++, rewindTable(t)){
        int printed = 0;
        printf("Busca %d\n", i+1);

        for(entry* e; (e = readNextEntry(t)) != NULL; deleteEntry(e, 1)){
            if(ENTRY_REMOVED(e)){
                continue;
            }

            field f_cmp = e->fields[where[i].field_type];

            if(fieldCmp(where[i], f_cmp) != 0){
                continue;
            }

            printed++;
            printEntry(e);
        }

        if(printed == 0){
            printf("Registro inexistente.\n\n");
        }

        printf("Numero de paginas de disco: %d\n\n", t->header->pages);
    }

    deleteTable(t);
    fclose(fp);
    freeTuples(where, n);
    free(bin_filename);
}

void commandDelete(void){
    char* bin_filename;
    int n;
    READ_INPUT("%ms %d", &bin_filename, &n);
    field* where = readTuples(n);

    FILE* fp;
    // needs a + for overwriting headers and deleting entries
    OPEN_FILE(fp, bin_filename, "rb+");

    table* t = readTableBinary(fp);
    if(t == NULL){
        printf("Falha no processamento do arquivo.\n");
        exit(EXIT_SUCCESS);
    }

    t->header->status = ERR_HEADER;
    writeHeader(t->fp, t->header);

    for(int i = 0; i < n; i++, rewindTable(t)){
        int rrn = 0;
        for(entry* e; (e = readNextEntry(t)) != NULL; deleteEntry(e, 1), rrn++){
            if(ENTRY_REMOVED(e)){
                continue;
            }

            field f_cmp = e->fields[where[i].field_type];

            if(fieldCmp(where[i], f_cmp) != 0){
                continue;
            }

            seekTable(t, rrn);
            writeEmptyEntry(t->fp, t->header->stack);
            t->header->entries_removed++;
            t->header->stack = rrn;
        }
    }


    t->header->status = OK_HEADER;
    writeHeader(t->fp, t->header);

    deleteTable(t);
    fclose(fp);
    freeTuples(where, n);

    binaryOnScreen(bin_filename);
    free(bin_filename);
}

void commandInsert(void){
    char* table_filename;
    int32_t num_insertions;
    READ_INPUT("%ms %d", &table_filename, &num_insertions);
    
    table* t = openTable(table_filename);
    entry* es = createEntry(1);
    free(table_filename);

    while(num_insertions > 0) {
        readEntryFromStdin(es);
        writeEntryOnTable(t, es);

        clearEntry(es);
        num_insertions--;
    }

    deleteEntry(es, 1);
    closeTable(t);
}

void commandCompact(void){
    char* bin_filename;
    READ_INPUT("%ms", &bin_filename);

    char* out_bin;
    XCALLOC(char, out_bin, strlen(bin_filename) + 2);
    out_bin[0] = '_';
    strcat(out_bin, bin_filename);

    FILE* fp_in;
    FILE* fp_out;
    OPEN_FILE(fp_in, bin_filename, "rb");
    OPEN_FILE(fp_out, out_bin, "wb");

    table* t = readTableBinary(fp_in);
    if(t == NULL){
        printf("Falha no processamento do arquivo.\n");
        exit(EXIT_SUCCESS);
    }
    if(!hasNextEntry(t)){
        exit(EXIT_SUCCESS);
    }

    header *head;
    XALLOC(header, head, 1);
    INIT_FILE_HEADER(head, false, EMPTY_STACK, 0, NO_ENTRIES_REMOVED, 1, NOT_COMPACTED);
    writeHeader(fp_out, head);

    for(entry* e; (e = readNextEntry(t)) != NULL; deleteEntry(e, 1)){
        if(e->fields[removed].value.carray[0] == NOT_REMOVED){
            writeEntry(fp_out, e);
            head->nextRRN++;
        }
    }

    head->pages = head->nextRRN/ENTRIES_PER_PAGE + ((head->nextRRN/ENTRIES_PER_PAGE)*ENTRIES_PER_PAGE != head->nextRRN) + 1;
    head->status = OK_HEADER;
    head->times_compacted  = getTimesCompacted(t) + 1;
    writeHeader(fp_out, head);
    deleteHeader(head);

    fclose(fp_in);
    fclose(fp_out);
    deleteTable(t);

    rename(out_bin, bin_filename);
    binaryOnScreen(bin_filename);

    free(bin_filename);
    free(out_bin);
}
