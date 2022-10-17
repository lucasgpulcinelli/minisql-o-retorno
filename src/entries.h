#ifndef __ENTRIES_H__
#define __ENTRIES_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#define FIELD_AMOUNT 9
#define MAX_SIZE_ENTRY 64

/*
 * enum FieldTypes represents all possible fields, ordered by apperance in the
 * binary file for an entry.
 */
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

/*
 * struct field represents a single field of any kind, be it variable sized 
 * (using a pointer to store values) or fixed size, possibly a character array
 * or an integer.
 */
typedef struct {
    uint8_t field_type;
    union {
        int32_t integer;
        char* cpointer;
        char carray[4];
    } value;
} field;

/*
 * struct entry represents an ordered collection of fields, in the same order as
 * in the FieldTypes enum.
 */
typedef struct {
    field fields[9];
} entry;

/*
 * createEntry creates a collection of entries, initializing all of them with 
 * trash or NULL pointers in all fields.
 */
entry* createEntry(uint32_t size);

/*
 * initEntry initialized a single entry with trash for fixed size fields and 
 * NULL pointers for variable sized fields.
 */
void initEntry(entry* e);

/*
 * deleteEntry frees all entries and fields associated (including pointers 
 * inside it).
 */
void deleteEntry(entry* e, uint32_t size);

/*
 * clearEntry frees all pointers inside fields and reinitializes the entry as if
 * it was never used.
 */
void clearEntry(entry* e);

/*
 * readFields reads a single field of the type given by f->field_type from fp,
 * having an extra value to keep track of the read index for the associated 
 * entry, not letting the read index be greater than the maximum size.
 */
int readField(FILE* fp, field* f, int read_for_entry);

//readEntry reads a full entry from fp.
void readEntry(FILE* fp, entry* e);

void readEntryFromCSV(char *csv_line, entry *es);

int writeField(FILE* fp, field* f);
void writeEntry(FILE* fp, entry* e);

//printField prints a single field to stdout.
void printField(field* f);

//printEntry prints a single entry to stdout.
void printEntry(entry* e);

/*
 * fieldCmp compares two fields of the same type, returning in the same way as 
 * strcmp, and aborting the program if the field types are different.
 */
int fieldCmp(field f1, field f2);

/*
 * findFieldType returns the FieldType enum number for a given string, or -1 if
 * no match was found.
 */
int findFieldType(char* str);

/*
 * copyEntry copies a single entry from src to dest, trowing all data from dest
 * away.
 */
void copyEntry(entry* dest, entry* src);

#endif
