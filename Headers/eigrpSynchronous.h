#ifndef EIGRPSYNCHRONOUS_H
#define EIGRPSYNCHRONOUS_H

#include <vector>

#include "protocol.h"
#include "enums.h"
#include "nodeOneDest.h"
#include "algebra.h"
#include "event.h"


template<class A> class EIGRPSync : public Protocol<A> {
public:
    EIGRPSync();
    ~EIGRPSync();

    std::vector<Event<A>> SynchronousIteration(RoutingAlgebra<A> &, std::vector<NodeOneDest<A>> &, int, int, bool=false);
    std::vector<Event<A>> processEvent(RoutingAlgebra<A> &, Event<A> &, std::vector<NodeOneDest<A>> &, int=-1, bool=false);
    bool assertCorrectness(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &, EventType, int, bool=true);
    int getPathLength(std::vector<NodeOneDest<A>> &, int, RoutingAlgebra<A> &, int);
    std::vector<int>& getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &);
    std::vector<int> &getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &, RoutingAlgebra<A> &);

private:
    bool* hasEvent;
    std::vector<bool> inBlackHole;
    std::vector<int> blackHoleFormed;
    std::vector<int> timeInBlackHole;
    std::vector<int> nBlackHoles;
    
    void processUpdateMessage(NodeOneDest<A> &, int, const A &, RoutingAlgebra<A> &);
    void processDiffusingMessage(NodeOneDest<A> &, int, const A &, RoutingAlgebra<A> &);
    void processClearMessage(int, NodeOneDest<A> &);
    void processAdvertise(NodeOneDest<A> &, int, A, bool=false);
    void processWithdrawal(NodeOneDest<A> &, int, RoutingAlgebra<A> &, bool=false);
    void processLinkCostChange(int, NodeOneDest<A> &, bool=false);
    void processLinkAddition(int, NodeOneDest<A> &, bool=false);
    void processLinkFailure(NodeOneDest<A> &, int, RoutingAlgebra<A> &, bool=false);
    
    void checkSafety(const NodeOneDest<A> &, RoutingAlgebra<A> &, std::vector<NodeOneDest<A>> &);
    void checkCycle(int, std::vector<NodeOneDest<A>> &);
    bool checkBlackHole(int, RoutingAlgebra<A> &, std::vector<NodeOneDest<A>> &);


};

#endif