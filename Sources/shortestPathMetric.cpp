#include <iostream>

#include "../Headers/shortestPathMetric.h"


ShortestPathAttribute::ShortestPathAttribute(int length) {
    this->attribute = length;
}

ShortestPathAttribute::ShortestPathAttribute(SPECIAL_ATTR spec) {
    if(spec == NEUTRAL)
        this->attribute = 0;
    else if(spec == INVALID)
        this->attribute = INT_MAX;
}

ShortestPathAttribute::ShortestPathAttribute(int length, int w) {
    this->attribute = length;
}

ShortestPathAttribute::ShortestPathAttribute(int asn, int length, int w) {
    this->attribute = length;
}
