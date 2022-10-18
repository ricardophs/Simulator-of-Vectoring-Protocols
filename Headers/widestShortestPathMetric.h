#ifndef WIDESTSHORTESTPATHMETRIC_H
#define WIDESTSHORTESTPATHMETRIC_H

#include <climits>
#include "enums.h"

class WidestShortestPathAttribute {
public:
    std::pair<int,int> attribute;
    
    WidestShortestPathAttribute(SPECIAL_ATTR);
    WidestShortestPathAttribute(){}
    WidestShortestPathAttribute(int);
    WidestShortestPathAttribute(int, int);
    WidestShortestPathAttribute(int, int, int);
    int length(){return 0;}

    friend bool operator== (WidestShortestPathAttribute const& a, WidestShortestPathAttribute const& b) {
        return a.attribute.first == b.attribute.first && a.attribute.second == b.attribute.second;
    }

    friend bool operator!= (WidestShortestPathAttribute const& a, WidestShortestPathAttribute const& b) {
        return a.attribute.first != b.attribute.first || a.attribute.second != b.attribute.second;
    }

    friend bool operator< (WidestShortestPathAttribute const& a, WidestShortestPathAttribute const& b) {
        return (a.attribute.second < b.attribute.second || (a.attribute.second == b.attribute.second && a.attribute.first > b.attribute.first));
    }

    friend bool operator> (WidestShortestPathAttribute const& a, WidestShortestPathAttribute const& b) {
        return (a.attribute.second > b.attribute.second || (a.attribute.second == b.attribute.second && a.attribute.first < b.attribute.first));
    }

    friend WidestShortestPathAttribute operator+ (WidestShortestPathAttribute const& a, WidestShortestPathAttribute const& b) {
        if(a.attribute.first == 0 || a.attribute.second == INT_MAX || b.attribute.first == 0 || b.attribute.second == INT_MAX)
            return WidestShortestPathAttribute(INVALID);
        return WidestShortestPathAttribute(std::min(a.attribute.first, b.attribute.first), a.attribute.second + b.attribute.second);
    }

    friend std::ostream & operator<< (std::ostream & os, const WidestShortestPathAttribute & a) {	
        if(a.attribute.first == 0 || a.attribute.second == INT_MAX)
            os << "<Inf>";
        else {
            os << "<";
            os <<  a.attribute.first << ",";
            os << a.attribute.second;
            os << ">";
        }
        return os; 
    }

};


#endif