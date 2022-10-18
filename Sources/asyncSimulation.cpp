#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <random>
#include <array>

#include "../Headers/enums.h"
#include "../Headers/stats.h"

#include "../Headers/nodeOneDest.h"
#include "../Headers/algebra.h"
#include "../Headers/event.h"
#include "../Headers/eventQueueNode.h"

#include "../Headers/eigrpOneDest.h"
#include "../Headers/eigrpOneDestFC.h"
#include "../Headers/bgpOneDest.h"
#include "../Headers/bgpQuasiSync.h"

#include "../Headers/vectoringProtocol.h"

#include "../Headers/utilities.h"

#include "../Headers/asyncSimulation.h"

extern Stats *stats;
extern unsigned long GseqNum;
extern int nLinks;

extern std::mt19937 rng;    // random-number engine used (Mersenne-Twister in this case)
extern std::uniform_int_distribution<int> uniform1; 
extern std::uniform_int_distribution<int> uniform2;
extern std::normal_distribution<double> gaussian;

// #define MRAI 30000 // election delay for BGP Quasi-sync. Acts as a MRAI
#define CONSTDELAY 1

/* If there is a link failure, removes all the events pertaining to the failed link from the queue of events */
template<class E>
std::priority_queue<E> removeFromQueue(std::priority_queue<E> &Queue, int src, int dst) {
    std::priority_queue<E> newQueue = {};
    while(!Queue.empty()) {
        auto [t,ev,seq] = Queue.top();
        Queue.pop();
        if(ev.from == dst && ev.to == src && (ev.type == UPDATE_MSG || ev.type == DIFFUSING_MSG) ) {
            stats->decrementCounters(ev.type);
            continue;
        }
        if(ev.from == src && ev.to == dst && ev.type == CLEAR_MSG) {
            stats->decrementCounters(ev.type);
            continue;
        }
        newQueue.push({t,ev,seq});
    }
    return newQueue;
}

long addDelay(std::vector<std::unordered_map<int,int>> &delays, int from, int to, int delay_opt) {
    /* delays in microseconds (us) */
    if(delay_opt == 0) { 
        // fixed delay of 1 milisecond
        return 1000*CONSTDELAY;
    }
    if(delay_opt == 1) { 
        // added delay equal to the link delay
        return 1000*delays[from].at(to);
    }
    if(delay_opt == 2) { 
        /* added delay from an uniform distribution around the link delay 
          (0.75 to 1.25 of the link delay) */
        return 1000*uniform2(rng)*delays[from].at(to)/100;
    }
    if(delay_opt == 3) { 
        // added delay from an uniform distribution (1 + [0:0.05:2]) [ms]
        return 1000*CONSTDELAY + 50 * uniform1(rng); 
    }
    return 1000*CONSTDELAY;
}

/* Variable that ensures FIFO order in the messages sent by a node to a neighbour */
void resetFIFOtimes(std::vector<std::unordered_map<int,int>> &fifo_times) {
    for(auto &mp : fifo_times) 
        for(auto &[neigh, time] : mp) 
            time = 0;
}


/* Simulates the Announcement of a destination/prefix and its withdrawal;
  Can be repeated for every node in the network ('dest' argument set to -1), or for a 
  single node, in which case the 'dest' argument is the ID of that node;
  Allows for only the announcement phase to take place ('withdr' argument set to false). 
  In that case, once a stable state is reached after a node announces a destination,
  all the state variables of every node are manually reset, and a new node announces a destination 
  (if 'dest' = -1; contrarily, return) */
template<class P, class A> void simulateAsyncAuxAdvertiseWithdrawOneDest(RoutingAlgebra<A> &Algebra,
                                                                         std::vector<NodeOneDest<A>> &nodes, 
                                                                         std::vector<std::unordered_map<int,int>> &fifo_times,
                                                                         std::vector<std::unordered_map<int,int>> &delays,
                                                                         int dest=-1, 
                                                                         bool withdr=true,
                                                                         int delay_opt=1 )
{
    
    /* Initialize the queue of events */
    std::priority_queue < EventQueueNode<long, Event<A>> > Q = {};

    /* Protocol choice */
    P *protocol = new P();

    int start = 0, end = nodes.size() - 1;
    if(dest != -1) 
        start = end = dest;

    int longestPathAdv = 0, longestPathW = 0;

    /* REPEAT FOR EACH NODE */
    for(int d = start; d <= end; ++d) {

        for(auto &n : nodes) 
            n.resetVariables();
        /* Simulation time and other variables */
        long sim_time = 0, new_time = 0, prev_time = 0;
        /* ADVERTISE PHASE : Node d Announces attribute \epsilon */
        Q.push({0, Event<A>(ADVERTISE, d, d, Algebra.Neutral)}); 
                
        while(!Q.empty()) {
            // get next event from queue
            auto [t,ev,seq] = Q.top(); 
            Q.pop();            
            // current simulation time
            sim_time = t; 
            /* Process the next event according to the protocol */
            std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t, !withdr);
            // std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t);
            
            /* Add each new event to the Event Queue */
            for(const auto &e : ret) {
                if(e.type == ELECT) {
                    // only in BGP Quasi Sync
                    Q.push({t+e.to, e});
                }
                else {
                    long delay = addDelay(delays, e.from, e.to, delay_opt);
                    if(delay_opt > 1) {
                        // only needed if link delays are variable
                        prev_time = fifo_times[e.from].at(e.to);
                        new_time = std::max(prev_time, t + delay);
                        fifo_times[e.from][e.to] = new_time;
                    } else
                        new_time = t + delay;
                    Q.push({new_time, e});
                    
                    stats->incrementCounters(e.type);
                }
            }

        }

        int LPath = 0;
        for(const auto &n : nodes) 
            LPath = std::max(LPath, n.longestPath);
        longestPathAdv = std::max(longestPathAdv, LPath);

        stats->saveAndReset(d, sim_time, 0, -1);
        stats->saveCycleBlackHole(d, sim_time, 0, -1, 
                                  protocol->getCycleBlackHoleTimes(nodes, Algebra),
                                  protocol->getCycleBlackHoleChanges(nodes, Algebra));

        if(delay_opt > 1) 
            resetFIFOtimes(fifo_times);
        GseqNum = 0;

        bool ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d);
        if(!ok) {
            std::cout << "SOMETHING WENT WRONG (ADVERTISE)! - protocol assertion\n";
            std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
        }
    
        /* If withdrawal flag is set to false, continue to the next node, without withdrawing the destination */
        if(!withdr) 
            continue;

        /**********************************************************************************************/
        sim_time = 0;

        /* WITHDRAW PHASE : Node d Withdraws itself */
        Q.push({0, Event<A>(WITHDRAWAL, d, d)}); 

        while(!Q.empty()) {
            auto [t,ev,seq] = Q.top();
            Q.pop();

            sim_time = t;

            std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t);
            
            for(const auto &e : ret) {
                if(e.type == ELECT) 
                    Q.push({t+e.to, e});
                else {
                    long delay = addDelay(delays, e.from, e.to, delay_opt);
                    if(delay_opt > 1) {
                        // only needed if link delays are variable
                        prev_time = fifo_times[e.from].at(e.to);
                        new_time = std::max(prev_time, t + delay);
                        fifo_times[e.from][e.to] = new_time;
                    } else
                        new_time = t + delay;
                    Q.push({new_time, e});
                    
                    stats->incrementCounters(e.type);
                }
            }
        }

        LPath = 0;
        for(const auto &n : nodes)
            LPath = std::max(LPath, n.longestPath);
        longestPathW = std::max(longestPathW, LPath);

        stats->saveAndReset(d, sim_time, 1, -1);
        if(delay_opt > 1) 
            resetFIFOtimes(fifo_times);
        GseqNum = 0;

        ok = protocol->assertCorrectness(nodes, Algebra, WITHDRAWAL, d);
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

/* Simulates the failure of a link, over all links in the network; Can be repeated for every node in 
  the network as the announcer of a destination ('dest' argument set to -1), or for a single node, in which 
  case the 'dest' argument is the ID of that node; The termination time per node, per link failure, measured 
  as the instant of time when the node changed attribute for the last time, is the maximum over all destinations;
  Messages exchanged per link failure are the sum of the messages exchanged for that link failure over all 
  destinations; The time it takes to reach a stable state per link failure is the maximum over all destiantions */
template<class P, class A> void simulateAsyncAuxAdvertiseLinkFailureOneDest(RoutingAlgebra<A> &Algebra,
                                                                            std::vector<NodeOneDest<A>> &nodes, 
                                                                            std::vector<std::unordered_map<int,int>> &fifo_times,
                                                                            std::vector<std::unordered_map<int,int>> &delays,
                                                                            int dest=-1, 
                                                                            int delay_opt=1 )
{
    
    /* Initialize the queue of events */
    std::priority_queue < EventQueueNode<long, Event<A>> > Q = {};

    int longestPath = 0, longestPathLF = 0;

    /* Protocol choice */
    P *protocol = new P();

    int start = 0, end = nodes.size() - 1;
    if(dest != -1) 
        start = end = dest;

    /* Simulation time and other variables */
    long sim_time = 0, new_time = 0, prev_time = 0;
    bool ok;

    /* Repeat for every node (if 'dest' == -1), of for a single node (otehrwise) */
    for(int d = start; d <= end; ++d) {
        /* Reset node information */
        for(auto &n : nodes) 
            n.resetVariables();

        sim_time = 0;
        /* Node d announces attribute \epsilon */
        Q.push({0, Event<A>(ADVERTISE, d, d, Algebra.Neutral)}); 
    
        while(!Q.empty()) {
            auto [t,ev,seq] = Q.top(); 
            Q.pop();

            sim_time = t; 

            std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t);
            
            for(const auto &e : ret) {
                if(e.type == ELECT) {
                    Q.push({t+e.to, e});
                }
                else {
                    long delay = addDelay(delays, e.from, e.to, delay_opt);
                    if(delay_opt > 1) {
                        // only needed if link delays are variable
                        prev_time = fifo_times[e.from].at(e.to);
                        new_time = std::max(prev_time, t + delay);
                        fifo_times[e.from][e.to] = new_time;
                    } else
                        new_time = t + delay;
                    Q.push({new_time, e});
                }
            }
        }

        stats->reset();
        if(delay_opt > 1) 
            resetFIFOtimes(fifo_times);
        GseqNum = 0;

        int LPath = 0;
        for(const auto &n : nodes) 
            LPath = std::max(LPath, n.longestPath);
        longestPath = std::max(longestPath, LPath);


        ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
        if(!ok) {
            std::cout << "SOMETHING WENT WRONG (ADVERTISE)! - protocol assertion\n";
            std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
        }
        /* END OF ADVERTISEMENT PHASE */

        int linkCounter = -1;
        sim_time = 0;
        /* START OF LINK FAILURE PHASE */
        for(auto &u : nodes) {
            for(int v = u.id + 1; v < (int) nodes.size(); ++v) {
                // if(u.id > 0 || v > 1) continue;
                // find the next link to remove
                auto it = u.outNeighbours.find(v);
                if(it == u.outNeighbours.end()) continue;
                // save link attributes for later
                auto attr1 = it->second, attr2 = nodes[v].outNeighbours.at(u.id);
                // remove the bidirectional link u-v
                u.removeInLink(v);
                nodes[v].removeOutLink(u.id);
                u.removeOutLink(v);
                nodes[v].removeInLink(u.id);
                Q = removeFromQueue<EventQueueNode<long, Event<A>>>(Q, u.id, v);
                Q = removeFromQueue<EventQueueNode<long, Event<A>>>(Q, v, u.id);

                ++linkCounter;

                /* Simulate Failure of bidirectional link u-v */
                Q.push({0, Event<A>(LINK_FAILURE, u.id, v)});  
                Q.push({0, Event<A>(LINK_FAILURE, v, u.id)}); 

                while(!Q.empty()) {
                    auto [t,ev,seq] = Q.top(); // get next event from queue
                    Q.pop();

                    sim_time = t;

                    std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t, true);
                    // std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t);
                    
                    for(const auto &e : ret) {
                        if(e.type == ELECT) {
                            Q.push({t+e.to, e});
                        }
                        else {
                            long delay = addDelay(delays, e.from, e.to, delay_opt);
                            if(delay_opt > 1) {
                                // only needed if link delays are variable
                                prev_time = fifo_times[e.from].at(e.to);
                                new_time = std::max(prev_time, t + delay);
                                fifo_times[e.from][e.to] = new_time;
                            } else
                                new_time = t + delay;
                            Q.push({new_time, e});
                            
                            stats->incrementCounters(e.type);
                        }
                    }
                }

                LPath = 0;
                for(const auto &n : nodes) {
                    int path = protocol->getPathLength(nodes, n.id, Algebra, d);
                    LPath = std::max(LPath, path);
                }
                longestPathLF = std::max(longestPathLF, LPath);
                
                stats->saveAndReset(d, sim_time, 2, linkCounter);
                stats->saveCycleBlackHole(d, sim_time, 2, linkCounter, 
                                          protocol->getCycleBlackHoleTimes(nodes, Algebra),
                                          protocol->getCycleBlackHoleChanges(nodes, Algebra));

                if(delay_opt > 1) 
                    resetFIFOtimes(fifo_times);
                GseqNum = 0;

                // check if everything is okay
                ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
                if(!ok) {
                    std::cout << "SOMETHING WENT WRONG (LINK FAILURE)! - protocol assertion\n";
                    std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
                    std::cout << "Link " << u << "-" << v << "\n";
                }

                sim_time = 0;
                /* Add back link u-v */
                Q.push({0, Event<A>(LINK_ADDITION, u.id, v, attr1)});  
                Q.push({0, Event<A>(LINK_ADDITION, v, u.id, attr2)}); 

                while(!Q.empty()) {
                    auto [t,ev,seq] = Q.top(); // get next event from queue
                    Q.pop();

                    sim_time = t;

                    if(ev.type == LINK_ADDITION) {
                        nodes[ev.to].addInLink(ev.from, ev.cost);
                        nodes[ev.from].addOutLink(ev.to, ev.cost);
                        if(delay_opt > 1) {
                            fifo_times[ev.from][ev.to] = t;
                            fifo_times[ev.to][ev.from] = t;
                        }
                    }

                    std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t);
                    
                    for(const auto &e : ret) {
                        if(e.type == ELECT) {
                            Q.push({t+e.to, e});
                        }
                        else {
                            long delay = addDelay(delays, e.from, e.to, delay_opt);
                            if(delay_opt > 1) {
                                // only needed if link delays are variable
                                prev_time = fifo_times[e.from].at(e.to);
                                new_time = std::max(prev_time, t + delay);
                                fifo_times[e.from][e.to] = new_time;
                            } else
                                new_time = t + delay;
                            Q.push({new_time, e});
                        }
                    }
                }

                stats->reset();
                if(delay_opt > 1) 
                    resetFIFOtimes(fifo_times);
                GseqNum = 0;

                // check if everything is okay
                ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
                if(!ok) {
                    std::cout << "SOMETHING WENT WRONG (LINK ADDITION)! - protocol assertion\n";
                    std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
                    std::cout << "Link " << u << "-" << v << "\n";
                }
            }
        }
    }

    std::cout << "Longest Path discovered (Advertise): " << longestPath << "\n";
    std::cout << "Longest Path discovered (Link Failure): " << longestPathLF << "\n";

    delete protocol;
}

/* Similar to 'simulateAsyncAuxAdvertiseLinkFailureOneDest', but now two links are removed simultaneously,
  so that the network gets disconnected */
template<class P, class A> void simulateAsyncAuxAdvertiseLinkFailureOneDestDisconnect(RoutingAlgebra<A> &Algebra,
                                                                                      std::vector<NodeOneDest<A>> &nodes, 
                                                                                      std::vector<std::unordered_map<int,int>> &fifo_times,
                                                                                      std::vector<std::unordered_map<int,int>> &delays,
                                                                                      int dest=-1, 
                                                                                      int delay_opt=1 )
{

    std::vector<std::pair<std::pair<int,int>, std::pair<int,int>>> linkPairs;

    /* Create a list with all link pairs whose simultaneous failure leaves the network disconnected */
    /* Choose first link */
    for(auto &u : nodes) {
        for(int v = u.id + 1; v < (int) nodes.size(); ++v) {
            // if(u.id > 0 || v > 1) continue;
            auto it1 = u.outNeighbours.find(v);
            if(it1 == u.outNeighbours.end()) continue;
            // save attributes of the first bidirectional link
            auto attr11 = it1->second, attr12 = nodes[v].outNeighbours.at(u.id);
            // remove the first bidirectional link from the network
            u.removeInLink(v);
            nodes[v].removeOutLink(u.id);
            u.removeOutLink(v);
            nodes[v].removeInLink(u.id);
            /* Choose second link so that there are no repeated link pairs */
            for(int x = u.id; x < (int) nodes.size(); ++x) {
                for(int w = (x == u.id ? v + 1 : x + 1); w < (int) nodes.size(); ++w) {
                    auto it2 = nodes[x].outNeighbours.find(w);
                    if(it2 == nodes[x].outNeighbours.end()) continue;
                    // save attributes of the second bidirectional link
                    auto attr21 = it2->second, attr22 = nodes[w].outNeighbours.at(x);
                    // remove the second bidirectional link from the network
                    nodes[x].removeInLink(w);
                    nodes[w].removeOutLink(x);
                    nodes[x].removeOutLink(w);
                    nodes[w].removeInLink(x);
                    // check if the network remained connected; if not, add the link pair to the list
                    if(!isConnected<A>(nodes)) 
                        linkPairs.push_back(std::make_pair(std::make_pair(u.id, v), std::make_pair(x, w)));
                    // add the second bidirectional link back to the network with the same attribute
                    nodes[x].addInLink(w, attr22);
                    nodes[w].addOutLink(x, attr22);
                    nodes[x].addOutLink(w, attr21);
                    nodes[w].addInLink(x, attr21);
                }
            }
            // add the first bidirectional link back to the network with the same attribute
            u.addInLink(v, attr12);
            nodes[v].addOutLink(u.id, attr12);
            u.addOutLink(v, attr11);
            nodes[v].addInLink(u.id, attr11);
        }
    }

    stats->createDoubleLinkFailureVec(linkPairs.size());

    /* START OF SIMULATION PROCESS */
    /* Initialize the queue of events */
    std::priority_queue < EventQueueNode<long, Event<A>> > Q = {};

    int longestPath = 0, longestPathLF = 0;

    /* Protocol choice */
    P *protocol = new P();

    int start = 0, end = nodes.size() - 1;
    if(dest != -1) 
        start = end = dest;

    /* Simulation time and other variables */
    long sim_time = 0, new_time = 0, prev_time = 0;
    bool ok;
    
    /* Repeat for every node (if 'dest' == -1), of for a single node (otehrwise) */
    for(int d = start; d <= end; ++d) {
        /* Reset node information */
        for(auto &n : nodes) 
            n.resetVariables();

        sim_time = 0;
        /* Node d Announces attribute \epsilon */
        Q.push({0, Event<A>(ADVERTISE, d, d, Algebra.Neutral)}); 
    
        while(!Q.empty()) {
            auto [t,ev,seq] = Q.top();
            Q.pop();

            sim_time = t;

            std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t);
            
            /* Add each new event to the Event Queue */
            for(const auto &e : ret) {
                if(e.type == ELECT) {
                    Q.push({t+e.to, e});
                }
                else {
                    long delay = addDelay(delays, e.from, e.to, delay_opt);
                    if(delay_opt > 1) {
                        // only needed if link delays are variable
                        prev_time = fifo_times[e.from].at(e.to);
                        new_time = std::max(prev_time, t + delay);
                        fifo_times[e.from][e.to] = new_time;
                    } else
                        new_time = t + delay;
                    Q.push({new_time, e});
                }
            }
        }

        stats->reset();
        if(delay_opt > 1) 
            resetFIFOtimes(fifo_times);
        GseqNum = 0;

        int LPath = 0;
        for(const auto &n : nodes)
            LPath = std::max(LPath, n.longestPath);
        longestPath = std::max(longestPath, LPath);


        ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
        if(!ok) {
            std::cout << "SOMETHING WENT WRONG (ADVERTISE)! - protocol assertion\n";
            std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ")\n";
        }
        /* END OF ADVERTISEMENT PHASE */

        int linkCounter = -1;
        sim_time = 0;
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

            Q.push({0, Event<A>(LINK_FAILURE, u1, v1)});  
            Q.push({0, Event<A>(LINK_FAILURE, v1, u1)}); 

            Q.push({0, Event<A>(LINK_FAILURE, u2, v2)});  
            Q.push({0, Event<A>(LINK_FAILURE, v2, u2)}); 

            while(!Q.empty()) {
                auto [t,ev,seq] = Q.top(); 
                Q.pop();

                sim_time = t;

                if(ev.type == LINK_FAILURE) {
                    nodes[ev.to].removeInLink(ev.from);
                    nodes[ev.from].removeOutLink(ev.to);
                    Q = removeFromQueue<EventQueueNode<long, Event<A>>>(Q, ev.from, ev.to);
                }

                std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t, true);
                // std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t);
                
                for(const auto &e : ret) {
                    if(e.type == ELECT)
                        Q.push({t+e.to, e});
                    else {
                        long delay = addDelay(delays, e.from, e.to, delay_opt);
                        if(delay_opt > 1) {
                            // only needed if link delays are variable
                            prev_time = fifo_times[e.from].at(e.to);
                            new_time = std::max(prev_time, t + delay);
                            fifo_times[e.from][e.to] = new_time;
                        } else
                            new_time = t + delay;
                        Q.push({new_time, e});
                        
                        stats->incrementCounters(e.type);
                    }
                }
            } /* end of while(!Q.empty()) */

            LPath = 0;
            for(const auto &n : nodes) 
                LPath = std::max(LPath, n.longestPath);
            longestPathLF = std::max(longestPathLF, LPath);

            stats->saveAndReset(d, sim_time, 3, linkCounter);
            stats->saveCycleBlackHole(d, sim_time, 3, linkCounter, 
                                      protocol->getCycleBlackHoleTimes(nodes, Algebra),
                                      protocol->getCycleBlackHoleChanges(nodes, Algebra));

            if(delay_opt > 1) 
                resetFIFOtimes(fifo_times);
            GseqNum = 0;
            
            // check if everything is okay
            ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
            if(!ok) {
                std::cout << "SOMETHING WENT WRONG (LINK FAILURE)! - protocol assertion\n";
                std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ") ";
                std::cout << "Links " << u1 << "-" << v1 << " and " << u2 << "-" << v2 << "\n";
            }

            
            sim_time = 0;
            /* Add the two links back */
            Q.push({0, Event<A>(LINK_ADDITION, u1, v1, attr11)});  
            Q.push({0, Event<A>(LINK_ADDITION, v1, u1, attr12)}); 

            Q.push({0, Event<A>(LINK_ADDITION, u2, v2, attr21)});  
            Q.push({0, Event<A>(LINK_ADDITION, v2, u2, attr22)}); 

            while(!Q.empty()) {
                auto [t,ev,seq] = Q.top();
                Q.pop();

                sim_time = t;

                if(ev.type == LINK_ADDITION) {
                    nodes[ev.to].addInLink(ev.from, ev.cost);
                    nodes[ev.from].addOutLink(ev.to, ev.cost);
                    if(delay_opt > 1) {
                        fifo_times[ev.from][ev.to] = t;
                        fifo_times[ev.to][ev.from] = t;
                    }
                }

                std::vector<Event<A>> ret = protocol->processEvent(Algebra, ev, nodes, t);
                
                for(const auto &e : ret) {
                    if(e.type == ELECT) 
                        Q.push({t+e.to, e});
                    else {
                        long delay = addDelay(delays, e.from, e.to, delay_opt);
                        if(delay_opt > 1) {
                            // only needed if link delays are variable
                            prev_time = fifo_times[e.from].at(e.to);
                            new_time = std::max(prev_time, t + delay);
                            fifo_times[e.from][e.to] = new_time;
                        } else
                            new_time = t + delay;
                        Q.push({new_time, e});
                    }
                }
            } /* end of while(!Q.empty()) */

            stats->reset();
            if(delay_opt > 1) 
                resetFIFOtimes(fifo_times);
            GseqNum = 0;
            // check if everything is okay
            ok = protocol->assertCorrectness(nodes, Algebra, ADVERTISE, d, true);
            if(!ok) {
                std::cout << "SOMETHING WENT WRONG (LINK ADDITION)! - protocol assertion\n";
                std::cout << "Node " << d << " (" << d-start+1 << " of " << end-start+1 << ") ";
                std::cout << "Links " << u1 << "-" << v1 << " and " << u2 << "-" << v2 << "\n";
            }

        } /* end of for(const auto &linkPair : linkPars) */

    } /* end of for(int d = start; d <= end; ++d) */

    std::cout << "Longest Path discovered (Advertise): " << longestPath << "\n";
    std::cout << "Longest Path discovered (Link Failure): " << longestPathLF << "\n";

    delete protocol;
}


template<class A> void simulateAsync(std::string file_path, int protocol, int metric, int dest, int test_type, int delay_opt) {
    /* Initialize the routing algebra */
    RoutingAlgebra<A> Algebra = RoutingAlgebra<A>();

    /* Create the Network */
    /* map whose entry u,v stores the time at which the last message from u to v
      is to be received; messages sent after cannot be scheduled to be received 
      before. Filled in the 'createGraph' function */
    std::vector<std::unordered_map<int, int>> fifo_times = {};
    /* vector that stores each link delay, according to the network file. 
      Filled in the 'createGraph' function */
    std::vector<std::unordered_map<int,int>> delays = {};
    // metric option, which also depends on the chosen protocol
    int opt = ( (protocol == 4 || protocol == 5) ? metric-1 : (protocol == 3 ? metric+5 : 6*(protocol-1)+metric-1));
    std::vector<NodeOneDest<A>> nodes = createGraph<A>(file_path, fifo_times, delays, opt);
    if(nodes.size() == 0) return;
    if(dest >= (int) nodes.size()) {
        std::cout << "Invalid destination node '"<< dest <<"'\nExiting\n";
        return;
    }

    stats = new Stats(nodes.size());

    switch(test_type) {
        case 0:
            if(protocol == 1)
                simulateAsyncAuxAdvertiseWithdrawOneDest<EIGRPOneDest<A>, A>(Algebra, nodes, fifo_times, delays, dest, true, delay_opt);
            else if(protocol == 2)
                simulateAsyncAuxAdvertiseWithdrawOneDest<BGPOneDest<A>, A>(Algebra, nodes, fifo_times, delays, dest, true, delay_opt);
            else if(protocol == 5) 
                simulateAsyncAuxAdvertiseWithdrawOneDest<EIGRPOneDestFC<A>, A>(Algebra, nodes, fifo_times, delays, dest, true, delay_opt);
            else
                simulateAsyncAuxAdvertiseWithdrawOneDest<BGPQuasiSync<A>, A>(Algebra, nodes, fifo_times, delays, dest, true, delay_opt);
            break;
        case 1:
            if(protocol == 1)
                simulateAsyncAuxAdvertiseWithdrawOneDest<EIGRPOneDest<A>, A>(Algebra, nodes, fifo_times, delays, dest, false, delay_opt);
            else if(protocol == 2)
                simulateAsyncAuxAdvertiseWithdrawOneDest<BGPOneDest<A>, A>(Algebra, nodes, fifo_times, delays, dest, false, delay_opt);
            else if(protocol == 3)
                simulateAsyncAuxAdvertiseWithdrawOneDest<BGPQuasiSync<A>, A>(Algebra, nodes, fifo_times, delays, dest, false, delay_opt);
            else if(protocol == 5) 
                simulateAsyncAuxAdvertiseWithdrawOneDest<EIGRPOneDestFC<A>, A>(Algebra, nodes, fifo_times, delays, dest, false, delay_opt);
            else
                simulateAsyncAuxAdvertiseWithdrawOneDest<VectoringProtocol<A>, A>(Algebra, nodes, fifo_times, delays, dest, false, delay_opt);
            break;
        case 2:
            if(protocol == 1)
                simulateAsyncAuxAdvertiseLinkFailureOneDest<EIGRPOneDest<A>, A>(Algebra, nodes, fifo_times, delays, dest, delay_opt);
            else if(protocol == 2)
                simulateAsyncAuxAdvertiseLinkFailureOneDest<BGPOneDest<A>, A>(Algebra, nodes, fifo_times, delays, dest, delay_opt);
            else if(protocol == 3)
                simulateAsyncAuxAdvertiseLinkFailureOneDest<BGPQuasiSync<A>, A>(Algebra, nodes, fifo_times, delays, dest, delay_opt);
            else if(protocol == 5) 
                simulateAsyncAuxAdvertiseLinkFailureOneDest<EIGRPOneDestFC<A>, A>(Algebra, nodes, fifo_times, delays, dest, delay_opt);
            else 
                simulateAsyncAuxAdvertiseLinkFailureOneDest<VectoringProtocol<A>, A>(Algebra, nodes, fifo_times, delays, dest, delay_opt);
            break;
        case 3:
            if(protocol == 1)
                simulateAsyncAuxAdvertiseLinkFailureOneDestDisconnect<EIGRPOneDest<A>, A>(Algebra, nodes, fifo_times, delays, dest, delay_opt);
            else if(protocol == 2)
                simulateAsyncAuxAdvertiseLinkFailureOneDestDisconnect<BGPOneDest<A>, A>(Algebra, nodes, fifo_times, delays, dest, delay_opt);
            else if(protocol == 5)
                simulateAsyncAuxAdvertiseLinkFailureOneDestDisconnect<EIGRPOneDestFC<A>, A>(Algebra, nodes, fifo_times, delays, dest, delay_opt);
            else
                simulateAsyncAuxAdvertiseLinkFailureOneDestDisconnect<BGPQuasiSync<A>, A>(Algebra, nodes, fifo_times, delays, dest, delay_opt);
            break;
        default:
        break;
    }

}

#include "../Instances/asyncSimulationInstance.h"


