#ifndef BGPONENODEINSTANCE_H
#define BGPONENODEINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"

template class BGPOneDest<BGPAttribute<ShortestPathAttribute>>;

template class BGPOneDest<BGPAttribute<ShortestWidestPathAttribute>>;

template class BGPOneDest<BGPAttribute<WidestShortestPathAttribute>>;

template class BGPOneDest<BGPAttribute<EIGRPMetricAttribute>>;


template class BGPOneDest<ShortestPathAttribute>;

template class BGPOneDest<ShortestWidestPathAttribute>;

template class BGPOneDest<WidestShortestPathAttribute>;

template class BGPOneDest<EIGRPMetricAttribute>;


#endif