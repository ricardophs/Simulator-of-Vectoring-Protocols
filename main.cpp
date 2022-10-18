#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <random>
#include <array>

#include "Headers/enums.h"
#include "Headers/stats.h"

#include "Headers/shortestWidestPathMetric.h"
#include "Headers/widestShortestPathMetric.h"
#include "Headers/shortestPathMetric.h"
#include "Headers/eigrpMetric.h"
#include "Headers/bgpAttribute.h"

#include "Headers/algebra.h"

#include "Headers/nodeOneDest.h"

#include "Headers/event.h"

#include "Headers/utilities.h"

#include "Headers/syncSimulation.h"
#include "Headers/asyncSimulation.h"


using std::stringstream, std::ofstream, std::string,
      std::vector, std::transform, std::cout,
      std::random_device, std::mt19937, 
      std::uniform_int_distribution, std::normal_distribution;

Stats *stats = nullptr;
int nNodes = 0;
unsigned long GseqNum = 0;
int nLinks = 0;

random_device rd;     // only used once to initialise (seed) engine
mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
uniform_int_distribution<int> uniform1(0,40);
uniform_int_distribution<int> uniform2(75,125);
normal_distribution<double> gaussian(10,2);


/* Get the program arguments from the command line */
bool getArgs(int argc, char **argv, string &ASFilePath, int *dest, int *prot, int *metr, int *sync, int *type, int *prints, int *del) {
    
    if (argc < 2) {
        cout << "Not enough arguments. Provide a network file.\nExiting\n";
        return false;
    }
    
    ASFilePath = "Networks/" + string(argv[1]) + ".csv";

    vector<string> args;

    for(int i = 2; i < argc ; ++i) {
        string arg(argv[i]);
        transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
        args.push_back(arg);
    }
    
    for(int i = 0; i < argc - 2 ; ++i) {
        if(args[i].compare("-p") == 0 || args[i].compare("-protocol") == 0) {
            if(i < argc - 3) {
                if(args[i+1].compare("eigrp") == 0) *prot = 1;
                else if(args[i+1].compare("bgp") == 0) *prot = 2;
                else if(args[i+1].compare("vp") == 0) *prot = 4;
                else if(args[i+1].compare("eigrpfc") == 0) *prot = 5;
                else {
                    cout << "Invalid protocol option following " << args[i] << " '"<< args[i+1] <<"'\nExiting\n";
                    return false;
                }
            } else{
                cout << "No protocol option following " << args[i] << "\nExiting\n";
                return false;
            }
        }
        else if(args[i].compare("-m") == 0 || args[i].compare("-metric") == 0) {
            if(i < argc - 3) {
                if(args[i+1].compare("sp") == 0) *metr = 1;
                else if(args[i+1].compare("swp") == 0) *metr = 2;
                else if(args[i+1].compare("wp") == 0) *metr = 3;
                else if(args[i+1].compare("hc") == 0) *metr = 4;
                else if(args[i+1].compare("em") == 0) *metr = 5;
                else if(args[i+1].compare("wsp") == 0) *metr = 6;
                else {
                    cout << "Invalid metric option following " << args[i] << " '"<< args[i+1] <<"'\nExiting\n";
                    return false;
                }
            } else {
                cout << "No protocol option following " << args[i] << "\nExiting\n";
                return false;
            }
        }
        else if(args[i].compare("-t") == 0 || args[i].compare("-type") == 0) {
            if(i < argc - 3) {
                if(args[i+1].compare("sync") == 0) *sync = 1;
                else if(args[i+1].compare("async") == 0) *sync = 0;
                else if(args[i+1].compare("quasi") == 0) *sync = 2;
                else{
                    cout << "Invalid type option following " << args[i] << " '"<< args[i+1] <<"'\nExiting\n";
                    return false;
                }
            } else {
                cout << "No type option following " << args[i] << "\nExiting\n";
                return false;
            }
        }
        else if(args[i].compare("-d") == 0 || args[i].compare("-destination") == 0) {
            if(i < argc - 3) {
                if(!(stringstream(args[i+1]) >> *dest) || *dest < 0) {
                    cout << "Invalid destination node following " << args[i] << " '"<< args[i+1] <<"'\nExiting\n";
                    return false;
                }
            } else {
                cout << "No destination node following " << args[i] << "\nExiting\n";
                return false;
            }
        }
        else if(args[i].compare("-w") == 0 || args[i].compare("-advertise") == 0) {
            *type = 0;
        }
        else if(args[i].compare("-adv") == 0 || args[i].compare("-advertise") == 0) {
            *type = 1;
        }
        else if(args[i].compare("-lf") == 0 || args[i].compare("-link_failure") == 0) {
            *type = 2;
        }
        else if(args[i].compare("-lf2") == 0 || args[i].compare("-double_link_failure") == 0) {
            *type = 3;
        }
        else if(args[i].compare("-v") == 0 || args[i].compare("-verbose") == 0) {
            *prints = true;
        }
        else if(args[i].compare("-del") == 0 || args[i].compare("-delays") == 0) {
            if(i < argc - 3) {
                if(!(stringstream(args[i+1]) >> *del) || *del < 0 || *del > 3){
                    cout << "Invalid delay option following " << args[i] << " '"<< args[i+1] <<"'\nExiting\n";
                    return false;
                }
            } else {
                cout << "No delay option following " << args[i] << "\nExiting\n";
                return false;
            }
        }
    }
    return true;
}

/* Turn program arguments into strings, for file naming purposes */
void opt2string(int prot, string &p_str, 
                int metr, string &m_str, 
                int sync, string &s_str,
                int type, string &t_str,
                string &ASFilePath, string &ASName) {

    p_str = (prot == 1) ? string("EIGRP") : ((prot == 4) ? string("VP") : ((prot == 5) ? string("EIGRPFC") : string("BGP")));
    s_str = (sync == 1) ? string("SYNC") : ((sync == 0) ? string("ASYNC") : string("QuasiSYNC"));
    t_str = (type == 0) ? string("_W") : (type == 2 ? string("_LF") : (type == 3 ? string("_LF2") : string("_ADV")));
    switch (metr) {
        case 1:
            m_str = string("SP");
            break;
        case 2:
            m_str = string("SWP");
            break;
        case 3:
            m_str = string("WP");
            break;
        case 4:
            m_str = string("HOPS");
            break;
        case 5:
            m_str = string("COMPOSITE");
            break;
        case 6:
            m_str = string("WSP");
            break;
        default:
            break;
    }
    ASName = ASFilePath.substr(9, ASFilePath.length() - 13);
}

int main(int argc, char **argv)
{
    int dest = -1;
    int prot = 1;
    int metr = 4;
    int sync = 1;
    int type = 0;
    int prints = false;
    int del = 1;
    string ASFilePath;

    if(!getArgs(argc, argv, ASFilePath, &dest, &prot, &metr, &sync, &type, &prints, &del))
        return 0;

    if(sync == 2 && (prot == 1 || prot == 4 || prot == 5)) {
        cout << "EIGRP / VP Quasi Sync versions not implemented!\n";
        cout << "Running EIGRP / VP Async instead.\n";
        sync = 0;
    }
    if(sync == 1 && (prot == 4 || prot == 5)) {
        cout << "EIGRP-FC / VP Sync version not implemented!\n";
        cout << "Running BGP Sync instead.\n";
        sync = 0;
        prot = 2;
    }
    if(prot == 4 && (type != 1 && type != 2)) {
        cout << "Only Announcement and Single Link Failure implemented for VP!\n";
        if(type == 0) {
            cout << "Running VP Announcement instead.\n";
            type = 1;
        }
        else if(type == 3) {
            cout << "Running VP Single Link Failure instead.\n";
            type = 2;
        }
    }

    string p_str, m_str, s_str, t_str, ASName;
    opt2string(prot, p_str, metr, m_str, sync, s_str, type, t_str, ASFilePath, ASName);

    cout << "Running " << ASName << " with metric " << m_str << " and protocol " << p_str << " " << s_str << " " << t_str << "\n";

    switch (metr) {
        case 1:
            if (sync == 1) {
                if (prot == 1)
                    simulateSync<ShortestPathAttribute>(ASFilePath, prot, metr, dest, type);
                else if (prot == 2)
                    simulateSync<BGPAttribute<ShortestPathAttribute>>(ASFilePath, prot, metr, dest, type);
            } else {
                if (prot == 1 || prot == 4 || prot == 5)
                    simulateAsync<ShortestPathAttribute>(ASFilePath, prot, metr, dest, type, del);
                else if (prot == 2)
                    simulateAsync<BGPAttribute<ShortestPathAttribute>>(ASFilePath, prot + (sync == 2), metr, dest, type, del);
            }
            break;
        case 2:
            if (sync == 1) {
                if (prot == 1)
                    simulateSync<ShortestWidestPathAttribute>(ASFilePath, prot, metr, dest, type);
                else if (prot == 2)
                    simulateSync<BGPAttribute<ShortestWidestPathAttribute>>(ASFilePath, prot, metr, dest, type);
            } else {
                if (prot == 1 || prot == 4 || prot == 5)
                    simulateAsync<ShortestWidestPathAttribute>(ASFilePath, prot, metr, dest, type, del);
                else if (prot == 2)
                    simulateAsync<BGPAttribute<ShortestWidestPathAttribute>>(ASFilePath, prot + (sync == 2), metr, dest, type, del);
            }
            break;
        case 3:
            if (sync == 1) {
                if (prot == 1)
                    simulateSync<ShortestWidestPathAttribute>(ASFilePath, prot, metr, dest, type);
                else if (prot == 2)
                    simulateSync<BGPAttribute<ShortestWidestPathAttribute>>(ASFilePath, prot, metr, dest, type);
            } else {
                if (prot == 1 || prot == 4 || prot == 5)
                    simulateAsync<ShortestWidestPathAttribute>(ASFilePath, prot, metr, dest, type, del);
                else if (prot == 2)
                    simulateAsync<BGPAttribute<ShortestWidestPathAttribute>>(ASFilePath, prot + (sync == 2), metr, dest, type, del);
            }
            break;
        case 4:
            if (sync == 1) {
                if (prot == 1)
                    simulateSync<ShortestPathAttribute>(ASFilePath, prot, metr, dest, type);
                else if (prot == 2)
                    simulateSync<BGPAttribute<ShortestPathAttribute>>(ASFilePath, prot, metr, dest, type);
            } else {
                if (prot == 1 || prot == 4 || prot == 5)
                    simulateAsync<ShortestPathAttribute>(ASFilePath, prot, metr, dest, type, del);
                else if (prot == 2)
                    simulateAsync<BGPAttribute<ShortestPathAttribute>>(ASFilePath, prot + (sync == 2), metr, dest, type, del);
            }
            break;
        case 5:
            if (sync == 1) {
                if (prot == 1)
                    simulateSync<EIGRPMetricAttribute>(ASFilePath, prot, metr, dest, type);
                else if (prot == 2)
                    simulateSync<BGPAttribute<EIGRPMetricAttribute>>(ASFilePath, prot, metr, dest, type);
            } else {
                if (prot == 1 || prot == 4 || prot == 5)
                    simulateAsync<EIGRPMetricAttribute>(ASFilePath, prot, metr, dest, type, del);
                else if (prot == 2)
                    simulateAsync<BGPAttribute<EIGRPMetricAttribute>>(ASFilePath, prot + (sync == 2), metr, dest, type, del);
            }
            break;
        case 6:
            if (sync == 1) {
                if (prot == 1)
                    simulateSync<WidestShortestPathAttribute>(ASFilePath, prot, metr, dest, type);
                else if (prot == 2)
                    simulateSync<BGPAttribute<WidestShortestPathAttribute>>(ASFilePath, prot, metr, dest, type);
            } else {
                if (prot == 1 || prot == 4 || prot == 5)
                    simulateAsync<WidestShortestPathAttribute>(ASFilePath, prot, metr, dest, type, del);
                else if (prot == 2)
                    simulateAsync<BGPAttribute<WidestShortestPathAttribute>>(ASFilePath, prot + (sync == 2), metr, dest, type, del);
            }
            break;
        default:
            break;
    }

    if (stats)
    {
        string outputFileName;
        outputFileName = "Outputs/" + ASName + "/" + m_str + "/" + p_str + "_" + s_str + t_str;
        
        ofstream file1(outputFileName + "_TerminationTimes.txt");
        ofstream file2(outputFileName + "_Messages.txt");
        ofstream file3(outputFileName + "_Transitions.txt");
        ofstream file4(outputFileName + "_AttributeChangesCount.txt");
        ofstream file5(outputFileName + "_CycleBlackHoleTimes.txt");
        ofstream file6(outputFileName + "_StableStateTimes.txt");
          

        if(type == 0 || type == 1) {
            stats->printGeneralStats((ofstream &)cout, dest, (type == 0), (prot == 1 || prot == 5), (sync == 1));
            // if (type == 0) 
            //     stats->printGeneralStats((ofstream &)cout, dest, true, (prot == 1 || prot == 5), (sync == 1));

            if(prints) {
                cout << "\nTermination Times:\n";
                stats->printNodeTerminationTimes((ofstream &)cout, (type == 0));
                cout << "\nMessages:\n";
                stats->printMessages((ofstream &)cout, (type == 0));
                cout << "\nUpdate->Diffusing Transitions:\n";
                stats->printNodeTransitions((ofstream &)cout, (type == 0), (prot == 1 || prot == 5));
                cout << "\nNumber of Attribute Changes:\n";
                stats->printNodeAttributeChanges((ofstream &)cout, (type == 0));
                cout << "\nTimes in Cycle / Black Hole:\n";
                stats->printNodeCycleBlackHoleTimes((ofstream &)cout, (type == 0));
                cout << "\nStable State Times:\n";
                stats->printStableStateTimes((ofstream &)cout, (type == 0));
            }
            stats->printNodeTerminationTimes((ofstream &)file1, (type == 0));
            stats->printMessages((ofstream &)file2, (type == 0));
            stats->printNodeTransitions((ofstream &)file3, (type == 0), (prot == 1 || prot == 5));
            stats->printNodeAttributeChanges((ofstream &)file4, (type == 0));
            stats->printNodeCycleBlackHoleTimes((ofstream &)file5, (type == 0));
            stats->printStableStateTimes((ofstream &)file6, (type == 0));
            
            // stats->printNodeCycleBlackHoleTimes((ofstream &)cout, (type == 0));
            // stats->printNodeCycleBlackHoleChanges((ofstream &)cout, (type == 0));
        }
        else {
            // if(type == 2)
            stats->printGeneralStatsLF((ofstream &)cout, dest, (type != 2), (prot == 1 || prot == 5), (sync == 1));
            // else
            //     stats->printGeneralStatsLF((ofstream &)cout, dest, true, (prot == 1 || prot == 5), (sync == 1));
            
            if(prints) {
                cout << "\nTermination Times:\n";
                stats->printNodeTerminationTimesLF((ofstream &)cout, (type == 3));
                cout << "\nMessages:\n";
                stats->printMessagesLF((ofstream &)cout, (type == 3));
                cout << "\nUpdate->Diffusing Transitions:\n";
                stats->printNodeTransitionsLF((ofstream &)cout, (prot == 1 || prot == 5), (type == 3));
                cout << "\nNumber of Attribute Changes:\n";
                stats->printNodeAttributeChangesLF((ofstream &)cout, (type == 3));
                cout << "\nTimes in Cycle / Black Hole:\n";
                stats->printNodeCycleBlackHoleTimesLF((ofstream &)cout, (type == 3));
                cout << "\nStable State Times:\n";
                stats->printStableStateTimesLF((ofstream &)cout, (type == 3));
            }
            stats->printNodeTerminationTimesLF((ofstream &)file1, (type == 3));
            stats->printMessagesLF((ofstream &)file2, (type == 3));
            stats->printNodeTransitionsLF((ofstream &)file3, (prot == 1 || prot == 5), (type == 3));
            stats->printNodeAttributeChangesLF((ofstream &)file4, (type == 3));
            stats->printNodeCycleBlackHoleTimesLF((ofstream &)file5, (type == 3));
            stats->printStableStateTimesLF((ofstream &)file6, (type == 3));
            
            // stats->printNodeCycleBlackHoleTimesLF((ofstream &)cout, (type == 3));
            // stats->printNodeCycleBlackHoleChangesLF((ofstream &)cout, (type == 3));
        }

        delete stats;
    }

    return 0;
}
