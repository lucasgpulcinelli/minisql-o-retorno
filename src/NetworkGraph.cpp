#include <string>
#include <ostream>
#include <stdexcept>
#include <limits>

#include "NetworkGraph.hpp"
#include "Graph.hxx"
#include "table.hpp"

extern "C" {
#include "entries.h"
}

std::ostream& operator<<(std::ostream& os, const NetworkNode& node){
    return os << node.idKey() << " " << node.POPsName << " " << 
           node.originCountryName << " " << 
           node.countryAcronym[0] << node.countryAcronym[1];
}

NetworkNode::NetworkNode(entry* es) : Node(GET_IDCONNECT(es)) {
    if(GET_IDCONNECT(es) == EMPTY_VALUE){
        throw std::runtime_error("Cannot construct NetworkNode from empty entry");
    }
    
    POPsName = std::string(GET_POPSNAME(es));
    originCountryName = std::string(GET_COUNTRYNAME(es));

    for(size_t index = 0; index < ACRONYM_SIZE; index++){
        countryAcronym[index] = GET_COUNTRYACRO(es)[index];
    }
}

std::ostream& operator<<(std::ostream& os, const Connection& conn){
    return os << conn.idTo() << " " << conn.connectionSpeed << "Mpbs";
}

double Connection::getSpeed(){
    return connectionSpeed;
}

Connection::Connection(entry* es) : Edge(GET_IDCONNECT(es), GET_CONNPOPSID(es)){
    if(idFrom() == EMPTY_VALUE){
        throw std::runtime_error("Cannot construct connection with empty "
                                 "idConnect. It must be a valid address.");
    } else if(idTo() == EMPTY_VALUE){
        throw std::runtime_error("Cannot construct connnection with empty "
                                 "Connected POP's ID. It must be a valid address.");
    }

    //Convert speed units to megabytes per second (Mbps).
    switch(GET_MEASUREMENT_UNIT(es)[0]){
        case 'K':
        case 'k':
        connectionSpeed = GET_SPEED(es)/(CONVERSION_FACTOR);
        break;

        case 'M':
        case 'm':
        connectionSpeed = GET_SPEED(es);
        break;

        case 'G':
        case 'g':
        connectionSpeed = GET_SPEED(es)*CONVERSION_FACTOR;
        break;

        default:
        throw std::runtime_error("Invalid measurement unit");
    }
}

Connection::Connection() : Edge(){
    connectionSpeed = -1;
}

NetworkGraph::NetworkGraph(const Table& table){
    while(table.hasNextEntry()){
        entry* es = table.readNextEntry();

        try{
            NetworkNode new_pop = NetworkNode(es);
            Graph::insertNode(new_pop);
        } catch(std::runtime_error& except){
            continue;
        }

        try{
            Connection new_connection = Connection(es);
            Graph::insertEdge(new_connection);
        } catch(std::runtime_error& except){}
        free(es);
    }
}

std::ostream& operator<<(std::ostream& os, const NetworkGraph& graph){
    for(auto node_it = graph.node_list.begin(); node_it != graph.node_list.end(); 
        node_it++){
        
        if(node_it->second.isEmpty()){
            continue; //do not print empty NetworkNodes.
        }

        std::vector<Connection> connections;

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

double NetworkGraph::getMaxSpeed(int32_t node_a_id, int32_t node_b_id){
    try{
        //if the origin or destination nodes do not exist, or do not have any
        //connections, no path exists
        if(adjacencies.at(node_a_id).size() == 0 || 
           adjacencies.at(node_b_id).size() == 0){
            
            return -1;
        }
    }catch(std::out_of_range& e){
        return -1;
    }

    std::map<Edge, int32_t> flow_used{};
    for(auto node : node_list){
        for(auto edge : adjacencies[node.first]){
            flow_used[edge] = 0;
        }
    }

    double speed = 0;

    while(true){
        std::queue<int32_t> q{};
        q.push(node_a_id);
        std::map<int32_t, Connection> path{};
        while(!q.empty()){
            int32_t node_curr = q.front();
            q.pop();
            for(auto edge : adjacencies[node_curr]){
                if(path[edge.idTo()].idFrom() == -1 && 
                    edge.idTo() != node_a_id && 
                    edge.getSpeed() > flow_used[edge]){
                    
                    path[edge.idTo()] = edge;
                    q.push(edge.idTo());
                }
            }
        }

        if(path[node_b_id].idFrom() == -1){
            break;
        }

        double speed_add = std::numeric_limits<double>::infinity();
        for(auto edge = path[node_b_id]; edge.idFrom() != -1; 
            edge = path[edge.idFrom()]){

            speed_add = std::min(speed_add, edge.getSpeed() - flow_used[edge]);
        }
        for(auto edge = path[node_b_id]; edge.idFrom() != -1;
            edge = path[edge.idFrom()]){

            flow_used[edge] += speed_add;
            edge.reverse();
            flow_used[edge] += speed_add;
            edge.reverse();
        }

        speed += speed_add;
    }

    return speed;
}

double NetworkGraph::getLen(std::map<int32_t, bool>& marks,
                             int32_t node_start_id, int32_t node_end_id, 
                             double max_plausable_len, 
                             double min_possible_len){

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
                connection.getSpeed() + min_possible_len);
            continue;
        }

        //if the connected node is not marked and not the ending node
        //mark it, we will go there now
        marks[connection.idTo()] = true;

        //recusion: start at the connected node with a new min_possible_len
        //to include the distance from the current node to the connected one
        max_plausable_len = getLen(marks, connection.idTo(), node_end_id,
            max_plausable_len, min_possible_len + connection.getSpeed());

        //unmark the node: other paths might use it
        marks[connection.idTo()] = false;
    }
    
    //return our best solution yet
    return max_plausable_len;
}

double NetworkGraph::getLen(int32_t node_a_id, int32_t node_b_id){
    try{
        //if the origin or destination nodes do not exist, or do not have any
        //connections, no path exists
        if(adjacencies.at(node_a_id).size() == 0 || 
           adjacencies.at(node_b_id).size() == 0){
            
            return -1;
        }
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
    double ret = getLen(marks, node_a_id, node_b_id, 
        std::numeric_limits<double>::infinity(), 0);
    marks[node_a_id] = false;

    if(ret == std::numeric_limits<double>::infinity()){
        return -1;
    }

    return ret;
}
