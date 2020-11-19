#include <cstdio>
#include <cassert>
#include <vector>
#include <queue>
#include <limits>
#include <utility>
#include <set>
#include <unordered_set>
#include <iostream>
#include <fstream>

using namespace std;

// Distances can grow out of int type
typedef long long Len;

struct edge {
    int v{};
    Len w{};
    edge (int v_, Len w_) 
        : v(v_), w(w_) {}

    bool operator <( const edge& e) const {
        return w > e.w ;
    }
};

struct compare {
    bool operator ()(const edge& e1, const edge& e2) {
        return e1.w > e2.w;
    }
};

//2 dimensional vector of edges for adjacency list representation of graph
typedef vector<vector<edge>> Adj;


// Vector of two priority queues - for forward and backward searches.
// Each priority queue stores the closest unprocessed node in its head.
typedef priority_queue<edge, vector<edge>, compare> PQueue; 

//typedef priority_queue<pair<Len, int>,vector<pair<Len,int>>,greater<pair<Len,int>>> PQueue;

//const Len INF = numeric_limits<Len>::max();
const Len INF = -1;

class PriorityQueue {
    public:
    vector<int
}














class Bidijkstra {
    // Number of nodes
    int n_;
    // Graph adj_[0] and cost_[0] correspond to the initial graph,
    // adj_[1] and cost_[1] correspond to the reversed graph.
    // Graphs are stored as vectors of adjacency lists corresponding
    // to nodes.
    // Adjacency list itself is stored in adj_, and the corresponding
    // edge costs are stored in cost_.
    Adj adjF_;
    Adj adjB_;
    // distance_[0] stores distances for the forward search,
    // and distance_[1] stores distances for the backward search.
    vector<Len> distF_;
    vector<Len> distB_;
    // Stores all the nodes visited either by forward or backward search.
    vector<bool> visitedF;
    vector<bool> visitedB;

public:
    Bidijkstra(int n, Adj adjF, Adj adjB)
        : n_(n), adjF_(adjF), adjB_(adjB),distF_(n, INF), distB_(n, INF), visitedF(n, 0), visitedB(n, 0) {}

    // Initialize the data structures before new query,
    // clear the changes made by the previous query.
    void clear() {
        for (int i=0; i < n_; ++i) {
            distF_[i] = -1;
            distB_[i] = -1;
            visitedF[i] = 0;
            visitedB[i] = 0;
        }
    }

//    void printQ() {
//        auto q1 = qF;
//        auto q2 = qB;
//
//        cout << "Front Queue: ";
//        while (!q1.empty()) {
//            cout << q1.top().v << " ";
//            q1.pop();
//        }
//        cout << "Back Queue: " ;
//        while (!q2.empty()) {
//            cout << q2.top().v << " ";
//            q2.pop();
//        }
//        cout << endl;
//
//        cout << "distF: ";
//        for (auto e: distF_) {
//            cout << e << " ";
//        }
//        cout << "distB: ";
//        for (auto e: distB_) {
//            cout << e << " ";
//        }
//        cout << endl;
//    }

    void relaxEdgeF(int s, PQueue& qF) {
        for (const auto& e: adjF_[s]) {
            if (distF_[e.v] == INF || distF_[e.v] > distF_[s] + e.w) {
                distF_[e.v] = distF_[s] + e.w;
                if (!visitedF[e.v])
                    qF.push(edge(e.v, distF_[e.v]));
            }
        }
    }

    void relaxEdgeB(int s, PQueue& qB) {
        for (const auto& e: adjB_[s]) {
            if (distB_[e.v] == INF || distB_[e.v] > distB_[s] + e.w) {
                distB_[e.v] = distB_[s] + e.w;
                if (!visitedB[e.v])
                    qB.push(edge(e.v, distB_[e.v]));
            }
        }
    }


    Len findShortestDist() {
        Len currDist = std::numeric_limits<Len>::max();
        for (int v = 0; v < n_; ++v) {
            if ((visitedF[v] == 1 || visitedB[v] == 1) && distF_[v] != INF && distB_[v] != INF && distF_[v] + distB_[v] < currDist) 
                currDist = distF_[v] + distB_[v];
        }


        if (currDist == std::numeric_limits<Len>::max()) 
            currDist = -1;

        return currDist;
    }
    
    // Returns the distance from s to t in the graph.
    Len query(int s, int t) {
        clear();
        PQueue qF;
        PQueue qB;
        distF_[s] = 0;
        distB_[t] = 0;
        qF.push(edge(s, 0));
        //visitedF[s] = 1;
        qB.push(edge(t, 0));
        //visitedB[t] = 1;

        while (!qF.empty() && !qB.empty()) {
            auto f = qF.top();
            qF.pop();
            if (!visitedF[f.v]) { 
                visitedF[f.v] = 1;
                relaxEdgeF(f.v, qF);
                if (visitedB[f.v] == 1) 
                    return findShortestDist();
            }
            auto b = qB.top();
            qB.pop();
            if (!visitedB[b.v]) {
                visitedB[b.v] = 1;
                relaxEdgeB(b.v, qB);
                if (visitedF[b.v] == 1) 
                    return findShortestDist();
            }
           // printQ();
        }
        return -1;
    }
};

int main() {
    int n, m;
    //ifstream test("test2.txt");
    //test >> n >> m;
    scanf("%d%d", &n, &m);
    Adj adjF(n);
    Adj adjB(n);
    for (int i=0; i<m; ++i) {
        int u, v, c;
        scanf("%d%d%d", &u, &v, &c);
        //test >> u >> v >> c;
        adjF[u-1].push_back(edge(v-1, c));
        adjB[v-1].push_back(edge(u-1, c));
    }

    Bidijkstra bidij(n, adjF, adjB);

    int t;
    scanf("%d", &t);
    //test >> t;
    for (int i=0; i<t; ++i) {
        int u, v;
        scanf("%d%d", &u, &v);
        //test >> u >> v;
        printf("%lld\n", bidij.query(u-1, v-1));
    }
    //return 0;
}

/*
int main2() {
    int n, m;
    ifstream test("test2.txt");
    test >> n >> m;
    Adj adjF(n);
    Adj adjB(n);
    for (int i=0; i<m; ++i) {
        int u, v, c;
        test >> u >> v >> c;
        adjF[u-1].push_back(edge(v-1, c));
        adjB[v-1].push_back(edge(u-1, c));
    }

    Bidijkstra bidij(n, adjF, adjB);

    int t;
    test >> t;
    for (int i=0; i<t; ++i) {
        int u, v;
        test >> u >> v;
        printf("%lld\n", bidij.query(u-1, v-1));
    }

    return 0;
}
*/
