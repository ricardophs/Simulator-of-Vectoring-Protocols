#ifndef VECTORINGPROTOCOLINSTANCE_H
#define VECTORINGPROTOCOLINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"


template class VectoringProtocol<ShortestPathAttribute>;

template class VectoringProtocol<ShortestWidestPathAttribute>;

template class VectoringProtocol<WidestShortestPathAttribute>;

template class VectoringProtocol<EIGRPMetricAttribute>;


template class VectoringProtocol<BGPAttribute<ShortestPathAttribute>>;

template class VectoringProtocol<BGPAttribute<ShortestWidestPathAttribute>>;

template class VectoringProtocol<BGPAttribute<WidestShortestPathAttribute>>;

template class VectoringProtocol<BGPAttribute<EIGRPMetricAttribute>>;


#endif