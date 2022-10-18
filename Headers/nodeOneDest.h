#ifndef NODEONEDEST_H
#define NODEONEDEST_H

#include <unordered_map>
#include <iostream>

#include "enums.h"

template <class A> class NodeOneDest {
public:
    int id;
    int V;
    int longestPath;
    std::unordered_map<int, A> inNeighbours;
    std::unordered_map<int, A> outNeighbours;
    State state;
    A E; // elected attribute
    A B; // elected attribute the last time the node was in update state
    A Av; // most preferred attribute
    A O; // origin attribute
    std::unordered_map<int, A> aTab;
    std::unordered_map<int, A> rTab;
    std::unordered_map<int, bool> clear;
    std::unordered_map<int, bool> diffusingMsg;
    std::unordered_map<int, bool> sendUpdate;
    int succ;
    bool sendClear;

    NodeOneDest(int, int);

    void addInLink(int, A);

    void addOutLink(int, A);

    void removeOutLink(int);

    void removeInLink(int);

    void resetVariables();

    friend std::ostream & operator<< (std::ostream & os, const NodeOneDest & n) {	
        os << "Node " << n.id << ":\n\t";
        os << "in-neighbours: ";
        for(const auto &item : n.inNeighbours)
            os << "( " << item.first << "," << item.second << ") ";
        os << "\n\tout-neighbours: ";
        for(const auto &item : n.outNeighbours)
            os << "( " << item.first << "," << item.second << ") ";
        os << "\n\tO: " << n.O;
        os << "\n\tA: " << n.Av;
        os << "\n\tE: " << n.E;
        os << "\n\tB: " << n.B;
        os << "\n\tsucc: " << n.succ;
        os << "\n\tsendClear: " << n.sendClear;
        os << "\n\tstate: " << (n.state == UPDATE ? "UPDATE" : "DIFFUSING");
        os << "\n\tclear:\n\t\t";
        for(const auto &b : n.inNeighbours) 
            os << "(" << b.first << ": " << n.clear.at(b.first) << ") ";
        os << "\n\taTab:\n\t\t";  
        for(const auto &t : n.outNeighbours) 
            os << "(" << t.first << ": " << n.aTab.at(t.first) << ") ";
        for(const auto &t : n.outNeighbours) 
            os << "(" << t.first << ": " << n.rTab.at(t.first) << ") ";
        os << "\n\tdiffusingMsg:\n\t\t";  
        for(const auto &t : n.outNeighbours) 
            os << "(" << t.first << ": " << n.diffusingMsg.at(t.first) << ") ";
        os << std::endl;
        return os; 
    }
    
};

#endif