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
        appendEntryOnTable(t, es);
        clearEntry(es);
    }

    deleteEntry(es, 1);
    fclose(fp_in);
    closeTable(t);
}

void commandFrom(void){
    char* bin_filename;
    READ_INPUT("%ms", &bin_filename);

    table* t = openTable(bin_filename, "rb");

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

    closeTable(t);
    free(bin_filename);
}

void commandWhere(void){
    char* bin_filename;
    int n;
    READ_INPUT("%ms %d", &bin_filename, &n);
    field* where = readTuples(n);

    table* t = openTable(bin_filename, "rb");

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

    closeTable(t);
    freeTuples(where, n);
    free(bin_filename);
}

void commandDelete(void){
    char* bin_filename;
    int n;
    READ_INPUT("%ms %d", &bin_filename, &n);
    field* where = readTuples(n);

    table* t = openTable(bin_filename, "r+b");

    for(int i = 0; i < n; i++, rewindTable(t)){
        size_t rrn = 0;
        for(entry* e; (e = readNextEntry(t)) != NULL; deleteEntry(e, 1), rrn++){
            if(ENTRY_REMOVED(e)){
                continue;
            }

            field f_cmp = e->fields[where[i].field_type];

            if(fieldCmp(where[i], f_cmp) != 0){
                continue;
            }

            removeEntryFromTable(t, rrn);
        }
    }

    closeTable(t);
    freeTuples(where, n);
    free(bin_filename);
}

void commandInsert(void){
    char* table_filename;
    int32_t num_insertions;
    READ_INPUT("%ms %d", &table_filename, &num_insertions);
    
    table* t = openTable(table_filename, "r+b");
    entry* es = createEntry(1);
    free(table_filename);

    while(num_insertions > 0) {
        readEntryFromStdin(es);
        appendEntryOnTable(t, es);

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

    table* t_in = openTable(bin_filename, "rb");
    table* t_out = createEmptyTable(out_bin);

    for(entry* e; (e = readNextEntry(t_in)) != NULL; deleteEntry(e, 1)){
        if(e->fields[removed].value.carray[0] == NOT_REMOVED){
            appendEntryOnTable(t_out, e);
        }
    }

    setTimesCompacted(t_out, getTimesCompacted(t_in) + 1);

    closeTable(t_in);
    closeTable(t_out);

    free(bin_filename);
    free(out_bin);
}
