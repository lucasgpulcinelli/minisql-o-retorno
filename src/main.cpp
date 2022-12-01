/*
 * Este programa faz parte do trabalho 1 da disciplina de Estrutura de Dados 3.
 *
 * Programa desenvolvido por:
 * - Lucas Eduardo Gulka Pulcinelli - N°USP: 12547336 - 100% de participação
 * - Gabriel Franceschi Libardi - NºUSP: 11760739 - 100% de participação
 *
 * Licença de código GPL-V3.
 */

#include <cstdio>
#include <cerrno>

#include "commands.hpp"
#include "file_control.hpp"

extern "C" {
#include "utils.h"
}

//command_funcs is a collection of functions to call depending on the input
static void (*const command_funcs[])(const Table&) = {
    commandPrint, commandCicles, commandSpeed, commandTravel
};


int main(){
    int32_t command;
    char* table_name;
    READ_INPUT("%d %ms", &command, &table_name);

    if(command < COMMANDS_OFFSET || command >= COMMANDS_SIZE+COMMANDS_OFFSET){
        errno = EINVAL;
        ABORT_PROGRAM("command number");
    }


    Table* t = new Table();
    t->openTable(table_name, "rb");

    //calls the function from the array with the correct order
    command_funcs[command-COMMANDS_OFFSET](*t);

    delete t;
    free(table_name);
}
