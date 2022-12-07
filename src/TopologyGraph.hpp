#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <cinttypes>
#include <string>
#include <ostream>

#include "Graph.hxx"

extern "C" {
#include "entries.h"
}

#define ACRONYM_SIZE 2
#define CONVERSION_FACTOR 1024


class NodeExtraData {
    friend std::ostream& operator<<(std::ostream& os, NodeExtraData& node);

    private:
    char countryAcronym[ACRONYM_SIZE];
    std::string POPsName;
    std::string originCountryName;

    public:
    NodeExtraData(entry* es);
    NodeExtraData(){};
};

std::ostream& operator<<(std::ostream& os, NodeExtraData& node);


class EdgeExtraData {
    private:
    double connectionSpeed;

    public:
    EdgeExtraData(entry* es);
    EdgeExtraData(){};
};


#endif
