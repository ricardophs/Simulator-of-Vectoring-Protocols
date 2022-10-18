#ifndef EIGRPSYNCINSTANCE_H
#define EIGRPSYNCINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"

template class EIGRPSync<ShortestPathAttribute>;

template class EIGRPSync<ShortestWidestPathAttribute>;

template class EIGRPSync<WidestShortestPathAttribute>;

template class EIGRPSync<EIGRPMetricAttribute>;


template class EIGRPSync<BGPAttribute<ShortestPathAttribute>>;

template class EIGRPSync<BGPAttribute<ShortestWidestPathAttribute>>;

template class EIGRPSync<BGPAttribute<WidestShortestPathAttribute>>;

template class EIGRPSync<BGPAttribute<EIGRPMetricAttribute>>;

#endif