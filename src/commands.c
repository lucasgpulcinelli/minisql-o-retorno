#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "commands.h"
#include "entries.h"
#include "file_control.h"
#include "inputs.h"
#include "utils.h"
#include "bTree.h"


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
    entry* es = createEntry(1);
    free(table_output_name);

    while(!feof(fp_in)){
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

void commandWhere(void){
    char* data_filename;
    char* indices_filename;
    int n;
    READ_INPUT("%ms %ms %d", &data_filename, &indices_filename, &n);
    field* where = readTuples(n);

    bTree* bt = openBTree(data_filename, indices_filename, "rb");

    for(int i = 0; i < n; i++, rewindBTree(bt)){
        int printed = 0;
        printf("Busca %d\n", i+1);

        if(where[i].field_type == idConnect){
            entry* e = bTreeSearch(bt, where[i].value.integer);
            if(e != NULL){
                printed++;
                printEntry(e);
                deleteEntry(e, 1);
            }
        }
        else{
            for(entry* e; (e = bTreeReadNextEntry(bt)) != NULL; 
                deleteEntry(e, 1)){

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
        }

        if(printed == 0){
            printf("Registro inexistente.\n\n");
        }

        printf("Numero de paginas de disco: %d\n\n", bt->table->header->pages);
    }

    closeBTree(bt);
    freeTuples(where, n);
    free(data_filename);
    free(indices_filename);
}

void commandInsert(void){
    char* table_filename;
    int32_t num_insertions;
    READ_INPUT("%ms %d", &table_filename, &num_insertions);
    
    table* t = openTable(table_filename, "r+b");
    entry* es = createEntry(1);
    free(table_filename);

    while(num_insertions > 0){
        readEntryFromStdin(es);
        appendEntryOnTable(t, es);

        clearEntry(es);
        num_insertions--;
    }

    deleteEntry(es, 1);
    closeTable(t);
}

void commandJoin(void){
    char *table1_filename, *table2_filename;
    char *field1_name, *field2_name;
    char *indices2_filename;

    READ_INPUT("%ms %ms", &table1_filename, &table2_filename);
    READ_INPUT("%ms %ms", &field1_name, &field2_name);
    READ_INPUT("%ms", &indices2_filename);

    if(findFieldType(field1_name) != connPoPsId 
       || findFieldType(field2_name) != idConnect){

        errno = ENOSYS;
        ABORT_PROGRAM("Invalid field names");
    }

    table* t = openTable(table1_filename, "rb");
    bTree* bt = openBTree(table2_filename, indices2_filename, "rb");

    int printed = 0;
    for(entry* e1; (e1 = tableReadNextEntry(t)) != NULL; deleteEntry(e1, 1)){
        entry* e2 = bTreeSearch(bt, e1->fields[connPoPsId].value.integer);
        if(e2 == NULL){
            continue;
        }

        printEntry(e1);
        printEntry(e2);
        
        printed++;
        deleteEntry(e2, 1);
    }

    if(printed == 0){
        printf("Registro inexistente.\n\n");
    }

    closeBTree(bt);
    closeTable(t);
    free(table1_filename);
    free(table2_filename);
    free(field1_name);
    free(field2_name);
    free(indices2_filename);
}