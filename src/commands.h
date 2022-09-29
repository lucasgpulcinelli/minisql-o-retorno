#ifndef __COMMANDS_H__
#define __COMMANDS_H__

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

void commandFrom(void);

void commandWhere(void);

void commandDelete(void);

void commandInsert(void);

void commandCompact(void);

#endif

