#include "../Headers/nodeOneDest.h"

using namespace std;

template <class A> NodeOneDest<A>::NodeOneDest(int V, int id) {
    this->id = id;
    this->V = V;
    longestPath = 0;
    state = UPDATE;
    E = B = Av = O = A(INVALID);
    aTab = unordered_map<int, A>();
    rTab = unordered_map<int, A>();
    clear = unordered_map<int, bool>();
    diffusingMsg = unordered_map<int, bool>();
    sendUpdate = unordered_map<int, bool>();
    succ = -1;
    sendClear = false;
}

template <class A> void NodeOneDest<A>::addInLink(int neigh, A weigth) {
    inNeighbours[neigh] = weigth;
    clear[neigh] = true;
    sendUpdate[neigh] = false;
}

template <class A> void NodeOneDest<A>::addOutLink(int neigh, A weigth) {
    outNeighbours[neigh] = weigth;
    aTab[neigh] = A(INVALID);
    rTab[neigh] = A(INVALID);
    diffusingMsg[neigh] = false;
}

template <class A> void NodeOneDest<A>::removeOutLink(int neigh) {
    outNeighbours.erase(neigh);
    diffusingMsg.erase(neigh);
    aTab.erase(neigh);
    rTab.erase(neigh);
}

template <class A> void NodeOneDest<A>::removeInLink(int neigh) {
    inNeighbours.erase(neigh);
    clear.erase(neigh);
    sendUpdate.erase(neigh);
}

template <class A> void NodeOneDest<A>::resetVariables() {
    state = UPDATE;
    E = B = Av = O = A(INVALID);
    succ = -1;
    sendClear = false;
    longestPath = 0;
    for(auto &[neigh,cost] : inNeighbours) {
        clear[neigh] = true;
        sendUpdate[neigh] = false;
    }
    for(auto &[neigh,cost] : outNeighbours) {
        aTab[neigh] = A(INVALID);
        rTab[neigh] = A(INVALID);
        diffusingMsg[neigh] = false;
    }
}


#include "../Instances/nodeOneDestInstances.h"
    