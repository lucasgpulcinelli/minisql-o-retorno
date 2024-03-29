#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entries.h"
#include "utils.h"

// the string names used for each field as ordered in FieldTypes enum
static const char fields_str_arr[FIELD_AMOUNT][20]
    = { "removido", "encadeamento", "idConecta", "siglaPais", "idPoPsConectado",
          "unidadeMedida", "velocidade", "nomePoPs", "nomePais" };

/*
 * sizes for each field as ordered in FieldTypes enum, -1 represents a variable
 * field
 */
static const int8_t fields_size_arr[] = { 1, 4, 4, 2, 4, 1, 4, -1, -1 };

/*
 * printing strings for each field (except for meta fields, these can't be
 * printed)
 */
static const char fields_print_arr[FIELD_AMOUNT][35]
    = { "", "", "Identificador do ponto", "Sigla do pais",
          "Identificador do ponto conectado", "", "Velocidade de transmissao",
          "Nome do ponto", "Pais de localizacao" };

static const char conn_fields_print_arr[FIELD_AMOUNT][35]
    = { "", "", "", "Sigla do pais", "", "", "", "Nome do ponto conectado",
          "Nome do pais conectado" };

entry* createEntry(uint32_t size) {
    entry* es;
    MEMSET_ALLOC(entry, es, size, '$');

    for (uint32_t i = 0; i < size; i++) {
        initEntry(es + i);
    }

    return es;
}

void deleteEntry(entry* es, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        clearEntry(es + i);
    }

    free(es);
}

void initEntry(entry* e) {
    for (uint32_t i = 0; i < FIELD_AMOUNT; i++) {
        e->fields[i].field_type = i;
    }

    e->fields[removed].value.carray[0] = NOT_REMOVED;
    e->fields[linking].value.integer = NULL_INT;
    e->fields[poPsName].value.cpointer = NULL;
    e->fields[countryName].value.cpointer = NULL;
}

void clearEntry(entry* e) {
    if (e->fields[poPsName].value.cpointer) {
        free(e->fields[poPsName].value.cpointer);
    }
    if (e->fields[countryName].value.cpointer) {
        free(e->fields[countryName].value.cpointer);
    }

    initEntry(e);
}

int32_t readField(FILE* fp, field* f, int read_for_entry) {
    // an entry cannot be bigger than its max size
    if (read_for_entry >= MAX_SIZE_ENTRY) {
        return read_for_entry;
    }

    if (fields_size_arr[f->field_type] > 0) {
        // fixed sized fields

        int32_t ret = fread(&(f->value), fields_size_arr[f->field_type], 1, fp);
        if (ret != 1) {
            ABORT_PROGRAM("unexpected EOF in read field %s",
                fields_str_arr[f->field_type]);
        }
        return fields_size_arr[f->field_type] + read_for_entry;
    }

    // variable sized fields
    char* str;
    XALLOC(char, str, MAX_SIZE_ENTRY);

    int32_t i, c;
    for (i = 0; (c = getc(fp)) != '|'
         && read_for_entry
             < MAX_SIZE_ENTRY; // if we go past the max size, break
         i++, read_for_entry++) {
        if (c == EOF) {
            ABORT_PROGRAM("unexpected EOF in read field %s",
                fields_str_arr[f->field_type], ftell(fp));
        }

        str[i] = c;
    }

    // the last character must always be '\0'
    str[i] = '\0';
    XREALLOC(char, str, i + 1);
    f->value.cpointer = str;

    return read_for_entry + 1;
}

void readEntry(FILE* fp, entry* e) {
    int32_t read_for_entry = readField(fp, e->fields, 0);
    if (ENTRY_REMOVED(e)) {
        // if the entry is deleted, only read the meta fields

        read_for_entry = readField(fp, e->fields + 1, read_for_entry);
        fseek(fp, MAX_SIZE_ENTRY - read_for_entry, SEEK_CUR);
        return;
    }

    for (uint32_t i = 1; i < FIELD_AMOUNT; i++) {
        read_for_entry = readField(fp, e->fields + i, read_for_entry);
    }
    fseek(fp, MAX_SIZE_ENTRY - read_for_entry, SEEK_CUR);
}

int32_t writeField(FILE* fp, field* f, ssize_t size) {
    if (fields_size_arr[f->field_type] > 0) {
        // fixed sized fields
        fwrite(&(f->value), fields_size_arr[f->field_type], 1, fp);
        return fields_size_arr[f->field_type];
    }

    // variable sized fields
    if (f->value.cpointer == NULL || (!strcmp(f->value.cpointer, NULL_STR))) {
        // string is empty, so just write separator char.
        putc('|', fp);
        return 1;
    }

    /*
     * If the string fits in the remaining bytes of the entry,
     * write_len is simply its size. Otherwise,
     * write_len is the number of bytes that are available
     * for writing, minus the number of separator chars that
     * are left to be written.
     */
    ssize_t write_len
        = min(strlen(f->value.cpointer), size - (FIELD_AMOUNT - f->field_type));
    fwrite(f->value.cpointer, write_len, 1, fp);
    putc('|', fp);
    return write_len + 1;
}

void writeEntry(FILE* fp, entry* e) {
    ssize_t bytes = MAX_SIZE_ENTRY; // Available bytes in entry
    for (uint32_t i = 0; i < FIELD_AMOUNT && bytes > 0; i++) {
        bytes -= writeField(fp, e->fields + i, bytes);
    }

    while (bytes > 0) {
        putc('$', fp);
        bytes--;
    }
}

void printField(field* f, bool connected_node) {
    switch (f->field_type) {
    case removed:
    case linking:
        // won't print meta fields
        break;

    case idConnect:
    case connPoPsId:
    case speed:
        if (f->value.integer == -1) {
            return;
        }
        printf("%s: %d%c", fields_print_arr[f->field_type], f->value.integer,
            (f->field_type == speed) ? ' ' : '\n');
        break;

    case measurmentUnit:
        if (f->value.carray[0] == '\0' || f->value.carray[0] == '$') {
            return;
        }
        printf("%cbps\n", f->value.carray[0]);
        break;

    case countryAcro:
        if (f->value.carray[0] == '\0' || f->value.carray[0] == '$') {
            return;
        }
        printf("%s: %c%c\n", fields_print_arr[f->field_type],
            f->value.carray[0], f->value.carray[1]);
        break;
    case poPsName:
    case countryName:
        if (f->value.cpointer == NULL || f->value.cpointer[0] == '\0') {
            return;
        }
        if (!connected_node) {
            printf(
                "%s: %s\n", fields_print_arr[f->field_type], f->value.cpointer);
        } else {
            printf("%s: %s\n", conn_fields_print_arr[f->field_type],
                f->value.cpointer);
        }
        break;

    default:
        ABORT_PROGRAM("field type %d is invalid", f->field_type);
    }
}

void printEntry(entry* e) {
    printField(e->fields + idConnect, false);
    printField(e->fields + poPsName, false);
    printField(e->fields + countryName, false);
    printField(e->fields + countryAcro, false);
    printField(e->fields + connPoPsId, false);

    // both fields are printed together, so if any is null, do not print neither
    if (e->fields[speed].value.integer != -1
        && e->fields[measurmentUnit].value.carray[0] != '$'
        && e->fields[measurmentUnit].value.carray[0] != '\0') {

        printField(e->fields + speed, false);
        printField(e->fields + measurmentUnit, false);
    }

    printf("\n");
}

int32_t fieldCmp(field f1, field f2) {
    if (f1.field_type != f2.field_type) {
        return 1; // different
    }

    switch (f1.field_type) {
    case poPsName:
    case countryName:
        return strcmp(f1.value.cpointer, f2.value.cpointer);

    case measurmentUnit:
    case countryAcro:
        return strncmp(
            f1.value.carray, f2.value.carray, fields_size_arr[f1.field_type]);

    default:
        return f1.value.integer != f2.value.integer;
    }
}

int32_t findFieldType(char* str) {
    for (int32_t i = 0; i < FIELD_AMOUNT; i++) {
        if (strcmp(str, fields_str_arr[i]) == 0) {
            return i;
        }
    }
    return -1;
}

void copyField(field* dest, field* src) {
    if (dest->field_type != src->field_type) {
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
        strncpy(dest->value.carray, src->value.carray,
            fields_size_arr[dest->field_type]);
        return;
    case poPsName:
    case countryName:
        XREALLOC(char, dest->value.cpointer, strlen(src->value.cpointer) + 1);
        strcpy(dest->value.cpointer, src->value.cpointer);
        return;
    default:
        ABORT_PROGRAM("invalid field");
    }
}

void copyEntry(entry* dest, entry* src) {
    for (int32_t i = 0; i < FIELD_AMOUNT; i++) {
        copyField(dest->fields + i, src->fields + i);
    }
}

void printJointEntry(entry* e1, entry* e2) {
    printField(e1->fields + idConnect, false);
    printField(e1->fields + poPsName, false);
    printField(e1->fields + countryName, false);
    printField(e1->fields + countryAcro, false);
    printField(e1->fields + connPoPsId, false);

    printField(e2->fields + poPsName, true);
    printField(e2->fields + countryName, true);
    printField(e2->fields + countryAcro, true);

    if (e1->fields[speed].value.integer != -1
        && e1->fields[measurmentUnit].value.carray[0] != '$'
        && e1->fields[measurmentUnit].value.carray[0] != '\0') {

        printField(e1->fields + speed, false);
        printField(e1->fields + measurmentUnit, false);
    }

    printf("\n");
}
