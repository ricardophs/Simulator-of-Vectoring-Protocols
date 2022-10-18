#ifndef BGPATTRIBUTEINSTANCE_H
#define BGPATTRIBUTEINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"

template class BGPAttribute<ShortestWidestPathAttribute>;
template class BGPAttribute<WidestShortestPathAttribute>;
template class BGPAttribute<ShortestPathAttribute>;
template class BGPAttribute<EIGRPMetricAttribute>;

#endif