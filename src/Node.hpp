#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <cinttypes>
#include <string>
#include <ostream>

extern "C" {
#include "entries.h"
}

#define ACRONYM_SIZE 2
#define CONVERSION_FACTOR 1024


class Node {
    friend std::ostream& operator<<(const Node& node, std::ostream& output);
    friend bool operator<(const Node& left_arg, const Node& right_arg);
    friend bool operator==(const Node& left_arg, const Node& right_arg);

    private:

    int32_t idConnection;
    char countryAcronym[ACRONYM_SIZE];
    std::string POPsName;
    std::string originCountryName;

    public:

    Node(entry* es);
};

std::ostream& operator<<(const Node& node, std::ostream& output);
bool operator<(const Node& left_arg, const Node& right_arg);
bool operator==(const Node& left_arg, const Node& right_arg);

class Edge {
    public:

    int32_t connectionPOPId;
    double connectionSpeed;

    Edge(entry* es);
};

std::ostream& operator<<(const Edge& edge, std::ostream& output);
bool operator<(const Edge& left_arg, const Edge& right_arg);
bool operator==(const Edge& left_arg, const Edge& right_arg);

#endif