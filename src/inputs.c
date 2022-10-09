#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "utils.h"
#include "entries.h"


field* readTuples(int n){
    field* fs;
    XALLOC(field, fs, n);

    char* field_name;
    char* field_value;
    for(int i = 0; i < n; i++){
        int ret = scanf("%ms %ms", &field_name, &field_value);
        if(ret != 2){
            if(ret == EOF){
                //errno já foi settado
                ABORT_PROGRAM("error reading input");
            }
            errno = EINVAL;
            ABORT_PROGRAM("formatting error in input");
        }

        int type = findFieldType(field_name);
        if(type == -1){
            errno = EINVAL;
            ABORT_PROGRAM("type of field");
        }
        fs[i].field_type = type;


        if(field_value[0] == '\"'){
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
                errno = EINVAL;
                ABORT_PROGRAM("type invalid for a string value");
            }
        }
        else{
            fs[i].value.integer = atoi(field_value);
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