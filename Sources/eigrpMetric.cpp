#include <iostream>
#include <climits>

#include "../Headers/eigrpMetric.h"

EIGRPMetricAttribute::EIGRPMetricAttribute(int bandwidth, int delay, int k) {
    this->bw = bandwidth; // bandwidth in Mbps
    this->delay = delay; // delay in ms
    if (k != -1) this->K = k;
}

EIGRPMetricAttribute::EIGRPMetricAttribute(SPECIAL_ATTR spec, int k) {
    if(spec == NEUTRAL){
        this->bw = INT_MAX;
        this->delay = 0;
    }
    else if(spec == INVALID){
        this->bw = 0;
        this->delay = INT_MAX;
    }
    if (k != -1) this->K = k;
}

EIGRPMetricAttribute::EIGRPMetricAttribute(int bandwidth) {}