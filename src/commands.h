#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "file_control.h"

#define COMMANDS_SIZE 6
#define CSV_HEADER "idConecta,nomePoPs,nomePais,siglaPais,idPoPsConectado," \
                   "unidadeMedida,velocidade"

/*
 * enum Commands enumerates the 6 commands the user may call.
 */
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
 * created table (stored as a binary file). The name of the CSV file 
 * and the name of the table are passed through stdin.
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

/*
 * commandInsert reads a specified number of entries from stdin
 * and inserts them in a table. The number n of entries and the name of
 * the table are read from stdin.
 */
void commandInsert(void);

/*
 * commandCompact compacts a table specified by the user. It only throws away
 * the entries that were deleted in the table, reducing its size in bytes.
 */
void commandCompact(void);


#endif
