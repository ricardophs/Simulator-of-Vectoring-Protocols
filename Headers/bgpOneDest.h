#ifndef BGPONEDEST_H
#define BGPONEDEST_H

#include "enums.h"
#include "protocol.h"
#include "nodeOneDest.h"
#include "algebra.h"
#include "event.h"


template<class A> class BGPOneDest : public Protocol<A> {
public:
    BGPOneDest();
    ~BGPOneDest(){}
    
    std::vector<Event<A>> processEvent(RoutingAlgebra<A> &, Event<A> &, std::vector<NodeOneDest<A>> &, int, bool=false);

    bool assertCorrectness(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &, EventType, int, bool=true);

    int getPathLength(std::vector<NodeOneDest<A>> &, int, RoutingAlgebra<A> &, int);

    std::vector<int>& getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &);

    std::vector<int>& getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &);

private:
    std::vector<int> nCycles;
    std::vector<bool> inCycle;
    std::vector<int> cycleFormed;
    std::vector<int> timeInCycle;

    std::vector<Event<A>> processUpdateMessage(NodeOneDest<A> &, int, int, const A &, RoutingAlgebra<A> &, int);

    std::vector<Event<A>> processAdvertise(NodeOneDest<A> &, int, A, RoutingAlgebra<A> &, int, bool=false);

    std::vector<Event<A>> processWithdrawal(NodeOneDest<A> &, int, RoutingAlgebra<A> &, int, bool=false);

    std::vector<Event<A>> processLinkCostChange(int, NodeOneDest<A> &, RoutingAlgebra<A> &, int, int, bool=false);

    std::vector<Event<A>> processLinkFailure(NodeOneDest<A> &, int, RoutingAlgebra<A> &, int, int, bool=false);

    std::vector<Event<A>> processLinkAddition(int, NodeOneDest<A> &, RoutingAlgebra<A> &, int, int, bool=false);

    bool auxCheckCycle(int, RoutingAlgebra<A> &, std::vector<NodeOneDest<A>> &);

};

#endif