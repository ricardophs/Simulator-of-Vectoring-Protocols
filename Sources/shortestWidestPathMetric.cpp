#include <iostream>

#include "../Headers/shortestWidestPathMetric.h"

ShortestWidestPathAttribute::ShortestWidestPathAttribute(SPECIAL_ATTR spec) {
    if(spec == NEUTRAL)
        this->attribute = std::make_pair(INT_MAX, 0);
    else if(spec == INVALID)
        this->attribute = std::make_pair(0, INT_MAX);
}

ShortestWidestPathAttribute::ShortestWidestPathAttribute(int bandwidth) {
    this->attribute = std::make_pair(bandwidth, 0);
}

ShortestWidestPathAttribute::ShortestWidestPathAttribute(int bandwidth, int delay) {
    this->attribute = std::make_pair(bandwidth, delay);
}

ShortestWidestPathAttribute::ShortestWidestPathAttribute(int asn, int delay, int bandwidth) {
    this->attribute = std::make_pair(bandwidth, delay);
}