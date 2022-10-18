
#include <fstream>
#include <unordered_map>
#include <iostream>

#include "../Headers/stats.h"
#include "../Headers/utilities.h"
#include "../Headers/bgpQuasiSync.h"

extern Stats *stats;

extern int nNodes;

const int MRAI = 1000; // election delay for BGP Quasi-sync. Acts as a MRAI

std::vector<bool> auxInCycleBGPQuasi;
std::vector<bool> visBGPQuasi;
std::vector<bool> changeAttrBGPQuasi;
std::vector<bool> firstChangeBGPQuasi;


template <class A> BGPQuasiSync<A>::BGPQuasiSync(){
    hasEvent = new bool[nNodes]{false};
    scheduledUpdate = new bool[nNodes]{false};
    lastUpdate = std::vector<int>(nNodes, -10000000);
    waitingQueue = new std::vector<Event<A>>[nNodes];

    nCycles = std::vector<int>(nNodes, 0);
    inCycle = std::vector<bool>(nNodes, false);
    cycleFormed = std::vector<int>(nNodes, 0);
    timeInCycle = std::vector<int>(nNodes, 0);
    visBGPQuasi = std::vector<bool>(nNodes, false);
    auxInCycleBGPQuasi = std::vector<bool>(nNodes, false);
    changeAttrBGPQuasi = std::vector<bool>(nNodes, false);
    firstChangeBGPQuasi = std::vector<bool>(nNodes, false);
}

template <class A> BGPQuasiSync<A>::~BGPQuasiSync(){
    delete[] hasEvent;
    delete[] scheduledUpdate;
    delete[] waitingQueue;
}

template <class A>
std::vector<Event<A>> BGPQuasiSync<A>::processUpdateMessage(NodeOneDest<A> &u, int v, int d, const A &v_attr, RoutingAlgebra<A> &Algebra, int t)
{
    std::vector<Event<A>> new_events;

    // Extension of the attribute advertised by v with the attribute of link uv
    u.aTab[v] = Algebra.Extend(u.outNeighbours[v], v_attr);

    // The attribute learnt from v is preferred to the currently elected attribute
    if(Algebra.Preferred(u.aTab.at(v), u.E)) {
        u.succ = v;
        u.Av = u.aTab.at(v);
    }
    
    // Node v was the successor of u and the attribute learnt from v is now worse than the previously elected attribute
    else if(u.succ == v && Algebra.Preferred(u.E, u.aTab.at(v))) {
        // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
        u.Av = u.O;
        u.succ = -1;
        for (const auto &[node, attribute] : u.aTab) {
            if(Algebra.Preferred(attribute, u.Av)) {
                u.Av = attribute;
                u.succ = node;
            }
        }
    } 
    else if(u.succ == v) {
        u.succ = v;
        u.Av = u.aTab.at(v);
    }
    /* else - preferred attribute did not change, do not change u.Av nor u.E nor u.succ */

    /* The Elected attribute changed */
    if (!Algebra.Equal(u.Av, u.E))
    {
        stats->stateChange(u.id, t);
        stats->attributeChange(u.id, t);
        changeAttrBGPQuasi[u.id] = true;
        firstChangeBGPQuasi[u.id] = true;

        u.E = u.Av;

        /* Send UPDATE message to all in-neighbours */
        waitingQueue[u.id].clear();

        if(Algebra.Equal(u.E, Algebra.Invalid)) 
            for (const auto &[in_neigh, cost] : u.inNeighbours) 
                new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
        else {
            if(t - lastUpdate[u.id] >= MRAI) {
                for (const auto &[in_neigh, cost] : u.inNeighbours) 
                    new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
                lastUpdate[u.id] = t;
            }
            else {
                for (const auto &[in_neigh, cost] : u.inNeighbours) 
                    waitingQueue[u.id].push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
                if(!scheduledUpdate[u.id]) {
                    scheduledUpdate[u.id] = true;
                    new_events.push_back(Event<A>(ELECT, u.id, MRAI - (t-lastUpdate[u.id]), d));
                }
            }
        }
    }

    return new_events;
}

template <class A>
std::vector<Event<A>> BGPQuasiSync<A>::processAdvertise(NodeOneDest<A> &u, int d, A origin, RoutingAlgebra<A> &Algebra, int t, bool check)
{
    std::vector<Event<A>> new_events;

    std::fill(lastUpdate.begin(), lastUpdate.end(), -10000000);

    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
        std::fill(firstChangeBGPQuasi.begin(), firstChangeBGPQuasi.end(), false);
    }

    origin = A(NEUTRAL, u.id);
    /* update the origin attribute to the advertised destination (node itself) */
    u.O = origin;

    /* compute the new most preferred attribute */
    u.Av = u.O;
    u.succ = -1;
    for (const auto &[node, attribute] : u.aTab) {
        if(Algebra.Preferred(attribute, u.Av)) {
            u.Av = attribute;
            u.succ = node;
        }
    }

    if (!Algebra.Equal(u.Av, u.E)) {
        stats->stateChange(u.id, t);
        stats->attributeChange(u.id, t);
        changeAttrBGPQuasi[u.id] = true;
        firstChangeBGPQuasi[u.id] = true;

        u.E = u.Av;

        /* Send UPDATE message to all in-neighbours */
        for (const auto &[in_neigh, cost] : u.inNeighbours)
            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
        lastUpdate[u.id] = t;
    }

    return new_events;
}

template <class A>
std::vector<Event<A>> BGPQuasiSync<A>::processWithdrawal(NodeOneDest<A> &u, int d, RoutingAlgebra<A> &Algebra, int t, bool check)
{
    std::vector<Event<A>> new_events;

    std::fill(lastUpdate.begin(), lastUpdate.end(), -10000000);

    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }

    /* update the origin attribute to the advertised destination (node itself) */
    u.O = Algebra.Invalid;

    /* compute the new most preferred attribute */
    u.Av = u.O;
    u.succ = -1;
    for (const auto &[node, attribute] : u.aTab) {
        if(Algebra.Preferred(attribute, u.Av)) {
            u.Av = attribute;
            u.succ = node;
        }
    }

    if (!Algebra.Equal(u.Av, u.E)) {
        stats->stateChange(u.id, t);
        stats->attributeChange(u.id, t);
        changeAttrBGPQuasi[u.id] = true;

        u.E = u.Av;
        /* Send UPDATE message to all in-neighbours */
        for (const auto &[in_neigh, cost] : u.inNeighbours) 
            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
        
        if(!Algebra.Equal(u.E, Algebra.Invalid))
            lastUpdate[u.id] = t; 
    }

    return new_events;
}

template <class A>
std::vector<Event<A>> BGPQuasiSync<A>::processLinkFailure(NodeOneDest<A> &u, int v, int d, RoutingAlgebra<A> &Algebra, int t, bool check) {
   
    std::vector<Event<A>> new_events;
    std::fill(lastUpdate.begin(), lastUpdate.end(), -10000000);

    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }
   
    /* Link u->v fails */

    /* Node v does nothing */

    /* Node u */
    // If node v was the successor of node u, the latter must recompute its preferred attribute
    if(u.succ == v) {
        // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
        u.Av = u.O;
        u.succ = -1;
        for (const auto &[node, attribute] : u.aTab) {
            if(Algebra.Preferred(attribute, u.Av)) {
                u.Av = attribute;
                u.succ = node;
            }
        }

        stats->stateChange(u.id, t);
        stats->attributeChange(u.id, t);
        changeAttrBGPQuasi[u.id] = true;

        u.E = u.Av;
        /* Send UPDATE message to all in-neighbours */
        for (const auto &[in_neigh, cost] : u.inNeighbours)
            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
        if(!Algebra.Equal(u.E, Algebra.Invalid))
            lastUpdate[u.id] = t; 
    }

    return new_events;
}

template <class A>
std::vector<Event<A>> BGPQuasiSync<A>::processLinkCostChange(int, NodeOneDest<A> &, int, bool){return {};}

template <class A>
std::vector<Event<A>> BGPQuasiSync<A>::processLinkAddition(int u, NodeOneDest<A> &v, RoutingAlgebra<A> &Algebra, int d, int t, bool check){
    /* link u->v */
    std::fill(lastUpdate.begin(), lastUpdate.end(), -10000000);

    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }

    std::vector<Event<A>> new_events;
    if(!Algebra.Equal(v.E, Algebra.Invalid)) {
        new_events.push_back(Event<A>(UPDATE_MSG, v.id, u, d, v.E));
        lastUpdate[v.id] = t; 
    }

    return new_events;
}

template <class A>
void BGPQuasiSync<A>::checkCycle(int u, std::vector<NodeOneDest<A>> &nodes) {
    // check for a cycle
    int node = u, cnt = 1;
    while(node != -1 && cnt++ < (int)nodes.size()) {
        node = nodes[node].succ;
        if(node == u) {
            std::cout << "A CYCLE WAS DETECTED!\n";
            std::cout << nodes[u].E << std::endl;
            break;
        }
    }
}

template <class A> 
bool BGPQuasiSync<A>::auxCheckCycle(int u, RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes) {
    if(!firstChangeBGPQuasi[u]) return false;
    const auto &node_u = nodes[u];
    if(node_u.succ == -1 && !Algebra.Equal(node_u.E, Algebra.Invalid)) return false;
    else if(Algebra.Equal(node_u.E, Algebra.Invalid)) {
        auxInCycleBGPQuasi[u] = true;
        visBGPQuasi[u] = true;
        return true;
    }
    if(auxInCycleBGPQuasi[u] || visBGPQuasi[u]) {
        auxInCycleBGPQuasi[u] = true;
        visBGPQuasi[u] = true;
        return true;
    }
    visBGPQuasi[u] = true;
    for(const auto &[v, attr] : node_u.outNeighbours) {
        if(!Algebra.Preferred(node_u.E, node_u.aTab.at(v))) {
            if(auxInCycleBGPQuasi[v] || visBGPQuasi[v] || auxCheckCycle(v, Algebra, nodes)) {
                auxInCycleBGPQuasi[u] = true;
                return true;
            }
        }
    }
    auxInCycleBGPQuasi[u] = false;
    visBGPQuasi[u] = false;
    return false;    
}

template <class A>
std::vector<Event<A>> BGPQuasiSync<A>::processEvent(RoutingAlgebra<A> &Algebra, Event<A> &event, std::vector<NodeOneDest<A>> &nodes, int t, bool check) {
    EventType type = event.type;
    auto &from = nodes[event.from];
    auto &to = nodes[event.to];
    int d = event.dest;
    A attr = event.cost;
        
    std::vector<Event<A>> ret;

    switch(type) {
        case ADVERTISE:
            ret = processAdvertise(from, d, attr, Algebra, t, check);
            break;
        case WITHDRAWAL:
            ret = processWithdrawal(from, d, Algebra, t, check);
            break;
        case UPDATE_MSG:
            ret = processUpdateMessage(to, from.id, d, attr, Algebra, t);
            break;
        case LINK_COST_CHANGE:
            ret = processLinkCostChange(from.id, to, t, check);
            break;
        case LINK_FAILURE:
            ret = processLinkFailure(from, to.id, d, Algebra, t, check);
            break;
        case LINK_ADDITION:
            ret = processLinkAddition(from.id, to, Algebra, d, t, check);
            break;
        case ELECT:
            scheduledUpdate[from.id] = false;
            if(waitingQueue[from.id].size() != 0)
                lastUpdate[from.id] = t;
            ret = waitingQueue[from.id];
        default:
            break;
    }

    if(type != ELECT) {
        if(!Algebra.Equal(to.E, Algebra.Invalid))
            to.longestPath = std::max(to.longestPath, to.E.length());
        if(!Algebra.Equal(from.E, Algebra.Invalid))
            from.longestPath = std::max(from.longestPath, from.E.length());

        if(check) {
            std::fill(auxInCycleBGPQuasi.begin(), auxInCycleBGPQuasi.end(), false);
            for(auto & n: nodes) {        
                std::fill(visBGPQuasi.begin(), visBGPQuasi.end(), false);
                if(auxCheckCycle(n.id, Algebra, nodes)) {
                    if(changeAttrBGPQuasi[n.id]) {
                        ++nCycles[n.id];
                        changeAttrBGPQuasi[n.id] = false;
                    }
                    if(!inCycle[n.id]) {
                        inCycle[n.id] = true;
                        cycleFormed[n.id] = t;
                    }
                }
                else {
                    if(inCycle[n.id]) {
                        timeInCycle[n.id] += (t - cycleFormed[n.id]);
                        inCycle[n.id] = false;
                    }
                }  
            }
        }
    }

    return ret;
}

template <class A>
bool BGPQuasiSync<A>::assertCorrectness(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra, EventType event, int d, bool print) {
        
    bool ok = true;
    if(event == ADVERTISE) {
        std::vector<A> EvTeo = EvDijkstra<A>(nodes, d, A{NEUTRAL,d}, false);
        for(const auto &n : nodes) {
            if(n.E != n.Av || n.E != EvTeo[n.id]){
                ok = false;
                if(print) {
                    std::cout << "Correction assertion failed at ";
                    std::cout << "Node: " << n.id << "\n";
                    std::cout << "E: " << n.E << "A: " << n.Av << " E_exp: " << EvTeo[n.id] << "\n";
                    std::cout << "state: " << n.state << "\n";
                }
                break;
            }
        }
    }
    else {
        for(const auto &n : nodes) {
            if(n.E != n.Av || n.E != Algebra.Invalid){
                ok = false;
                if(print) {
                    std::cout << "Correction assertion failed at ";
                    std::cout << "Node: " << n.id << "\n";
                    std::cout << "E: " << n.E << "A: " << n.Av << " E_exp: " << Algebra.Invalid << "\n";
                    std::cout << "state: " << n.state << "\n";
                }
                break;
            }
        }
    }
    return ok;
}

template <class A>
int BGPQuasiSync<A>::getPathLength(std::vector<NodeOneDest<A>> &nodes, int u, RoutingAlgebra<A> &Algebra, int d) {
    return nodes[u].longestPath;
}

template<class A> 
std::vector<int>& BGPQuasiSync<A>::getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u) 
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid)) 
            timeInCycle[u] = -1;

    return timeInCycle;
}

template<class A> 
std::vector<int>& BGPQuasiSync<A>::getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u) 
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid)) 
            nCycles[u] = -1;

    return nCycles;
}


#include "../Instances/bgpQuasiSyncInstance.h"
