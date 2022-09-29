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


int main(){
    int command;
    scanf("%d", &command);

    switch(command){
    case create:
        commandCreate();
        break;
    case from:
        commandFrom();
        break;
    case where:
        commandWhere();
        break;
    case delete:
        commandDelete();
        break;
    case insert:
        commandInsert();
        break;
    case compact:
        commandCompact();
        break;
    default:
        errno = EINVAL;
        ABORT_PROGRAM("command number");
    }
}
