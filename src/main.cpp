/*
 * Este programa faz parte do trabalho 1 da disciplina de Estrutura de Dados 3.
 *
 * Programa desenvolvido por:
 * - Lucas Eduardo Gulka Pulcinelli - N°USP: 12547336 - 100% de participação
 * - Gabriel Franceschi Libardi - NºUSP: 11760739 - 100% de participação
 *
 * Licença de código GPL-V3.
 */

#include <cerrno>
#include <iostream>

#include "table.hpp"
#include "Graph.hxx"
#include "TopologyGraph.hpp"

extern "C" {
#include "utils.h"
}

int main(){
    int32_t command;
    char* table_name;
    READ_INPUT("%d %ms", &command, &table_name);

    Table* t = new Table(table_name, "rb");
    auto g = t->createGraph();

    switch(command){
    case 11:
        std::cout << *g;
        break;    
    default:
        errno = EINVAL;
        ABORT_PROGRAM("command number");
    }

    delete g;
    delete t;
    free(table_name);
}
