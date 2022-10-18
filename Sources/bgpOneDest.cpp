
#include <fstream>
#include <unordered_map>

#include "../Headers/utilities.h"
#include "../Headers/stats.h"
#include "../Headers/bgpOneDest.h"

// Object where all the statistics are saved
extern Stats *stats;

extern int nNodes;

std::vector<bool> auxInCycleBGP;
std::vector<bool> visBGP;
std::vector<bool> changeAttrBGP;
std::vector<bool> firstChangeBGP;

template <class A>
BGPOneDest<A>::BGPOneDest() {
    nCycles = std::vector<int>(nNodes, 0);
    inCycle = std::vector<bool>(nNodes, false);
    cycleFormed = std::vector<int>(nNodes, 0);
    timeInCycle = std::vector<int>(nNodes, 0);
    visBGP = std::vector<bool>(nNodes, false);
    auxInCycleBGP = std::vector<bool>(nNodes, false);
    changeAttrBGP = std::vector<bool>(nNodes, false);
    firstChangeBGP = std::vector<bool>(nNodes, false);
}

template <class A> 
bool BGPOneDest<A>::auxCheckCycle(int u, RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes) {
    if(!firstChangeBGP[u]) return false; 
    const auto &node_u = nodes[u];
    if(node_u.succ == -1 && !Algebra.Equal(node_u.E, Algebra.Invalid)) return false;
    else if(Algebra.Equal(node_u.E, Algebra.Invalid)) {
        auxInCycleBGP[u] = true;
        visBGP[u] = true;
        return true;
    }
    if(auxInCycleBGP[u] || visBGP[u]) {
        auxInCycleBGP[u] = true;
        visBGP[u] = true;
        return true;
    }
    visBGP[u] = true;
    for(const auto &[v, attr] : node_u.outNeighbours) {
        if(!Algebra.Preferred(node_u.E, node_u.aTab.at(v))) {
            if(auxInCycleBGP[v] || visBGP[v] || auxCheckCycle(v, Algebra, nodes)) {
                auxInCycleBGP[u] = true;
                return true;
            }
        }
    }
    auxInCycleBGP[u] = false;
    visBGP[u] = false;
    return false;    
}

template <class A>
std::vector<Event<A>> BGPOneDest<A>::processEvent(RoutingAlgebra<A> &Algebra, Event<A> &event, std::vector<NodeOneDest<A>> &nodes, int t, bool check)
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
        ret = processAdvertise(nodes[u], d, attr, Algebra, t, check);
        break;
    case WITHDRAWAL:
        ret = processWithdrawal(nodes[u], d, Algebra, t, check);
        break;
    case UPDATE_MSG:
        ret = processUpdateMessage(nodes[v], u, d, attr, Algebra, t);
        break;
    case LINK_COST_CHANGE:
        ret = processLinkCostChange(u, nodes[v], Algebra, d, t, check);
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

    if(!Algebra.Equal(nodes[u].E, Algebra.Invalid))
        nodes[u].longestPath = std::max(nodes[u].longestPath, nodes[u].E.length());
    if(!Algebra.Equal(nodes[v].E, Algebra.Invalid))
        nodes[v].longestPath = std::max(nodes[v].longestPath, nodes[v].E.length());

    if(check) {
        std::fill(auxInCycleBGP.begin(), auxInCycleBGP.end(), false);
        for(auto & n: nodes) {        
            std::fill(visBGP.begin(), visBGP.end(), false);
            if(auxCheckCycle(n.id, Algebra, nodes)) {
                if(changeAttrBGP[n.id]) {
                    ++nCycles[n.id];
                    changeAttrBGP[n.id] = false;
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
bool BGPOneDest<A>::assertCorrectness(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra, EventType event, int d, bool print) {    
    bool ok = true;

    if(event == ADVERTISE) {
        std::vector<A> EvTeo = EvDijkstra<A>(nodes, d, A{NEUTRAL,d}, false);
        for(const auto &n : nodes) {
            if(n.E != EvTeo[n.id]){
                ok = false;
                std::cout << "Node: " << n.id << "\n";
                std::cout << "E: " << n.E << " E_exp: " << EvTeo[n.id] << "\n";
                break;
            }
        }
    }
    else {
        for(const auto &n : nodes) {
            if(n.E != Algebra.Invalid){
                ok = false;
                std::cout << "Node: " << n.id << "\n";
                std::cout << "E: " << n.E << " E_exp: " << Algebra.Invalid << "\n";
                break;
            }
        }
    }
    return ok; 
}

template <class A>
std::vector<Event<A>> BGPOneDest<A>::processUpdateMessage(NodeOneDest<A> &u, int v, int d, const A &v_attr, RoutingAlgebra<A> &Alg, int t)
{
    std::vector<Event<A>> new_events;

    // Extension of the attribute advertised by v with the attribute of link uv
    u.aTab[v] = Alg.Extend(u.outNeighbours[v], v_attr);

    // The attribute learnt from v is preferred to the currently elected attribute
    if(Alg.Preferred(u.aTab.at(v), u.E)) {
        u.succ = v;
        u.Av = u.aTab.at(v);
    }
    // Node v was the successor of u and the attribute learnt from v is now worse than the previously elected attribute
    else if(u.succ == v && Alg.Preferred(u.E, u.aTab.at(v))) {
        // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
        u.Av = u.O;
        u.succ = -1;
        for (const auto &[node, attribute] : u.aTab) {
            if(Alg.Preferred(attribute, u.Av)) {
                u.Av = attribute;
                u.succ = node;
            }
        }
        // if(!(u.E != u.Av)) 
        //     std::cout << v << ": " << u.E << " -> " << u.succ << ": " << u.Av << "\n";
    } 
    else if(u.succ == v) {
        u.succ = v;
        u.Av = u.aTab.at(v);
    }
    /* else - preferred attribute did not change, do not change u.Av nor u.E nor u.succ */

    /* The Elected attribute changed */
    if (!Alg.Equal(u.Av, u.E))
    {
        stats->stateChange(u.id, t);
        stats->attributeChange(u.id, t);
        changeAttrBGP[u.id] = true;
        firstChangeBGP[u.id] = true;

        u.E = u.Av;
        /* Send UPDATE message to all in-neighbours */
        for (const auto &[in_neigh, cost] : u.inNeighbours)
            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
    }

    return new_events;
}

template <class A>
std::vector<Event<A>> BGPOneDest<A>::processAdvertise(NodeOneDest<A> &u, int d, A origin, RoutingAlgebra<A> &Alg, int t, bool check)
{
    std::vector<Event<A>> new_events;

    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
        std::fill(firstChangeBGP.begin(), firstChangeBGP.end(), false);
    }

    origin = A(NEUTRAL, u.id);
    /* update the origin attribute to the advertised destination (node itself) */
    u.O = origin;

    /* compute the new most preferred attribute */
    u.Av = u.O;
    u.succ = -1;
    for (const auto &[node, attribute] : u.aTab) {
        if(Alg.Preferred(attribute, u.Av)) {
            u.Av = attribute;
            u.succ = node;
        }
    }

    if (!Alg.Equal(u.Av, u.E)) {
        stats->stateChange(u.id, t);
        stats->attributeChange(u.id, t);
        changeAttrBGP[u.id] = true;
        firstChangeBGP[u.id] = true;

        u.E = u.Av;
        /* Send UPDATE message to all in-neighbours */
        for (const auto &[in_neigh, cost] : u.inNeighbours)
            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
    }

    return new_events;
}

template <class A>
std::vector<Event<A>> BGPOneDest<A>::processWithdrawal(NodeOneDest<A> &u, int d, RoutingAlgebra<A> &Alg, int t, bool check)
{
    std::vector<Event<A>> new_events;

    /* update the origin attribute to the advertised destination (node itself) */
    u.O = Alg.Invalid;

    /* compute the new most preferred attribute */
    u.Av = u.O;
    u.succ = -1;
    for (const auto &[node, attribute] : u.aTab) {
        if(Alg.Preferred(attribute, u.Av)) {
            u.Av = attribute;
            u.succ = node;
        }
    }

    if (!Alg.Equal(u.Av, u.E)) {
        stats->stateChange(u.id, t);
        stats->attributeChange(u.id, t);
        changeAttrBGP[u.id] = true;

        u.E = u.Av;
        /* Send UPDATE message to all in-neighbours */
        for (const auto &[in_neigh, cost] : u.inNeighbours)
            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
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
std::vector<Event<A>> BGPOneDest<A>::processLinkCostChange(int u, NodeOneDest<A> &v, RoutingAlgebra<A> &Alg, int d, int t, bool check) {
    /* link u->v */
    std::vector<Event<A>> new_events;

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

template <class A>
std::vector<Event<A>> BGPOneDest<A>::processLinkFailure(NodeOneDest<A> &u, int v, RoutingAlgebra<A> &Alg, int d, int t, bool check) {
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
        stats->attributeChange(u.id, t);
        changeAttrBGP[u.id] = true;

        u.E = u.Av;
        /* Send UPDATE message to all in-neighbours */
        for (const auto &[in_neigh, cost] : u.inNeighbours)
            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
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
std::vector<Event<A>> BGPOneDest<A>::processLinkAddition(int u, NodeOneDest<A> &v, RoutingAlgebra<A> &Alg, int d, int t, bool check) {
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

template <class A> 
int BGPOneDest<A>::getPathLength(std::vector<NodeOneDest<A>> &nodes, int u, RoutingAlgebra<A> &Algebra, int d){
    return nodes[u].longestPath;
}

template<class A> 
std::vector<int>& BGPOneDest<A>::getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u) 
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid)) 
            timeInCycle[u] = -1;

    return timeInCycle;
}

template<class A> 
std::vector<int>& BGPOneDest<A>::getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u) 
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid)) 
            nCycles[u] = -1;

    return nCycles;
}


#include "../Instances/bgpOneDestInstance.h"