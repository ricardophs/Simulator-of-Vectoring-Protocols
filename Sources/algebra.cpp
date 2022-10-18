#include <algorithm>
#include <iostream>
#include <vector>

#include "../Headers/algebra.h"
#include "../Headers/enums.h"

template <class A> RoutingAlgebra<A>::RoutingAlgebra() {
    this->Neutral = A(NEUTRAL);
    this->Invalid = A(INVALID);
}

template <class A> A RoutingAlgebra<A>::Extend(A const& a, A const& b) {
    return (a == this->Invalid or b == this->Invalid) ? this->Invalid : a + b;
}

template <class A> bool RoutingAlgebra<A>::Preferred(A const& a, A const& b) {
    return a < b;
}

template <class A> bool RoutingAlgebra<A>::Equal(A const& a, A const& b) {
    return a == b;
}

#include "../Instances/algebraInstance.h"
    
