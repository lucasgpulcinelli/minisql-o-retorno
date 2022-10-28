#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "file_control.h"

#define COMMANDS_SIZE 6
#define CSV_HEADER "idConecta,nomePoPs,nomePais,siglaPais,idPoPsConectado,"   \
                   "unidadeMedida,velocidade"

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

/*
 * commandCreate reads a CSV and stores it in a newly
 * created table (stored as a binary file).
 */
void commandCreate(void);

/*
 * commandFrom gets the name of a binary file from stdin and prints all 
 * entries contained in it. 
 */
void commandFrom(void);

/*
 * commandWhere gets the name of a binary file and a number n of queries from 
 * stdin, as well as a series of n key-value pairs (composed of a field name,
 * defined as in entries.c fields_str_arr, and an integer or strings between 
 * quotes). It prints, for each query, the fields in the binary file that have
 * the specified value.
 */
void commandWhere(void);

/*
 * commandDelete deletes all query matches, provided in the same way as 
 * commandWhere.
 */
void commandDelete(void);

void commandInsert(void);

void commandCompact(void);


#endif
