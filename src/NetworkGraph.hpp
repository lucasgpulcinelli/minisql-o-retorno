#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <cinttypes>
#include <string>
#include <ostream>

#include "Graph.hxx"
#include "table.hpp"

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
    friend std::ostream& operator<<(std::ostream& os, const Connection& conn);
    
    private:
    double connectionSpeed;

    public:
    Connection getDual();
    
    Connection(entry* es);
    Connection();
};

std::ostream& operator<<(std::ostream& os, const Connection& conn);

class NetworkGraph : Graph<NetworkNode, Connection> {
    friend std::ostream& operator<<(std::ostream& os, 
                                    const NetworkGraph& graph);

    public:
    NetworkGraph(const Table& table);
};

std::ostream& operator<<(std::ostream& os, const NetworkGraph& graph);

#endif
