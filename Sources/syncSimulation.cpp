#include <vector>
#include <queue>
#include <unordered_map>

#include "../Headers/enums.h"
#include "../Headers/stats.h"
#include "../Headers/algebra.h"
#include "../Headers/nodeOneDest.h"
#include "../Headers/event.h"

#include "../Headers/utilities.h" // for createGraph()

#include "../Headers/eigrpSynchronous.h"
#include "../Headers/bgpSynchronous.h"

#include "../Headers/syncSimulation.h"

extern Stats *stats;
extern int nLinks;

/* If there is a link failure, removes all the events pertaining to the failed link from the queue of events */
template<class A> void removeFromQueueSync(std::vector<Event<A>> &Queue, int src, int dst) {
    for(auto ev = Queue.begin(); ev != Queue.end(); ++ev) {
        if(ev->from == dst && ev->to == src && (ev->type == UPDATE_MSG || ev->type == DIFFUSING_MSG) ) {
            Queue.erase(ev);
            stats->decrementCounters(ev->type);
            --ev;
        }
        else if(ev->from == src && ev->to == dst && ev->type == CLEAR_MSG) {
            Queue.erase(ev);
            stats->decrementCounters(ev->type);
            --ev;       
        }
    }
}


template<class P, class A> void simulateSyncAuxAdvertiseWithdraw(RoutingAlgebra<A> &Algebra,
                                                                 std::vector<NodeOneDest<A>> &nodes, 
                                                                 int dest=-1, 
                                                                 bool withdr=true) 
{
    
    /* Initialize the queue of events */
    std::vector<Event<A>> Q = {};
    
    /* P choice */
    P *protocol = new P();

    int start = 0, end = nodes.size() - 1;
    if(dest != -1) 
        start = end = dest;

    int longestPathAdv = 0, longestPathW = 0;

    for(int d = start; d <= end; ++d) {
        for(auto &n : nodes) 
            n.resetVariables();
        
        /* Node d announces attribute \epsilon */
        Q.push_back(Event<A>(ADVERTISE, d, d, Algebra.Neutral)); 

        /* Simulation time and other variables */
        int syncIters = -1; // Advertisement happens at iteration 0

        while(Q.size() != 0) {
            ++syncIters;

            for(auto &ev : Q) 
                /* Process the next event according to the protocol */
                protocol->processEvent(Algebra, ev, nodes, syncIters, !(withdr));
                // protocol->processEvent(Algebra, ev, nodes, syncIters);

            Q.clear();
            /* Events to be processed in the next synchronous iteration */
            Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters, !(withdr));
            // Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters);
            
            /* Update Statistic counters */
            for(const auto &e : Q)             
                stats->incrementCounters(e.type);     
        }

        int LPath = 0;
        for(const auto &n : nodes) 
            LPath = std::max(LPath, n.longestPath);
        longestPathAdv = std::max(longestPathAdv, LPath);

        stats->saveAndReset(d, syncIters, 0, -1);
        stats->saveCycleBlackHole(d, syncIters, 0, -1, 
                                  protocol->getCycleBlackHoleTimes(nodes, Algebra),
                                  protocol->getCycleBlackHoleChanges(nodes, Algebra));

        bool ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
        if(!ok) {
            std::cout << "SOMETHING WENT WRONG (ADVERTISE)! - protocol assertion\n";
            std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
        }

        /*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/
        if(!withdr) 
            continue;

        /* Node d Withdraws the destination */
        Q.push_back(Event<A>(WITHDRAWAL, d, d)); 
        syncIters = -1; // Withdrawal happens at iteration 0

        while(Q.size() != 0) {
            ++syncIters;

            for(auto &ev : Q) 
                protocol->processEvent(Algebra, ev, nodes);

            Q.clear();
            Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters);
            
            for(const auto &e : Q)             
                stats->incrementCounters(e.type);     
        }

        LPath = 0;
        for(const auto &n : nodes)
            LPath = std::max(LPath, n.longestPath);
        longestPathW = std::max(longestPathW, LPath);

        stats->saveAndReset(d, syncIters, 1, -1);
        stats->saveCycleBlackHole(d, syncIters, 1, -1, 
                                  protocol->getCycleBlackHoleTimes(nodes, Algebra),
                                  protocol->getCycleBlackHoleChanges(nodes, Algebra));

        ok = protocol->assertCorrectness(nodes, Algebra, WITHDRAWAL, d, true);
        if(!ok) {
            std::cout << "SOMETHING WENT WRONG (WITHDRAWAL)! - protocol assertion\n";
            std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
        }
    }

    std::cout << "Longest Path discovered (Advertise): " << longestPathAdv << "\n";
    if(withdr)
        std::cout << "Longest Path discovered (Withdrawal): " << longestPathW << "\n";

    delete protocol;
}


template<class P, class A> void simulateSyncAuxLinkFailure(RoutingAlgebra<A> &Algebra,
                                                           std::vector<NodeOneDest<A>> &nodes, 
                                                           int dest=-1)
{
    
    /* Initialize the queue of events */
    std::vector<Event<A>> Q = {};

    /* P choice */
    P *protocol = new P();

    int start = 0, end = nodes.size() - 1;
    if(dest != -1) 
        start = end = dest;

    int longestPath = 0, longestPathLF = 0;

    /* Repeat for each node in the network or for a single node only */
    for(int d = start; d <= end; ++d) {
        /* Reset node information */
        for(auto &n : nodes) 
            n.resetVariables();
        /* Node d announces attribute \epsilon */
        Q.push_back(Event<A>(ADVERTISE, d, d, Algebra.Neutral)); 
        /* Simulation time */
        int syncIters = -1; // Announcement happens at iteration 0
    
        while(Q.size() != 0) {
            ++syncIters;

            for(auto &ev : Q) {
                /* Process the next event according to the protocol */
                protocol->processEvent(Algebra, ev, nodes);
            }

            Q.clear();
            Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters);
        } 

        /* Do not save statistics about the announcement */
        stats->reset();

        int LPath = 0;
        for(const auto &n : nodes) {
            int path = protocol->getPathLength(nodes, n.id, Algebra, d);
            LPath = std::max(LPath, path);
        }
        longestPath = std::max(longestPath, LPath);

        bool ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
        if(!ok) {
            std::cout << "SOMETHING WENT WRONG (ADVERTISE)! - protocol assertion\n";
            std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
        }
        
        int linkCounter = -1;

        /* START OF LINK FAILURE PHASE */
        for(auto &u : nodes) {
            for(int v = u.id + 1; v < (int) nodes.size(); ++v) {
                // if(u.id > 0 || v > 1) continue;
                /* Find the next link to remove */
                auto it = u.outNeighbours.find(v);
                if(it == u.outNeighbours.end()) continue;
                ++linkCounter;
                syncIters = 0;
                /* Save information about the link - attributes of link u->v and v->u */
                auto attr1 = it->second, attr2 = nodes[v].outNeighbours.at(u.id);
                
                /* Simulate Failure of bidirectional link u-v */
                Q.push_back(Event<A>(LINK_FAILURE, u.id, v));  
                Q.push_back(Event<A>(LINK_FAILURE, v, u.id)); 

                while(Q.size() != 0) {
                    ++syncIters;

                    for(auto &ev : Q) {
                        if(ev.type == LINK_FAILURE) {
                            nodes[ev.to].removeInLink(ev.from);
                            nodes[ev.from].removeOutLink(ev.to);
                            removeFromQueueSync<A>(Q, ev.from, ev.to);
                        }
                        /* Process the next event according to the protocol */
                        protocol->processEvent(Algebra, ev, nodes, syncIters, true);
                        // protocol->processEvent(Algebra, ev, nodes, syncIters);
                    }
                    Q.clear();
                    Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters, true);
                    // Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters);
                    
                    for(const auto &e : Q)             
                        stats->incrementCounters(e.type);     
                }

                LPath = 0;
                for(const auto &n : nodes) {
                    int path = protocol->getPathLength(nodes, n.id, Algebra, d);
                    LPath = std::max(LPath, path);
                }
                longestPathLF = std::max(longestPathLF, LPath);

                stats->saveAndReset(d, syncIters, 2, linkCounter);
                stats->saveCycleBlackHole(d, syncIters, 2, linkCounter, 
                                          protocol->getCycleBlackHoleTimes(nodes, Algebra),
                                          protocol->getCycleBlackHoleTimes(nodes, Algebra));

                ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
                if(!ok) {
                    std::cout << "SOMETHING WENT WRONG (LINK FAILURE)! - protocol assertion\n";
                    std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
                }

                /* Add link u-v back to the network and wait for the protocol to reconverge */
                Q.push_back(Event<A>(LINK_ADDITION, u.id, v, attr1));  
                Q.push_back(Event<A>(LINK_ADDITION, v, u.id, attr2)); 
                syncIters = -1;

                while(Q.size() != 0) {
                    ++syncIters;
                    for(auto &ev : Q) {
                        if(ev.type == LINK_ADDITION) {
                            nodes[ev.to].addInLink(ev.from, ev.cost);
                            nodes[ev.from].addOutLink(ev.to, ev.cost);
                        }
                        /* Process the next event according to the protocol */
                        protocol->processEvent(Algebra, ev, nodes);
                    }
                    Q.clear();
                    Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters);
                }

                /* Do not save statistics about the link addition */
                stats->reset();

                ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
                if(!ok) {
                    std::cout << "SOMETHING WENT WRONG (LINK ADDITION)! - protocol assertion\n";
                    std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
                }
            }
        }
    }

    std::cout << "Longest Path discovered (Advertise): " << longestPath << "\n";
    std::cout << "Longest Path discovered (Link Failure): " << longestPathLF << "\n";

    delete protocol;
}


template<class P, class A> void simulateSyncAuxLinkFailureDisconnect(RoutingAlgebra<A> &Algebra,
                                                                     std::vector<NodeOneDest<A>> &nodes, 
                                                                     int dest=-1)
{

    // Get the pairs of links that disconnect the network
    std::vector<std::pair<std::pair<int,int>, std::pair<int,int>>> linkPairs;
    /* Procedure: remove one link of the network, link1; then, iterate over all links that are lexicographically
      bigger than link1 (for example, link 2-4 is lexicographically bigger than link 1-4); for each such link, link2,
      remove it and check if the network remains connected. If not, add the pair link1, link2 to the variable linkPairs:
      the failure of this pair of nodes disconnects the network; add back link2 and repeat the procedure for another 
      link2; add back link1 and repeat for every node in the network. */
    // First Link
    for(auto &u : nodes) {
        for(int v = u.id + 1; v < (int) nodes.size(); ++v) {
            auto it1 = u.outNeighbours.find(v);
            if(it1 == u.outNeighbours.end()) continue;
            auto attr11 = it1->second, attr12 = nodes[v].outNeighbours.at(u.id);
            u.removeInLink(v);
            nodes[v].removeOutLink(u.id);
            u.removeOutLink(v);
            nodes[v].removeInLink(u.id);
            // Second Link
            for(int x = u.id; x < (int) nodes.size(); ++x) {
                for(int w = (x == u.id ? v + 1 : x + 1); w < (int) nodes.size(); ++w) {
                    auto it2 = nodes[x].outNeighbours.find(w);
                    if(it2 == nodes[x].outNeighbours.end()) continue;
                    auto attr21 = it2->second, attr22 = nodes[w].outNeighbours.at(x);
                    nodes[x].removeInLink(w);
                    nodes[w].removeOutLink(x);
                    nodes[x].removeOutLink(w);
                    nodes[w].removeInLink(x);

                    if(!isConnected<A>(nodes))
                        linkPairs.push_back(std::make_pair(std::make_pair(u.id, v), std::make_pair(x, w)));
                    
                    // Add back second link
                    nodes[x].addInLink(w, attr22);
                    nodes[w].addOutLink(x, attr22);
                    nodes[x].addOutLink(w, attr21);
                    nodes[w].addInLink(x, attr21);
                }
            }
            // Add back first link
            u.addInLink(v, attr12);
            nodes[v].addOutLink(u.id, attr12);
            u.addOutLink(v, attr11);
            nodes[v].addInLink(u.id, attr11);
        }
    }

    std::cout << "Number of failures: " << linkPairs.size() << "\n";
    stats->createDoubleLinkFailureVec(linkPairs.size());

    /* Start the simulation process. Similar to the case of a single link failure, but now
      a pair of links is removed at the same time */

    /* Initialize the queue of events */
    std::vector<Event<A>> Q = {};

    /* P choice */
    P *protocol = new P();

    int start = 0, end = nodes.size() - 1;
    if(dest != -1) 
        start = end = dest;

    int longestPath = 0, longestPathLF = 0;

    for(int d = start; d <= end; ++d) {
        for(auto &n : nodes) 
            n.resetVariables();

        Q.push_back(Event<A>(ADVERTISE, d, d, Algebra.Neutral)); 

        int syncIters = -1;
        while(Q.size() != 0) {
            ++syncIters;
            for(auto &ev : Q) 
                protocol->processEvent(Algebra, ev, nodes);
            Q.clear();
            Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters);
        } 

        stats->reset();

        int LPath = 0;
        for(const auto &n : nodes) {
            int path = protocol->getPathLength(nodes, n.id, Algebra, d);
            LPath = std::max(LPath, path);
        }
        longestPath = std::max(longestPath, LPath);

        bool ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
        if(!ok) {
            std::cout << "SOMETHING WENT WRONG (ADVERTISE)! - protocol assertion\n";
            std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
        }
        
        int linkCounter = -1;
        /* START OF LINK FAILURE PHASE */
        for(const auto &linkPair : linkPairs) {
            // first link to fail
            const auto &link1 = linkPair.first;
            int u1 = link1.first, v1 = link1.second;
            auto attr11 = nodes[u1].outNeighbours.at(v1), attr12 = nodes[v1].outNeighbours.at(u1);
            // second link to fail
            const auto &link2 = linkPair.second;
            int u2 = link2.first, v2 = link2.second;
            auto attr21 = nodes[u2].outNeighbours.at(v2), attr22 = nodes[v2].outNeighbours.at(u2);

            ++linkCounter;

            Q.push_back(Event<A>(LINK_FAILURE, u1, v1));  
            Q.push_back(Event<A>(LINK_FAILURE, v1, u1)); 

            Q.push_back(Event<A>(LINK_FAILURE, u2, v2));  
            Q.push_back(Event<A>(LINK_FAILURE, v2, u2)); 
            
            syncIters = 0;
            while(Q.size() != 0) {
                ++syncIters;
                for(auto &ev : Q) {
                    if(ev.type == LINK_FAILURE) {
                        nodes[ev.to].removeInLink(ev.from);
                        nodes[ev.from].removeOutLink(ev.to);
                        removeFromQueueSync<A>(Q, ev.from, ev.to);
                    }
                    /* Process the next event according to the protocol */
                    protocol->processEvent(Algebra, ev, nodes, syncIters, true);
                    // protocol->processEvent(Algebra, ev, nodes, syncIters);
                }
                Q.clear();
                Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters, true);
                // Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters);
                
                for(const auto &e : Q)             
                    stats->incrementCounters(e.type);     
            }

            LPath = 0;
            for(const auto &n : nodes) {
                int path = protocol->getPathLength(nodes, n.id, Algebra, d);
                LPath = std::max(LPath, path);
            }
            longestPathLF = std::max(longestPathLF, LPath);

            stats->saveAndReset(d, syncIters, 3, linkCounter);
            stats->saveCycleBlackHole(d, syncIters, 3, linkCounter, 
                                      protocol->getCycleBlackHoleTimes(nodes, Algebra),
                                      protocol->getCycleBlackHoleChanges(nodes, Algebra));

            ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
            if(!ok) {
                std::cout << "SOMETHING WENT WRONG (LINK FAILURE)! - protocol assertion\n";
                std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
            }

            /* Add the two links back */
            Q.push_back(Event<A>(LINK_ADDITION, u1, v1, attr11));  
            Q.push_back(Event<A>(LINK_ADDITION, v1, u1, attr12)); 

            Q.push_back(Event<A>(LINK_ADDITION, u2, v2, attr21));  
            Q.push_back(Event<A>(LINK_ADDITION, v2, u2, attr22)); 
            syncIters = -1;

            while(Q.size() != 0) {
                ++syncIters;
                for(auto &ev : Q) {
                    if(ev.type == LINK_ADDITION) {
                        nodes[ev.to].addInLink(ev.from, ev.cost);
                        nodes[ev.from].addOutLink(ev.to, ev.cost);
                    }
                    protocol->processEvent(Algebra, ev, nodes);
                }
                Q.clear();
                Q = protocol->SynchronousIteration(Algebra, nodes, d, syncIters); 
            }

            stats->reset();
            ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
            if(!ok) {
                std::cout << "SOMETHING WENT WRONG (LINK ADDITION)! - protocol assertion\n";
                std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
            }
        }
    }
    
    std::cout << "Longest Path discovered (Advertise): " << longestPath << "\n";
    std::cout << "Longest Path discovered (Link Failure): " << longestPathLF << "\n";

    delete protocol;

}


template<class A> void simulateSync(std::string file_path, int protocol, int metric, int dest, int test_type) {
    /* Initialize the routing algebra */
    RoutingAlgebra<A> Algebra = RoutingAlgebra<A>();

    /* Create the Network */
    std::vector<std::unordered_map<int, int>> aux; // will not be considered
    std::vector<NodeOneDest<A>> nodes = createGraph<A>(file_path, aux, aux, 6*(protocol-1)+metric-1, true); 
    if(nodes.size() == 0) return;
    if(dest >= (int) nodes.size()) {
        std::cout << "Invalid destination node '"<< dest <<"'\nExiting\n";
        return;
    }

    stats = new Stats(nodes.size());

    switch(test_type) {
        case 0:
            if(protocol == 1)
                simulateSyncAuxAdvertiseWithdraw<EIGRPSync<A>, A>(Algebra, nodes, dest);
            else
                simulateSyncAuxAdvertiseWithdraw<BGPSync<A>, A>(Algebra, nodes, dest);
            break;
        case 1:
            if(protocol == 1)
                simulateSyncAuxAdvertiseWithdraw<EIGRPSync<A>, A>(Algebra, nodes, dest, false);
            else
                simulateSyncAuxAdvertiseWithdraw<BGPSync<A>, A>(Algebra, nodes, dest, false);
            break;
        case 2:
            if(protocol == 1)
                simulateSyncAuxLinkFailure<EIGRPSync<A>, A>(Algebra, nodes, dest);
            else
                simulateSyncAuxLinkFailure<BGPSync<A>, A>(Algebra, nodes, dest);
            break;
        case 3:
            if(protocol == 1)
                simulateSyncAuxLinkFailureDisconnect<EIGRPSync<A>, A>(Algebra, nodes, dest);
            else
                simulateSyncAuxLinkFailureDisconnect<BGPSync<A>, A>(Algebra, nodes, dest);
            break;
        default:
        break;
    }
}


#include "../Instances/SyncSimulationInstance.h"