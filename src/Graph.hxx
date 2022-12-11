#ifndef __GRAPH_HXX__
#define __GRAPH_HXX__

#include <iostream>
#include <cinttypes>

#include "Graph.hpp"

template<class Node, class Edge>
std::ostream& operator<<(std::ostream& os, const Graph<Node, Edge>& graph){
    for(auto node_it = graph.node_list.begin(); node_it != graph.node_list.end(); 
        node_it++){
        if(!((*node_it).second.isEmpty())){
            os << (*node_it).second << std::endl;
        }
    }

    return os;
}

template<class Node, class Edge>
void Graph<Node, Edge>::insertEdge(const Edge& new_edge){
    insertAdjacency(new_edge, new_edge.idFrom());

    Edge new_edge_dual = new_edge.getDual();
    insertAdjacency(new_edge_dual, new_edge.idTo());

    node_list.insert(std::pair<int32_t, Node>(new_edge.idFrom(), Node()));
    node_list.insert(std::pair<int32_t, Node>(new_edge.idTo(), Node()));
}

template<class Node, class Edge>
void Graph<Node, Edge>::insertAdjacency(const Edge& new_edge, int32_t node){
    std::vector<Edge> adjacent_nodes = adjacencies[node];

    if(adjacent_nodes.size() == 0){
        adjacent_nodes.push_back(new_edge);
    }

    ssize_t start = 0;
    ssize_t end = adjacent_nodes.size() -1;

    while(true){
        ssize_t middle = (start + end)/2;

        if(adjacent_nodes[middle] == new_edge ){
            return;

        } else if ((adjacent_nodes[middle] < new_edge) && (end > start)){
            start = middle + 1;

        } else if ((new_edge < adjacent_nodes[middle]) && (end > start)){
            end = middle;

        } else if (adjacent_nodes[middle] < new_edge){
            adjacent_nodes.insert(adjacent_nodes.begin() + middle + 1, new_edge);
            return;

        } else {
            adjacent_nodes.insert(adjacent_nodes.begin() + middle, new_edge);
            return;
        }
    }
}

template<class Node, class Edge>
void Graph<Node, Edge>::insertNode(const Node& new_node){
    auto result = node_list.insert(std::pair<int32_t, Node>(new_node.idKey(), 
                                                            new_node));

    if(result.second == false && node_list[new_node.idKey()].isEmpty()){
        node_list[new_node.idKey()] = new_node;
    }
}

#endif