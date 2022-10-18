#ifndef SHORTESTPATHMETRIC_H
#define SHORTESTPATHMETRIC_H

#include <climits>
#include "enums.h"

class ShortestPathAttribute {
public:
    int attribute;
    
    ShortestPathAttribute(SPECIAL_ATTR);
    ShortestPathAttribute(){}
    ShortestPathAttribute(int);
    ShortestPathAttribute(int, int);
    ShortestPathAttribute(int, int, int);
    int length(){return 0;}

    friend bool operator== (ShortestPathAttribute const& a, ShortestPathAttribute const& b) {
        return a.attribute == b.attribute;
    }

    friend bool operator!= (ShortestPathAttribute const& a, ShortestPathAttribute const& b) {
        return a.attribute != b.attribute;
    }

    friend bool operator< (ShortestPathAttribute const& a, ShortestPathAttribute const& b) {
        return a.attribute < b.attribute;
    }

    friend bool operator> (ShortestPathAttribute const& a, ShortestPathAttribute const& b) {
        return a.attribute > b.attribute;
    }

    friend ShortestPathAttribute operator+ (ShortestPathAttribute const& a, ShortestPathAttribute const& b) {
        if(a.attribute == INT_MAX || b.attribute == INT_MAX)
            return ShortestPathAttribute(INVALID);
        return ShortestPathAttribute(a.attribute + b.attribute);
    }

    friend std::ostream & operator<< (std::ostream & os, const ShortestPathAttribute & a) {	
        os << " <";
        if(a.attribute == INT_MAX)
            os << "Inf";
        else 
            os << a.attribute;
        os << "> ";
        return os; 
    }

};


#endif