#ifndef ALGEBRAINSTANCE_H
#define ALGEBRAINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/algebra.h"

template class RoutingAlgebra<ShortestWidestPathAttribute>;
template class RoutingAlgebra<WidestShortestPathAttribute>;
template class RoutingAlgebra<ShortestPathAttribute>;
template class RoutingAlgebra<EIGRPMetricAttribute>;


#include "../Headers/BGPattribute.h"
template class RoutingAlgebra<BGPAttribute<ShortestWidestPathAttribute>>;
template class RoutingAlgebra<BGPAttribute<WidestShortestPathAttribute>>;
template class RoutingAlgebra<BGPAttribute<ShortestPathAttribute>>;
template class RoutingAlgebra<BGPAttribute<EIGRPMetricAttribute>>;

#endif