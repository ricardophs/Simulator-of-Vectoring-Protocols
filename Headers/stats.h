#ifndef STATS_H
#define STATS_H

#include "enums.h"
#include <fstream>
#include <vector>

class Stats {
    int n;
    int messages = 0;
    int* messages_vec;
    int u_messages = 0;
    int* u_messages_vec;
    int d_messages = 0;
    int* d_messages_vec;
    int c_messages = 0;
    int* c_messages_vec;

    int* iterations_vec;

    int* transitions;
    int* transitions_vec;
    int* isotBreak;
    int* isotBreak_vec;

    int* lastStateChange;
    int* lastStateChange_vec;
    int* lastAttributeChange;
    int* lastAttributeChange_vec;

    int* attributeChangeCounter;
    int* attributeChangeCounter_vec;

    std::vector<float> timeInCycleBlackHole_vec;
    std::vector<float> changesCycleBlackHole_vec;


    int doubleTrials = 0;

    public:
    Stats(int);
    ~Stats();
    void createDoubleLinkFailureVec(int);

    void stateChange(int, int);
    void attributeChange(int, int);
    void incrementCounters(EventType);
    void decrementCounters(EventType);
    void updateIsotonicityBreak(int);
    void updateTransitions(int);

    void reset();
    void saveAndReset(int, int, int, int);
    void saveCycleBlackHole(int, int, int, int, std::vector<int> &, std::vector<int>&);

    void printGeneralStats(std::ofstream &, int, bool, bool=true, bool=true);
    void printGeneralStatsLF(std::ofstream &, int, bool=false, bool=true, bool=true);

    void printStableStateTimes(std::ofstream &, bool);
    void printStableStateTimesLF(std::ofstream &, bool=false);

    void printNodeLastStateChangeTimes(std::ofstream &, bool);
    void printNodeLastStateChangeTimesLF(std::ofstream &, bool=false);

    void printNodeTerminationTimes(std::ofstream &, bool);
    void printNodeTerminationTimesLF(std::ofstream &, bool=false);

    void printMessages(std::ofstream &, bool);
    void printMessagesLF(std::ofstream &, bool=false);

    void printNodeTransitions(std::ofstream &, bool, bool=false);
    void printNodeTransitionsLF(std::ofstream &, bool=false, bool=false);

    void printNodeAttributeChanges(std::ofstream &, bool);
    void printNodeAttributeChangesLF(std::ofstream &, bool=false);

    void printNodeCycleBlackHoleTimes(std::ofstream &, bool);
    void printNodeCycleBlackHoleTimesLF(std::ofstream &, bool=false);

    void printNodeCycleBlackHoleChanges(std::ofstream &, bool);
    void printNodeCycleBlackHoleChangesLF(std::ofstream &, bool=false);


};

#endif