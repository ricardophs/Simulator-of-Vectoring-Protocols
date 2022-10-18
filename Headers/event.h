#ifndef EVENT_H
#define EVENT_H

#include "enums.h"
#include <ostream>

template <class A> class Event {
public:
    EventType type;
    int from;
    int to;
    int dest;
    A cost;

    Event();

    Event(EventType, int, int);

    Event(EventType, int, int, int);

    Event(EventType, int, int, A);

    Event(EventType, int, int, int, A);

    friend std::ostream & operator<< (std::ostream & os, const Event & ev) {	
        os << "[";
        os << ev.type << ", ";
        os << ev.from << ", ";
        os << ev.to << ", ";
        os << ev.dest << ", ";
        os << ev.cost;
        os << "]";
        return os; 
    }

};

#endif