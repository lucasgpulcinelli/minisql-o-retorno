/*
 * Este programa faz parte do trabalho 1 da disciplina de Estrutura de Dados 3.
 *
 * Programa desenvolvido por:
 * - Lucas Eduardo Gulka Pulcinelli - N°USP: 12547336 - 100% de participação
 * - Gabriel Franceschi Libardi - NºUSP: 11760739 - 100% de participação
 *
 * Licença de código GPL-V3.
 */

#include <stdio.h>
#include <errno.h>

#include "commands.h"
#include "utils.h"
#include "inputs.h"

//command_funcs is a collection of functions to call depending on the input
static void (*const command_funcs[])(void) = {
    commandCreate, commandWhere, commandInsert, commandJoin
};


int main(){
    int32_t command;
    READ_INPUT("%d", &command);

    if(command < COMMANDS_OFFSET || command >= COMMANDS_SIZE+COMMANDS_OFFSET){
        errno = EINVAL;
        ABORT_PROGRAM("command number");
    }

    //calls the function from the array with the correct order
    command_funcs[command-COMMANDS_OFFSET]();
}
