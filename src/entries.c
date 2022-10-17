#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>


#include "entries.h"
#include "utils.h"


static const char fields_str_arr[][20] = {   
    "removido", "encadeamento", "idConecta", "siglaPais", "idPoPsConectado", 
    "unidadeMedida", "velocidade", "nomePoPs", "nomePais"
};

static const int8_t fields_size_arr[] = {
    1, 4, 4, 2, 4, 1, 4, -1, -1
};

static const int8_t data_types_map[] = {
    boolean, integer, integer, char_array, integer, 
    char_array, integer, char_p, char_p
};


entry* createEntry(uint32_t size){
    entry* es;
    XALLOC(entry, es, size);
    memset(es, '$', size); //places trash at pointers too!

    for(uint32_t i = 0; i < size; i++){

        for(uint32_t j = 0; j < FIELD_AMOUNT; j++){
            es[i].fields[j].field_type = j;
        }

        es[i].fields[removed].value.integer = 0;
        es[i].fields[linking].value.integer = NOT_ERASED;
        es[i].fields[poPsName].value.cpointer = NULL;
        es[i].fields[countryName].value.cpointer = NULL;

    }

    return es;
}

void deleteEntry(entry* es, uint32_t size){
    for(uint32_t i = 0; i < size; i++){
        clearEntry(es+i);
    }

    free(es);
}

void clearEntry(entry* e){
    if(e->fields[poPsName].value.cpointer){
            free(e->fields[poPsName].value.cpointer);
            e->fields[poPsName].value.cpointer = NULL;
        }
    if(e->fields[countryName].value.cpointer){
            free(e->fields[countryName].value.cpointer);
            e->fields[countryName].value.cpointer = NULL;
    }
}

int readField(FILE* fp, field* f, int read_for_entry){
    if(read_for_entry >= MAX_SIZE_ENTRY){
        return read_for_entry;
    }

    if(fields_size_arr[f->field_type] > 0){
        //fixed sized fields

        int ret = fread(&(f->value), fields_size_arr[f->field_type], 1, fp);
        if(ret != 1){
            //in case of a read error, return EOF if that is the case and abort
            //the program if any other error ocurred
            if(feof(fp)){
                return EOF;
            }

            ABORT_PROGRAM("read field %s at position %d failed", 
                fields_str_arr[f->field_type], 
                ftell(fp)
            );
        }
        return fields_size_arr[f->field_type] + read_for_entry;
    }

    //variable sized fields
    char* str;
    XALLOC(char, str, MAX_SIZE_ENTRY);

    int i, c;
    for(i = 0; 
        (c = getc(fp)) != '|' 
        && read_for_entry+1 < MAX_SIZE_ENTRY;
        i++, read_for_entry++
    ){
        str[i] = c;
        if(str[i] < 0 || i >= MAX_SIZE_ENTRY){
            ABORT_PROGRAM("read field %s at position %d failed", 
                fields_str_arr[f->field_type], 
                ftell(fp)
            );
        }
    }
    str[i] = '\0';
    f->value.cpointer = str;

    return read_for_entry+1;
}

int readEntry(FILE* fp, entry* e){
    int read_for_entry = 0;

    for(uint32_t i = 0; i < FIELD_AMOUNT; i++){
        read_for_entry = readField(fp, e->fields+i, read_for_entry);
        if(read_for_entry < 0){
            return read_for_entry;
        }
    }

    fseek(fp, MAX_SIZE_ENTRY-read_for_entry, SEEK_CUR);
    return 1;
}

void readEntryFromCSV(char *csv_line, entry *es) {
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
                es->fields[num_fields].value.cpointer = malloc((strlen(field) + 1)*sizeof(char));
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

void readEntryFromStdin(entry *es) {
    char* input_line;
    READ_INPUT("%ms\n", input_line);
}

int writeField(FILE* fp, field* f, ssize_t size){
    if(fields_size_arr[f->field_type] > 0){
        //fixed sized fields
        fwrite(&(f->value), fields_size_arr[f->field_type], 1, fp);
        return fields_size_arr[f->field_type];
    }

    //variable sized fields
    if(f->value.cpointer == NULL || (!strcmp(f->value.cpointer, NULL_STR))){
        putc('|', fp);
        return 1;
    }

    ssize_t write_len = min(strlen(f->value.cpointer), size) - 1;
    fwrite(f->value.cpointer, write_len, 1, fp);
    putc('|', fp);

    return write_len + 1;
}

void writeEntry(FILE* fp, entry* e){
    ssize_t bytes = MAX_SIZE_ENTRY;      //Available bytes in entry
    for(uint32_t i = 0; i < FIELD_AMOUNT && bytes > 0; i++){
        bytes -= writeField(fp, e->fields+i, bytes);
    }

    for(int32_t i = 0; i < MAX_SIZE_ENTRY - bytes; i++) {
        putc('$', fp);
    }
}

void printField(field* f){
    switch(f->field_type){
    case removed:
    case linking:
    case idConnect:
    case connPoPsId:
        printf("%s: %d\n", fields_str_arr[f->field_type], f->value.integer);
        break;

    case speed:
        printf("%s: %d ", fields_str_arr[f->field_type], f->value.integer);
        break;
    case measurmentUnit:
        printf("%cbps\n", f->value.carray[0]);
        break;

    case countryAcro:
        printf("%s: %c%c\n", 
            fields_str_arr[f->field_type], 
            f->value.carray[0], f->value.carray[1]
        );
        break;
    case poPsName:
    case countryName:
        printf("%s: %s\n", fields_str_arr[f->field_type], f->value.cpointer);
        break;
    
    default:
        ABORT_PROGRAM("field type %d is invalid", f->field_type);
    }
}

void printEntry(entry* e){
    for(int i = 0; i < 5; i++){
        printField(e->fields + i);
    }
    
    printField(e->fields + speed);
    printField(e->fields + measurmentUnit);

    for(int i = 7; i < FIELD_AMOUNT; i++){
        printField(e->fields + i);
    }
}

int fieldCmp(field f1, field f2){
    switch(f1.field_type){
    case poPsName:
    case countryName:
        return strcmp(f1.value.cpointer, f2.value.cpointer);

    case measurmentUnit:
    case countryAcro:
        return strncmp(f1.value.carray, f2.value.carray, 
            fields_size_arr[f1.field_type]
        );

    default:
        return f1.value.integer != f2.value.integer;
    }
}

int findFieldType(char* str){
    for(int i = 0; i < FIELD_AMOUNT; i++){
        if(strcmp(str, fields_str_arr[i]) == 0){
            return i;
        }
    }
    return -1;
}
