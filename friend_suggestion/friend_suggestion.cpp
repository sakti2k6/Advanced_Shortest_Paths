#include <cstdio>
#include <cassert>
#include <vector>
//#include <queue>
#include <limits>
#include <utility>
#include <iostream>
//#include <fstream>

using namespace std;

// Distances can grow out of int type
typedef long long Len;

struct edge {
    int v{};
    Len w{};
    edge (int v_, Len w_) 
        : v(v_), w(w_) {}

    bool operator <( const edge& e) const {
        return w < e.w ;
    }

    void print() {
        cout << "(" << v << "," << w << "), " ;
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
//typedef priority_queue<edge, vector<edge>, compare> PQueue; 

//typedef priority_queue<pair<Len, int>,vector<pair<Len,int>>,greater<pair<Len,int>>> PQueue;

//const Len INF = numeric_limits<Len>::max();
const Len INF = -1;

//template<typename T>
class PQueue {
    public:
    int d = 2;
    int size = 0;
    vector< edge > minH;
    PQueue(int n, int d_) {
        minH.reserve(n);
        d = d_;
        size = 0;
    }

    int parent(int i) {
        return (i-1)/d;
    }

    int child (int p, int i) {
        return d * p + (i + 1);
    }

    void shiftUp(int i) {
        int pi;
        while (i != 0) {
            pi = parent(i);
            if (minH[i] < minH[pi]) {
                swap(minH[i], minH[pi]);
                i = pi;
            } else {
                return;
            }
        }
    }

    void shiftDown(int i) {
        while ( i < size) {
            int id = i;
            int c = 0;
            while (c < d) {
                int ci = child (i, c);
                if (ci >= size) break;
                if (minH[ci] < minH[id]) {
                    id = ci;
                }
                c += 1;
            }

            if (i == id) return;
            swap(minH[i], minH[id]);
            i = id;
        }
    }
    
    edge top() {
        return minH[0];
    }

    void push(edge e) {
        size += 1;
        minH.push_back(e);
        //minH[size-1] = e;
        shiftUp(size-1);
    }

    bool empty() {
        return size == 0;
    }

    edge pop() {
        auto r = top();
        swap(minH[size-1], minH[0]);
        size -= 1;
        minH.pop_back();
        shiftDown(0);
        return r;
    }

    void print() {
        for (Len i = 0; i < size; ++i) 
            minH[i].print();
        cout << endl;
    }

    void reset() {
        minH.clear();
        size = 0;
    }
};


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
    vector<char> visitedF;
    vector<char> visitedB;
    PQueue qF;
    PQueue qB;

public:
    Bidijkstra(int n, Len k, Adj& adjF, Adj& adjB)
        : n_(n), adjF_(adjF), adjB_(adjB),distF_(n, INF), distB_(n, INF), visitedF(n, 0), visitedB(n, 0), qF(k, 4), qB(k, 4) {}

    // Initialize the data structures before new query,
    // clear the changes made by the previous query.
    void clear() {
        for (int i=0; i < n_; ++i) {
            distF_[i] = -1;
            distB_[i] = -1;
            visitedF[i] = '0';
            visitedB[i] = '0';
        }
        qF.reset();
        qB.reset();
    }


    void relaxEdgeF(int s) {
        for (const auto& e: adjF_[s]) {
            if (visitedF[e.v] == '2') continue;
            if (distF_[e.v] == INF || distF_[e.v] > distF_[s] + e.w) {
                distF_[e.v] = distF_[s] + e.w;
               // if (visitedF[e.v] == '0') {
                //    visitedF[e.v] = '1';
                    qF.push(edge(e.v, distF_[e.v]));
               // }
            }
        }
    }

    void relaxEdgeB(int s) {
        for (const auto& e: adjB_[s]) {
            if (visitedB[e.v] == '2') continue;
            if (distB_[e.v] == INF || distB_[e.v] > distB_[s] + e.w) {
                distB_[e.v] = distB_[s] + e.w;
              //  if (visitedB[e.v] == '0') {
              //      visitedB[e.v] = '1';
                    qB.push(edge(e.v, distB_[e.v]));
              //  }
            }
        }
    }


    Len findShortestDist() {
        Len currDist = std::numeric_limits<Len>::max();
        for (int v = 0; v < n_; ++v) {
            if ((visitedF[v] == '2' || visitedB[v] == '2') && distF_[v] != INF && distB_[v] != INF && distF_[v] + distB_[v] < currDist) 
                currDist = distF_[v] + distB_[v];
        }


        if (currDist == std::numeric_limits<Len>::max()) 
            currDist = -1;

        return currDist;
    }
    
    // Returns the distance from s to t in the graph.
    Len query(int s, int t) {
        clear();
        distF_[s] = 0;
        distB_[t] = 0;
        qF.push(edge(s, 0));
        qB.push(edge(t, 0));

        while (!qF.empty() && !qB.empty()) {
            auto f = qF.pop();
            //qF.pop();
            visitedF[f.v] = '2';
            relaxEdgeF(f.v);
            if (visitedB[f.v] == '2') 
                return findShortestDist();
            
            auto b = qB.pop();
            //qB.pop();

            visitedB[b.v] = '2';
            relaxEdgeB(b.v);
            if (visitedF[b.v] == '2') 
                return findShortestDist();
            //qF.print();
            //qB.print();
        }
        return -1;
    }
};
/*
int main() {
    int n, m;
    scanf("%d%d", &n, &m);
    Adj adjF(n);
    Adj adjB(n);
    int s = 0;
    for (int i=0; i<m; ++i) {
        int u, v, c;
        scanf("%d%d%d", &u, &v, &c);
        adjF[u-1].push_back(edge(v-1, c));
        adjB[v-1].push_back(edge(u-1, c));
        if (adjF[u-1].size() > s) 
            s = adjF[u-1].size();
        if (adjB[v-1].size() > s) 
            s = adjB[u-1].size();
    }

    Bidijkstra bidij(n, 2 * s, adjF, adjB);

    int t;
    scanf("%d", &t);
    for (int i=0; i<t; ++i) {
        int u, v;
        scanf("%d%d", &u, &v);
        printf("%lld\n", bidij.query(u-1, v-1));
    }
    //return 0;
}
*/

int main() {
    int n, m;
    std::ios::sync_with_stdio(false);
    cin >> n >> m;
    Adj adjF(n);
    Adj adjB(n);
    int s = 0;
    for (int i=0; i<m; ++i) {
        int u, v, c;
        cin >> u >> v >> c;
        adjF[u-1].push_back(edge(v-1, c));
        adjB[v-1].push_back(edge(u-1, c));
        if (adjF[u-1].size() > s) 
            s = adjF[u-1].size();
        if (adjB[v-1].size() > s) 
            s = adjB[u-1].size();
    }

    Bidijkstra bidij(n, 2 * s, adjF, adjB);

    int t;
    cin >> t;
    for (int i=0; i<t; ++i) {
        int u, v;
        cin >> u >> v ;
        cout << bidij.query(u-1, v-1) << "\n";
    }
    //return 0;
}

/*

int main() {
    int n, m;
    std::ios::sync_with_stdio(false);
    ifstream test("tests/eu-2005w.test2");
    test >> n >> m;
    Adj adjF(n);
    Adj adjB(n);
    int s = 0;
    for (int i=0; i<m; ++i) {
        int u, v, c;
        test >> u >> v >> c;
        adjF[u-1].push_back(edge(v-1, c));
        adjB[v-1].push_back(edge(u-1, c));
        if (adjF[u-1].size() > s) 
            s = adjF[u-1].size();
        if (adjB[v-1].size() > s) 
            s = adjB[u-1].size();
    }

    cout << "SIZE: " << s << endl;
    Bidijkstra bidij(n, 2 * s, adjF, adjB);

    int t;
    test >> t;
    for (int i=0; i<t; ++i) {
        int u, v;
        test >> u >> v;
        cout << bidij.query(u-1, v-1) << "\n";
    }

    return 0;
}

int main4() {
    PQueue pq(10, 2);
    pq.push(edge(3, 5));
    pq.push(edge(2, 6));
    pq.push(edge(10, 1));
    pq.print();
    cout << pq.pop().w << endl;
    pq.push(edge(11,8));
    pq.print();
    return 0;
}

*/
