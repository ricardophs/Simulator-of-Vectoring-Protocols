#ifndef UTILITIESINSTANCE_H
#define UTILITIESINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"
#include "../Headers/nodeOneDest.h"

/*** EIGRP ***/
/* WSP */
template vector<WidestShortestPathAttribute>
EvDijkstra<WidestShortestPathAttribute>(vector<NodeOneDest<WidestShortestPathAttribute>> &,
                                        int,
                                        WidestShortestPathAttribute,
                                        bool);

template vector<NodeOneDest<WidestShortestPathAttribute>>
createGraph<WidestShortestPathAttribute>(string,
                                         vector<unordered_map<int, int>> &,
                                         vector<unordered_map<int, int>> &,
                                         int,
                                         bool);

template bool 
isConnected<WidestShortestPathAttribute>(vector<NodeOneDest<WidestShortestPathAttribute>> &);

/* SWP */
template vector<ShortestWidestPathAttribute>
EvDijkstra<ShortestWidestPathAttribute>(vector<NodeOneDest<ShortestWidestPathAttribute>> &,
                                        int,
                                        ShortestWidestPathAttribute,
                                        bool);

template vector<NodeOneDest<ShortestWidestPathAttribute>>
createGraph<ShortestWidestPathAttribute>(string,
                                         vector<unordered_map<int, int>> &,
                                         vector<unordered_map<int, int>> &,
                                         int,
                                         bool);

template bool 
isConnected<ShortestWidestPathAttribute>(vector<NodeOneDest<ShortestWidestPathAttribute>> &);

/* SP */
template std::vector<ShortestPathAttribute>
EvDijkstra<ShortestPathAttribute>(vector<NodeOneDest<ShortestPathAttribute>> &,
                                  int,
                                  ShortestPathAttribute,
                                  bool);

template vector<NodeOneDest<ShortestPathAttribute>>
createGraph<ShortestPathAttribute>(string,
                                   vector<unordered_map<int, int>> &,
                                   vector<unordered_map<int, int>> &,
                                   int,
                                   bool);

template bool 
isConnected<ShortestPathAttribute>(vector<NodeOneDest<ShortestPathAttribute>> &);

/* EIGRP Metric */
template std::vector<EIGRPMetricAttribute>
EvDijkstra<EIGRPMetricAttribute>(vector<NodeOneDest<EIGRPMetricAttribute>> &,
                                 int,
                                 EIGRPMetricAttribute,
                                 bool);

template vector<NodeOneDest<EIGRPMetricAttribute>>
createGraph<EIGRPMetricAttribute>(string,
                                  vector<unordered_map<int, int>> &,
                                  vector<unordered_map<int, int>> &,
                                  int,
                                  bool);

template bool 
isConnected<EIGRPMetricAttribute>(vector<NodeOneDest<EIGRPMetricAttribute>> &);

/*** BGP ***/
/* WSP */
template vector<BGPAttribute<WidestShortestPathAttribute>>
EvDijkstra<BGPAttribute<WidestShortestPathAttribute>>(vector<NodeOneDest<BGPAttribute<WidestShortestPathAttribute>>> &,
                                                      int,
                                                      BGPAttribute<WidestShortestPathAttribute>,
                                                      bool);

template vector<NodeOneDest<BGPAttribute<WidestShortestPathAttribute>>>
createGraph<BGPAttribute<WidestShortestPathAttribute>>(string,
                                                       vector<unordered_map<int, int>> &,
                                                       vector<unordered_map<int, int>> &,
                                                       int,
                                                       bool);

template bool 
isConnected<BGPAttribute<WidestShortestPathAttribute>>(vector<NodeOneDest<BGPAttribute<WidestShortestPathAttribute>>> &);

/* SWP */
template vector<BGPAttribute<ShortestWidestPathAttribute>>
EvDijkstra<BGPAttribute<ShortestWidestPathAttribute>>(vector<NodeOneDest<BGPAttribute<ShortestWidestPathAttribute>>> &,
                                                      int,
                                                      BGPAttribute<ShortestWidestPathAttribute>,
                                                      bool);

template vector<NodeOneDest<BGPAttribute<ShortestWidestPathAttribute>>>
createGraph<BGPAttribute<ShortestWidestPathAttribute>>(string,
                                                       vector<unordered_map<int, int>> &,
                                                       vector<unordered_map<int, int>> &,
                                                       int,
                                                       bool);

template bool 
isConnected<BGPAttribute<ShortestWidestPathAttribute>>(vector<NodeOneDest<BGPAttribute<ShortestWidestPathAttribute>>> &);

/* SP */
template std::vector<BGPAttribute<ShortestPathAttribute>>
EvDijkstra<BGPAttribute<ShortestPathAttribute>>(vector<NodeOneDest<BGPAttribute<ShortestPathAttribute>>> &,
                                                int,
                                                BGPAttribute<ShortestPathAttribute>,
                                                bool);

template vector<NodeOneDest<BGPAttribute<ShortestPathAttribute>>>
createGraph<BGPAttribute<ShortestPathAttribute>>(string,
                                                 vector<unordered_map<int, int>> &,
                                                 vector<unordered_map<int, int>> &,
                                                 int,
                                                 bool);

template bool 
isConnected<BGPAttribute<ShortestPathAttribute>>(vector<NodeOneDest<BGPAttribute<ShortestPathAttribute>>> &);

/* EIGRP Metric */
template std::vector<BGPAttribute<EIGRPMetricAttribute>>
EvDijkstra<BGPAttribute<EIGRPMetricAttribute>>(vector<NodeOneDest<BGPAttribute<EIGRPMetricAttribute>>> &,
                                               int,
                                               BGPAttribute<EIGRPMetricAttribute>,
                                               bool);

template vector<NodeOneDest<BGPAttribute<EIGRPMetricAttribute>>>
createGraph<BGPAttribute<EIGRPMetricAttribute>>(string,
                                                vector<unordered_map<int, int>> &,
                                                vector<unordered_map<int, int>> &,
                                                int,
                                                bool);

template bool 
isConnected<BGPAttribute<EIGRPMetricAttribute>>(vector<NodeOneDest<BGPAttribute<EIGRPMetricAttribute>>> &);

#endif