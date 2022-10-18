#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "inputs.h"
#include "utils.h"
#include "entries.h"

static const int8_t data_types_map[] = {
    boolean, integer, integer, char_array, integer, 
    char_array, integer, char_p, char_p
};


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

void readEntryFromCSV(char *csv_line, entry *es){
    char *field = (char *)strtok(csv_line, ",");
    size_t num_fields = 0;

    for(num_fields = 0; field && num_fields < FIELD_AMOUNT; 
        num_fields++, field = (char *)strtok(NULL, ",")) {
        switch(data_types_map[num_fields]){
            case boolean:
                es->fields[num_fields].value.cbool = atoi(field);
                break;
            
            case integer:
                es->fields[num_fields].value.integer = atoi(field);
                break;
            
            case char_array:
                strncpy(es->fields[num_fields].value.carray, field, CHAR_ARRAY_LEN);
                break;

            case char_p:
                XALLOC(char, es->fields[num_fields].value.cpointer, strlen(field) + 1);
                strcpy(es->fields[num_fields].value.cpointer, field);
                break;
            
            default:
                break;
        }
    }

    if(num_fields < FIELD_AMOUNT) {
        errno = EINVAL;
        ABORT_PROGRAM("Bad number of fields: found %lu in input, "
                      "but there should be %d",
                      num_fields, FIELD_AMOUNT);

    } else if(field) {
        errno = EINVAL;
        ABORT_PROGRAM("Bad number of fields: found more then"
                      " %lu in input.", FIELD_AMOUNT);
    }
}

void readEntryFromStdin(entry *es){
    static const int8_t order[] = {
        removed, linking, idConnect, poPsName, countryName, 
        countryAcro, connPoPsId, measurmentUnit, speed
    };

    for(size_t field = idConnect; field < FIELD_AMOUNT; field++){
        char* field_str;
        READ_INPUT("%ms", &field_str);

        int8_t data_type = data_types_map[order[field]];
        if(!strcmp(field_str, "NULO")){
           data_type += NULL_OFFSET;
        }

        switch (data_type){
        case integer:
            es->fields[order[field]].value.integer = atoi(field_str);
            break;
        
        case char_array:
            strncat(es->fields[order[field]].value.carray, field_str, 
                    CHAR_ARRAY_SIZE);
            break;
        
        case char_p:
            ssize_t str_size = strlen(field_str);
            XALLOC(char, es->fields[order[field]].value.cpointer, str_size - 1);

            strncpy(es->fields[order[field]].value.cpointer, field_str + 1, str_size - 2);
            es->fields[order[field]].value.cpointer[str_size - 2] = '\0';
            break;
        
        case null_int:
            es->fields[order[field]].value.integer = NULL_INT;
            break;
        
        case null_char_array:
            strncpy(es->fields[order[field]].value.carray, "$$$$", 
                    CHAR_ARRAY_SIZE);
            break;
        case null_char_p:
            es->fields[order[field]].value.cpointer = NULL;
            break;
        }

        free(field_str);
    }
}