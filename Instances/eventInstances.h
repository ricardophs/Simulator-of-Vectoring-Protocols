#ifndef EVENTINSTANCES_H
#define EVENTINSTANCES_H

#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"
#include "../Headers/event.h"

template class Event<ShortestWidestPathAttribute>;
template class Event<WidestShortestPathAttribute>;
template class Event<ShortestPathAttribute>;
template class Event<EIGRPMetricAttribute>;

#include "../Headers/BGPattribute.h"
template class Event<BGPAttribute<ShortestWidestPathAttribute>>;
template class Event<BGPAttribute<WidestShortestPathAttribute>>;
template class Event<BGPAttribute<ShortestPathAttribute>>;
template class Event<BGPAttribute<EIGRPMetricAttribute>>;

#endif