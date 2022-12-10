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
void Graph<NodeMetadata, EdgeMetadata>::insertNode(Node<NodeMetadata> new_node){
    auto result = node_list.insert(new_node.id, new_node);
    adjacencies.insert(new_node.id, std::vector<Edge<EdgeMetadata>>());

    if(result.second == false && (!node_list[new_node.id].isEmpty())){
        node_list[new_node.id] = new_node;
    }
}

#endif