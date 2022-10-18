#ifndef EIGRPONEDESTFCINSTANCE_H
#define EIGRPONEDESTFCINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"

template class EIGRPOneDestFC<ShortestPathAttribute>;

template class EIGRPOneDestFC<ShortestWidestPathAttribute>;

template class EIGRPOneDestFC<WidestShortestPathAttribute>;

template class EIGRPOneDestFC<EIGRPMetricAttribute>;


template class EIGRPOneDestFC<BGPAttribute<ShortestPathAttribute>>;

template class EIGRPOneDestFC<BGPAttribute<ShortestWidestPathAttribute>>;

template class EIGRPOneDestFC<BGPAttribute<WidestShortestPathAttribute>>;

template class EIGRPOneDestFC<BGPAttribute<EIGRPMetricAttribute>>;

#endif