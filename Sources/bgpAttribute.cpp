#include "../Headers/bgpAttribute.h"

template <class A> BGPAttribute<A>::BGPAttribute(int asn, int l, int w){
    if(w == -1)
        this->attribute = A(l);
    else 
        this->attribute = A(w,l);
    this->AS_PATH = {asn};
}

template <class A> int BGPAttribute<A>::length() {
    return (int) this->AS_PATH.size();
}

template <class A> BGPAttribute<A>::BGPAttribute(SPECIAL_ATTR attr_type, int asn) {
    this->attribute = A(attr_type);
    this->AS_PATH = (attr_type == INVALID) ? std::vector<int>() : std::vector<int>(1,asn);
}

template <class A> BGPAttribute<A>::BGPAttribute(std::vector<int> &as_path, int l, int w) {
    if(w == -1)
        this->attribute = A(l);
    else
        this->attribute = A(w,l);
    this->AS_PATH = as_path;
}

template <class A> BGPAttribute<A>::BGPAttribute(std::vector<int> &as_path, A attr){
    this->attribute = attr;
    this->AS_PATH = as_path;
}


#include "../Instances/bgpAttributeInstance.h"