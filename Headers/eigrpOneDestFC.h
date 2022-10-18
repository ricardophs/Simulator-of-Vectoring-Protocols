#ifndef EIGRPONEDESTFC_H
#define EIGRPONEDESTFC_H

#include "enums.h"
#include "protocol.h"
#include "event.h"
#include "algebra.h"
#include "nodeOneDest.h"

template<class A> class EIGRPOneDestFC : public Protocol<A> {
public:
    EIGRPOneDestFC();
    ~EIGRPOneDestFC(){}
    std::vector<Event<A>> processEvent(RoutingAlgebra<A> &, Event<A> &, std::vector<NodeOneDest<A>> &, int, bool=false);
    bool assertCorrectness(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &, EventType, int, bool=true);
    int getPathLength(std::vector<NodeOneDest<A>> &, int, RoutingAlgebra<A> &, int);
    std::vector<int>& getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &);
    std::vector<int>& getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &);

private:
    std::vector<bool> inBlackHole;
    std::vector<int> blackHoleFormed;
    std::vector<int> timeInBlackHole;
    std::vector<int> nBlackHoles;

    std::vector<Event<A>> processUpdateMessage(NodeOneDest<A> &, int, int, const A &, RoutingAlgebra<A> &, int);

    std::vector<Event<A>> processDiffusingMessage(NodeOneDest<A> &, int, int, const A &, RoutingAlgebra<A> &, int);

    std::vector<Event<A>> processClearMessage(NodeOneDest<A> &, int, int, RoutingAlgebra<A> &, int);

    std::vector<Event<A>> processAdvertise(NodeOneDest<A> &, int, A, RoutingAlgebra<A> &, int, bool=false);

    std::vector<Event<A>> processWithdrawal(NodeOneDest<A> &, int, RoutingAlgebra<A> &, int, bool=false);

    std::vector<Event<A>> processLinkCostChange(int, NodeOneDest<A> &, RoutingAlgebra<A> &, int, bool=false);

    std::vector<Event<A>> processLinkFailure(NodeOneDest<A> &, NodeOneDest<A> &, RoutingAlgebra<A> &, int, int, bool=false);

    std::vector<Event<A>> processLinkAddition(int, NodeOneDest<A> &, RoutingAlgebra<A> &, int, bool=false);
    
    void checkSafety(const NodeOneDest<A> &, RoutingAlgebra<A> &, std::vector<NodeOneDest<A>> &);
    void checkCycle(int, std::vector<NodeOneDest<A>> &);
    bool checkBlackHole(int, RoutingAlgebra<A> &, std::vector<NodeOneDest<A>> &);

};

#endif