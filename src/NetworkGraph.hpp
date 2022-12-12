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


class NetworkNode : public Node {
    friend std::ostream& operator<<(std::ostream& os, const NetworkNode& node);

    private:
    char countryAcronym[ACRONYM_SIZE];
    std::string POPsName;
    std::string originCountryName;

    public:
    NetworkNode(entry* es);
    NetworkNode(){};
};

std::ostream& operator<<(std::ostream& os, const NetworkNode& node);


class Connection : public Edge {
    private:
    double connectionSpeed;

    public:
    Connection(entry* es);
    Connection();
};


#endif
