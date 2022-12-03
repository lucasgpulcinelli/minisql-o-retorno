#include <string>
#include <ostream>
#include "Node.hpp"

extern "C" {
#include "entries.h"
}

std::ostream& operator<<(const Node& node, std::ostream& output){
    output << node.idConnect << " ";
    output << node.POPsName << " " << node.originCountryName << " ";
    output << node.countryAcronym;
    return output;
}

Node::Node(entry* es){
    idConnect = GET_IDCONNECT(es);
    POPsName = std::string(GET_POPSNAME(es));
    originCountryName = std::string(GET_COUNTRYNAME(es));

    for(size_t index = 0; index < ACRONYM_SIZE; index++){
        countryAcronym[0] = GET_COUNTRYACRO(es)[index];
    }
}