#include <fstream>
#include <unordered_map>
#include <iostream>

#include "../Headers/stats.h"
#include "../Headers/utilities.h"
#include "../Headers/eigrpSynchronous.h"

extern Stats *stats;

extern int nNodes;

extern std::ofstream &file_out;

std::vector<bool> auxInBlackHole;
std::vector<bool> changeState;
std::vector<bool> firstAttrChange;

template<class A> EIGRPSync<A>::EIGRPSync(){
    hasEvent = new bool[nNodes]{false};
    inBlackHole = std::vector<bool>(nNodes, false);
    blackHoleFormed = std::vector<int>(nNodes, 0);
    timeInBlackHole = std::vector<int>(nNodes, 0);
    auxInBlackHole = std::vector<bool>(nNodes, false);
    nBlackHoles = std::vector<int>(nNodes, 0);
    changeState = std::vector<bool>(nNodes, false);
    firstAttrChange = std::vector<bool>(nNodes, false);
}

template<class A> EIGRPSync<A>::~EIGRPSync(){
    delete[] hasEvent;
}

template <class A> void EIGRPSync<A>::processUpdateMessage(NodeOneDest<A> &u, int v, const A &v_attr, RoutingAlgebra<A> &Algebra) {
    // Extension of the cost advertised by v with the uv link cost
    u.aTab[v] = Algebra.Extend(u.outNeighbours[v], v_attr);

    hasEvent[u.id] = true;
}

template <class A> void EIGRPSync<A>::processDiffusingMessage(NodeOneDest<A> &u, int v, const A &v_attr, RoutingAlgebra<A> &Algebra) {
    // Extension of the cost advertised by v with the uv link cost
    u.aTab[v] = Algebra.Extend(u.outNeighbours[v], v_attr);
    u.diffusingMsg[v] = true;

    hasEvent[u.id] = true;
}

template <class A> void EIGRPSync<A>::processClearMessage(int u, NodeOneDest<A> &v) {
    v.clear[u] = true;

    hasEvent[v.id] = true;
}

template <class A> void EIGRPSync<A>::processAdvertise(NodeOneDest<A> &u, int d, A origin, bool check) {
    /* update the origin attribute to the advertised destination: node itself */
    u.O = origin; 
    hasEvent[u.id] = true;
    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeState.begin(), changeState.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
        std::fill(firstAttrChange.begin(), firstAttrChange.end(), false);
    }
}

template <class A> void EIGRPSync<A>::processWithdrawal(NodeOneDest<A> &u, int d, RoutingAlgebra<A> &Algebra, bool check) {
    /* Just as processAdvertise, but now the origin attribute is set to infinity*/
    u.O = Algebra.Invalid; 
    hasEvent[u.id] = true;
    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeState.begin(), changeState.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
    }
}

template <class A> void EIGRPSync<A>::processLinkCostChange(int u, NodeOneDest<A> &v, bool check) {
    /* link u->v */
    v.sendUpdate[u] = true;
    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeState.begin(), changeState.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
    }
}

template <class A> void EIGRPSync<A>::processLinkAddition(int u, NodeOneDest<A> &v, bool check) {
    /* link u->v */
    v.sendUpdate[u] = true;
    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeState.begin(), changeState.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
    }
}

template <class A> void EIGRPSync<A>::processLinkFailure(NodeOneDest<A> &u, int v, RoutingAlgebra<A> &Algebra, bool check) {
    /* Link u->v fails */

    /* Node v: u is removed from the set of in-neighbours of v */
    hasEvent[v] = true;

    /* Node u: v is removed from the set of out-neighbours of u */
    // If v is the successor of u, set u.succ to -1; 
    // If u is in diffusing state, it continues the diffusing computation without a successor and with an elected attribute of Inf
    if(u.succ == v) {
        u.succ = -1;
        if(u.state == DIFFUSING) {
            u.E = Algebra.Invalid;
            u.sendClear = false;
        }
        hasEvent[u.id] = true;
    }

    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeState.begin(), changeState.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
    }
}


template <class A> void EIGRPSync<A>::checkSafety(const NodeOneDest<A> &u, RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes) {
    // check Safety properties
    if(u.state == UPDATE && !Algebra.Equal(u.B, u.E))
        std::cout << "ERROR - UPDATE state but B_u != E_u\n";
    if(u.state == DIFFUSING && !Algebra.Preferred(u.B, u.E))
        std::cout << "ERROR - DIFFUSING state but B_u >= E_u\n";

    if(u.state == UPDATE && !Algebra.Equal(u.E, Algebra.Invalid)) {
        for(const auto &[v, attr] : u.outNeighbours) {
            if(Algebra.Equal(u.aTab.at(v), u.E) && !Algebra.Preferred(nodes[v].B, u.B)) {
                std::cout << "ERROR - B_u <= B_v, with v = succ_u\n";
                std::cout << "state_u = UPDATE; ";
                std::cout << "state_v = " << (nodes[v].state == UPDATE ? "UPDATE\n" : "DIFFUSING\n");
                std::cout << "u = "<< u.id << " B_u =" << u.B << "E_u =" << u.E << "A_u =" << u.Av << "\n";
                std::cout << "v = "<< v << " B_v =" << nodes[v].B << "E_v =" << nodes[v].E << "A_v =" << nodes[v].Av << "\n";
                std::cout << "\n\n";
                std::cout << u << "\n";
                std::cout << nodes[v] << "\n";
            }
        }
    } else if(u.state == DIFFUSING && u.succ != -1) {
        if(!Algebra.Preferred(nodes[u.succ].B, u.B)) {
            std::cout << "ERROR - B_u <= B_v, with v = succ_u\n";
            std::cout << "state_u = DIFFUSING; ";
            std::cout << "state_v = " << (nodes[u.succ].state == UPDATE ? "UPDATE\n" : "DIFFUSING\n");
            std::cout << "u = "<< u.id << " B_u =" << u.B << "E_u =" << u.E << "A_u =" << u.Av << "\n";
            std::cout << "v = "<< u.succ << " B_v =" << nodes[u.succ].B << "E_v =" << nodes[u.succ].E << "A_v =" << nodes[u.succ].Av << "\n";
            std::cout << "\n\n";
            std::cout << u << "\n";
            std::cout << nodes[u.succ] << "\n";           
        }
    }
        
    for(const auto &[in_neigh, attr] : u.inNeighbours) {
        const auto &t = nodes[in_neigh];
        if(t.state == UPDATE && !Algebra.Equal(t.E, Algebra.Invalid)) {
            if(Algebra.Equal(t.aTab.at(u.id), t.E) && !Algebra.Preferred(u.B, t.B)) {
                std::cout << "ERROR - B_t <= B_u, with u = succ_t\n";
                std::cout << "state_t = UPDATE; ";
                std::cout << "state_u = " << (u.state == UPDATE ? "UPDATE\n" : "DIFFUSING\n");
                std::cout << "t = "<< t.id << " B_t =" << t.B << "E_t =" << t.E << "A_t =" << t.Av << "\n";
                std::cout << "u = "<< u.id << " B_u =" << u.B << "E_u =" << u.E << "A_u =" << u.Av << "\n";
                std::cout << "\n\n";
                std::cout << t << "\n";
                std::cout << u << "\n";
            }
        } else if(t.state == DIFFUSING && t.succ == u.id) {
            if(!Algebra.Preferred(u.B, t.B)) {
                std::cout << "ERROR - B_t <= B_u, with u = succ_t\n";
                std::cout << "state_t = DIFFUSING; ";
                std::cout << "state_u = " << (u.state == UPDATE ? "UPDATE\n" : "DIFFUSING\n");
                std::cout << "t = "<< t.id << " B_t =" << t.B << "E_t =" << t.E << "A_t =" << t.Av << "\n";
                std::cout << "u = "<< u.id << " B_u =" << u.B << "E_u =" << u.E << "A_u =" << u.Av << "\n";
                std::cout << "\n\n";
                std::cout << t << "\n";
                std::cout << u << "\n";     
            }
        }
    }
}

template <class A> void EIGRPSync<A>::checkCycle(int u, std::vector<NodeOneDest<A>> &nodes) {
    // check for a cycle
    int node = u, cnt = 0; 
    while(node != -1 && ++cnt <= (int)nodes.size()) {
        node = nodes[node].succ;
        if(node == u) {
            std::cout << "\nA CYCLE WAS DETECTED!\n";
            break;
        }
    }
    if(cnt == (int)nodes.size())
        std::cout << "A CYCLE WAS DETECTED!\n";
}


template <class A> bool EIGRPSync<A>::checkBlackHole(int u, RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes) {
    if(!firstAttrChange[u]) return false;
    if(auxInBlackHole[u]) return true;
    if(nodes[u].succ == -1 && Algebra.Equal(nodes[u].E, Algebra.Invalid)) {
        auxInBlackHole[u] = true;
        return true;
    } else if(nodes[u].succ == -1) {
        auxInBlackHole[u] = false;
        return false;
    }
    if(nodes[u].state == UPDATE && Algebra.Equal(nodes[u].E, Algebra.Invalid)) {
        auxInBlackHole[u] = true;
        return true;
    } else if(nodes[u].state == UPDATE && !Algebra.Equal(nodes[u].E, Algebra.Invalid)) {
        for(const auto &[v, attr] : nodes[u].outNeighbours) {
            if(Algebra.Equal(nodes[u].aTab.at(v), nodes[u].E)) {
                if(checkBlackHole(v, Algebra, nodes)) {
                    auxInBlackHole[u] = true;
                    return true;
                }
            }
        }
    } else if(nodes[u].state == DIFFUSING && nodes[u].succ == -1) {
        auxInBlackHole[u] = true;
        return true;
    } else if(nodes[u].state == DIFFUSING && nodes[u].succ != -1) {
        if(checkBlackHole(nodes[u].succ, Algebra, nodes)) {
            auxInBlackHole[u] = true;
            return true;
        }
    }
    auxInBlackHole[u] = false;
    return false;
}


template<class A> 
std::vector<Event<A>> EIGRPSync<A>::SynchronousIteration(RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes, int d, int iter, bool check) {
    std::vector<Event<A>> new_events;
    for(auto &u : nodes) {
        if(hasEvent[u.id]) {
            if(u.state == UPDATE) { /* Node u is in the UPDATE state */
                // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
                int succ = -1;
                u.Av = u.O;
                for(const auto & [outNeigh, attribute] : u.aTab) {
                    if(Algebra.Preferred(attribute, u.Av)) {
                        u.Av = attribute;
                        succ = outNeigh; 
                    }
                }
                /* Av <= Ev */
                if(!Algebra.Preferred(u.E, u.Av) || u.inNeighbours.size() == 0) { 
                    firstAttrChange[u.id] = true;
                    /* Av != Ev */
                    if(!Algebra.Equal(u.Av, u.E)) { 
                        stats->stateChange(u.id, iter);
                        stats->attributeChange(u.id, iter);
                        changeState[u.id] = true;
                        u.E = u.Av; u.B = u.Av;
                        /* Send UPDATE message to all in-neighbours */
                        for(const auto &[in_neigh, cost] : u.inNeighbours) {
                            new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
                            u.sendUpdate[in_neigh] = false; // no need to send another <UPDATE> message due to a link addition or cost change
                        }
                    }
                    u.succ = succ;
                }
                /* Av > Ev */
                else { /* Node u goes to DIFFUSING state */
                    u.state = DIFFUSING;
                    if(u.succ != -1) { // Node u has a successor; keeps that node as a successor
                        u.E = u.aTab.at(u.succ);
                        u.sendClear = u.diffusingMsg.at(u.succ);
                        if(!u.sendClear)
                            stats->updateIsotonicityBreak(u.id);
                        u.diffusingMsg[u.succ] = false; // so that node u does not send a <CLEAR> message to its successor right away
                    }
                    else { // Node u does not have a successor (for exemple, the node that advertises a prefix); starts a diffusing computation without successor
                        u.E = u.O;
                        u.sendClear = false;
                    }

                    /* Starts a diffusing computation: sends a DIFFUSING message to every in-neighbour */
                    for(const auto &[in_neigh,cost] : u.inNeighbours) {
                        new_events.push_back(Event<A>(DIFFUSING_MSG, u.id, in_neigh, d, u.E));
                        u.clear[in_neigh] = false;
                    }
                    /* Update statistic counters */
                    stats->updateTransitions(u.id);
                    stats->stateChange(u.id, iter);
                    stats->attributeChange(u.id, iter);
                    changeState[u.id] = true;
                }
            } /* end of if(u.state == UPDATE) */
            else { /* u.state == DIFFUSING */
                if(u.succ != -1 && u.diffusingMsg.at(u.succ)) {
                    u.sendClear = true;
                    u.diffusingMsg[u.succ] = false;
                }
                bool clear = true;
                for(const auto &[in_neigh,clr] : u.clear) {
                    if(!clr) {  
                        clear = false;
                        break;
                    }
                }
                if(clear) {
                    stats->stateChange(u.id, iter);
                    // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
                    int succ = -1;
                    u.Av = u.O;
                    for(const auto & [out_neigh, attribute] : u.aTab) {
                        if(Algebra.Preferred(attribute, u.Av)) {
                            u.Av = attribute;
                            succ = out_neigh;
                        }
                    }
                    /* If the new elected attribute is preferred or equal to the previous one */
                    if(!Algebra.Preferred(u.E, u.Av)) { /* Return to UPDATE state */
                        u.state = UPDATE;
                        if(!Algebra.Equal(u.E, u.Av)) {
                            stats->attributeChange(u.id, iter);
                            changeState[u.id] = true;
                        }
                        u.E = u.Av; u.B = u.Av;
                        if(!Algebra.Equal(u.E, Algebra.Invalid)) {
                            /* Send UPDATE message to all in-neighbours */
                            for(const auto &[in_neigh,cost] : u.inNeighbours) {
                                new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
                                u.sendUpdate[in_neigh] = false; // no need to send another <UPDATE> message due to a link addition or cost change
                            }
                        }
                        if(u.sendClear)  // send <CLEAR> to the successor;
                            new_events.push_back(Event<A>(CLEAR_MSG, u.id, u.succ, d));
                        
                        u.succ = succ; // New node that provides the best path
                    }
                    else { /* Remain in DIFFUSING state and start/relay a new diffusing computation */
                        stats->attributeChange(u.id, iter);
                        u.E = (u.succ == -1 ? u.O : u.aTab.at(u.succ));
                        for(const auto &[in_neigh,cost] : u.inNeighbours) {
                            new_events.push_back(Event<A>(DIFFUSING_MSG, u.id, in_neigh, d, u.E));
                            u.clear[in_neigh] = false;
                        }
                    }
                } /* end of if(clear) */
            } /* end of else (u.state == DIFFUSING) */

            /* Sends a <CLEAR> message to all nodes that sent a <DIFFUSING> message and are not the successor of the node */
            for(auto &[out_neigh, difmsg] : u.diffusingMsg) {
                if(difmsg) {
                    new_events.push_back(Event<A>(CLEAR_MSG, u.id, out_neigh, d));
                    difmsg = false;
                }
            }

            /* Checks if the Invariant Bu > Bv holds */
            checkSafety(u, Algebra, nodes);

            hasEvent[u.id] = false;
        } /* end of if(hasEvent[u.id]) */
        
        /* Sends an <UPDATE> message to all nodes whose link to changed attribute or was added */
        if(u.state == UPDATE && u.E != Algebra.Invalid) {
            for(auto [in_neigh, updt] : u.sendUpdate) {
                if(updt) {
                    new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));

                    u.sendUpdate[in_neigh] = false;
                }
            }
        }

    } /* end of for(u : nodes)*/

    /* Checks if a black hole is formed */
    if(check) {
        std::fill(auxInBlackHole.begin(), auxInBlackHole.end(), false);
        for(auto & u: nodes) {
            if(checkBlackHole(u.id, Algebra, nodes)) {
                if(changeState[u.id]) {
                    ++nBlackHoles[u.id];
                    changeState[u.id] = false;
                }
                if(!inBlackHole[u.id]) {
                    inBlackHole[u.id] = true;
                    blackHoleFormed[u.id] = iter;
                }
            }
            else {
                if(inBlackHole[u.id]) {
                    timeInBlackHole[u.id] += (iter - blackHoleFormed[u.id]);
                    inBlackHole[u.id] = false;
                }
            }  
        }
    }
    
    for(auto & u: nodes) {
        if(!Algebra.Equal(u.E, Algebra.Invalid)) {
            int path_len = 1;
            for(int succ = u.succ; succ != -1; ++path_len, succ = nodes[succ].succ);
            u.longestPath = std::max(u.longestPath, path_len);
        }
    }

    return new_events;
}

template<class A> 
std::vector<Event<A>> EIGRPSync<A>::processEvent(RoutingAlgebra<A> &Algebra, Event<A> &event, std::vector<NodeOneDest<A>> &nodes, int t, bool check) {
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
        case DIFFUSING_MSG:
            processDiffusingMessage(to, from.id, attr, Algebra);
            break;
        case CLEAR_MSG:
            processClearMessage(from.id, to);
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
bool EIGRPSync<A>::assertCorrectness(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra, EventType event, int d, bool print) {

    bool ok = true;
    if(event == ADVERTISE) {
        std::vector<A> EvTeo = EvDijkstra<A>(nodes, d, Algebra.Neutral, false);
        for(const auto &n : nodes) {
            if(n.E != n.Av || n.E != n.B || n.state != UPDATE || n.E != EvTeo[n.id]){
                ok = false;
                if(print) {
                    std::cout << "Correction assertion failed at ";
                    std::cout << "Node: " << n.id << "\n";
                    std::cout << "E: " << n.E << "B: " << n.B << "A: " << n.Av << " E_exp: " << EvTeo[n.id] << "\n";
                    std::cout << "state: " << (n.state == UPDATE ? "UPDATE" : "DIFFUSING") << "\n";
                    std::cout << n << "\n";
                }
                break;
            }
        }
    }
    else {
        for(const auto &n : nodes) {
            if(n.E != n.Av || n.E != n.B || n.state != UPDATE || n.E != Algebra.Invalid){
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
int EIGRPSync<A>::getPathLength(std::vector<NodeOneDest<A>> &nodes, int u, RoutingAlgebra<A> &Algebra, int d) {
    return nodes[u].longestPath;
}

template<class A> 
std::vector<int>& EIGRPSync<A>::getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u)
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid))
            timeInBlackHole[u] = -1;

    return timeInBlackHole;
}

template<class A> 
std::vector<int>& EIGRPSync<A>::getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u)
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid))
            nBlackHoles[u] = -1;

    return nBlackHoles;
}


#include "../Instances/eigrpSyncInstance.h"
