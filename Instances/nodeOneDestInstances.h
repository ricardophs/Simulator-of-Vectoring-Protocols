#ifndef NODEONEDESTINSTANCES_H
#define NODEONEDESTINSTANCES_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/nodeOneDest.h"

template class NodeOneDest<ShortestWidestPathAttribute>;
template class NodeOneDest<WidestShortestPathAttribute>;
template class NodeOneDest<ShortestPathAttribute>;
template class NodeOneDest<EIGRPMetricAttribute>;

#include "../Headers/BGPattribute.h"
template class NodeOneDest<BGPAttribute<ShortestWidestPathAttribute>>;
template class NodeOneDest<BGPAttribute<WidestShortestPathAttribute>>;
template class NodeOneDest<BGPAttribute<ShortestPathAttribute>>;
template class NodeOneDest<BGPAttribute<EIGRPMetricAttribute>>;

#endif