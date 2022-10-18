#ifndef ALGEBRA_H
#define ALGEBRA_H


template <class A> class RoutingAlgebra {
public:
    A Neutral, Invalid;

    RoutingAlgebra();
    A Extend(A const &, A const &);
    bool Preferred(A const &, A const &);
    bool Equal(A const &, A const &);
};

#endif