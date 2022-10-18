#ifndef SYNCSIMULATIONINSTANCE_H
#define SYNCSIMULATIONINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"

template void simulateSync<ShortestWidestPathAttribute>(std::string, int, int, int, int);
template void simulateSync<WidestShortestPathAttribute>(std::string, int, int, int, int);
template void simulateSync<ShortestPathAttribute>(std::string, int, int, int, int);
template void simulateSync<EIGRPMetricAttribute>(std::string, int, int, int, int);

template void simulateSync<BGPAttribute<ShortestWidestPathAttribute>>(std::string, int, int, int, int);
template void simulateSync<BGPAttribute<WidestShortestPathAttribute>>(std::string, int, int, int, int);
template void simulateSync<BGPAttribute<ShortestPathAttribute>>(std::string, int, int, int, int);
template void simulateSync<BGPAttribute<EIGRPMetricAttribute>>(std::string, int, int, int, int);


#endif