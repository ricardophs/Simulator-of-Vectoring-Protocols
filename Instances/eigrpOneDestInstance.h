#ifndef EIGRPONEDESTINSTANCE_H
#define EIGRPONEDESTINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"

template class EIGRPOneDest<ShortestPathAttribute>;

template class EIGRPOneDest<ShortestWidestPathAttribute>;

template class EIGRPOneDest<WidestShortestPathAttribute>;

template class EIGRPOneDest<EIGRPMetricAttribute>;


template class EIGRPOneDest<BGPAttribute<ShortestPathAttribute>>;

template class EIGRPOneDest<BGPAttribute<ShortestWidestPathAttribute>>;

template class EIGRPOneDest<BGPAttribute<WidestShortestPathAttribute>>;

template class EIGRPOneDest<BGPAttribute<EIGRPMetricAttribute>>;

#endif