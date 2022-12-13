#ifndef __GRAPH_HXX__
#define __GRAPH_HXX__

#include <iostream>
#include <cinttypes>
#include <queue>
#include <stdexcept>

#include "Graph.hpp"

template<class Node, class Edge>
std::ostream& operator<<(std::ostream& os, const Graph<Node, Edge>& graph){
    for(auto node_it = graph.node_list.begin(); node_it != graph.node_list.end(); 
        node_it++){
        
        if(node_it->second.isEmpty()){
            continue; //Do not print empty nodes.
        }

        std::vector<Edge> connections;

        try{
            connections = graph.adjacencies.at(node_it->second.idKey());
        } catch(std::out_of_range& e){
            continue;
        }

        for(auto edge_it = connections.begin(); edge_it != connections.end();
            edge_it++){
            os << (*node_it).second << " " << *edge_it << std::endl;
        }
    }

    return os;
}

template<class Node, class Edge>
void Graph<Node, Edge>::insertEdge(Edge& new_edge){
    if(new_edge.idFrom() == EMPTY_VALUE){
        throw std::runtime_error("Cannot insert Edge with invalid idConnect.");

    } else if(new_edge.idTo() == EMPTY_VALUE){
        throw std::runtime_error("Cannot insert Edge with invalid Connected "
                                 "POP's ID. It must be a valid address.");
    }

    insertAdjacency(new_edge);
    /*
     * Reverse edge and insert reversed edge in graph. This ensures that
     * the graph remais non-directional: because instances of class Edge
     * encapsulate direction, it is necessary to insert Edges in both
     * directions in order to get rid of that property.
     */
    new_edge.reverse();
    insertAdjacency(new_edge);
    new_edge.reverse();        //Reverse new_edge back to leave it unchanged.    

    /*
     * If either node in the ends of edge has not been inserted yet, then
     * insert an empty node mapped to the key that it is supposed to have.
     * That way, if the user inserts a non-empty node  with that key, the 
     * empty node is overwritten. See the documentation for insertNode.
     */
    node_list.insert(std::pair<int32_t, Node>(new_edge.idFrom(), Node()));
    node_list.insert(std::pair<int32_t, Node>(new_edge.idTo(), Node()));
}

template<class Node, class Edge>
void Graph<Node, Edge>::insertAdjacency(const Edge& new_edge){
    std::vector<Edge>& adjacent_nodes = adjacencies[new_edge.idFrom()];

    if(adjacent_nodes.size() == 0){         //If adjacency list is empty, insert
        adjacent_nodes.push_back(new_edge); //new_node directly. Binary search
    }                                       //does not work in empty vectors.

    //Initialize binary search parameters.
    ssize_t start = 0;
    ssize_t end = adjacent_nodes.size() -1;

    while(true){
        ssize_t middle = (start + end)/2;

        if(adjacent_nodes[middle] == new_edge ){
            return; //Equivalent edge already exists, so do nothing.

        } else if ((adjacent_nodes[middle] < new_edge) && (end > start)){
            start = middle + 1; //edge is in the upper half of current interval.

        } else if ((new_edge < adjacent_nodes[middle]) && (end > start)){
            end = middle;       //edge is in the lower half of current interval.

        } else if (adjacent_nodes[middle] < new_edge){
            //No equivalent edge was found, so make an ordered insertion.
            adjacent_nodes.insert(adjacent_nodes.begin() + middle + 1, new_edge);
            return;

        } else {
            //No equivalent edge was found, so make an ordered insertion.
            adjacent_nodes.insert(adjacent_nodes.begin() + middle, new_edge);
            return;
        }
    }
}

template<class Node, class Edge>
void Graph<Node, Edge>::insertNode(const Node& new_node){
    if(new_node.idKey() == EMPTY_VALUE){
        throw std::runtime_error("Cannot insert Node with empty idConnect");
    }

    //Insert only if the node does not exist.
    auto result = node_list.insert(std::pair<int32_t, Node>(new_node.idKey(), 
                                                            new_node));

    //If the node does exist but is empty, then insert new_node anyway.
    if(result.second == false && node_list[new_node.idKey()].isEmpty()){
        node_list[new_node.idKey()] = new_node;
    }
}

template<class Node, class Edge>
int32_t Graph<Node, Edge>::getNumCicles(void){
    if(node_list.size() == 0){
        //if the graph is empty, there are no cicles.
        return 0;
    }

    int32_t cicles = 0;
    for(auto node : node_list){
        //for every node, calculate the number of cicles starting from it.
        //because of the increasing indices property, no duplicates are counted.
        cicles += getNumCicles(node.second, node.first);

        for(auto edge : adjacencies[node.first]){
            /*
             * some extra connections (those that go from the first node back to
             * it) should not count as cicles, so remove them
             */
            if(node.first < edge.idTo()){
                cicles--;
            }
        }
    }

    return cicles;
}

template<class Node, class Edge>
int32_t Graph<Node, Edge>::getNumCicles(Node& node_start, int32_t node_id){
    int32_t cicles = 0;
    for(auto edge : adjacencies[node_id]){
        //for every connection

        if(edge.idTo() == node_start.idKey()){
            //if the connection is the starting node, this is a cicle!
            //no need to consider the increase in index, this is the only case
            //that this can be ignored.
            cicles++;
            continue;
        }
        if(edge.idTo() < node_id){
            //to remove duplates, only recurse in increasing indices.
            continue;
        }

        //recurse inside the edge with the same starting node
        cicles += getNumCicles(node_start, edge.idTo());
    }

    return cicles;
}

#endif