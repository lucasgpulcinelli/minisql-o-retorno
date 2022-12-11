#include <iostream>
#include <cinttypes>

#include "Graph.hpp"

int32_t Node::idKey() const{
    return id;
}

bool Node::isEmpty() const{
    return is_empty;
}

bool operator<(const Node& left_arg, const Node& right_arg){
    return left_arg.idKey() < right_arg.idKey();
}

bool operator==(const Node& left_arg, const Node& right_arg){
    return left_arg.idKey() == right_arg.idKey();
}

std::ostream& operator<<(std::ostream& os, const Node& node){
    return os << node.idKey();
}

Node::Node(){
    is_empty = true;
    id = EMPTY_VALUE;
}

Node::Node(int32_t id){
    is_empty = false;
    this->id = id; 
}

int32_t Edge::idFrom() const{
    return id_from;
}

int32_t Edge::idTo() const{
    return id_to;
}

bool operator==(const Edge& left_arg, const Edge& right_arg){
    return left_arg.idFrom() == right_arg.idFrom() && 
        left_arg.idTo() == right_arg.idTo();
}

bool operator<(const Edge& left_arg, const Edge& right_arg){
    return left_arg.idTo() < right_arg.idTo();
}

Edge Edge::getDual() const{
    Edge dual = *this;

    int32_t tmp = dual.id_from;
    dual.id_from = dual.id_to;
    dual.id_to = tmp;

    return dual;
}

Edge Edge::operator=(const Edge& right_arg){
    this->id_from = right_arg.id_from;
    this->id_to = right_arg.id_to;

    return *this;
}

Edge::Edge(const Edge& edge){
    this->id_from = edge.id_from;
    this->id_to = edge.id_to;
}

Edge::Edge(int32_t id_from, int32_t id_to){
    this->id_from = id_from;
    this->id_to = id_to;
}

Edge::Edge(){}