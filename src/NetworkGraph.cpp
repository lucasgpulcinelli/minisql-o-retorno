#include <string>
#include <ostream>
#include <stdexcept>

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
    return os << conn.idTo() << " " << conn.connectionSpeed << "Mbps";
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

NetworkGraph::NetworkGraph(const Table& table){
    while(table.hasNextEntry()){
        entry* es = table.readNextEntry();

        try{
            NetworkNode new_pop = NetworkNode(es);
            Graph::insertNode(new_pop);
        } catch(std::runtime_error& except){
            continue; //Do not insert empty nodes and edges.
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
            os << (*node_it).second << " " << *edge_it << "\n";
        }
    }

    return os;
}
