#ifndef __GRAPH_HXX__
#define __GRAPH_HXX__

#include <iostream>
#include <cinttypes>

#include "Graph.hpp"

template<typename T>
bool operator<(const Node<T>& left_arg, const Node<T>& right_arg){
    return left_arg.id < right_arg.id;
}

template<typename T>
bool operator==(const Node<T>& left_arg, const Node<T>& right_arg){
    return left_arg.id == right_arg.id;
}

template<typename T>
bool operator==(const Edge<T>& left_arg, const Edge<T>& right_arg){
    return left_arg.id_from == right_arg.id_from && 
        left_arg.id_to == right_arg.id_to;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, Node<T>& node){
    return os << node.id << " " << node.t << std::endl;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, Graph<T1, T2>& g){
    for(size_t node = 0; node < g.num_nodes; node++){
        os << g.node_list[node];
    }
    return os;
}

template<typename T>
Node<T>::Node(int id, T t){
    this->id = id;
    this->t = t;
}

template<typename T>
Edge<T>::Edge(int id_from, int id_to, T t){
    this->id_from = id_from;
    this->id_to = id_to;
    this->t = t;
}

template<typename T1, typename T2>
void Graph<T1, T2>::insertEdge(Edge<T2> new_edge){
}

template<typename T1, typename T2>
size_t Graph<T1, T2>::insertNode(Node<T1> new_node){
    size_t start = 0;
    size_t end = num_nodes-1;

    if(num_nodes == 0){
        node_list.push_back(new_node);
        num_nodes++;
        return 0;
    }

    while(true){
        size_t middle = (start + end)/2;

        if(node_list[middle] == new_node){
            return middle;

        } else if ((node_list[middle] < new_node) && (end > start)){
            start = middle + 1;

        } else if ((new_node < node_list[middle]) && (end > start)){
            end = middle;

        } else if (node_list[middle] < new_node){
            node_list.insert(node_list.begin() + middle + 1, new_node);
            num_nodes++;
            return middle + 1;

        } else {
            node_list.insert(node_list.begin() + middle, new_node);
            num_nodes++;
            return middle;
        }
    }
}

template<typename T1, typename T2>
Graph<T1, T2>::~Graph(void){
}

#endif