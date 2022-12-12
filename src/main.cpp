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
#include "NetworkGraph.hpp"

extern "C" {
#include "utils.h"
}

int main(){
    int32_t command;
    char* table_name;
    READ_INPUT("%d %ms", &command, &table_name);

    Table* topology = new Table(table_name, "rb");
    NetworkGraph* graph = new NetworkGraph(*topology);

    switch(command){
    case 11:
        std::cout << *graph;
        break;    
    default:
        errno = EINVAL;
        ABORT_PROGRAM("command number");
    }

    delete graph;
    delete topology;
    free(table_name);
}
