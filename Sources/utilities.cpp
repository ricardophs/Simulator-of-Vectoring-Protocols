#include <iostream>
#include <queue>
#include <fstream>
#include <sstream>
#include <random>

#include "../Headers/algebra.h"
#include "../Headers/utilities.h"

using std::cout, std::endl, std::vector, std::unordered_map,
    std::string, std::ifstream, std::getline, std::stringstream,
    std::priority_queue, std::pair, std::random_device, std::mt19937,
    std::uniform_int_distribution, std::queue;

extern random_device rd;  // only used once to initialise (seed) engine
extern mt19937 rng;       // random-number engine used (Mersenne-Twister in this case)
extern uniform_int_distribution<int> uniform1; 
extern int nLinks;
extern int nNodes;

/* Dijkstra algorithm */
template <class A>
vector<A> EvDijkstra(vector<NodeOneDest<A>> &nodes, int d, A initial, bool print)
{
    RoutingAlgebra<A> Algebra = RoutingAlgebra<A>();

    /* Comparator function */
    auto comp = [](const auto &lhs, const auto &rhs) {
        return rhs.first < lhs.first;
    };

    /* Priority Queue */
    priority_queue<pair<A, int>, vector<pair<A, int>>, decltype(comp)> PQ(comp);

    int n_nodes = nodes.size();
    vector<A> w(n_nodes, Algebra.Invalid);
    vector<int> succ(n_nodes, -1);
    vector<bool> outFromQueue(n_nodes, false);

    w[d] = initial;
    succ[d] = d;

    PQ.push({w[d], d});

    while (!PQ.empty()) {
        auto [attr, v] = PQ.top();
        PQ.pop();

        if (outFromQueue[v])
            continue;
        outFromQueue[v] = true;

        for (const auto &[u, link_cost] : nodes[v].inNeighbours)
        {
            A extended = Algebra.Extend(link_cost, attr);
            if (Algebra.Preferred(extended, w[u]))
            {
                PQ.push({extended, u});
                succ[u] = v;
                w[u] = extended;
            }
        }
    }
    if (print) {
        int i = 0;
        for (const auto &at : w) {
            cout << i << ":" << at << "- succ: " << succ[i] << endl;
            ++i;
        }
    }

    return w;
}

/* Create a */
template <class A>
vector<NodeOneDest<A>> createGraph(string filename,
                      vector<unordered_map<int, int>> &fifo_times,
                      vector<unordered_map<int, int>> &delays,
                      int opt,
                      bool sync)
{
    vector<NodeOneDest<A>> graph;
    ifstream graphFile;
    graphFile.open(filename);
    if (!graphFile)
    {
        cout << "Could not open file '" << filename << "'.\nExiting\n";
        return {};
    }
    int n_nodes, a;
    string buff;

    getline(graphFile, buff);
    stringstream(buff) >> n_nodes;
    cout << "|V| = " << n_nodes << " ";
    nNodes = n_nodes;

    if (!sync) {
        fifo_times = vector<unordered_map<int, int>>(n_nodes, unordered_map<int, int>());
        delays = vector<unordered_map<int, int>>(n_nodes, unordered_map<int, int>());
    }

    for (int u = 0; u < n_nodes; ++u)
        graph.push_back(NodeOneDest<A>(n_nodes, u));

    while (getline(graphFile, buff))
    {
        int u, v, w, d;
        stringstream line(buff);
        vector<int> data;
        while (getline(line, buff, ','))
        {
            stringstream(buff) >> a;
            data.push_back(a);
        }
        u = data[0];
        v = data[1];
        w = data[2]; // bandwidth in Mbps
        d = data[3]; // delay in ms
        if (!sync)
        {
            fifo_times[u][v] = 0;
            fifo_times[v][u] = 0;
            // int delay = uniform1(rng);
            int delay = d;
            delays[u][v] = delay;
            delays[v][u] = delay;

        }
        A linkCost;
        switch (opt)
        {
        /*** Cases for EIGRP ***/
        case 0:
            // Shortest Paths - A is assumed to be of type ShortestWidestPathAttribute
            linkCost = A(d);
            break;
        case 1:
            // Shortest Widest Paths - A is assumed to be of type ShortestWidestPathAttribute
            linkCost = A(w, d);
            break;
        case 2:
            // Widest Paths - A is assumed to be of type ShortestWidestPathAttribute
            linkCost = A(w);
            break;
        case 3:
            // Shortest Paths w/ Unitary Costs - A is assumed to be of type ShortestPathAttribute
            linkCost = A(1);
            break;
        case 4:
            // EIGRP Metric - A is assumed to be of type EIGRPMetricAttribute
            linkCost = A(w, d);
            break;
        case 5: 
            // Widest Shortest Paths - A is assumed to be of type WidestShortestPathAttribute
            linkCost = A(w, d);
            break;
        /*** Cases for BGP ***/
        case 6:
            // Shortest Paths - A is assumed to be of type ShortestWidestPathAttribute
            linkCost = A(u, d);
            break;
        case 7:
            // Shortest Widest Paths - A is assumed to be of type ShortestWidestPathAttribute
            linkCost = A(u, d, w);
            break;
        case 8:
            // Widest Paths - A is assumed to be of type ShortestWidestPathAttribute
            linkCost = A(u, 0, w);
            break;
        case 9:
            // Shortest Paths w/ Unitary Costs - A is assumed to be of type ShortestPathAttribute
            linkCost = A(u, 1);
            break;
        case 10:
            // EIGRP Metric - A is assumed to be of type EIGRPMetricAttribute
            linkCost = A(u, d, w);
            break;
        case 11:
            // Widest Shortest Paths - A is assumed to be of type WidestShortestPathAttribute
            linkCost = A(u, d, w);
            break;
        default:
            cout << "Invalid 'opt' parameter in createGraph(filename, fifo_times, opt).\n";
            cout << "Exiting\n";
            break;
        }
        graph[u].addOutLink(v, linkCost);
        graph[v].addInLink(u, linkCost);
        ++nLinks;

        data.clear();
    }

        cout << "|E| = " << nLinks << '\n';

    graphFile.close();

    return graph;
}


/* BFS algorithm to find if the network is connected */
template <class A> bool isConnected(vector<NodeOneDest<A>> &nodes) {
    vector<bool> visited = vector<bool>(nodes.size());
    queue<int> Q;

    Q.push(0);
    visited[0] = true;

    while(!Q.empty()) {
        int n = Q.front();
        Q.pop();
        for(const auto &[neigh, attr] : nodes[n].inNeighbours) {
            if(!visited[neigh]) {
                Q.push(neigh);
                visited[neigh] = true;
            }
        }
    }

    for(int i = 0; i < (int)nodes.size(); ++i) {
        if(!visited[i])
            return false;
    }

    return true;
}

#include "../Instances/utilitiesInstance.h"
