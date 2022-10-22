#ifndef __INPUTS_H__
#define __INPUTS_H__

#include "entries.h"

#define NULL_OFFSET 3

enum NullTypes {
    null_int = 4,
    null_char_p,
    null_char_array
};

/*
 * readTuples reads n lines of key-value pairs with field names followed by 
 * values, returning the associated fields.
 */
field* readTuples(int n);

//freeTuples frees a collection of tuples alloced by readTuples
void freeTuples(field* fs, int n);

/*
 * readEntryFromCSV reads an entry from the string csv_line
 * that must follow a csv ("comma separated values") format.
 */
void readEntryFromCSV(char *csv_line, entry *es);

/*
 * readEntryFromStdin reads an entry from stdin in format
 * specified by commandInsert.
 */
void readEntryFromStdin(entry *es);

/*
 * readFieldFromStdin reads a string from Stdin formated to be
 * a field for commandInsert.
 */
void readFieldFromStdin(char** field_str);

/*
 * storeField stores a field of es specified by its number (size_t field)
 * and its data type (int8_t type). If a field has null value, you
 * should pass a nullType.
 */
void storeField(entry *es, size_t field, int8_t type, char* field_str);

#endif
