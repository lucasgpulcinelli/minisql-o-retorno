#ifndef __INPUTS_H__
#define __INPUTS_H__

#include "entries.h"

#define NULL_INT -1
#define CHAR_ARRAY_SIZE 4
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

#endif
