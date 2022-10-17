#ifndef __INPUTS_H__
#define __INPUTS_H__

#include "entries.h"

/*
 * readTuples reads n lines of key-value pairs with field names followed by 
 * values, returning the associated fields.
 */
field* readTuples(int n);

//freeTuples frees a collection of tuples alloced by readTuples
void freeTuples(field* fs, int n);

#endif
