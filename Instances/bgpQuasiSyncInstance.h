#ifndef BGPQUASISYNCINSTANCE_H
#define BGPQUASISYNCINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"

template class BGPQuasiSync<BGPAttribute<ShortestPathAttribute>>;

template class BGPQuasiSync<BGPAttribute<ShortestWidestPathAttribute>>;

template class BGPQuasiSync<BGPAttribute<WidestShortestPathAttribute>>;

template class BGPQuasiSync<BGPAttribute<EIGRPMetricAttribute>>;


template class BGPQuasiSync<ShortestPathAttribute>;

template class BGPQuasiSync<ShortestWidestPathAttribute>;

template class BGPQuasiSync<WidestShortestPathAttribute>;

template class BGPQuasiSync<EIGRPMetricAttribute>;


#endif