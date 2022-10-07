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
    parentsAcro,
    connPoPsId,
    measurmentUnit,
    speed,
    poPsName,
    parentsName
};

typedef struct {
    uint8_t field_type;
    union {
        int32_t removed;
        int32_t linking;
        int32_t idconnect;
        char parentsAcro[2];
        int32_t connPoPsId;
        char measurmentUnit;
        int32_t speed;
        char* poPsName;
        char* parentsName;

        int32_t integer;
        char* cpointer;
    } value;
} field;

typedef struct {
    field fields[9];
} entry;


entry* createEntry(void);
void deleteEntry(entry* e);
int readField(FILE* fp, field* f, int read_for_entry);
entry* readEntry(FILE* fp);
void printField(field* f);
void printEntry(entry* e);


#endif
