#ifndef BGPATTRIBUTE_H
#define BGPATTRIBUTE_H

#include "enums.h"
#include <iostream>
#include <vector>
#include <algorithm>

template <class A> class BGPAttribute {
public:
    A attribute;
    std::vector<int> AS_PATH;    
    
    BGPAttribute(){}
    BGPAttribute(int){}
    BGPAttribute(int asn, int l, int w=-1);
    BGPAttribute(SPECIAL_ATTR attr_type, int asn=0);
    BGPAttribute(std::vector<int> &as_path, int l, int w=-1);
    BGPAttribute(std::vector<int> &as_path, A attr);
    int length();

    friend bool operator== (BGPAttribute const& a, BGPAttribute const& b) {
        return a.attribute == b.attribute && a.AS_PATH == b.AS_PATH;
    }

    /* Two BGP attributes are deemed different if their attribute is different (or if the
     length of AS_PATH is different); != is not the negation of ==, in this case */
    friend bool operator!= (BGPAttribute const& a, BGPAttribute const& b) {
        return a.attribute != b.attribute;
        // return (a.attribute != b.attribute || a.AS_PATH.size() != b.AS_PATH.size());
    }

    friend bool operator< (BGPAttribute const& a, BGPAttribute const& b) {
        /* Preorder in BGP attributes: the AS_PATH is not used for ordering purposes */
        return ( a.attribute < b.attribute );
        /* Preorder in BGP attributes: only the AS_PATH length is used for ordering purposes;
          if two atributes have different AS_PATHS of equal lengths, they are equaly preferred, although different */
        // return ( a.attribute < b.attribute || (a.attribute == b.attribute && a.AS_PATH.size() < b.AS_PATH.size()) );
        /* Total order in BGP attributes: if two AS_PATHS have the same length, the one that is lexicographically smaller 
          is preferred */
        // return ( a.attribute < b.attribute || 
        //          (a.attribute == b.attribute && a.AS_PATH.size() < b.AS_PATH.size()) ||
        //          (a.attribute == b.attribute && a.AS_PATH.size() == b.AS_PATH.size() && a.AS_PATH.size() > 1 && a.AS_PATH[a.AS_PATH.size()-2] < b.AS_PATH[b.AS_PATH.size()-2]) );
    }

    friend BGPAttribute operator+ (BGPAttribute const& link, BGPAttribute const& a) {
        // Paths with more than 25 hops are discarded
        if(a.AS_PATH.size() > 35)
            return BGPAttribute(INVALID);
        // If the node at the tail of the link is already in the AS_PATH of the attribute, the extension yields an invalid path
        if(std::find(a.AS_PATH.begin(), a.AS_PATH.end(), link.AS_PATH[0]) != a.AS_PATH.end())
            return BGPAttribute(INVALID);
        // Append the node at the tail of the link to the AS_PATH
        std::vector newAS_PATH(a.AS_PATH);
        newAS_PATH.push_back(link.AS_PATH[0]);

        return BGPAttribute(newAS_PATH, a.attribute + link.attribute);
    }

    friend std::ostream & operator<< (std::ostream & os, const BGPAttribute & a) {	
        os << " <";
        os << a.attribute << ",[ ";
        for(auto as : a.AS_PATH)
            os << as << " ";
        os << "]> ";
        return os; 
    }

};


#endif