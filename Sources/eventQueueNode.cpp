#include <iostream>

#include "../Headers/eventQueueNode.h"

extern unsigned long GseqNum;

/* Creates a new event with a sequence number greater than the last created event */
template <typename Key, class Value> 
EventQueueNode<Key,Value>::EventQueueNode(Key key, Value value) {
    this->key = key;
    this->value = value;
    this->seqNum = (GseqNum++);
}

/* Creates a new event with a given sequence number. Useful when an event that was 
  already in queue is reinserted, for example, due to a link failure (check function 
  'removeFromQueue' of asyncSimulation.cpp) */
template <typename Key, class Value> 
EventQueueNode<Key,Value>::EventQueueNode(Key key, Value value, unsigned long seqNum) {
    this->key = key;
    this->value = value;
    this->seqNum = seqNum;
}

#include "../Instances/eventQueueNodeInstances.h"