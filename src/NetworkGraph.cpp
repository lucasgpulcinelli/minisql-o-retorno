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
    POPsName = std::string(GET_POPSNAME(es));
    originCountryName = std::string(GET_COUNTRYNAME(es));

    for(size_t index = 0; index < ACRONYM_SIZE; index++){
        countryAcronym[index] = GET_COUNTRYACRO(es)[index];
    }
}

std::ostream& operator<<(std::ostream& os, const Connection& conn){
    return os << conn.idTo() << " " << conn.connectionSpeed << "Mpbs";
}

Connection::Connection(entry* es) : Edge(GET_IDCONNECT(es), GET_CONNPOPSID(es)) {
    if(idTo() == EMPTY_VALUE){
        connectionSpeed = EMPTY_VALUE;
        return;
    }

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

        NetworkNode new_pop = NetworkNode(es);
        Connection new_connection = Connection(es);

        Graph::insertNode(new_pop);
        Graph::insertEdge(new_connection);

        free(es);
    }
}

std::ostream& operator<<(std::ostream& os, const NetworkGraph& graph){
    for(auto node_it = graph.node_list.begin(); node_it != graph.node_list.end(); 
        node_it++){
        
        if(node_it->second.isEmpty()){
            continue;
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
