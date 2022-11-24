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
    char* table_name;
    char* indices_name;
    READ_INPUT("%ms %ms", &table_name, &indices_name);

    bTree* tree = createBTreeFromTable(table_name, indices_name);
    bTreeIndexTreehashOnScreen(tree);
    closeBTree(tree);
}

void commandWhere(void){
    char* data_filename;
    char* indices_filename;
    int n;
    READ_INPUT("%ms %ms %d", &data_filename, &indices_filename, &n);
    field* where = readTuples(n);

    bTree* bt = openBTree(data_filename, indices_filename, "rb");

    int nodes_read_prev = 0;
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

            if(printed == 0){
                printf("Registro inexistente.\n\n");
            }

            printf("Numero de paginas de disco: %d\n\n",
                bt->tree->nodes_read - nodes_read_prev + 3);
            nodes_read_prev = bt->tree->nodes_read;

            continue;
        }

        for(entry* e; (e = bTreeReadNextEntry(bt)) != NULL; deleteEntry(e, 1)){

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

        printJointEntry(e1, e2);
        
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
