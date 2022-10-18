
#include <fstream>
#include <unordered_map>
#include <iostream>
#include <cstring>

#include "../Headers/stats.h"
#include "../Headers/utilities.h"
#include "../Headers/bgpSynchronous.h"

extern Stats *stats;

extern int nNodes;

std::vector<bool> auxInCycle;
std::vector<bool> visited;
std::vector<bool> changeAttr;
std::vector<bool> firstChange;

template<class A> BGPSync<A>::BGPSync(){
    hasEvent = new bool[nNodes]{false};
    nCycles = std::vector<int>(nNodes, 0);
    inCycle = std::vector<bool>(nNodes, false);
    cycleFormed = std::vector<int>(nNodes, 0);
    timeInCycle = std::vector<int>(nNodes, 0);
    visited = std::vector<bool>(nNodes, false);
    auxInCycle = std::vector<bool>(nNodes, false);
    changeAttr = std::vector<bool>(nNodes, false);
    firstChange = std::vector<bool>(nNodes, false);
}

template<class A> BGPSync<A>::~BGPSync(){
    delete[] hasEvent;
}

template <class A> void BGPSync<A>::processUpdateMessage(NodeOneDest<A> &u, int v, const A &v_attr, RoutingAlgebra<A> &Algebra) {
    // Extension of the cost advertised by v with the uv link cost
    u.aTab[v] = Algebra.Extend(u.outNeighbours[v], v_attr);
    
    hasEvent[u.id] = true;
}

template <class A> void BGPSync<A>::processAdvertise(NodeOneDest<A> &u, int d, A origin, bool check) {
    /* update the origin attribute to the advertised destination: node itself */
    origin = A(NEUTRAL, u.id);
    u.O = origin; 
    hasEvent[u.id] = true;
    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(changeAttr.begin(), changeAttr.end(), false);
        std::fill(firstChange.begin(), firstChange.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }
}

template <class A> void BGPSync<A>::processWithdrawal(NodeOneDest<A> &u, int d, RoutingAlgebra<A> &Algebra, bool check) {
    /* Just as processAdvertise, but now the origin attribute is set to infinity */
    u.O = Algebra.Invalid; 
    hasEvent[u.id] = true;
    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(changeAttr.begin(), changeAttr.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }
}

template <class A> void BGPSync<A>::processLinkCostChange(int u, NodeOneDest<A> &v, bool check) {
    /* link u->v */
    v.sendUpdate[u] = true;
    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(changeAttr.begin(), changeAttr.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }
}

template <class A> void BGPSync<A>::processLinkAddition(int u, NodeOneDest<A> &v, bool check) {
    /* link u->v */
    v.sendUpdate[u] = true;
    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(changeAttr.begin(), changeAttr.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }
}

template <class A> void BGPSync<A>::processLinkFailure(NodeOneDest<A> &u, int v, RoutingAlgebra<A> &Algebra, bool check) {
    /* Link u->v fails */

    /* Node v:
     u is removed from the set of in-neighbours of v
     Node v does nothing else */

    /* Node u
     v is removed from the set of out-neighbours of u;
     If v was the successor of u, the latter must recompute its preferred attribute */
    if(u.succ == v) {
        hasEvent[u.id] = true;
        u.succ = -1;
    }
    if(check) {
        std::fill(nCycles.begin(), nCycles.end(), 0);
        std::fill(inCycle.begin(), inCycle.end(), false);
        std::fill(changeAttr.begin(), changeAttr.end(), false);
        std::fill(cycleFormed.begin(), cycleFormed.end(), 0);
        std::fill(timeInCycle.begin(), timeInCycle.end(), 0);
    }
}


template <class A> bool BGPSync<A>::auxCheckCycle(int u, RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes) {
    if(!firstChange[u]) return false;
    const auto &node_u = nodes[u];
    if(node_u.succ == -1 && !Algebra.Equal(node_u.E, Algebra.Invalid)) return false;
    else if(Algebra.Equal(node_u.E, Algebra.Invalid)) {
        auxInCycle[u] = true;
        visited[u] = true;
        return true;
    }
    if(auxInCycle[u] || visited[u]) {
        auxInCycle[u] = true;
        visited[u] = true;
        return true;
    }
    visited[u] = true;
    for(const auto &[v, attr] : node_u.outNeighbours) {
        if(!Algebra.Preferred(node_u.E, node_u.aTab.at(v))) {
            if(auxInCycle[v] || visited[v] || auxCheckCycle(v, Algebra, nodes)) {
                auxInCycle[u] = true;
                return true;
            }
        }
    }
    auxInCycle[u] = false;
    visited[u] = false;
    return false;    
}

template <class A> bool BGPSync<A>::checkCycle(int u, std::vector<NodeOneDest<A>> &nodes, int iteration, bool print) {
    // check for a cycle
    int node = u, cnt = 0;
    bool vis[nNodes]{false};

    while(node != -1 && cnt++ < nNodes) {
        if(auxInCycle[node] || vis[node]) {
            if(print) {
                std::cout << "A CYCLE WAS DETECTED at iteration " << iteration << "!\n";
                std::cout << "\tNode: " << u << "; Length of cycle: " << cnt << std::endl;
                std::cout << "\tAttribute elected: ";
                std::cout << nodes[u].E << std::endl;
            }
            visited[node] = true;
            auxInCycle[u] = true;
            return true;
        }
        vis[node] = true;
        node = nodes[node].succ;
    }
    return false;
}


template<class A> 
std::vector<Event<A>> BGPSync<A>::SynchronousIteration(RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes, int d, int iter, bool check) {
    std::vector<Event<A>> new_events;

    // std::vector<bool> changeAttr(nNodes, false);

    for(auto &u : nodes) {
        if(hasEvent[u.id]) {
            // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours}), the new preferred attribute
            u.Av = u.succ == -1 ? u.O : u.aTab.at(u.succ);
            for (const auto &[node, attribute] : u.aTab) {
                if(Algebra.Preferred(attribute, u.Av)) {
                    u.Av = attribute;
                    u.succ = node;
                }
            }
            if(Algebra.Preferred(u.O, u.Av)) {
                    u.Av = u.O;
                    u.succ = -1;
            }
            if(Algebra.Equal(u.Av, Algebra.Invalid))
                u.succ = -1;
            /* If the Elected attribute changed */
            if (!Algebra.Equal(u.Av, u.E))
            {
                stats->stateChange(u.id, iter);
                stats->attributeChange(u.id, iter);
                changeAttr[u.id] = true;
                firstChange[u.id] = true;

                u.E = u.Av;
                /* Send UPDATE message to all in-neighbours */
                for (const auto &[in_neigh, cost] : u.inNeighbours)
                {
                    new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
                    u.sendUpdate[in_neigh] = false; // does not send repeated UPDATE messages to in-neighbours
                }
            }

            hasEvent[u.id] = false;

        } /* end of if(hasEvent[u.id]) */
        
        /* Sends an <UPDATE> message to all nodes whose link to changed attribute or was added */
        if(u.E != Algebra.Invalid) {
            for(auto &[in_neigh, updt] : u.sendUpdate) {
                if(updt) {
                    new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
                    u.sendUpdate[in_neigh] = false;
                }
            }
        }

    } /* end of for(u : nodes)*/

    if(check) {
        std::fill(auxInCycle.begin(), auxInCycle.end(), false);
        for(auto & u: nodes) {
            std::fill(visited.begin(), visited.end(), false);
            if(auxCheckCycle(u.id, Algebra, nodes)) {
                if(changeAttr[u.id]) {
                    ++nCycles[u.id];
                    changeAttr[u.id] = false;
                }
                if(!inCycle[u.id]) {
                    inCycle[u.id] = true;
                    cycleFormed[u.id] = iter;
                }
            }
            else {
                if(inCycle[u.id]) {
                    timeInCycle[u.id] += (iter - cycleFormed[u.id]);
                    inCycle[u.id] = false;
                }
            }  
        }

    }

    for(auto & u: nodes) {
        if(!Algebra.Equal(u.E, Algebra.Invalid))
            u.longestPath = std::max(u.longestPath, u.E.length());
    }

    return new_events;
}

template<class A> 
std::vector<Event<A>> BGPSync<A>::processEvent(RoutingAlgebra<A> &Algebra, Event<A> &event, std::vector<NodeOneDest<A>> &nodes, int t, bool check) {
    EventType type = event.type;
    auto &from = nodes[event.from];
    auto &to = nodes[event.to];
    int d = event.dest;
    A attr = event.cost;

    switch(type) {
        case ADVERTISE:
            processAdvertise(from, d, attr, check);
            break;
        case WITHDRAWAL:
            processWithdrawal(from, d, Algebra, check);
            break;
        case UPDATE_MSG:
            processUpdateMessage(to, from.id, attr, Algebra);
            break;
        case LINK_COST_CHANGE:
            processLinkCostChange(from.id, to, check);
            break;
        case LINK_FAILURE:
            processLinkFailure(from, to.id, Algebra, check);
            break;
        case LINK_ADDITION:
            processLinkAddition(from.id, to, check);
            break;
        default:
            break;
    }

    return {};
}

template<class A> 
bool BGPSync<A>::assertCorrectness(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra, EventType event, int d, bool print) {
        
    bool ok = true;
    if(event == ADVERTISE) {
        std::vector<A> EvTeo = EvDijkstra<A>(nodes, d, A{NEUTRAL,d}, false);
        for(const auto &n : nodes) {
            if(n.E != EvTeo[n.id]){
                ok = false;
                if(print) {
                    std::cout << "Correction assertion failed at ";
                    std::cout << "Node: " << n.id << "\n";
                    std::cout << "E: " << n.E << "B: " << n.B << "A: " << n.Av << " E_exp: " << EvTeo[n.id] << "\n";
                    std::cout << "state: " << n.state << "\n";
                }
                break;
            }
        }
    }
    else {
        for(const auto &n : nodes) {
            if(n.E != Algebra.Invalid){
                ok = false;
                if(print) {
                    std::cout << "Correction assertion failed at ";
                    std::cout << "Node: " << n.id << "\n";
                    std::cout << "E: " << n.E << "B: " << n.B << "A: " << n.Av << " E_exp: " << Algebra.Invalid << "\n";
                    std::cout << "state: " << n.state << "\n";
                }
                break;
            }
        }
    }
    return ok;
}

template<class A> 
int BGPSync<A>::getPathLength(std::vector<NodeOneDest<A>> &nodes, int u, RoutingAlgebra<A> &, int) {
    return nodes[u].longestPath;
}

template<class A> 
std::vector<int>& BGPSync<A>::getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u) 
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid)) 
            timeInCycle[u] = -1;

    return timeInCycle;
}

template<class A> 
std::vector<int>& BGPSync<A>::getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u) 
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid)) 
            nCycles[u] = -1;

    return nCycles;
}



#include "../Instances/bgpSyncInstance.h"
