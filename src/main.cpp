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
#include "Graph.hpp"
#include "TopologyGraph.hpp"

extern "C" {
#include "utils.h"
}

int main(){
    int32_t command;
    char* table_name;
    READ_INPUT("%d %ms", &command, &table_name);

    Table* t = new Table(table_name, "rb");
    auto g = new Graph<Node, Edge>();

    for(entry* e; (e = t->readNextEntry()) != NULL; deleteEntry(e, 1)){
        auto node = NetworkNode(e);
        auto conn = Connection(e);
        g->insertNode(node);
        g->insertEdge(conn);
    }


    switch(command){
    case 11:
        std::cout << *g;
        break;    
    case 12:
        std::cout << "Quantidade de ciclos: " << g->getNumCicles() << std::endl;
        break;
    default:
        errno = EINVAL;
        ABORT_PROGRAM("command number");
    }

    delete g;
    delete t;
    free(table_name);
}
