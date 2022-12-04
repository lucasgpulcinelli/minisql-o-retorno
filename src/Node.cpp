#include <string>
#include <ostream>
#include "Node.hpp"

extern "C" {
#include "entries.h"
}

std::ostream& operator<<(const Node& node, std::ostream& output){
    output << node.idConnection << " ";
    output << node.POPsName << " " << node.originCountryName << " ";
    output << node.countryAcronym;
    return output;
}

bool operator<(const Node& left_arg, const Node& right_arg){
    return left_arg.idConnection < right_arg.idConnection;
}

bool operator==(const Node& left_arg, const Node& right_arg){
    return left_arg.idConnection == right_arg.idConnection;
}

Node::Node(entry* es){
    idConnection = GET_IDCONNECT(es);
    POPsName = std::string(GET_POPSNAME(es));
    originCountryName = std::string(GET_COUNTRYNAME(es));

    for(size_t index = 0; index < ACRONYM_SIZE; index++){
        countryAcronym[0] = GET_COUNTRYACRO(es)[index];
    }
}

Edge::Edge(entry* es){
    connectionPOPId = GET_CONNPOPSID(es);

    switch(GET_MEASUREMENT_UNIT(es)[0]){
        case 'K':
        connectionSpeed = GET_SPEED(es)/(CONVERSION_FACTOR*CONVERSION_FACTOR);
        break;

        case 'M':
        connectionSpeed = GET_SPEED(es)/CONVERSION_FACTOR;
        break;

        case 'G':
        connectionSpeed = GET_SPEED(es);
        break;

        default:
        throw std::runtime_error("Invalid measurement unit");
    }
}

std::ostream& operator<<(const Edge& edge, std::ostream& output){
    output << edge.connectionPOPId << " " << edge.connectionSpeed;
    output << "Gbps";
    return output;
}

bool operator<(const Edge& left_arg, const Edge& right_arg){
    return left_arg.connectionPOPId < right_arg.connectionPOPId;
}

bool operator==(const Edge& left_arg, const Edge& right_arg){
    return left_arg.connectionPOPId == right_arg.connectionPOPId;
}