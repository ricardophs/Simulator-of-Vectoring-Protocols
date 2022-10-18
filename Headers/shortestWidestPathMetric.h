#ifndef SHORTESTWIDESTPATHMETRIC_H
#define SHORTESTWIDESTPATHMETRIC_H

#include <climits>
#include "enums.h"

class ShortestWidestPathAttribute {
public:
    std::pair<int,int> attribute;
    
    ShortestWidestPathAttribute(SPECIAL_ATTR);
    ShortestWidestPathAttribute(){}
    ShortestWidestPathAttribute(int);
    ShortestWidestPathAttribute(int, int);
    ShortestWidestPathAttribute(int, int, int);
    int length(){return 0;}

    friend bool operator== (ShortestWidestPathAttribute const& a, ShortestWidestPathAttribute const& b) {
        return a.attribute.first == b.attribute.first && a.attribute.second == b.attribute.second;
    }

    friend bool operator!= (ShortestWidestPathAttribute const& a, ShortestWidestPathAttribute const& b) {
        return a.attribute.first != b.attribute.first || a.attribute.second != b.attribute.second;
    }

    friend bool operator< (ShortestWidestPathAttribute const& a, ShortestWidestPathAttribute const& b) {
        return (a.attribute.first > b.attribute.first || (a.attribute.first == b.attribute.first && a.attribute.second < b.attribute.second));
    }

    friend bool operator> (ShortestWidestPathAttribute const& a, ShortestWidestPathAttribute const& b) {
        return (a.attribute.first < b.attribute.first || (a.attribute.first == b.attribute.first && a.attribute.second > b.attribute.second));
    }

    friend ShortestWidestPathAttribute operator+ (ShortestWidestPathAttribute const& a, ShortestWidestPathAttribute const& b) {
        if(a.attribute.first == 0 || a.attribute.second == INT_MAX || b.attribute.first == 0 || b.attribute.second == INT_MAX)
            return ShortestWidestPathAttribute(INVALID);
        return ShortestWidestPathAttribute(std::min(a.attribute.first, b.attribute.first), a.attribute.second + b.attribute.second);
    }

    friend std::ostream & operator<< (std::ostream & os, const ShortestWidestPathAttribute & a) {	
        if(a.attribute.first == 0 || a.attribute.second == INT_MAX)
            os << " <Inf> ";
        else {
            os << " <";
            os <<  a.attribute.first << " , ";
            os << a.attribute.second;
            os << "> ";
        }
        return os; 
    }

};


#endif