#include <iostream>
#include <array>
#include <cstring>

#include "../Headers/stats.h"

using std::endl, std::vector, std::max, std::min;

extern int nLinks;

Stats::Stats(int n){
    this->n = n;

    this->isotBreak = new int[n]{0};
    this->transitions = new int[n]{0};
    this->lastStateChange = new int[n]{0};
    this->lastAttributeChange = new int[n]{0};
    this->attributeChangeCounter = new int[n]{0};

    const unsigned int size = max(2 * n, nLinks / 2);

    this->iterations_vec = new int[size]{0};
    this->messages_vec = new int[size]{0};
    this->u_messages_vec = new int[size]{0};
    this->d_messages_vec = new int[size]{0};
    this->c_messages_vec = new int[size]{0};

    this->isotBreak_vec = new int[size * n]{0};

    this->transitions_vec = new int[size * n]{0};
    this->lastStateChange_vec = new int[size * n]{0};
    this->lastAttributeChange_vec = new int[size * n]{0};
    this->attributeChangeCounter_vec = new int[size * n]{0};    

    this->timeInCycleBlackHole_vec = vector<float>(size * n, -1);
    this->changesCycleBlackHole_vec = vector<float>(size * n, -1);
}

Stats::~Stats() {
    delete[] this->transitions;
    delete[] this->transitions_vec;

    delete[] this->isotBreak;
    delete[] this->isotBreak_vec;

    delete[] this->iterations_vec;
    delete[] this->messages_vec;
    delete[] this->u_messages_vec;
    delete[] this->d_messages_vec;
    delete[] this->c_messages_vec;

    delete[] this->lastStateChange;
    delete[] this->lastStateChange_vec;

    delete[] this->lastAttributeChange;
    delete[] this->lastAttributeChange_vec;

    delete[] this->attributeChangeCounter;
    delete[] this->attributeChangeCounter_vec;

}

void Stats::createDoubleLinkFailureVec(int size) {
    // std::cout << "Size: " << size << "\n";
    this->doubleTrials = size;

    delete[] this->transitions_vec;
    delete[] this->attributeChangeCounter_vec;
    delete[] this->iterations_vec;
    delete[] this->messages_vec;
    delete[] this->u_messages_vec;
    delete[] this->d_messages_vec;
    delete[] this->c_messages_vec;
    delete[] this->isotBreak_vec;
    delete[] this->lastAttributeChange_vec;
    delete[] this->lastStateChange_vec;
    
    this->iterations_vec = new int[size]{0};
    this->messages_vec = new int[size]{0};
    this->u_messages_vec = new int[size]{0};
    this->d_messages_vec = new int[size]{0};
    this->c_messages_vec = new int[size]{0};
    this->isotBreak_vec = new int[size * n]{0};
    this->transitions_vec = new int[size * n]{0};
    this->lastAttributeChange_vec = new int[size * n]{0};
    this->lastStateChange_vec = new int[size * n]{0};
    this->attributeChangeCounter_vec = new int[size * n]{0};
    this->timeInCycleBlackHole_vec = vector<float>(size * n, -1);
    this->changesCycleBlackHole_vec = vector<float>(size * n, -1);
}


void Stats::updateIsotonicityBreak(int node) {
    ++(this->isotBreak[node]);
}

void Stats::updateTransitions(int node) {
    ++(this->transitions[node]);
}

void Stats::incrementCounters(EventType ev){
    if(ev == UPDATE_MSG)
        ++(this->u_messages);
    else if(ev == DIFFUSING_MSG)
        ++(this->d_messages);
    else if(ev == CLEAR_MSG)
        ++(this->c_messages);
    else 
        return;
    ++(this->messages);
}

void Stats::decrementCounters(EventType ev){
    if(ev == UPDATE_MSG)
        --(this->u_messages);
    else if(ev == DIFFUSING_MSG)
        --(this->d_messages);
    else if(ev == CLEAR_MSG)
        --(this->c_messages);
    else 
        return;
    --(this->messages);
}

void Stats::stateChange(int u, int t) {
    this->lastStateChange[u] = t;
}

void Stats::attributeChange(int u, int t) {
    this->lastAttributeChange[u] = t;
    ++(this->attributeChangeCounter[u]);
}

void Stats::reset() {
    this->messages = 0;
    this->u_messages = 0;
    this->d_messages = 0;
    this->c_messages = 0;
    memset(this->transitions, 0, sizeof(int)*this->n);
    memset(this->isotBreak, 0, sizeof(int)*this->n);
    memset(this->lastStateChange, 0, sizeof(int)*this->n);
    memset(this->lastAttributeChange, 0, sizeof(int)*this->n);
    memset(this->attributeChangeCounter, 0, sizeof(int)*this->n);
}

void Stats::saveCycleBlackHole(int node, int iter, int type, int linkFailureNum, vector<int> &timesInCycleBlackHole, vector<int> &changesInCycleBlackHole) {
    // for(const auto a : timesInCycleBlackHole) 
    //     std::cout << a << " ";
    // std::cout << "\n";    
    if(linkFailureNum != -1) {
        for(int i = 0; i < this->n; ++i) {
            if(changesInCycleBlackHole[i] != -1) {
                // this->timeInCycleBlackHole_vec[linkFailureNum * this->n + i] = max(
                //                 this->timeInCycleBlackHole_vec[linkFailureNum * this->n + i], 
                //                  (float)timesInCycleBlackHole[i]);
                if(this->changesCycleBlackHole_vec[linkFailureNum * this->n + i] == -1) 
                    ++this->changesCycleBlackHole_vec[linkFailureNum * this->n + i];
                this->changesCycleBlackHole_vec[linkFailureNum * this->n + i] += (float)changesInCycleBlackHole[i];
            }
            if(timesInCycleBlackHole[i] != -1) {
                // this->timeInCycleBlackHole_vec[linkFailureNum * this->n + i] = max(
                //                 this->timeInCycleBlackHole_vec[linkFailureNum * this->n + i], 
                //                  (float)timesInCycleBlackHole[i]);
                if(this->timeInCycleBlackHole_vec[linkFailureNum * this->n + i] == -1) 
                    ++this->timeInCycleBlackHole_vec[linkFailureNum * this->n + i];
                this->timeInCycleBlackHole_vec[linkFailureNum * this->n + i] += (float)timesInCycleBlackHole[i];
            }
        }
    } 
    else {
        if(type == 1) 
            node += this->n;
   
        for(int i = 0; i < this->n; ++i) {
            if(changesInCycleBlackHole[i] != -1)
                this->changesCycleBlackHole_vec[node * this->n + i] = changesInCycleBlackHole[i];
            if(timesInCycleBlackHole[i] != -1)
                this->timeInCycleBlackHole_vec[node * this->n + i] = timesInCycleBlackHole[i];
        }
    }
}

void Stats::saveAndReset(int node, int iter, int type, int linkFailureNum) {

    if(type == 2 || type == 3) {
        this->iterations_vec[linkFailureNum] = max(this->iterations_vec[linkFailureNum], iter);
        this->messages_vec[linkFailureNum] += this->messages;
        this->u_messages_vec[linkFailureNum] += this->u_messages;
        this->d_messages_vec[linkFailureNum] += this->d_messages;
        this->c_messages_vec[linkFailureNum] += this->c_messages;

        for(int i = 0; i < this->n; ++i) {
            this->lastStateChange_vec[linkFailureNum * this->n + i] = max(this->lastStateChange_vec[linkFailureNum * this->n + i], this->lastStateChange[i]);
            this->lastAttributeChange_vec[linkFailureNum * this->n + i] = max(this->lastAttributeChange_vec[linkFailureNum * this->n + i], this->lastAttributeChange[i]);
            this->transitions_vec[linkFailureNum * this->n + i] = max(this->transitions_vec[linkFailureNum * this->n + i], this->transitions[i]);
            this->attributeChangeCounter_vec[linkFailureNum * this->n + i] = max(this->attributeChangeCounter_vec[linkFailureNum * this->n + i], this->attributeChangeCounter[i]);
            this->isotBreak_vec[linkFailureNum * this->n + i] = this->isotBreak[i];
        }
    } 
    else {
        if(type == 1) 
            node += this->n;
        
        if(node != -1) {
            this->iterations_vec[node] = iter;
            this->messages_vec[node] = this->messages;
            this->u_messages_vec[node] = this->u_messages;
            this->d_messages_vec[node] = this->d_messages;
            this->c_messages_vec[node] = this->c_messages;
        
            for(int i = 0; i < this->n; ++i) {
                this->transitions_vec[node * this->n + i] = this->transitions[i];
                this->attributeChangeCounter_vec[node * this->n + i] = this->attributeChangeCounter[i];
                this->isotBreak_vec[node * this->n + i] = this->isotBreak[i];
                this->lastStateChange_vec[node * this->n + i] = this->lastStateChange[i];
                this->lastAttributeChange_vec[node * this->n + i] = this->lastAttributeChange[i];
            }
        }
    }

    this->messages = 0;
    this->u_messages = 0;
    this->d_messages = 0;
    this->c_messages = 0;
    memset(this->transitions, 0, sizeof(int)*this->n);
    memset(this->attributeChangeCounter, 0, sizeof(int)*this->n);
    memset(this->isotBreak, 0, sizeof(int)*this->n);
}


/***** PRINTS *****/

void Stats::printGeneralStats(std::ofstream & out, int dest, bool withdraw, bool eigrp, bool sync) {
    if(!withdraw)
        out << "\n##### Stats of Announcement Phase #####\n\n";
    else
        out << "\n##### Stats of Withdrawal Phase #####\n\n";
    
    int start = 0, end = this->n - 1;
    if(dest != -1) start = end = dest;
    if(withdraw) {
        start += this->n;
        end += this->n;
    }
    
    float avg_iter = 0, avg_msg = 0, avg_u_msg = 0, avg_d_msg = 0, avg_c_msg = 0;
    int min_msg = (int)1e8, max_msg = 0;
    float avg_isot_break = 0, avg_transit = 0;
    int min_isot_break = (int)1e8, min_transit = (int)1e8, max_isot_break = 0, max_transit = 0;
    float min_iter = 1e8, max_iter = 0;
    float avg_term_time = 0, min_term_time = 1e8, max_term_time = 0;
    float avg_attr_changes = 0;
    int min_attr_changes = (int)1e8, max_attr_changes = 0;

    float avg_cycleTime = 0;
    float min_cycleTime = (int)1e8, max_cycleTime = 0;
    float avg_cycleChanges = 0;
    float min_cycleChanges = (int)1e8, max_cycleChanges = 0;
    int counter = 0, counter2 = 0;

    for(int d = start; d <= end; ++d) {
        avg_iter += this->iterations_vec[d];
        min_iter = min(min_iter, (float) this->iterations_vec[d]);
        max_iter = max(max_iter, (float) this->iterations_vec[d]);
        avg_msg += this->messages_vec[d];
        min_msg = min(min_msg, this->messages_vec[d]);
        max_msg = max(max_msg, this->messages_vec[d]);
        avg_u_msg += this->u_messages_vec[d];
        avg_d_msg += this->d_messages_vec[d];
        avg_c_msg += this->c_messages_vec[d];
        for(int i = 0; i < this->n; ++i) {
            avg_transit += this->transitions_vec[d * this->n + i];
            min_transit = min(min_transit, this->transitions_vec[d * this->n + i]);
            max_transit = max(max_transit, this->transitions_vec[d * this->n + i]);
            avg_isot_break += this->isotBreak_vec[d * this->n + i];
            min_isot_break = min(min_isot_break, this->isotBreak_vec[d * this->n + i]);
            max_isot_break = max(max_isot_break, this->isotBreak_vec[d * this->n + i]);
            avg_term_time += this->lastAttributeChange_vec[d * this->n + i];
            min_term_time = min(min_term_time, (float) this->lastAttributeChange_vec[d * this->n + i]);
            max_term_time = max(max_term_time, (float) this->lastAttributeChange_vec[d * this->n + i]);
            avg_attr_changes += this->attributeChangeCounter_vec[d * this->n + i];
            min_attr_changes = min(min_attr_changes, this->attributeChangeCounter_vec[d * this->n + i]);
            max_attr_changes = max(max_attr_changes, this->attributeChangeCounter_vec[d * this->n + i]);
            if(timeInCycleBlackHole_vec[d * this->n + i] != -1) {
                ++counter;
                avg_cycleTime += this->timeInCycleBlackHole_vec[d * this->n + i];
                min_cycleTime = min(min_cycleTime, this->timeInCycleBlackHole_vec[d * this->n + i]);
                max_cycleTime = max(max_cycleTime, this->timeInCycleBlackHole_vec[d * this->n + i]);
            }
            if(changesCycleBlackHole_vec[d * this->n + i] != -1) {
                ++counter2;
                avg_cycleChanges += this->changesCycleBlackHole_vec[d * this->n + i];
                min_cycleChanges = min(min_cycleChanges, this->changesCycleBlackHole_vec[d * this->n + i]);
                max_cycleChanges = max(max_cycleChanges, this->changesCycleBlackHole_vec[d * this->n + i]);
            }
        }
    }

    avg_iter /= (end - start + 1); 
    avg_msg /= (end - start + 1);
    avg_u_msg /= (end - start + 1);
    avg_d_msg /= (end - start + 1);
    avg_c_msg /= (end - start + 1);
    avg_isot_break /= ( (end - start + 1) * this->n);
    avg_transit /= ( (end - start + 1) * this->n) ;
    avg_term_time /= ( (end - start + 1) * this->n);
    avg_attr_changes /= ( (end - start + 1) * this->n);

    // std::cout << counter << " " << counter2 << "\n";

    if(counter != 0)
        avg_cycleTime /= counter;
    if(counter2 != 0)
        avg_cycleChanges /= counter2;

    if(!sync) {
        avg_iter /= 1000;
        min_iter /= 1000;
        max_iter /= 1000;
        avg_term_time /= 1000;
        min_term_time /= 1000;
        max_term_time /= 1000;
        avg_cycleTime /= 1000;
        min_cycleTime /= 1000;
        max_cycleTime /= 1000;
    }

    out << "# Number of " << (withdraw ? "Withdrawal trials: " : "Announcement trials: ") << (end - start + 1) << "\n\n";

    out << "# Average (Minimum / Maximum) " << (sync ? "Number of Iterations" : "Time");
    out << " until Stable State: " << avg_iter << " (" << min_iter << " / " << max_iter << (sync ? ") iterations" : ") ms") << "\n\n";

    out << "# Average (Minimum / Maximum) Termination Times per Node: " << avg_term_time << " (" << min_term_time << " / " << max_term_time << (sync ? ") iterations" : ") ms") << "\n\n";

    out << "# Average (Minimum / Maximum) Number of Attribute Changes per Node: " << avg_attr_changes << " (" << min_attr_changes << " / " << max_attr_changes << ") changes \n\n";
        
    out << "# Average (Minimum / Maximum) Number of Messages Exchanged: ";
    out << avg_msg << " (" << min_msg << " / " << max_msg << ") messages\n";
    if(eigrp) {
        out << "\tAverage Number of UPDATE Messages Exchanged: " << avg_u_msg << "\n";
        if(avg_d_msg > 0) {
            out << "\tAverage Number of DIFFUSING (CLEAR) Messages Exchanged: " << avg_d_msg << "\n";
            out << "Average (Minimum/Maximum) Number of UPDATE->DIFFUSING transitions per node: ";
            out << avg_transit << " (" << min_transit << " / " << max_transit << ")\n";
            out << "Average (Minimum/Maximum) Number of isotonicity breaks (UPDATE message causes a diffusion): ";
            out << avg_isot_break << " (" << min_isot_break << " / " << max_isot_break << ")\n";
        }
    }
    if(counter != 0) {
        out << "\n# Average (Minimum / Maximum) Time that a node is ";
        out << (eigrp ? "Black Holed" : "in a Cycle") << ": ";
        out << avg_cycleTime << " (" << min_cycleTime << " / " << max_cycleTime << ")";
        out << (sync ? " iterations" : " ms") << "\n";
    }
    if(counter2 != 0) {
        out << "\n# Average (Minimum / Maximum) Attribute changes that formed/maintained a ";
        out << (eigrp ? "Black Holed" : "Cycle") << ": ";
        out << avg_cycleChanges << " (" << min_cycleChanges << " / " << max_cycleChanges << ")\n";
    }
    out << "\n#######################################\n\n";
}

void Stats::printGeneralStatsLF(std::ofstream & out, int dest, bool doub, bool eigrp, bool sync) {
    if(!doub)
        out << "\n##### Stats for Single Link Failure #####\n\n";
    else
        out << "\n##### Stats for Double Link Failure #####\n\n";
    
    int size = (doub ? this->doubleTrials : nLinks/2);
    int *vec = (doub ? this->lastAttributeChange_vec : this->lastAttributeChange_vec); 
    
    float avg_iter = 0, avg_msg = 0, avg_u_msg = 0, avg_d_msg = 0, avg_c_msg = 0;
    int min_msg = (int)1e8, max_msg = 0;
    float avg_isot_break = 0, avg_transit = 0;
    int min_isot_break = (int)1e8, min_transit = (int)1e8, max_isot_break = 0, max_transit = 0;
    float min_iter = 1e8, max_iter = 0;
    float avg_term_time = 0, min_term_time = 1e8, max_term_time = 0;
    float avg_attr_changes = 0;
    int min_attr_changes = (int)1e8, max_attr_changes = 0;

    float avg_cycleTime = 0;
    float min_cycleTime = (int)1e8, max_cycleTime = 0;
    float avg_cycleChanges = 0;
    float min_cycleChanges = (int)1e8, max_cycleChanges = 0;
    int counter = 0, counter2 = 0;
    
    for(int l = 0; l < size; ++l) {
        avg_iter += this->iterations_vec[l];
        min_iter = min(min_iter, (float) this->iterations_vec[l]);
        max_iter = max(max_iter, (float) this->iterations_vec[l]);
        avg_msg += this->messages_vec[l];
        min_msg = min(min_msg, this->messages_vec[l]);
        max_msg = max(max_msg, this->messages_vec[l]);
        avg_u_msg += this->u_messages_vec[l];
        avg_d_msg += this->d_messages_vec[l];
        avg_c_msg += this->c_messages_vec[l];
        for(int i = 0; i < this->n; ++i) {
            avg_transit += this->transitions_vec[l * this->n + i];
            min_transit = min(min_transit, this->transitions_vec[l * this->n + i]);
            max_transit = max(max_transit, this->transitions_vec[l * this->n + i]);
            avg_isot_break += this->isotBreak_vec[l * this->n + i];
            min_isot_break = min(min_isot_break, this->isotBreak_vec[l * this->n + i]);
            max_isot_break = max(max_isot_break, this->isotBreak_vec[l * this->n + i]);
            avg_term_time += vec[l * this->n + i];
            min_term_time = min(min_term_time, (float) vec[l * this->n + i]);
            max_term_time = max(max_term_time, (float) vec[l * this->n + i]);
            avg_attr_changes += this->attributeChangeCounter_vec[l * this->n + i];
            min_attr_changes = min(min_attr_changes, this->attributeChangeCounter_vec[l * this->n + i]);
            max_attr_changes = max(max_attr_changes, this->attributeChangeCounter_vec[l * this->n + i]);
            if(timeInCycleBlackHole_vec[l * this->n + i] != -1) {
                ++counter;
                avg_cycleTime += this->timeInCycleBlackHole_vec[l * this->n + i];
                min_cycleTime = min(min_cycleTime, this->timeInCycleBlackHole_vec[l * this->n + i]);
                max_cycleTime = max(max_cycleTime, this->timeInCycleBlackHole_vec[l * this->n + i]);
            }
            if(changesCycleBlackHole_vec[l * this->n + i] != -1) {
                ++counter2;
                avg_cycleChanges += this->changesCycleBlackHole_vec[l * this->n + i];
                min_cycleChanges = min(min_cycleChanges, this->changesCycleBlackHole_vec[l * this->n + i]);
                max_cycleChanges = max(max_cycleChanges, this->changesCycleBlackHole_vec[l * this->n + i]);
            }
        }
    }

    avg_iter /= size; 
    avg_msg /= size;
    avg_u_msg /= size;
    avg_d_msg /= size;
    avg_c_msg /= size;
    avg_isot_break /= ( size * this->n );
    avg_transit /= ( size * this->n ) ;
    avg_term_time /= ( size * this->n );
    avg_attr_changes /= ( size * this->n );
    // std::cout << counter << " " << counter2 << "\n";
    if(counter != 0) 
        avg_cycleTime /= counter;
    if(counter2 != 0)
        avg_cycleChanges /= counter2;

    if(!sync) {
        avg_iter /= 1000;
        min_iter /= 1000;
        max_iter /= 1000;
        avg_term_time /= 1000;
        min_term_time /= 1000;
        max_term_time /= 1000;
        avg_cycleTime /= 1000;
        min_cycleTime /= 1000;
        max_cycleTime /= 1000;
    }

    out << "# Number of Link Failure trials: " << size << "\n\n";

    out << "# Average (Minimum / Maximum) " << (sync ? "Number of Iterations" : "Time");
    out << " until Stable State: " << avg_iter << " (" << min_iter << " / " << max_iter << (sync ? ") iterations" : ") ms") << "\n\n";

    out << "# Average (Minimum / Maximum) Termination Times per Node: " << avg_term_time << " (" << min_term_time << " / " << max_term_time << (sync ? ") iterations" : ") ms") << "\n\n";

    out << "# Average (Minimum / Maximum) Number of Attribute Changes per Node: " << avg_attr_changes << " (" << min_attr_changes << " / " << max_attr_changes << ") changes \n\n";
        
    out << "# Average (Minimum / Maximum) Number of Messages Exchanged: ";
    out << avg_msg << " (" << min_msg << " / " << max_msg << ") messages\n";
    if(eigrp) {
        out << "\tAverage Number of UPDATE Messages Exchanged: " << avg_u_msg << "\n";
        if(avg_d_msg > 0) {
            out << "\tAverage Number of DIFFUSING (CLEAR) Messages Exchanged: " << avg_d_msg << "\n";
            out << "Average (Minimum/Maximum) Number of UPDATE->DIFFUSING transitions per node: ";
            out << avg_transit << " (" << min_transit << " / " << max_transit << ")\n";
            out << "Average (Minimum/Maximum) Number of isotonicity breaks (UPDATE message causes a diffusion): ";
            out << avg_isot_break << " (" << min_isot_break << " / " << max_isot_break << ")\n";
        }
    }
    if(counter != 0) {
        out << "\n# Average (Minimum / Maximum) Time that a node is ";
        out << (eigrp ? "Black Holed" : "in a Cycle") << ": ";
        out << avg_cycleTime << " (" << min_cycleTime << " / " << max_cycleTime << ")";
        out << (sync ? " iterations" : " ms") << "\n";
    }
    if(counter2 != 0) {
        out << "\n# Average (Minimum / Maximum) Attribute changes that formed/maintained a ";
        out << (eigrp ? "Black Holed" : "Cycle") << ": ";
        out << avg_cycleChanges << " (" << min_cycleChanges << " / " << max_cycleChanges << ")\n";
    }
    out << "\n#######################################\n\n";
}


void Stats::printStableStateTimes(std::ofstream & out, bool withdraw) {  
    int start = 0, end = this->n - 1;
    int inc = (withdraw ? this->n : 0);

    out << this->n << "\n";
    for(int d = start; d <= end; ++d) {
        out << d << " " << this->iterations_vec[d+inc] << "\n";
    } 
}

void Stats::printStableStateTimesLF(std::ofstream & out, bool doub) {  
    int size = (doub ? this->doubleTrials : nLinks/2);

    out << this->n << " " << size << "\n";
    for(int l = 0; l < size; ++l) {
        out << l << " " << this->iterations_vec[l] << "\n";     
    }
}


void Stats::printNodeLastStateChangeTimes(std::ofstream & out, bool withdraw) {  
    int start = 0, end = this->n - 1;
    int inc = (withdraw ? this->n : 0);
    out << this->n << "\n";
    for(int d = start; d <= end; ++d) {
        out << d << " ";
        for(int u = 0; u < this->n; ++u)
            out << this->lastStateChange_vec[(d + inc)*this->n + u] << " ";
        out << "\n";        
    }
}

void Stats::printNodeLastStateChangeTimesLF(std::ofstream & out, bool doub) {  
    int start = 0, end = this->n - 1;
    int size = (doub ? this->doubleTrials : nLinks/2);

    out << this->n << " " << size << "\n";
    for(int l = 0; l < size; ++l) {
        out << l << ": ";
        for(int d = start; d <= end; ++d) 
            out << this->lastStateChange_vec[l * this->n + d] << " ";
        out << "\n";        
    }
}


void Stats::printNodeTerminationTimes(std::ofstream & out, bool withdraw) {  
    int start = 0, end = this->n - 1;
    int inc = (withdraw ? this->n : 0);
    out << this->n << "\n";
    for(int d = start; d <= end; ++d) {
        out << d << " ";
        for(int u = 0; u < this->n; ++u)
            out << this->lastAttributeChange_vec[(d + inc)*this->n + u] << " ";
        out << "\n";        
    }
}

void Stats::printNodeTerminationTimesLF(std::ofstream & out, bool doub) {  
    int start = 0, end = this->n - 1;
    int size = (doub ? this->doubleTrials : nLinks/2);

    out << this->n << " " << size << "\n";
    for(int l = 0; l < size; ++l) {
        out << l << ": ";
        for(int d = start; d <= end; ++d) 
            out << this->lastAttributeChange_vec[l * this->n + d] << " ";
        out << "\n";        
    }
}


void Stats::printNodeTransitions(std::ofstream & out, bool withdraw, bool isEIGRP) {  
    if(!isEIGRP) return;
    int start = 0, end = this->n - 1;
    int inc = (withdraw ? this->n : 0);
    out << this->n << "\n";
    for(int d = start; d <= end; ++d) {
        out << d << " ";
        for(int u = 0; u < this->n; ++u)
            out << this->transitions_vec[(d + inc)*this->n + u] << " ";
        out << "\n";        
    }
}

void Stats::printNodeTransitionsLF(std::ofstream & out, bool isEIGRP, bool doub) {  
    if(!isEIGRP) return;
    int start = 0, end = this->n - 1;
    int size = (doub ? this->doubleTrials : nLinks/2);

    out << this->n << " " << size << "\n";
    for(int l = 0; l < size; ++l) {
        out << l << ": ";
        for(int d = start; d <= end; ++d) 
            out << this->transitions_vec[l * this->n + d] << " ";
        out << "\n";        
    }
}


void Stats::printNodeAttributeChanges(std::ofstream & out, bool withdraw) {  
    int start = 0, end = this->n - 1;
    int inc = (withdraw ? this->n : 0);
    out << this->n << "\n";
    for(int d = start; d <= end; ++d) {
        out << d << " ";
        for(int u = 0; u < this->n; ++u)
            out << this->attributeChangeCounter_vec[(d + inc)*this->n + u] << " ";
        out << "\n";        
    }
}

void Stats::printNodeAttributeChangesLF(std::ofstream & out, bool doub) {  
    int start = 0, end = this->n - 1;
    int size = (doub ? this->doubleTrials : nLinks/2);

    out << this->n << " " << size << "\n";
    for(int l = 0; l < size; ++l) {
        out << l << ": ";
        for(int d = start; d <= end; ++d) 
            out << this->attributeChangeCounter_vec[l * this->n + d] << " ";
        out << "\n";        
    }
}


void Stats::printMessages(std::ofstream & out, bool withdraw) {  
    int start = 0, end = this->n - 1;
    int inc = (withdraw ? this->n : 0);
    out << this->n << "\n";
    for(int d = start; d <= end; ++d) {
        out << d << " ";
        out << this->messages_vec[d + inc] << " " << this->u_messages_vec[d + inc] << " " << this->d_messages_vec[d + inc];
        out << "\n";        
    }
}

void Stats::printMessagesLF(std::ofstream & out, bool doub) {  
    int size = (doub ? this->doubleTrials : nLinks/2);

    out << this->n << " " << size << "\n";
    for(int l = 0; l < size; ++l) {
        out << l << ": " <<  this->messages_vec[l] << " ";
        out << this->u_messages_vec[l] << " ";
        out << this->d_messages_vec[l] << " ";
        out << this->c_messages_vec[l] << "\n";
    }
}


void Stats::printNodeCycleBlackHoleTimes(std::ofstream & out, bool withdraw) {  
    int start = 0, end = this->n - 1;
    int inc = (withdraw ? this->n : 0);
    out << this->n << "\n";
    for(int d = start; d <= end; ++d) {
        out << d << " ";
        for(int u = 0; u < this->n; ++u)
            out << this->timeInCycleBlackHole_vec[(d + inc)*this->n + u] << " ";
        out << "\n";        
    }
}

void Stats::printNodeCycleBlackHoleTimesLF(std::ofstream & out, bool doub) {  
    int start = 0, end = this->n - 1;
    int size = (doub ? this->doubleTrials : nLinks/2);

    out << this->n << " " << size << "\n";
    for(int l = 0; l < size; ++l) {
        out << l << ": ";
        for(int d = start; d <= end; ++d) 
            out << this->timeInCycleBlackHole_vec[l * this->n + d] << " ";
        out << "\n";        
    }
}


void Stats::printNodeCycleBlackHoleChanges(std::ofstream & out, bool withdraw) {  
    int start = 0, end = this->n - 1;
    int inc = (withdraw ? this->n : 0);
    out << this->n << "\n";
    for(int d = start; d <= end; ++d) {
        out << d << " ";
        for(int u = 0; u < this->n; ++u)
            out << this->changesCycleBlackHole_vec[(d + inc)*this->n + u] << " ";
        out << "\n";        
    }
}

void Stats::printNodeCycleBlackHoleChangesLF(std::ofstream & out, bool doub) {  
    int start = 0, end = this->n - 1;
    int size = (doub ? this->doubleTrials : nLinks/2);

    out << this->n << " " << size << "\n";
    for(int l = 0; l < size; ++l) {
        out << l << ": ";
        for(int d = start; d <= end; ++d) 
            out << this->changesCycleBlackHole_vec[l * this->n + d] << " ";
        out << "\n";        
    }
}


