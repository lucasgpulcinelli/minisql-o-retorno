/*
 * Este programa faz parte do trabalho 1 da disciplina de Estrutura de Dados 3.
 *
 * Programa desenvolvido por:
 * - Lucas Eduardo Gulka Pulcinelli - N°USP: 12547336
 * - Gabriel Franceschi Libardi - NºUSP: 11760739
 *
 * Licença de código GPL-V3.
 */

#include <stdio.h>
#include <errno.h>

#include "commands.h"
#include "utils.h"


static void (* const command_funcs[])(void) = {
    commandCreate, commandFrom, commandWhere,
    commandDelete, commandInsert, commandCompact
};


int main(){
    int command;
    scanf("%d", &command);

    if(command < 1 || command > COMMANDS_SIZE){
        errno = EINVAL;
        ABORT_PROGRAM("command number");
    }

    command_funcs[command-1]();
}
