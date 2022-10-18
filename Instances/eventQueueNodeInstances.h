#ifndef EVENTQUEUENODEINSTANCES_H
#define EVENTQUEUENODEINSTANCES_H

#include "../Headers/eventQueueNode.h"
#include "../Headers/event.h"
#include "../Headers/shortestWidestPathMetric.h"
#include "../Headers/widestShortestPathMetric.h"
#include "../Headers/shortestPathMetric.h"
#include "../Headers/eigrpMetric.h"

template class EventQueueNode<long, Event<ShortestWidestPathAttribute>>;
template class EventQueueNode<long, Event<WidestShortestPathAttribute>>;
template class EventQueueNode<long, Event<ShortestPathAttribute>>;
template class EventQueueNode<long, Event<EIGRPMetricAttribute>>;

#include "../Headers/BGPattribute.h"
template class EventQueueNode<long, Event<BGPAttribute<ShortestWidestPathAttribute>>>;
template class EventQueueNode<long, Event<BGPAttribute<WidestShortestPathAttribute>>>;
template class EventQueueNode<long, Event<BGPAttribute<ShortestPathAttribute>>>;
template class EventQueueNode<long, Event<BGPAttribute<EIGRPMetricAttribute>>>;

#endif