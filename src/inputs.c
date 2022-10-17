#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "entries.h"


field* readTuples(int n){
    field* fs;
    XALLOC(field, fs, n);

    char* field_name;
    char* field_value;
    for(int i = 0; i < n; i++){
        READ_INPUT("%ms %ms", &field_name, &field_value);

        int type = findFieldType(field_name);
        if(type == -1){
            ABORT_PROGRAM("invalid type of field");
        }
        fs[i].field_type = type;

        if(strcmp("NULO", field_value) == 0){
            //NULL value

            switch(type){
            case countryAcro:
            case measurmentUnit:
                memset(fs[i].value.carray, '$', sizeof(char)*4);
                break;
            case countryName:
            case poPsName:
                fs[i].value.cpointer = NULL;
                break;
            case idConnect:
                ABORT_PROGRAM("idConnect cannot be NULL");
            default:
                fs[i].value.integer = -1;
            }

            continue;
        }

        if(field_value[0] != '\"'){
            //integer value
            fs[i].value.integer = atoi(field_value);
            free(field_value);
            free(field_name);
            continue;
        }
        
        //string value

        field_value[strlen(field_value)-1] = '\0';

        switch(fs[i].field_type){
        case countryAcro:
        case measurmentUnit:
            strncpy(fs[i].value.carray, field_value+1, 4);
            break;
        
        case poPsName:
        case countryName:
            XALLOC(char, fs[i].value.cpointer, strlen(field_value+1)+1);
            strcpy(fs[i].value.cpointer, field_value+1);
            break;
        
        default:
            ABORT_PROGRAM("type invalid for a string value");
        }

        free(field_value);
        free(field_name);
    }

    return fs;
}

void freeTuples(field* fs, int n){
    for(int i = 0; i < n; i++){
        if(fs[i].field_type == poPsName || fs[i].field_type == countryName){
            free(fs[i].value.cpointer);
        }
    }
    free(fs);
}