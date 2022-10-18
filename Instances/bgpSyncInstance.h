#ifndef BGPSYNCINSTANCE_H
#define BGPSYNCINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"

template class BGPSync<BGPAttribute<ShortestPathAttribute>>;

template class BGPSync<BGPAttribute<ShortestWidestPathAttribute>>;

template class BGPSync<BGPAttribute<WidestShortestPathAttribute>>;

template class BGPSync<BGPAttribute<EIGRPMetricAttribute>>;


template class BGPSync<ShortestPathAttribute>;

template class BGPSync<ShortestWidestPathAttribute>;

template class BGPSync<WidestShortestPathAttribute>;

template class BGPSync<EIGRPMetricAttribute>;

#endif