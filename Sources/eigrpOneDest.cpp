
#include <fstream>
#include <unordered_map>
#include <iostream>

#include "../Headers/stats.h"
#include "../Headers/utilities.h"
#include "../Headers/eigrpOneDest.h"

extern Stats *stats;

extern int nNodes;

extern std::ofstream &file_out;

// #define DEBUG

std::vector<bool> auxInBlackHoleEIGRP;
std::vector<bool> changeStateEIGRP;
std::vector<bool> firstAttrChangeEIGRP;

template<class A> EIGRPOneDest<A>::EIGRPOneDest() {
    inBlackHole = std::vector<bool>(nNodes, false);
    blackHoleFormed = std::vector<int>(nNodes, 0);
    timeInBlackHole = std::vector<int>(nNodes, 0);
    auxInBlackHoleEIGRP = std::vector<bool>(nNodes, false);
    nBlackHoles = std::vector<int>(nNodes, 0);
    changeStateEIGRP = std::vector<bool>(nNodes, false);
    firstAttrChangeEIGRP = std::vector<bool>(nNodes, false);
}

template <class A> 
std::vector<Event<A>> EIGRPOneDest<A>::processUpdateMessage(NodeOneDest<A> &u, int v, int d, const A &v_attr, RoutingAlgebra<A> &Alg, int t) {
    
    std::vector<Event<A>> new_events;
    #ifdef DEBUG
    std::cout << u.id << " received <U," << v_attr << "> from " << v << "\n";
    #endif
    // Extension of the cost advertised by v with the uv link cost
    u.aTab[v] = Alg.Extend(u.outNeighbours[v], v_attr);
    #ifdef DEBUG
    std::cout << "After extension: " << u.aTab.at(v) << "\n";
    #endif

    /* Node u is in the UPDATE state */
    if(u.state == UPDATE) {
        // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
        u.Av = u.O;
        u.succ = -1;
        for(const auto & [node, attribute] : u.aTab) 
            if(Alg.Preferred(attribute, u.Av)) {
                u.Av = attribute;
                u.succ = node; // Node from which the best elected attribute is learnt
            }
        
        // If the most preferred attribute changed
        if(!Alg.Equal(u.Av, u.E)) {
            stats->stateChange(u.id, t);
            stats->attributeChange(u.id, t);
            changeStateEIGRP[u.id] = true;
            firstAttrChangeEIGRP[u.id] = true;
            /* If A'_u < E_u or u has no in-neighbours */
            if(Alg.Preferred(u.Av, u.E) || u.inNeighbours.size() == 0) {
                #ifdef DEBUG
                std::cout << u.id << " remains in U state: Av = " << u.Av << "\n";
                std::cout << "Previously E = " << u.E << "; B = " << u.B << "\n ";
                #endif
                u.E = u.Av; // update elected attribute
                u.B = u.Av; // update best attribute

                /* Send UPDATE message to all in-neighbours */
                for(const auto &[in_neigh, cost] : u.inNeighbours) {
                    new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
                    #ifdef DEBUG
                    std::cout << "\t" << u.id << " sends <U," << u.E << "> to " << in_neigh << "\n";
                    #endif
                }
            }
            /* If the new elected attribute is less preferred to the previous one 
            * and the Node has in-neighbours */
            else {
                /**** Update statistic counters ****/
                stats->updateIsotonicityBreak(u.id);
                stats->updateTransitions(u.id);

                stats->stateChange(u.id, t);
                stats->attributeChange(u.id, t);

                /* Node u goes to DIFFUSING state */
                u.state = DIFFUSING;
                u.E = u.aTab.at(v);
                // u.B remains the same as it was prior to the transition to DIFFUSING state
                u.succ = v;
                u.sendClear = false;
                #ifdef DEBUG
                std::cout << u.id << " transits to D state: Av = " << u.Av << "; E =  " << u.E << "; B = " << u.B << "\n";
                #endif
                /* Starts a diffusing computation: sends a DIFFUSING message to every in-neighbour */
                for(const auto &[in_neigh,cost] : u.inNeighbours) {
                    new_events.push_back(Event<A>(DIFFUSING_MSG, u.id, in_neigh, d, u.E));
                    u.clear[in_neigh] = false;
                    #ifdef DEBUG
                    std::cout << "\t" << u.id << " sends <D," << u.E << "> to " << in_neigh << "\n";
                    #endif
                }
            }
        }
        /* else, A'_u == E_u; do nothing */
    }
    /* else, state_u == DIFFUSING; do nothing */
    #ifdef DEBUG
    std::cout << "\n";
    #endif

    return new_events;
}


template <class A> 
std::vector<Event<A>> EIGRPOneDest<A>::processDiffusingMessage(NodeOneDest<A> &u, int v, int d, const A &v_attr, RoutingAlgebra<A> &Alg, int t) {
    
    std::vector<Event<A>> new_events;

    // Extension of the cost advertised by v with the uv link cost
    u.aTab[v] = Alg.Extend(u.outNeighbours[v], v_attr);
    #ifdef DEBUG
    std::cout << u.id << " received <D," << v_attr << "> from " << v << "\n";
    std::cout << "After extension: " << u.aTab.at(v) << "\n";
    #endif

    /* Node u is in the UPDATE state */
    if(u.state == UPDATE) {
        // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
        int succ = -1;
        u.Av = u.O;
        for(const auto & [Node, attribute] : u.aTab) {
            if(Alg.Preferred(attribute, u.Av)) {
                u.Av = attribute;
                succ = Node;
            }
        }
        /* If A'_u <= E_u or the u has no in-neighbours */
        if(!Alg.Preferred(u.E, u.Av) || u.inNeighbours.size() == 0) {
            
            /* If A'_u != E_u */
            if(!Alg.Equal(u.Av, u.E)) {
                stats->stateChange(u.id, t);
                stats->attributeChange(u.id, t);
                changeStateEIGRP[u.id] = true;
                #ifdef DEBUG
                std::cout << u.id << " remains in U state: Av = " << u.Av << "\n";
                std::cout << "Previously E = " << u.E << "; B = " << u.B << "\n ";
                #endif

                u.E = u.Av;
                u.B = u.Av;
                /* Send UPDATE message to all in-neighbours */
                for(const auto &[in_neigh,cost] : u.inNeighbours) {
                    new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
                    #ifdef DEBUG
                    std::cout << "\t" << u.id << " sends <U," << u.E << "> to " << in_neigh << "\n";
                    #endif
                }
            }

            // send <Clear> to v;
            new_events.push_back(Event<A>(CLEAR_MSG, u.id, v, d));
            #ifdef DEBUG
            std::cout << "\t" << u.id << " sends <Clear> to " << v << "\n";
            #endif
            
            u.succ = succ; // Node from which the elected attribute is learnt
        }
        /* If the new elected attribute is less preferred to the previous one 
        * and the Node has in-neighbours */
        else {
            stats->stateChange(u.id, t);
            stats->attributeChange(u.id, t);
            stats->updateTransitions(u.id);
            changeStateEIGRP[u.id] = true;

            /* Node u goes to DIFFUSING state */
            u.state = DIFFUSING;
            u.E = u.aTab[v];
            u.succ = v;
            u.sendClear = true;

            #ifdef DEBUG
            std::cout << u.id << " transits to D state: Av = " << u.Av << "; E =  " << u.E << "; B = " << u.B << "\n";
            #endif

            /* Relays the diffusing computation: sends a DIFFUSING message to every in-neighbour */
            for(const auto &[in_neigh,cost] : u.inNeighbours) {
                new_events.push_back(Event<A>(DIFFUSING_MSG, u.id, in_neigh, d, u.E));
                u.clear[in_neigh] = false;
                #ifdef DEBUG
                std::cout << "\t" << u.id << " sends <D," << u.E << "> to " << in_neigh << "\n";
                #endif
            }
        }
    }
    /* state_u = DIFFUSING */
    else {
        if(v != u.succ) {
            // send <clear> to v;
            new_events.push_back(Event<A>(CLEAR_MSG, u.id, v, d));
            #ifdef DEBUG
            std::cout << "\t" << u.id << " sends <Clear> to " << v << "\n";
            #endif
        }
        else 
            u.sendClear = true;
    }

    #ifdef DEBUG
    std::cout << "\n";
    #endif

    return new_events;
}

template <class A> 
std::vector<Event<A>> EIGRPOneDest<A>::processClearMessage(NodeOneDest<A> &u, int v, int d, RoutingAlgebra<A> &Alg, int t) {

    std::vector<Event<A>> new_events;

    u.clear[v] = true;
    for(const auto &[in_neigh,clr] : u.clear) 
        if(!clr)
            /* clear_u[x] = False, for some in-neighbour x of u */
            return {};

    /* clear_u[x] = True, for all x in-neighbour of u */
    stats->stateChange(u.id, t);
    #ifdef DEBUG
    std::cout << u.id << " ready to return to U state\n";
    #endif
    // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours of u})
    int succ = -1;
    u.Av = u.O;
    for(const auto & [Node, attribute] : u.aTab) {
        if(Alg.Preferred(attribute, u.Av)) {
            u.Av = attribute;
            succ = Node;
        }
    }    
    /* If A'_u <= E_u */
    if(!Alg.Preferred(u.E, u.Av)) {
        u.state = UPDATE;        
        #ifdef DEBUG
        std::cout << u.id << " returns to the U state: Av = " << u.Av << "\n";
        #endif

        /* If A'_u != E_u */
        if(!Alg.Equal(u.Av, u.E)) {
            stats->attributeChange(u.id, t);
            changeStateEIGRP[u.id] = true;
        }

        u.E = u.Av; // update the elected attribute
        if(!Alg.Equal(u.E, Alg.Invalid)) {
            /* Send UPDATE message to all in-neighbours */
            for(const auto &[in_neigh,cost] : u.inNeighbours) {
                new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
                #ifdef DEBUG
                std::cout << "\t" << u.id << " sends <U," << u.E << "> to " << in_neigh << "\n";
                #endif
            }
        }

        if(u.sendClear) { // send <clear> to the successor;
            new_events.push_back(Event<A>(CLEAR_MSG, u.id, u.succ, d));
            #ifdef DEBUG
            std::cout << "\t" << u.id << " sends <Clear> to " << u.succ << "\n";
            #endif
        }
        
        u.B = u.Av;
        u.succ = succ;
    }
    /* A'_u > E_u */
    else {
        /* Start/ relay a new diffusion computation */
        stats->attributeChange(u.id, t);
        u.E = (u.succ == -1 ? u.O : u.aTab.at(u.succ));
        #ifdef DEBUG
        std::cout << u.id << " remains in D state: Av = " << u.Av << "; ";
        std::cout << "E = " << u.E << "; B = " << u.B << "\n ";
        #endif

        for(const auto &[in_neigh,cost] : u.inNeighbours) {
            new_events.push_back(Event<A>(DIFFUSING_MSG, u.id, in_neigh, d, u.E));
            u.clear[in_neigh] = false;
            #ifdef DEBUG
            std::cout << "\t" << u.id << " sends <D," << u.E << "> to " << in_neigh << "\n";
            #endif
        }
    }

    return new_events;
}

template <class A> 
std::vector<Event<A>> EIGRPOneDest<A>::processAdvertise(NodeOneDest<A> &u, int dest, A origin, RoutingAlgebra<A> &Alg, int t, bool check) {
    std::vector<Event<A>> new_events;

    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeStateEIGRP.begin(), changeStateEIGRP.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
        std::fill(firstAttrChangeEIGRP.begin(), firstAttrChangeEIGRP.end(), false);
    }

    /* update the origin attribute to the advertised destination */
    u.O = origin; 
    /* compute the new most preferred attribute */
    u.Av = u.O;
    u.succ = -1;
    for(const auto & [Node, attribute] : u.aTab) {
        if(Alg.Preferred(attribute, u.Av)) {
            u.Av = attribute;
            u.succ = Node;
        }
    }

    /* The same as in processUpdateMessage */
    /* Node u is in the UPDATE state and A'_u != E_u */
    if(u.state == UPDATE && !Alg.Equal(u.Av, u.E)) {
        stats->stateChange(u.id, t);
        stats->attributeChange(u.id, t);
        changeStateEIGRP[u.id] = true;
        firstAttrChangeEIGRP[u.id] = true;
        /* If A'_u < E_u or there are no in-neighbours */
        if(Alg.Preferred(u.Av, u.E) || u.inNeighbours.size() == 0) {
            u.E = u.Av;
            u.B = u.Av;
            /* Send UPDATE message to all in-neighbours */
            for(const auto &[in_neigh, cost] : u.inNeighbours) 
                new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, dest, u.E));
        }
        /* If A'_u > E_u and the node has in-neighbours */
        else {
            /* Node u goes to DIFFUSING state */
            u.state = DIFFUSING;
            u.E = u.O;
            u.succ = -1;
            u.sendClear = false;
            /* Starts a diffusion computation: sends a DIFFUSING message to every in-neighbour */
            for(const auto &[in_neigh,cost] : u.inNeighbours) {
                new_events.push_back(Event<A>(DIFFUSING_MSG, u.id, in_neigh, dest, u.E));
                u.clear[in_neigh] = false;
            }
        }
        /* else, A'_u == E_u; do nothing */
    }
    /* else, state_u == DIFFUSING; do nothing */

    return new_events;
}

template <class A> 
std::vector<Event<A>> EIGRPOneDest<A>::processWithdrawal(NodeOneDest<A> &u, int d, RoutingAlgebra<A> &Alg, int t, bool check) {
    std::vector<Event<A>> new_events;
    
    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeStateEIGRP.begin(), changeStateEIGRP.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
    }
    
    /* Just as processAdvertise, but now the origin attribute is set to infinity */
    u.O = Alg.Invalid; 
    /* compute the new most preferred attribute */
    u.Av = u.O;
    int succ = -1;
    for(const auto & [node, attribute] : u.aTab) {
        if(Alg.Preferred(attribute, u.Av)) {
            u.Av = attribute;
            succ = node;
        }
    }
    
    /* Node u is in the UPDATE state and A'_u != E_u */
    if(u.state == UPDATE && !Alg.Equal(u.Av, u.E)) {
        stats->stateChange(u.id, t);
        stats->attributeChange(u.id, t);
        changeStateEIGRP[u.id] = true;

        /* A'_u < E_u or u has no in-neighbours */
        if(Alg.Preferred(u.Av, u.E) || u.inNeighbours.size() == 0) {
            u.E = u.Av;
            u.B = u.Av;
            u.succ = succ;
            /* Send UPDATE message to all in-neighbours */
            for(const auto &[in_neigh, cost] : u.inNeighbours) 
                new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
        }
        /* A'_u > E_u and u has in-neighbours */
        else {
            stats->updateTransitions(u.id);
            /* Node u goes to DIFFUSING state */
            u.state = DIFFUSING;
            u.E = Alg.Invalid;
            u.succ = -1;
            u.sendClear = false;
            /* Starts a diffusion computation: sends a DIFFUSING message to every in-neighbour */
            for(const auto &[in_neigh,cost] : u.inNeighbours) {
                new_events.push_back(Event<A>(DIFFUSING_MSG, u.id, in_neigh, d, u.E));
                u.clear[in_neigh] = false;
            }
        }
        /* else, A'_u == E_u; do nothing */
    }
    /* else, state_u == DIFFUSING; do nothing */

    return new_events;
}

template <class A> 
std::vector<Event<A>> EIGRPOneDest<A>::processLinkCostChange(int u, NodeOneDest<A> &v, RoutingAlgebra<A> &Alg, int d, bool check) {
    
    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeStateEIGRP.begin(), changeStateEIGRP.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
    }
    
    /* link u->v */
    std::vector<Event<A>> new_events;
    /* v sends an update message to u if state_v = U and E_u != \bullet */
    if(v.state == UPDATE && !Alg.Equal(v.E, Alg.Invalid)) 
        new_events.push_back(Event<A>(UPDATE_MSG, v.id, u, d, v.E));
    return new_events;
}

template <class A> 
std::vector<Event<A>> EIGRPOneDest<A>::processLinkFailure(NodeOneDest<A> &u, NodeOneDest<A> &v, RoutingAlgebra<A> &Alg, int d, int t, bool check) {
    std::vector<Event<A>> new_events;

    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeStateEIGRP.begin(), changeStateEIGRP.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
    }
    
    /* Link u->v fails */

    /* Node v */
    /* u is removed from the set of in-neighbours of v */
    if(v.state == DIFFUSING) {
        bool clear = true;
        for(const auto &[in_neigh,clc] : v.clear) {
            if(!clc) {
                /* clear_v[x] = False, for some in-neighbour x of v */
                clear = false;
                break;
            }
        }
        /* clear_v[x] = True, for all in-neighbours x of v */
        if(clear) {
            /* Node v is ready to return to the UPDATE state */
            stats->stateChange(v.id, t);
            changeStateEIGRP[u.id] = true;
            // Compute Av = min({Ov} U {aTab[t] | t \in outNeighbours})
            v.Av = v.O;
            int succ = -1;
            for(const auto & [node, attribute] : v.aTab) {
                if(Alg.Preferred(attribute, v.Av)) {
                    v.Av = attribute;
                    succ = node;
                }
            }

            /* If A'_u <= E_u */
            if(!Alg.Preferred(v.E, v.Av)) {
                v.state = UPDATE;
                if(!Alg.Equal(v.E, v.Av)) 
                    stats->attributeChange(v.id, t);

                v.E = v.Av; // update the elected attribute
                if(v.E != Alg.Invalid) {
                    /* Send UPDATE message to all in-neighbours */
                    for(const auto &[in_neigh,cost] : v.inNeighbours) 
                        new_events.push_back(Event<A>(UPDATE_MSG, v.id, in_neigh, d, v.E));
                }
                if(v.sendClear) // send <clear> to the successor node;
                    new_events.push_back(Event<A>(CLEAR_MSG, v.id, v.succ, d));
                
                v.B = v.Av;
                v.succ = succ;          
            }
            else {
                /* Start / Relay a new diffusion computation */
                stats->attributeChange(v.id, t);
                v.E = (v.succ == -1) ? v.O : v.aTab.at(v.succ);
                for(const auto &[in_neigh,cost] : v.inNeighbours) {
                    new_events.push_back(Event<A>(DIFFUSING_MSG, v.id, in_neigh, d, v.E));
                    v.clear[in_neigh] = false;
                }
            }
        }
    }

    /* Node u */
    /* v is removed from the set of out-neighbours of u */
    if(u.state == UPDATE) {
        u.Av = u.O;
        u.succ = -1;
        for(const auto & [node, attribute] : u.aTab) {
            if(Alg.Preferred(attribute, u.Av)) {
                u.Av = attribute;
                u.succ = node;
            }
        }
        if(!Alg.Equal(u.Av, u.E)) {
            stats->stateChange(u.id, t);
            stats->attributeChange(u.id, t);

            if(Alg.Preferred(u.Av, u.E) || u.inNeighbours.size() == 0) {
                u.E = u.Av; 
                u.B = u.Av;

                /* Send UPDATE message to all in-neighbours */
                for(const auto &[in_neigh, cost] : u.inNeighbours)
                    new_events.push_back(Event<A>(UPDATE_MSG, u.id, in_neigh, d, u.E));
            }
            else {
                /* Start a new diffusion with an infinite cost and no successor */
                u.state = DIFFUSING;
                u.E = Alg.Invalid; 
                u.succ = -1;
                u.sendClear = false;
                /* Starts a diffusion computation: sends a DIFFUSING message to every in-neighbour */
                for(const auto &[in_neigh,cost] : u.inNeighbours) {
                    new_events.push_back(Event<A>(DIFFUSING_MSG, u.id, in_neigh, d, u.E));
                    u.clear[in_neigh] = false;
                }
            }  
        }          
    }
    else { // u.state == DIFFUSING
        if(v.id == u.succ) {
            stats->stateChange(u.id, t);
            if(!Alg.Equal(u.E, Alg.Invalid)) 
                stats->attributeChange(u.id, t);
            u.E = Alg.Invalid;
            u.succ = -1;
            u.sendClear = false;
        }
    }
    return new_events;
}

template <class A> 
std::vector<Event<A>> EIGRPOneDest<A>::processLinkAddition(int u, NodeOneDest<A> &v, RoutingAlgebra<A> &Alg, int d, bool check) {
    
    if(check) {
        std::fill(nBlackHoles.begin(), nBlackHoles.end(), 0);
        std::fill(changeStateEIGRP.begin(), changeStateEIGRP.end(), false);
        std::fill(inBlackHole.begin(), inBlackHole.end(), false);
        std::fill(blackHoleFormed.begin(), blackHoleFormed.end(), 0);
        std::fill(timeInBlackHole.begin(), timeInBlackHole.end(), 0);
    }
    
    /* link u->v */
    std::vector<Event<A>> new_events;
    /* v sends an update message to u if state_v = U and E_u != \bullet */
    if(v.state == UPDATE && !Alg.Equal(v.E, Alg.Invalid))
        new_events.push_back(Event<A>(UPDATE_MSG, v.id, u, d, v.E));

    return new_events;
}

template <class A> 
void EIGRPOneDest<A>::checkSafety(const NodeOneDest<A> &u, RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes) {
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

template <class A> 
void EIGRPOneDest<A>::checkCycle(int u, std::vector<NodeOneDest<A>> &nodes) {
    // check for a cycle
    int node = u, cnt = 0; 
    while(node != -1 && ++cnt <= (int)nodes.size()) {
        node = nodes[node].succ;
        if(node == u) {
            std::cout << "\nA CYCLE WAS DETECTED 1!\n";
            break;
        }
    }
    if(cnt > (int)nodes.size())
        std::cout << "A CYCLE WAS DETECTED 2!\n";
}

template <class A> 
bool EIGRPOneDest<A>::checkBlackHole(int u, RoutingAlgebra<A> &Algebra, std::vector<NodeOneDest<A>> &nodes) {
    if(!firstAttrChangeEIGRP[u]) {
        auxInBlackHoleEIGRP[u] = false;
        return false;
    }
    if(nodes[u].succ == -1 && Algebra.Equal(nodes[u].E, Algebra.Invalid)) {
        auxInBlackHoleEIGRP[u] = true;
        return true;
    } else if(nodes[u].succ == -1) {
        auxInBlackHoleEIGRP[u] = false;
        return false;
    }
    if(auxInBlackHoleEIGRP[u]) return true;
    if(nodes[u].state == UPDATE && Algebra.Equal(nodes[u].E, Algebra.Invalid)) {
        auxInBlackHoleEIGRP[u] = true;
        return true;
    } else if(nodes[u].state == UPDATE && !Algebra.Equal(nodes[u].E, Algebra.Invalid)) {
        for(const auto &[v, attr] : nodes[u].outNeighbours) {
            if(Algebra.Equal(nodes[u].aTab.at(v), nodes[u].E)) {
                if(checkBlackHole(v, Algebra, nodes)) {
                    auxInBlackHoleEIGRP[u] = true;
                    return true;
                }
            }
        }
    } else if(nodes[u].state == DIFFUSING && nodes[u].succ == -1) {
        auxInBlackHoleEIGRP[u] = true;
        return true;
    } else if(nodes[u].state == DIFFUSING && nodes[u].succ != -1) {
        if(checkBlackHole(nodes[u].succ, Algebra, nodes)) {
            auxInBlackHoleEIGRP[u] = true;
            return true;
        }
    }
    auxInBlackHoleEIGRP[u] = false;
    return false;
}

template <class A> 
std::vector<Event<A>> EIGRPOneDest<A>::processEvent(RoutingAlgebra<A> &Algebra, Event<A> &event, std::vector<NodeOneDest<A>> &nodes, int t, bool check) {
    EventType type = event.type;
    int u = event.from;
    int v = event.to;
    int d = event.dest;
    A attr = event.cost;

    std::vector<Event<A>> ret;

    switch(type) {
        case ADVERTISE:
            ret = processAdvertise(nodes[u], d, attr, Algebra, t, check);
            break;
        case WITHDRAWAL:
            ret = processWithdrawal(nodes[u], d, Algebra, t, check);
            break;
        case UPDATE_MSG:
            ret = processUpdateMessage(nodes[v], u, d, attr, Algebra, t);
            break;
        case DIFFUSING_MSG:
            ret = processDiffusingMessage(nodes[v], u, d, attr, Algebra, t);
            break;
        case CLEAR_MSG:
            ret = processClearMessage(nodes[v], u, d, Algebra, t);
            break;
        case LINK_COST_CHANGE:
            ret = processLinkCostChange(u, nodes[v], Algebra, d, check);
            break;
        case LINK_FAILURE:
            ret = processLinkFailure(nodes[u], nodes[v], Algebra, d, t, check);
            break;
        case LINK_ADDITION:
            ret = processLinkAddition(u, nodes[v], Algebra, d, check);
            break;
        default:
            break;
    }

    if(type == ADVERTISE || type == WITHDRAWAL || type == LINK_FAILURE) {
        int path_len = -1;
        if(!Algebra.Equal(nodes[u].E, Algebra.Invalid)) {
            path_len = 1;
            for(int succ = nodes[u].succ; succ != -1; ++path_len, succ = nodes[succ].succ);
        }
        nodes[u].longestPath = std::max(nodes[u].longestPath, path_len);
    }
    if(type == UPDATE_MSG || type == DIFFUSING_MSG || type == CLEAR_MSG || type == LINK_FAILURE) {
        int path_len = -1;
        if(!Algebra.Equal(nodes[v].E, Algebra.Invalid)) {
            path_len = 1;
            for(int succ = nodes[v].succ; succ != -1; ++path_len, succ = nodes[succ].succ);
        }
        nodes[v].longestPath = std::max(nodes[v].longestPath, path_len);
    }

    if(check) {
        std::fill(auxInBlackHoleEIGRP.begin(), auxInBlackHoleEIGRP.end(), false);
        /* Checks if a black hole is formed */
        for(auto & n: nodes) {
            if(checkBlackHole(n.id, Algebra, nodes)) {
                if(changeStateEIGRP[n.id]) {
                    ++nBlackHoles[n.id];
                    changeStateEIGRP[n.id] = false;
                }
                if(!inBlackHole[n.id]) {
                    inBlackHole[n.id] = true;
                    blackHoleFormed[n.id] = t;
                }
            }
            else {
                if(inBlackHole[n.id]) {
                    timeInBlackHole[n.id] += (t - blackHoleFormed[n.id]);
                    inBlackHole[n.id] = false;
                }
            }  
        }
    }

    /* Checks if the Safety Properties hold and if a cycle was formed */
    checkSafety(nodes[u], Algebra, nodes);
    checkSafety(nodes[v], Algebra, nodes);
    checkCycle(u, nodes);
    checkCycle(v, nodes);   

    return ret;
}

template <class A> 
bool EIGRPOneDest<A>::assertCorrectness(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra, EventType event, int d, bool print) {
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
                    std::cout << "state: " << n.state << "\n";
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

template <class A> 
int EIGRPOneDest<A>::getPathLength(std::vector<NodeOneDest<A>> &nodes, int u, RoutingAlgebra<A> &Algebra, int d) {
    return nodes[u].longestPath;
}

template<class A> 
std::vector<int>& EIGRPOneDest<A>::getCycleBlackHoleTimes(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u) 
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid))
            timeInBlackHole[u] = -1;

    return timeInBlackHole;
}

template<class A> 
std::vector<int>& EIGRPOneDest<A>::getCycleBlackHoleChanges(std::vector<NodeOneDest<A>> &nodes, RoutingAlgebra<A> &Algebra) {
    for(int u = 0; u < nNodes; ++u)
        if(Algebra.Equal(nodes[u].E, Algebra.Invalid))
            nBlackHoles[u] = -1;

    return nBlackHoles;
}

#include "../Instances/eigrpOneDestInstance.h"