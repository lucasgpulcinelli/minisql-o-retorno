#ifndef __GRAPH_HXX__
#define __GRAPH_HXX__

#include <iostream>
#include <cinttypes>
#include <queue>

#include "Graph.hpp"

template<class Node, class Edge>
std::ostream& operator<<(std::ostream& os, const Graph<Node, Edge>& graph){
    for(auto node_it = graph.node_list.begin(); node_it != graph.node_list.end(); 
        node_it++){
        
        if(node_it->second.isEmpty()){
            continue;
        }

        std::vector<Edge> connections;

        try{
            connections = graph.adjacencies.at(node_it->second.idKey());
        } catch(std::out_of_range& e){
            continue;
        }

        for(auto edge_it = connections.begin(); edge_it != connections.end();
            edge_it++){
            
            os << (*node_it).second << " " << edge_it->idTo() << " " << edge_it->c_speed << std::endl;
        }
    }

    return os;
}

template<class Node, class Edge>
void Graph<Node, Edge>::insertEdge(const Edge& new_edge){
    if(new_edge.idTo() == EMPTY_VALUE){
        return;
    }

    insertAdjacency(new_edge, new_edge.idFrom());

    Edge new_edge_dual = new_edge.getDual();
    insertAdjacency(new_edge_dual, new_edge.idTo());

    node_list.insert(std::pair<int32_t, Node>(new_edge.idFrom(), Node()));
    node_list.insert(std::pair<int32_t, Node>(new_edge.idTo(), Node()));
}

template<class Node, class Edge>
void Graph<Node, Edge>::insertAdjacency(const Edge& new_edge, int32_t node){
    std::vector<Edge>& adjacent_nodes = adjacencies[node];

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

template<class Node, class Edge>
int32_t Graph<Node, Edge>::getMaxSpeed(int32_t node_a_id, int32_t node_b_id){
    try{
        adjacencies.at(node_a_id);
        adjacencies.at(node_b_id);
    }catch(std::out_of_range& e){
        return -1;
    }

    std::map<Edge, int32_t> flow_used{};
    for(auto node : node_list){
        for(auto edge : adjacencies[node.first]){
            flow_used[edge] = 0;
        }
    }

    int32_t speed = 0;

    while(true){
        std::queue<int32_t> q{};
        q.push(node_a_id);
        std::map<int32_t, Edge> path{};
        while(!q.empty()){
            int32_t node_curr = q.front();
            q.pop();
            for(auto edge : adjacencies[node_curr]){
                if(path[edge.idTo()].idFrom() == -1 && edge.idTo() != node_a_id && edge.c_speed > flow_used[edge]){
                    path[edge.idTo()] = edge;
                    q.push(edge.idTo());
                }
            }
        }

        if(path[node_b_id].idFrom() == -1){
            break;
        }

        int32_t speed_add = INT32_MAX;
        for(auto edge = path[node_b_id]; edge.idFrom() != -1; edge = path[edge.idFrom()]){
            speed_add = std::min(speed_add, edge.c_speed - flow_used[edge]);
        }
        for(auto edge = path[node_b_id]; edge.idFrom() != -1; edge = path[edge.idFrom()]){
            flow_used[edge] += speed_add;
            flow_used[edge.getDual()] += speed_add;
        }

        speed += speed_add;
    }

    return speed;
}

template<class Node, class Edge>
int32_t Graph<Node, Edge>::getLen(std::map<int32_t, bool>& marks,
                                  int32_t node_start_id, int32_t node_end_id, 
                                  int32_t max_plausable_len, 
                                  int32_t min_possible_len){

    if(min_possible_len >= max_plausable_len){
        /*
         * if we already have a solution (max_plausable_len) that has a smaller
         * distance than what we can ever hope to achieve in this recursion 
         * (min_possible_len), we can just ignore this recursion: even if we get
         * a solution it will be worst than what we already have
         */
        return max_plausable_len;
    }

    //for all connections in our starting node
    for(auto connection : adjacencies[node_start_id]){
        //if the connected node is marked, return: we have already gone there
        if(marks[connection.idTo()]){
            continue;
        }
        //if the connected node is our destiny
        if(connection.idTo() == node_end_id){
            //get the better solution: what we already have or the new solution
            max_plausable_len = std::min(max_plausable_len, 
                connection.c_speed + min_possible_len);
            continue;
        }

        //if the connected node is not marked and not the ending node
        //mark it, we will go there now
        marks[connection.idTo()] = true;

        //recusion: start at the connected node with a new min_possible_len
        //to include the distance from the current node to the connected one
        max_plausable_len = getLen(marks, connection.idTo(), node_end_id,
            max_plausable_len, min_possible_len + connection.c_speed);

        //unmark the node: other paths might use it
        marks[connection.idTo()] = false;
    }
    
    //return our best solution yet
    return max_plausable_len;
}

template<class Node, class Edge>
int32_t Graph<Node, Edge>::getLen(int32_t node_a_id, int32_t node_b_id){
    try{
        //if the origin or destination nodes do not exist, no path exists
        adjacencies.at(node_a_id);
        adjacencies.at(node_b_id);
    }catch(std::out_of_range& e){
        return -1;
    }

    //all marked nodes, keeps track of recursion to prohibit infinite loops
    std::map<int32_t, bool> marks{};

    for(auto node : node_list){
        //initialize all nodes as unmarked
        marks[node.first] = false;
    }

    //initialize the recursive version, starting with the first node
    marks[node_a_id] = true;
    int32_t ret = getLen(marks, node_a_id, node_b_id, INT32_MAX, 0);
    marks[node_a_id] = false;

    if(ret == INT32_MAX){
        return -1;
    }

    return ret;
}

#endif