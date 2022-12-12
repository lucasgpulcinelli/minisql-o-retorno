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
    case 13:
        int n;
        std::cin >> n;
        
        for(int i = 0; i < n; i++){
            int a, b;
            std::cin >> a >> b;
            std::cout << "Fluxo máximo entre " << a << " e " << b
                << ": ";

            int32_t max_speed = g->getMaxSpeed(a, b);

            if(max_speed == -1){
                std::cout << max_speed << std::endl;
                continue;
            }
            std::cout << max_speed << " Mbps" << std::endl;
        }
        break;
    case 14:
        std::cin >> n;
        
        for(int i = 0; i < n; i++){
            int a, b, c;
            std::cin >> a >> b >> c;
            std::cout << "Comprimento do caminho entre " << a << " e " << b 
                << " parando em " << c << ": ";

            int32_t min_len_ac = g->getLen(a, c);
            int32_t min_len_cb = g->getLen(c, b);
            if(min_len_ac < 0 || min_len_cb < 0){
                std::cout << -1 << std::endl;
                continue;
            }

            std::cout << min_len_ac + min_len_cb << "Mbps" << std::endl;
        }
        break;
    default:
        errno = EINVAL;
        ABORT_PROGRAM("command number");
    }

    delete g;
    delete t;
    free(table_name);
}
