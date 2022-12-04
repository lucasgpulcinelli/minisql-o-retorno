#include <iostream>
#include <cinttypes>
#include "Graph.hpp"

template <class Node, class Edge> void Graph<class Node, class Edge>::print(){
    for(int32_t node = 0; node < num_nodes; node++){
        for(int32_t edge = 0; edge < adjacencies[node].size(); edge++){
            std::cout << node_list[node] << " ";
            std::cout << adjacencies[node][edge].second << std::endl;
        }
    }
}

template <class Node, class Edge> 
Graph<class Node, class Edge>::Graph(const Table& topology){
    while(topology.hasNextEntry()){
        entry* next_entry = topology->readNextEntry();

        Edge edge = Edge(next_entry);
        Node node = Node(next_entry);
        
        insertNewEdge(node, edge);
    }
}

template <class Node, class Edge> void 
Graph<class Node, class Edge>::insertNewEdge(Node exit_node, Edge new_edge){
    size_t exit_node_index = insertNode(exit_node);
    
}

template <class Node, class Edge> 
size_t Graph<class Node, class Edge>::insertNode(Node new_node){
    size_t start = 0;
    size_t end = num_nodes - 1;

    while(true){
        size_t middle = (start + end)/2;

        if(node_list[middle] == new_node){
            return middle;

        } else if ((node_list[middle] < new_node) && (end >= start)){
            beggining = middle + 1;

        } else if ((new_node < node_list[middle]) && (end >= start)){
            end = middle - 1;

        } else if (node_list[middle] < new_node){
            node_list.insert(node_list.begin() + middle + 1;, new_node);
            adjacencies.insert(adjacencies.begin() + middle + 1;, 
                               std::vector<std::pair<size_t, Edge>>());
            return middle + 1;

        } else {
            node_list.insert(node_list.begin() + middle;, new_node);
            adjacencies.insert(adjacencies.begin() + middle;, 
                               std::vector<std::pair<size_t, Edge>>());
            return middle;
        }
    }
}

template <class Node, class Edge>
void Graph<class Node, class Edge>::insertNewNode(Node new_node){
    insertNode(new_node);
}
