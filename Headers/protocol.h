#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <vector>

#include "enums.h"
#include "event.h"
#include "algebra.h"
#include "nodeOneDest.h"

template<class A> class Protocol {
public:
    Protocol() {}
    virtual ~Protocol(){}
    virtual std::vector<Event<A>> processEvent(RoutingAlgebra<A> &, Event<A> &, std::vector<NodeOneDest<A>> &, int=-1, bool=false) = 0;
    virtual bool assertCorrectness(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &, EventType, int, bool=false) = 0;
    virtual std::vector<int> &getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &) = 0;
    virtual std::vector<int> &getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &) = 0;
};

#endif