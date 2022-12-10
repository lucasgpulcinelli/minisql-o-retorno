#ifndef __GRAPH_HXX__
#define __GRAPH_HXX__

#include <iostream>
#include <cinttypes>

#include "Graph.hpp"

template<typename NodeMetadata>
bool operator<(const Node<NodeMetadata>& left_arg, 
               const Node<NodeMetadata>& right_arg){
    return left_arg.id < right_arg.id;
}

template<typename NodeMetadata>
bool operator==(const Node<NodeMetadata>& left_arg, 
                const Node<NodeMetadata>& right_arg){
    return left_arg.id == right_arg.id;
}

template<typename EdgeMetadata>
bool operator==(const Edge<EdgeMetadata>& left_arg, 
                const Edge<EdgeMetadata>& right_arg){
    return left_arg.id_from == right_arg.id_from && 
        left_arg.id_to == right_arg.id_to;
}

template<typename NodeMetadata>
std::ostream& operator<<(std::ostream& os, Node<NodeMetadata>& node){
    return os << node.id << " " << node.t << std::endl;
}

template<typename NodeMetadata, typename EdgeMetadata>
std::ostream& operator<<(std::ostream& os, 
                         Graph<NodeMetadata, EdgeMetadata>& g){
    for(size_t node = 0; node < g.num_nodes; node++){
        os << g.node_list[node];
    }

    return os;
}

template<typename NodeMetadata>
Node<NodeMetadata>::Node(int32_t id, NodeMetadata t){
    this->id = id;
    this->t = t;
}

template<typename EdgeMetadata>
Edge<EdgeMetadata>::Edge(int id_from, int id_to, EdgeMetadata t){
    this->id_from = id_from;
    this->id_to = id_to;
    this->t = t;
}

template<typename NodeMetadata, typename EdgeMetadata>
void Graph<NodeMetadata, EdgeMetadata>::insertEdge(Edge<EdgeMetadata> new_edge){
}

template<typename NodeMetadata, typename EdgeMetadata>
size_t Graph<NodeMetadata, EdgeMetadata>::insertNode(Node<NodeMetadata> new_node){
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

template<typename NodeMetadata, typename EdgeMetadata>
Graph<NodeMetadata, EdgeMetadata>::~Graph(void){
}

#endif