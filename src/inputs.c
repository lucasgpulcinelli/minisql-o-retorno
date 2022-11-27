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

static const int8_t order[] = {
    removed, linking, idConnect, poPsName, countryName, 
    countryAcro, connPoPsId, measurmentUnit, speed
};


field* readTuples(int32_t n){
    field* fs;
    XALLOC(field, fs, n);

    char* field_name;
    char* field_value;
    for(int32_t i = 0; i < n; i++){
        //for each input, read the line
        READ_INPUT("%ms %m[^\n]", &field_name, &field_value);

        //find the reffered field
        int32_t type = findFieldType(field_name);
        if(type == -1){
            ABORT_PROGRAM("invalid type of field");
        }
        fs[i].field_type = type;

        //if the value is null, set the value as null for the field type
        if(strcmp(NULL_STR, field_value) == 0){
            //NULL value

            switch(type){
            case countryAcro:
            case measurmentUnit:
                memset(fs[i].value.carray, '$', sizeof(char)*CHAR_ARRAY_LEN);
                break;
            case countryName:
            case poPsName:
                fs[i].value.cpointer = NULL;
                break;
            case idConnect:
                ABORT_PROGRAM("idConnect cannot be NULL");
                break;
            default:
                fs[i].value.integer = -1;
            }

            continue;
        }

        //if the value is an integer, just read it
        if(field_value[0] != '\"'){
            //integer value
            fs[i].value.integer = atoi(field_value);
            free(field_value);
            free(field_name);
            continue;
        }
        
        //if the value is a string, read it and copy to the right field

        field_value[strlen(field_value)-1] = '\0';

        switch(fs[i].field_type){
        case countryAcro:
        case measurmentUnit:
            strncpy(fs[i].value.carray, field_value+1, CHAR_ARRAY_LEN);
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

void freeTuples(field* fs, int32_t n){
    for(int32_t i = 0; i < n; i++){
        if(fs[i].field_type == poPsName || fs[i].field_type == countryName){
            free(fs[i].value.cpointer);
        }
    }
    free(fs);
}

void readEntryFromCSV(char* csv_line, entry* es){
    char* field = (char*)strsep(&csv_line, ",");
    size_t num_fields = 0;

    for(num_fields = idConnect; field && num_fields < FIELD_AMOUNT; 
        num_fields++, field = (char*)strsep(&csv_line, ",")){
        ssize_t data_type = data_types_map[order[num_fields]];
        if(IS_NULL(field)){
            data_type += NULL_DATA_TYPES_OFFSET;
        }
        
        storeField(es, order[num_fields], data_type, field);
    }

    if(num_fields < CSV_FIELD_AMOUNT){
        errno = EINVAL;
        ABORT_PROGRAM("Bad number of fields: found %lu in input, "
                      "but there should be %d",
                      num_fields, CSV_FIELD_AMOUNT);

    }else if(field){
        errno = EINVAL;
        ABORT_PROGRAM("Bad number of fields: found more then"
                      " %lu in input.", FIELD_AMOUNT);
    }
}

void readEntryFromStdin(entry* es){
    for(size_t field = idConnect; field < FIELD_AMOUNT; field++){
        char* field_str;
        readFieldFromStdin(&field_str);

        int8_t data_type = data_types_map[order[field]];
        if(!strcmp(field_str, NULL_STR)){
           data_type += NULL_DATA_TYPES_OFFSET;
        }

        storeField(es, order[field], data_type, field_str);
        free(field_str);
    }
}

void readFieldFromStdin(char** field_str){
    char first_char;
    while((first_char = fgetc(stdin)) == ' ');
    ungetc(first_char, stdin);

    if(first_char != '"'){
        READ_INPUT("%ms", field_str);
        return;
    }
    
    first_char = fgetc(stdin);
    size_t field_len = INIT_LEN;
    size_t char_index = 0;
    XALLOC(char, *field_str, field_len);

    do{
        if(field_len == char_index){
            field_len *= STR_GROWTH_FACTOR;
            XREALLOC(char, *field_str, field_len);
        }

        (*field_str)[char_index] = fgetc(stdin);
        char_index++;
    }while(!feof(stdin) && (*field_str)[char_index - 1] != '"');

    (*field_str)[char_index - 1] = '\0';
}

void storeField(entry* es, size_t field, int8_t type, char* field_str){
    char* copy = strdup(field_str);
    strStrip(&copy);
    switch(type){
        case boolean:
            es->fields[field].value.carray[0] = atoi(copy);
            break;
            
        case integer:
            es->fields[field].value.integer = atoi(copy);
            break;
            
        case char_array:
            strncpy(es->fields[field].value.carray, copy, CHAR_ARRAY_LEN);
            break;

        case char_p:
            XALLOC(char, es->fields[field].value.cpointer, strlen(copy) + 1);
            strcpy(es->fields[field].value.cpointer, copy);
            break;
            
        case null_int:
            es->fields[field].value.integer = NULL_INT;
            break;
        
        case null_char_array: 
            memset(es->fields[field].value.carray, '$', CHAR_ARRAY_LEN);
            break;

        case null_char_p:
            es->fields[field].value.cpointer = NULL;
            break;
            
        default:
            break;
    }

    free(copy);
}