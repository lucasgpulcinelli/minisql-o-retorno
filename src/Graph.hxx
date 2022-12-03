#include <iostream>
#include <cinttypes>
#include "Graph.hpp"

template <class Node, class Edge> void Graph<class Node, class Edge>::print(){
    for(int32_t node = 0; node < num_nodes; node++){
        for(int32_t edge = 0; edge < adjacencies[node].size(); edge++){
            std::cout << node_list[node] << " ";
            std::cout << adjacencies[node][edge].first << std::endl;
        }
    }
}