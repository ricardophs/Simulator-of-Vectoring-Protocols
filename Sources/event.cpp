#include <iostream>

#include "../Headers/event.h"

template <class A> Event<A>::Event() {}

template <class A> Event<A>::Event(EventType type, int from, int to_dest) {
    this->type = type;
    this->from = from;
    this->to = to_dest;
    this->dest = to_dest;    
}

template <class A> Event<A>::Event(EventType type, int from, int to, int dest) {
    this->type = type;
    this->from = from;
    this->to = to;
    this->dest = dest;
}

template <class A> Event<A>::Event(EventType type, int from, int to_dest, A cost) {
    this->type = type;
    this->from = from;
    this->to = to_dest;
    this->dest = to_dest;
    this->cost = cost;
}

template <class A> Event<A>::Event(EventType type, int from, int to, int dest, A cost) {
    this->type = type;
    this->from = from;
    this->to = to;
    this->dest = dest;
    this->cost = cost;
}

#include "../Instances/eventInstances.h"


