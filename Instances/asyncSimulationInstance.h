#ifndef ASYNCSIMULATIONINSTANCE_H
#define ASYNCSIMULATIONINSTANCE_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/bgpAttribute.h"

template void simulateAsync<ShortestWidestPathAttribute>(std::string, int, int, int, int, int);
template void simulateAsync<WidestShortestPathAttribute>(std::string, int, int, int, int, int);
template void simulateAsync<ShortestPathAttribute>(std::string, int, int, int, int, int);
template void simulateAsync<EIGRPMetricAttribute>(std::string, int, int, int, int, int);

template void simulateAsync<BGPAttribute<ShortestWidestPathAttribute>>(std::string, int, int, int, int, int);
template void simulateAsync<BGPAttribute<WidestShortestPathAttribute>>(std::string, int, int, int, int, int);
template void simulateAsync<BGPAttribute<ShortestPathAttribute>>(std::string, int, int, int, int, int);
template void simulateAsync<BGPAttribute<EIGRPMetricAttribute>>(std::string, int, int, int, int, int);


#endif