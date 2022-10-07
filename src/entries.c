#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "entries.h"
#include "utils.h"


static const char fields_str_arr[][20] = {   
    "removido", "encadeamento", "idConecta", "siglaPais", "idPoPsConectado", 
    "unidadeMedida", "velocidade", "nomePoPs", "nomePais"
};

static const int8_t fields_size_arr[] = {
    1, 4, 4, 2, 4, 1, 4, -1, -1
};


entry* createEntry(void){
    entry* e;
    XALLOC(entry, e, 1);
    memset(e, '$', 1); //coloca lixo nos ponteiros para char também!

    for(int i = 0; i < FIELD_AMOUNT; i++){
        e->fields[i].field_type = i;
    }

    e->fields[0].value.removed = 0;
    e->fields[1].value.linking = -1;
    e->fields[7].value.poPsName = NULL;
    e->fields[8].value.parentsName = NULL;

    return e;
}

void deleteEntry(entry* e){
    if(e->fields[7].value.poPsName){
        free(e->fields[7].value.poPsName);
    }
    if(e->fields[8].value.parentsName){
        free(e->fields[8].value.parentsName);
    }

    free(e);
}

int readField(FILE* fp, field* f, int read_for_entry){
    if(read_for_entry >= MAX_SIZE_ENTRY){
        return read_for_entry;
    }

    if(fields_size_arr[f->field_type] > 0){
        //campo de tamanho fixo:

        //como não há campos de tamanho fixo após um campo de tamanho variável,
        //não é possível que read_for_entry seja >= MAX_SIZE_ENTRY, então o
        //campo inteiro pode ser lido sempre.
        fread(&(f->value), fields_size_arr[f->field_type], 1, fp);
        return fields_size_arr[f->field_type] + read_for_entry;
    }

    //campo de tamanho variável
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
            errno = EBADFD;
            ABORT_PROGRAM("leitura de field %s na posicao %d", 
                fields_str_arr[f->field_type], 
                ftell(fp)
            );
        }
    }
    str[i] = '\0';
    f->value.cpointer = str;

    return read_for_entry+1;
}

entry* readEntry(FILE* fp){
    entry* e = createEntry();
    
    int read_for_entry = 0;

    for(int i = 0; i < FIELD_AMOUNT && read_for_entry < MAX_SIZE_ENTRY; i++){
        read_for_entry = readField(fp, e->fields+i, read_for_entry);
    }

    fseek(fp, MAX_SIZE_ENTRY-read_for_entry, SEEK_CUR);

    printf("%d\n", read_for_entry);

    return e;
}

int writeField(FILE* fp, field* f){
    //campos de tamanho fixo
    if(fields_size_arr[f->field_type] > 0){
        fwrite(&(f->value), fields_size_arr[f->field_type], 1, fp);
        return fields_size_arr[f->field_type];
    }

    //campos de tamanho variável
    if(f->value.cpointer == NULL){
        putc('|', fp);
        return 1;
    }

    for(uint32_t i = 0; i < strlen(f->value.cpointer); i++){
        putc(f->value.cpointer[i], fp);
    }
    putc('|', fp);

    return strlen(f->value.cpointer) + 1;
}

void writeEntry(FILE* fp, entry* e){
    int position = 0;
    for(int i = 0; i < FIELD_AMOUNT; i++){
        position += writeField(fp, e->fields+i);
    }

    for(int i = 0; i < MAX_SIZE_ENTRY-position; i++){
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
        printf("%s: %d ", fields_str_arr[f->field_type], f->value.speed);
        break;
    case measurmentUnit:
        printf("%cbps\n", f->value.measurmentUnit);
        break;

    case parentsAcro:
        printf("%s: %s\n", 
            fields_str_arr[f->field_type], 
            f->value.parentsAcro
        );
        break;
    case poPsName:
    case parentsName:
        printf("%s: %s\n", fields_str_arr[f->field_type], f->value.cpointer);
        break;
    
    default:
        errno = EINVAL;
        ABORT_PROGRAM("tipo de field %d", f->field_type);
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
