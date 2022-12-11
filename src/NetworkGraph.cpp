#include <string>
#include <ostream>
#include <stdexcept>

#include "NetworkGraph.hpp"
#include "Graph.hxx"

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
