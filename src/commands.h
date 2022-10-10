#ifndef __COMMANDS_H__
#define __COMMANDS_H__


#define COMMANDS_SIZE 6


enum Commands {
    create = 1,
    /*
     * originally, "select" was the name for the next command, however, select 
     * is a standard library function name.
     */
    from, 
    where,
    delete,
    insert,
    compact
};


void commandCreate(void);

/*
 * commandFrom gets the name of a binary file from stdin and prints all 
 * entries contained in it. 
 */
void commandFrom(void);

/*
 * commandWhere gets the name of a binary file and a number n of queries from 
 * stdin, as well as a series of n key-value pairs (composed of a field name,
 * defined as in entries.c fields_str_arr, and a integer or strings between 
 * quotes). It prints, for each query, the fields in the binary file that have
 * the specified value.
 */
void commandWhere(void);

void commandDelete(void);

void commandInsert(void);

void commandCompact(void);


#endif
