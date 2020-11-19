#include <cassert>
#include <vector>
#include <queue>
#include <limits>
#include <utility>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

// See the explanations of these typedefs and constants in the starter for friendsuggestion
typedef long long Len;
//typedef vector<priority_queue<pair<Len, int>,vector<pair<Len,int>>,greater<pair<Len,int>>>> Queue;

//const Len INF = numeric_limits<Len>::max() / 4;
const Len INF = -1;
const int f = 0;
const int b = 1;

struct Edge {
    int vertex{};
    Len cost{};
    Edge (int v_, Len w_) 
        : vertex(v_), cost(w_) {}
    
    bool operator <( const Edge& e) const {
        return cost < e.cost ;
    }

    void print() {
        cout << "(" << vertex << "," << cost << "), " ;
    }
};

class PQueue {
    public:
    int d = 2;
    int size = 0;
    vector< Edge > minH;
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
    
    Edge top() {
        return minH[0];
    }

    void push(Edge e) {
        size += 1;
        minH.emplace_back(e);
        //minH[size-1] = e;
        shiftUp(size-1);
    }

    bool empty() {
        return size == 0;
    }

    int pop() {
        auto r = top().vertex;
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

//typedef vector<vector<vector<int>>> Adj;
//typedef vector<priority_queue<pair<Len, int>,vector<pair<Len,int>>,greater<pair<Len,int>>>> Queue;

typedef vector<vector<vector<Edge>>> Adj;

class AStar {
    // See the descriptions of these fields in the starter for friendsuggestion
    int n;
    Adj adj;
    vector<vector<Len>> cost_so_far;
    //vector<int> workset;
    vector<vector<char>> visited;
    // Coordinates of the nodes
    std::vector<std::pair<Len,Len>> xy;

    vector<PQueue> frontier;

public:
    AStar(int n_, int k_, Adj& adj_, vector<pair<Len,Len>>& xy_)
        : n(n_), adj(adj_), cost_so_far(2, vector<Len>(n_, INF)), xy(xy_), frontier(2, PQueue(k_, 4)), visited(2, vector<char>(n))
    {}

    // See the description of this method in the starter for friendsuggestion
    void clear() {
        for (int i = 0; i < n; ++i) {
            cost_so_far[f][i] = INF;
            cost_so_far[b][i] = INF;
            visited[f][i] = '0';
            visited[b][i] = '0';
        }
        frontier[f].reset();
        frontier[b].reset();
    }

    //Calculate pi function for bidirectional A*
    inline Len pi(int vertex, int source, int goal, int side) {
        Len lenf = sqrt( pow( (xy[goal].first   - xy[vertex].first), 2 ) + pow( (xy[goal].second   - xy[vertex].second), 2 ));
        Len lenb = sqrt( pow( (xy[source].first - xy[vertex].first), 2 ) + pow( (xy[source].second - xy[vertex].second), 2 ));
        Len pif = (lenf - lenb) / 2;
        return side ? -pif : pif;
    }

    void relaxEdge (int current, int source, int goal, int side) {
        for (int e = 0; e < adj[side][current].size(); ++e) {
            int next = adj[side][current][e].vertex;
            if (visited[side][next] == '2') continue;
            //Len edge_cost = adj[f][current][e].cost;
            Len new_cost = cost_so_far[side][current] + adj[side][current][e].cost;
            if (cost_so_far[side][next] == INF || new_cost < cost_so_far[side][next]) {
                cost_so_far[side][next] = new_cost;
                //Here you have to update the priority and add it into pq. Otherwise the algorithm fails functionally.
                Len priority = cost_so_far[side][next] + pi(next, source, goal, side); 
                frontier[side].push(Edge(next, priority));
            }
        } 
    }

    Len shortestPathDistance () {
        Len best = numeric_limits<Len>::max();
        for (int i = 0; i < n; ++i) {
            if ( (visited[f][i] == '2' || visited[b][i] == '2') && cost_so_far[f][i] != INF && cost_so_far[b][i] != INF && cost_so_far[f][i] + cost_so_far[b][i] < best) {
                best = cost_so_far[f][i] + cost_so_far[b][i];
                //cout << "Best loop: " << best << endl;
            }
        }

        //cout << "Best: " << best << endl;
        return best;
    }

    // Returns the cost_so_far from s to t in the graph
    Len query(int source, int goal) {
        clear();
        frontier[f].push(Edge(source, 0));
        cost_so_far[f][source] = 0;
        frontier[b].push(Edge(goal, 0));
        cost_so_far[b][goal] = 0;
        int current;
        bool pathExists = false;

        while (!frontier[f].empty() && !frontier[b].empty()) {
            current = frontier[f].pop();
            visited[f][current] = '2';
            if (visited[b][current] == '2') {
                pathExists = true;
                break;
            }
            relaxEdge(current, source, goal, f);

            current = frontier[b].pop();
            visited[b][current] = '2';
            if (visited[f][current] == '2') {
                pathExists = true;
                break;
            }
            relaxEdge(current, source, goal, b);
        }

        //Calculate Shortest Path distance
        //cout << "PathExists: " << pathExists << endl;
        return pathExists ? shortestPathDistance() : -1;
    }
};

int main() {
    int n, m;
    std::ios::sync_with_stdio(false);
    cin >> n >> m;
    std::vector<std::pair<Len,Len>> xy(n);
    for (int i=0;i<n;++i){
        int a, b;
        cin >> a >> b;
        xy[i] = make_pair(a,b);
    }
    Adj adj(2, vector<vector<Edge>>(n));
    int s = 0;
    for (int i=0; i<m; ++i) {
        int u, v, c;
        cin >> u >> v >> c;
        adj[0][u-1].push_back(Edge(v-1, c));
        adj[1][v-1].push_back(Edge(u-1, c));
        if (adj[0][u-1].size() > s) 
            s = adj[0][u-1].size();
        if (adj[1][v-1].size() > s) 
            s = adj[1][u-1].size();
    }

    AStar astar(n, 2 * s, adj, xy);

    int t;
    cin >> t ;
    for (int i=0; i<t; ++i) {
        int u, v;
        cin >> u >> v;
        cout << astar.query(u-1, v-1) << "\n";
    }
    return 0;
}

/*
int main2() {
    int n, m;
    std::ios::sync_with_stdio(false);
    ifstream test("tests/03");
    test >> n >> m;
    std::vector<std::pair<Len,Len>> xy(n);
    for (int i=0;i<n;++i){
        int a, b;
        test >> a >> b;
        xy[i] = make_pair(a,b);
    }
    Adj adj(2, vector<vector<Edge>>(n));
    int s = 0;
    for (int i=0; i<m; ++i) {
        int u, v, c;
        test >> u >> v >> c;
        adj[0][u-1].push_back(Edge(v-1, c));
        adj[1][v-1].push_back(Edge(u-1, c));
        if (adj[0][u-1].size() > s) 
            s = adj[0][u-1].size();
        if (adj[1][v-1].size() > s) 
            s = adj[1][u-1].size();
    }

    AStar astar(n, 2 * s, adj, xy);

    int t;
    test >> t ;
    for (int i=0; i<t; ++i) {
        int u, v;
        test >> u >> v;
        cout << astar.query(u-1, v-1) << "\n";
    }
    return 0;
}
*/
