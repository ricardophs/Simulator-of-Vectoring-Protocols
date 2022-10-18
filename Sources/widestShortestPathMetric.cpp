#include <iostream>

#include "../Headers/widestShortestPathMetric.h"

WidestShortestPathAttribute::WidestShortestPathAttribute(SPECIAL_ATTR spec) {
    if(spec == NEUTRAL)
        this->attribute = std::make_pair(INT_MAX, 0);
    else if(spec == INVALID)
        this->attribute = std::make_pair(0, INT_MAX);
}

WidestShortestPathAttribute::WidestShortestPathAttribute(int bandwidth) {
    this->attribute = std::make_pair(bandwidth, 0);
}

WidestShortestPathAttribute::WidestShortestPathAttribute(int bandwidth, int delay) {
    this->attribute = std::make_pair(bandwidth, delay);
}

WidestShortestPathAttribute::WidestShortestPathAttribute(int asn, int delay, int bandwidth) {
    this->attribute = std::make_pair(bandwidth, delay);
}