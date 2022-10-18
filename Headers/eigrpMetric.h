#ifndef EIGRPMETRIC_H
#define EIGRPMETRIC_H

#include "enums.h"

class EIGRPMetricAttribute {
public:
    // 10^7/BW[kbps] + delay[us]/10 = 10^4/BW[Mbps] + 100*delay[ms]
    long bw = 0; // in Mbps
    int delay = 0; // in ms
    int K = 1e5;
    // the arguments are as such: bw in Mbps, delay in ms
    EIGRPMetricAttribute(int, int, int=-1);
    EIGRPMetricAttribute(SPECIAL_ATTR, int=-1);
    EIGRPMetricAttribute(int);
    EIGRPMetricAttribute(){}
    int length(){return 0;}

    friend bool operator== (EIGRPMetricAttribute const& a, EIGRPMetricAttribute const& b) {
        // int a_val, b_val;
        // if(a.bw == 0 || a.delay == INT_MAX) a_val = -1;
        // else a_val = a.K/a.bw + a.delay*100;
        // if(b.bw == 0 || b.delay == INT_MAX) b_val = -1;
        // else b_val = b.K/b.bw + b.delay*100;
        // return a_val == b_val;
        return (a.bw == b.bw && a.delay == b.delay);
    }

    friend bool operator!= (EIGRPMetricAttribute const& a, EIGRPMetricAttribute const& b) {
        return !(a == b);
    }

    friend bool operator< (EIGRPMetricAttribute const& a, EIGRPMetricAttribute const& b) {
        if(a.bw == 0 || a.delay == INT_MAX) return false;
        if(b.bw == 0 || b.delay == INT_MAX) return true;
        return (a.K/a.bw + a.delay*100 < b.K/b.bw + b.delay*100) || 
                ((a.K/a.bw + a.delay*100 == b.K/b.bw + b.delay*100) && a.bw > b.bw);
    }

    friend bool operator> (EIGRPMetricAttribute const& a, EIGRPMetricAttribute const& b) {
        if(b.bw == 0 || b.delay == INT_MAX) return false;
        if(a.bw == 0 || a.delay == INT_MAX) return true;
        return (a.K/a.bw + a.delay*100 > b.K/b.bw + b.delay*100) || 
                ((a.K/a.bw + a.delay*100 == b.K/b.bw + b.delay*100) && a.bw < b.bw);
    }

    friend EIGRPMetricAttribute operator+ (EIGRPMetricAttribute const& a, EIGRPMetricAttribute const& b) {
        if(a.bw == 0 || a.delay == INT_MAX || b.bw == 0 || b.delay == INT_MAX) return EIGRPMetricAttribute(INVALID);
        return EIGRPMetricAttribute(std::min(a.bw, b.bw), a.delay + b.delay);
    }

    friend std::ostream & operator<< (std::ostream & os, const EIGRPMetricAttribute & a) {	
        if(a.bw == 0 || a.delay == INT_MAX)
            os << " <Inf> ";
        else {
            os << " <";
            os << a.K/a.bw + a.delay*100;
            os << ": [";
            os << a.bw << ",";
            os << a.delay;
            os << "]> ";
        }
        return os; 
    }

};


#endif