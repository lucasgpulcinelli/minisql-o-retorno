#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "entries.h"
#include "utils.h"

//the string names used for each field as ordered in FieldTypes enum
static const char fields_str_arr[][20] = {
    "removido", "encadeamento", "idConecta", "siglaPais", "idPoPsConectado",
    "unidadeMedida", "velocidade", "nomePoPs", "nomePais"
};

/*
 * sizes for each field as ordered in FieldTypes enum, -1 represents a variable
 * field
 */
static const int8_t fields_size_arr[] = {
    1, 4, 4, 2, 4, 1, 4, -1, -1
};  


entry* createEntry(uint32_t size){
    entry* es;
    XALLOC(entry, es, size);

    for(uint32_t i = 0; i < size; i++){
        initEntry(es+i);
    }

    return es;
}

void deleteEntry(entry* es, uint32_t size){
    for(uint32_t i = 0; i < size; i++){
        clearEntry(es+i);
    }

    free(es);
}

void initEntry(entry* e){
    memset(e, '$', sizeof(entry));

    for(uint32_t i = 0; i < FIELD_AMOUNT; i++){
        e->fields[i].field_type = i;
    }

    e->fields[removed].value.cbool = false;
    e->fields[linking].value.integer = -1;
    e->fields[poPsName].value.cpointer = NULL;
    e->fields[countryName].value.cpointer = NULL;
}

void clearEntry(entry* e){
    if(e->fields[poPsName].value.cpointer){
            free(e->fields[poPsName].value.cpointer);
    }
    if(e->fields[countryName].value.cpointer){
            free(e->fields[countryName].value.cpointer);
    }

    initEntry(e);
}

int readField(FILE* fp, field* f, int read_for_entry){
    //an entry cannot be bigger than its max size
    if(read_for_entry >= MAX_SIZE_ENTRY){
        return read_for_entry;
    }

    if(fields_size_arr[f->field_type] > 0){
        //fixed sized fields

        int ret = fread(&(f->value), fields_size_arr[f->field_type], 1, fp);
        if(ret != 1){
            ABORT_PROGRAM("unexpected EOF in read field %s",
                fields_str_arr[f->field_type]
            );
        }
        return fields_size_arr[f->field_type] + read_for_entry;
    }

    //variable sized fields
    char* str;
    XALLOC(char, str, MAX_SIZE_ENTRY);

    int i, c;
    for(i = 0; (c = getc(fp)) != '|' &&
        read_for_entry <= MAX_SIZE_ENTRY; //if we go past the max size, break
        i++, read_for_entry++
    ){
        if(c == EOF){
            ABORT_PROGRAM("unexpected EOF in read field %s",
                fields_str_arr[f->field_type],
                ftell(fp)
            );
        }
        
        str[i] = c;
    }

    //the last character must always be '\0'
    str[i] = '\0';
    f->value.cpointer = str;

    return read_for_entry+1;
}

void readEntry(FILE* fp, entry* e){
    int read_for_entry = 0;

    for(uint32_t i = 0; i < FIELD_AMOUNT; i++){
        read_for_entry = readField(fp, e->fields+i, read_for_entry);
    }

    fseek(fp, MAX_SIZE_ENTRY-read_for_entry, SEEK_CUR);
}

void readEntryFromStdin(entry *es) {
    char* input_line;
    READ_INPUT("%ms\n", &input_line);
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

void copyField(field* dest, field* src){
    if(dest->field_type != src->field_type){
        ABORT_PROGRAM("tried to copy different fields");
    }

    switch (dest->field_type) {
    case removed:
    case linking:
    case idConnect:
    case connPoPsId:
    case speed:
        dest->value.integer = src->value.integer;
        return;
    case countryAcro:
    case measurmentUnit:
        strncpy(dest->value.carray, src->value.carray, fields_size_arr[dest->field_type]);
        return;
    case poPsName:
    case countryName:
        XREALLOC(char, dest->value.cpointer, strlen(src->value.cpointer)+1);
        strcpy(dest->value.cpointer, src->value.cpointer);
        return;
    default:
        ABORT_PROGRAM("invalid field");
    }
}

void copyEntry(entry* dest, entry* src){
    for(int i = 0; i < FIELD_AMOUNT; i++){
        copyField(dest->fields+i, src->fields+i);
    }
}
