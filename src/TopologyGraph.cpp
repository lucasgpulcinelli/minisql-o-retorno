#include <string>
#include <ostream>
#include <stdexcept>

#include "TopologyGraph.hpp"

extern "C" {
#include "entries.h"
}

std::ostream& operator<<(std::ostream& os, NodeExtraData& node){
    return os << node.POPsName << " " << node.originCountryName << 
    " " << node.countryAcronym[0] << node.countryAcronym[1];
}

NodeExtraData::NodeExtraData(entry* es) {
    POPsName = std::string(GET_POPSNAME(es));
    originCountryName = std::string(GET_COUNTRYNAME(es));

    for(size_t index = 0; index < ACRONYM_SIZE; index++){
        countryAcronym[index] = GET_COUNTRYACRO(es)[index];
    }
}


EdgeExtraData::EdgeExtraData(entry* es){
    int connected_id = GET_CONNPOPSID(es);
    if(connected_id == -1){
        connectionSpeed = 0;
        return;
    }

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
