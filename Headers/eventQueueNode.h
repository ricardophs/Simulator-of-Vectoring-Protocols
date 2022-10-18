#ifndef EVENTQUEUENODE_H
#define EVENTQUEUENODE_H

template <typename Key, class Value> class EventQueueNode {
    
public:
    Key key;
    Value value;
    unsigned long seqNum;
    EventQueueNode(Key key, Value value);
    EventQueueNode(Key key, Value value, unsigned long seqNum);

    friend bool operator< (EventQueueNode const& a, EventQueueNode const& b) {
        return a.key > b.key || (a.key == b.key && a.seqNum > b.seqNum);
    }

    friend std::ostream & operator<< (std::ostream & os, const EventQueueNode & e) {	
        os << " < ";
        os << "key = " << e.key << " , ";
        os << "value = " << e.value << " , ";
        os << "seqNum = " << e.seqNum;
        os << " > ";
        return os; 
    };

};

#endif