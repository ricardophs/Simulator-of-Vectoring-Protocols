#include <fstream>

#include "../Headers/utilities.h"
#include "../Headers/stats.h"
#include "../Headers/vectoringProtocol.h"

// Object where all the statistics are saved
extern Stats *stats;

extern int nNodes;

std::vector<bool> auxInCycleVP;
std::vector<bool> visVP;
std::vector<bool> changeAttrVP;
std::vector<bool> firstChangeVP;

template <class A>
VectoringProtocol<A>::VectoringProtocol() {
    nCycles = std::vector<int>(nNodes, 0);
    inCycle = std::vector<bool>(nNodes, false);
    cycleFormed = std::vector<int>(nNodes, 0);
    timeInCycle = std::vector<int>(nNodes, 0);
    visVP = std::vector<bool>(nNodes, false);
    auxInCycleVP = std::vector<bool>(nNodes, false);
    changeAttrVP = std::vector<bool>(nNodes, false);
    firstChangeVP = std::vector<bool>(nNodes, false);
}

template <class A> 
bool VectoringProtocol<A>::auxCheckCycle(int u, RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes) {
    if(!firstChangeVP[u]) return false; 
    const auto &node_u = nodes[u];
    if(node_u.succ == -1 && !Algebra.Equal(node_u.E, Algebra.Invalid)) return false;
    else if(Algebra.Equal(node_u.E, Algebra.Invalid)) {
        auxInCycleVP[u] = true;
        visVP[u] = true;
        return true;
    }
    if(auxInCycleVP[u] || visVP[u]) {
        auxInCycleVP[u] = true;
        visVP[u] = true;
        return true;
    }
    visVP[u] = true;
    for(const auto &[v, attr] : node_u.outNeighbours) {
        if(!Algebra.Preferred(node_u.E, node_u.aTab.at(v))) {
            if(auxInCycleVP[v] || visVP[v] || auxCheckCycle(v, Algebra, nodes)) {
                auxInCycleVP[u] = true;
                return true;
            }
        }
    }
    auxInCycleVP[u] = false;
    visVP[u] = false;
    return false;    
}

template <class A>
std::vector<Event<A>> VectoringProtocol<A>::processEvent(RoutingAlgebra<A> &Algebra, Event<A> &event, std::vector<NodeOneDest<A>> &nodes, int t, bool check)
{
    EventType type = event.type;
    int u = event.from;
    int v = event.to;
    int d = event.dest;
    A attr = event.cost;

    std::vector<Event<A>> ret;

    switch (type)
    {
    case ADVERTISE:
        ret = processAdvertise(nodes[u], d, attr, t, check);
        break;
    case UPDATE_MSG:
        ret = processUpdateMessage(nodes[v], u, d, attr, t);
        break;
    case LINK_FAILURE:
        ret = processLinkFailure(nodes[u], v, Algebra, d, t, check);
        break;
    case LINK_ADDITION:
        ret = processLinkAddition(u, nodes[v], Algebra, d, t, check);
        break;
    default:
        break;
    }

    if(check) {
        std::fill(auxInCycleVP.begin(), auxInCycleVP.end(), false);
        for(auto & n: nodes) {        
            std::fill(visVP.begin(), visVP.end(), false);
            if(auxCheckCycle(n.id, Algebra, nodes)) {
                if(changeAttrVP[n.id]) {
                    ++nCycles[n.id];
                    changeAttrVP[n.id] = false;
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

    return ret;
}

template <class A>
bool VectoringProtocol<A>::assertCorrectness(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra, EventType event, int d, bool print) {    
    bool ok = true;

    std::vector<A> EvTeo = EvDijkstra<A>(nodes, d, A{NEUTRAL}, false);
    for(const auto &n : nodes) {
        if(n.E != EvTeo[n.id]) {
            ok = false;
            std::cout << "Node: " << n.id << "\n";
            std::cout << "E: " << n.E << " E_exp: " << EvTeo[n.id] << "\n";
            break;
        }
    }

    return ok; 
}

template <class A>
std::vector<Event<A>> VectoringProtocol<A>::processUpdateMessage(NodeOneDest<A> &u, int v, int d, const A &v_attr, int t)
{
    std::vector<Event<A>> new_events;

    // Extension of the attribute advertised by v with the attribute of link uv
    u.aTab[v] = u.outNeighbours[v] + v_attr;

    // The attribute learnt from v is preferred to the currently elected attribute
    if(u.aTab.at(v) < u.E) {
        u.succ = v;
        u.Av = u.aTab.at(v);
    }
    // Node v was the successor of u and the attribute learnt from v is now worse than the previously elected attribute
    else if(u.succ == v && u.E < u.aTab.at(v)) {
        // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
        u.Av = u.O;
        u.succ = -1;
        for (const auto &[node, attribute] : u.aTab) {
            if(attribute < u.Av) {
                u.Av = attribute;
                u.succ = node;
            }
        }
    }
    /* else - preferred attribute did not change, do not change u.Av nor u.E nor u.succ */

    /* The Elected attribute changed */
    if (u.Av != u.E)
    {
        stats->attributeChange(u.id, t);
        changeAttrVP[u.id] = true;
        firstChangeVP[u.id] = true;
        u.E = u.Av;
        /* Send UPDATE message to all in-neighbours */
        for (const auto &[in_neigh, cost] : u.inNeighbours)
            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
    }

    return new_events;
}

template <class A>
std::vector<Event<A>> VectoringProtocol<A>::processAdvertise(NodeOneDest<A> &u, int d, A origin, int t, bool check)
{
    std::vector<Event<A>> new_events;

    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
        std::fill(firstChangeVP.begin(), firstChangeVP.end(), false);
    }

    origin = A(NEUTRAL);
    /* update the origin attribute to the advertised destination (node itself) */
    u.O = origin;

    /* compute the new most preferred attribute */
    u.Av = u.O;
    u.succ = -1;
    for (const auto &[node, attribute] : u.aTab) {
        if(attribute < u.Av) {
            u.Av = attribute;
            u.succ = node;
        }
    }

    if (u.Av != u.E) {
        stats->attributeChange(u.id, t);
        changeAttrVP[u.id] = true;
        firstChangeVP[u.id] = true;
        u.E = u.Av;
        /* Send UPDATE message to all in-neighbours */
        for (const auto &[in_neigh, cost] : u.inNeighbours)
            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
    }

    return new_events;
}


template <class A>
std::vector<Event<A>> VectoringProtocol<A>::processLinkFailure(NodeOneDest<A> &u, int v, RoutingAlgebra<A> &Alg, int d, int t, bool check) {
std::vector<Event<A>> new_events;
   
    /* Link u->v fails */

    // If node v was the successor of node u, the latter must recompute its preferred attribute
    if(u.succ == v) {
        // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
        u.Av = u.O;
        u.succ = -1;
        for (const auto &[node, attribute] : u.aTab) {
            if(Alg.Preferred(attribute, u.Av)) {
                u.Av = attribute;
                u.succ = node;
            }
        }

        stats->stateChange(u.id, t);
        changeAttrVP[u.id] = true;

        if(u.E != u.Av) {
            stats->attributeChange(u.id, t);
            u.E = u.Av;
            /* Send UPDATE message to all in-neighbours */
            for (const auto &[in_neigh, cost] : u.inNeighbours)
                new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
        }  
    }

    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }

    return new_events;
}

template <class A> 
std::vector<Event<A>> VectoringProtocol<A>::processLinkAddition(int u, NodeOneDest<A> &v, RoutingAlgebra<A> &Alg, int d, int t, bool check) {
    /* link u->v */
    std::vector<Event<A>> new_events;
    /* Uncoment if more than one destination is being considered */
    if(!Alg.Equal(v.E, Alg.Invalid))
        new_events.push_back(Event<A>(UPDATE_MSG, v.id, u, d, v.E));

    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }
    
    return new_events;
}


template<class A> 
std::vector<int>& VectoringProtocol<A>::getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u) 
        if(nodes[u].E == Algebra.Invalid) 
            timeInCycle[u] = -1;

    return timeInCycle;
}

template<class A> 
std::vector<int>& VectoringProtocol<A>::getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u) 
        if(nodes[u].E == Algebra.Invalid) 
            nCycles[u] = -1;

    return nCycles;
}

template <class A> 
int VectoringProtocol<A>::getPathLength(std::vector<NodeOneDest<A>> &nodes, int u, RoutingAlgebra<A> &, int d){
    return nodes[u].longestPath;
}



#include "../Instances/vectoringProtocolInstance.h"