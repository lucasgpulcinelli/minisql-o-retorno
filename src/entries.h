#ifndef __ENTRIES_H__
#define __ENTRIES_H__

#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>

#define NOT_ERASED -1

#define FIELD_AMOUNT 9
#define MAX_SIZE_ENTRY 64
#define CHAR_ARRAY_LEN 4

#define NULL_STR "NULO"


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

enum DataTypes{
    boolean = 0,
    integer,
    char_p,
    char_array
};

typedef struct {
    uint8_t field_type;
    union {
        bool cbool;
        int32_t integer;
        char* cpointer;
        char carray[CHAR_ARRAY_LEN];
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

/*
 * WriteField writes a field f on the file fp if the contents
 * of the field are less than or equal to size. On success, it
 * returns the size of used space. Otherwise, it returns -1.
 */
int writeField(FILE* fp, field* f, ssize_t size);
void writeEntry(FILE* fp, entry* e);
void printField(field* f);
void printEntry(entry* e);
int fieldCmp(field f1, field f2);
int findFieldType(char* str);


#endif
