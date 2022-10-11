#ifndef __ENTRIES_H__
#define __ENTRIES_H__

#include <inttypes.h>
#include <stdbool.h>


#define FIELD_AMOUNT 9
#define MAX_SIZE_ENTRY 64


enum FieldsTypes{
    removed = 0,
    linking,
    idConnect,
    countryAcro,
    connPoPsId,
    measurmentUnit,
    speed,
    poPsName,
    countryName
};

typedef struct {
    uint8_t field_type;
    union {
        int32_t integer;
        char* cpointer;
        char carray[4];
    } value;
} field;

typedef struct {
    field fields[9];
} entry;


entry* createEntry(uint32_t size);
void deleteEntry(entry* e, uint32_t size);
void clearEntry(entry* e);
int readField(FILE* fp, field* f, int read_for_entry);
int readEntry(FILE* fp, entry* e);
void readEntryFromCSV(char *csv_line, entry *es);
int writeField(FILE* fp, field* f);
void writeEntry(FILE* fp, entry* e);
void printField(field* f);
void printEntry(entry* e);
int fieldCmp(field f1, field f2);
int findFieldType(char* str);


#endif
