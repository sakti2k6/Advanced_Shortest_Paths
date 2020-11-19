#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <limits>
#include <queue>
#include <iostream>
#include <memory>
#include <cassert>
#include <unordered_map>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

typedef long long int intL;
typedef long long int Distance;
typedef int Vertex;
//using Adj = vector<vector<Edge>>; 
using distMap = unordered_map<int, intL> ;
static constexpr intL INF = numeric_limits<intL>::max() / 2;


/*##########################################*/
/*# Custom Structures and Classes          #*/
/*##########################################*/

struct Edge {
    int vertex{};
    intL cost{};
    Edge() {};
    Edge(int v, intL c) : vertex(v), cost(c) {}
};

struct compare {
    bool operator()(const Edge& e1, const Edge& e2) {
        return e1.cost > e2.cost;
    }
};

// Wrapper around STL priority_queue
class StlHeap {
    public:
    using Queue = priority_queue<Edge, vector<Edge>, compare>;
    
    StlHeap() {
        queue.reset(new Queue());
    }
    
    bool empty() const {
        return queue->empty();
    }
    
    void push(Vertex v, Distance c) {
        queue->push(Edge(v, c));
    }
    
    void clear() {
        queue.reset(new Queue());
    }
    
    Edge top() {
        return queue->top();
    }
    
    Edge pop() {
        Edge top = queue->top();
        queue->pop();
        return top;
    }
    
    void print() {
        auto pq = queue;
        while (!pq->empty()) {
            auto e = pq->top();
            pq->pop();
            cout << "V: " << e.vertex << " Imp: " << e.cost << endl;
        }
    }

    private:
    shared_ptr<Queue> queue;
};

struct Shortcut {
    int from;
    int to;
    int cost;
    Shortcut( int from_, int to_, int cost_) : from(from_), to(to_), cost(cost_) {};
};

struct VertexMap {
    int numSC{};
    bool contracted{};
    int shortC{};
    int level{};
    int rank{};
    intL importance{};

    bool operator >(const VertexMap& v) {
        return importance > v.importance;
    }
};

/*##########################################*/
/*# GRAPH IMPLEMENTATIONS STARTS FROM HERE #*/
/*##########################################*/


class Graph {
public:

    
/*##########################################*/
/*# Private members of the Graph Class #*/
/*##########################################*/

    // Number of nodes
    int N;
    // Lists of edges outgoing from each node
    vector<vector<Edge>> outgoing_edges;
    // Lists of edges incoming to each node
    vector<vector<Edge>> incoming_edges;
    
    // Ranks of nodes - positions in the node ordering
    int cntRank{};
    
    // Store distances in Bidirectional Dijkstra's
    vector<Distance> distF;
    vector<Distance> distR;
    vector<bool> visitedF;
    vector<bool> visitedR;
    
    // Priority queues for forward and backward searches
    StlHeap minpqF;
    StlHeap minpqR;

    // Priority queue will store vertex and its importance with the least important node in the head
    StlHeap impQ;

    //Used for getting priority and preprocessing
    vector<VertexMap> vertices;

/*##########################################*/
/*# Initializer Functions                  #*/
/*##########################################*/

    void add_edge_to_list(vector<Edge>& elist, int v, int c, int u) {
       
/* FIXME: Removing the following loop exceeds the time limit for preprocessing. That means there are a lot of duplicated edges which gets added */
        for (int i = 0; i < elist.size(); ++i) {
            Edge& p = elist[i];
            if (p.vertex == v) {
                if (p.cost > c) {
                    p.cost = c;
                }
                return;
            }

         //   if (vertices[u].rank > vertices[p.vertex].rank) {
         //       elist.erase(elist.begin() + i--);
         //   }
        }
        elist.push_back(Edge(v, c));
        //cout << "Vert: " << u << " " << v << " " << c << endl;
        //cout << u << " " << v << " " << c << endl;
    }
    
    void add_directed_edge(int u, int v, int c) {
        add_edge_to_list(outgoing_edges[u], v, c, u);
        add_edge_to_list(incoming_edges[v], u, c, v);
    }
    
    void add_edge(int u, int v, int c) {
        add_directed_edge(u, v, c);
    }

    void add_edge2(int u, int v, int c, int k) {
        outgoing_edges[u].push_back(Edge(v, c));
        incoming_edges[v].push_back(Edge(u, c));
        cout << "Vert: " << u << " " << v << " " << k << " " << c << endl;
      //  cout << "Rank: " << vertices[u].rank << " " << vertices[v].rank <<  " " << vertices[k].rank << endl;
      //  cout << "Cntr: " << vertices[u].contracted << " " << vertices[v].contracted <<  " " << vertices[k].contracted << endl;
    }

    void set_n(int n) {
        N = n;
        outgoing_edges.resize(N);
        incoming_edges.resize(N);
        // initialize dist vectors for bidirectional dijkstra
        distF.resize(N, INF);
        distR.resize(N, INF);
        visitedF.resize(N, false);
        visitedR.resize(N, false);
        vertices.resize(N);
        cntRank = 0;
    }

    Graph(int n) { set_n(n); }

/*##########################################*/
/*# Pre-Process the Graph                   #*/
/*##########################################*/

    void computeInitialImportance () {
        for (int i = 0; i < N; ++i) {
            vertices[i].numSC = 0;
            vertices[i].contracted = false;
            vertices[i].shortC = incoming_edges[i].size() + outgoing_edges[i].size();
            vertices[i].level = 0;
            vertices[i].rank = 0;
            //vertices[i].importance = (incoming_edges[i].size() * outgoing_edges[i].size() - incoming_edges[i].size() - outgoing_edges[i].size()) + 1 * vertices[i].shortC ;
            /* FIXME: Following line of heuristics is better than the above line */
            vertices[i].importance = incoming_edges[i].size() * outgoing_edges[i].size() ;
            impQ.push(i, vertices[i].importance);
        }
    }
    
    //One directional Dijkstra
    void dijkstra (int s, int maxD) {
        distF[s] = 0;
        StlHeap minPQ;
        minPQ.push(s, 0);
        int hops = 4;
    
        while (hops && !minPQ.empty()) {
            auto current = minPQ.pop().vertex;
            if (visitedF[current]) continue;
            visitedF[current] = true;
            hops -= 1;
            /* FIXME: I need hops to be atleast 5 to pass testcase 6 */
            if (distF[current] > maxD) return;
    
            for (int i = 0; i < outgoing_edges[current].size(); ++i) {
                int next = outgoing_edges[current][i].vertex;
                if (visitedF[next]) continue;
                intL edge_cost = outgoing_edges[current][i].cost;
                //if (vertices[next].rank < vertices[current].rank || next == current || vertices[next].contracted ) continue;
                if (vertices[next].contracted ) continue;
    
                intL next_cost = distF[current] + edge_cost;
                if (next_cost < distF[next] && next_cost <= maxD) {
                    distF[next] = next_cost;
                    minPQ.push(next, next_cost);
                }
            }
        }
    }
    
    void simulateContraction (int s, int inDist, int v, int maxD) {
      //  vector<Shortcut> shortcuts;
        for (int i = 0; i < N; ++i) {
            distF[i] = INF;
            visitedF[i] = false;
        }
        //distMap distM = dijkstra (s, maxD);
        dijkstra (s, maxD);
        for (int i = 0; i < outgoing_edges[v].size(); ++i) {
            int next = outgoing_edges[v][i].vertex;
            if (vertices[next].contracted || next == s) continue;
            int outDist = outgoing_edges[v][i].cost;

            /* NOTE::: ADDING !vertices[next]... really matters for preprocessing time. This was causing the preprocessing to exceed time limit*/
            if (inDist + outDist < distF[next]) {
                //shortcuts.emplace_back( Shortcut( s, next, inDist + outDist) );
                //cout << "inDist: " << inDist << " outDist: " << outDist << " distF: " << distF[next] << endl;
                //add_edge2 ( s, next, inDist + outDist, v);
                add_edge ( s, next, inDist + outDist);
            }
        }
    
       // return shortcuts;
    }
    
    void do_shortcut(vector<Shortcut>& shortcuts) {
        for (int i = 0; i < shortcuts.size(); ++i) {
            add_edge( shortcuts[i].from, shortcuts[i].to, shortcuts[i].cost);
        }
    }
 
    void finalize() {
        //Remove unnecessary edges (u->v), where rank[u] > rank[v];
    }

    intL get_importance (int v) {
        return incoming_edges[v].size() * outgoing_edges[v].size();
    }

    void preprocess() {
        //printEdges();
        computeInitialImportance();
    
        while(!impQ.empty()) {
            int current = impQ.pop().vertex;
            auto currentImp = get_importance(current);
            if (currentImp > impQ.top().cost) {
                impQ.push(current, currentImp);
                continue;
            }

            vertices[current].rank = cntRank++;
            if (vertices[current].contracted) continue;
            vertices[current].contracted = true;
    
            //Find maximum incoming edge
            int inMax = 0;
            int inMaxV = -1;
            for (int i = 0; i < incoming_edges[current].size(); ++i) {
                int v = incoming_edges[current][i].vertex;
                if (vertices[v].contracted) continue;
                if (incoming_edges[current][i].cost > inMax) {
                    inMax = incoming_edges[current][i].cost;
                    inMaxV = v;
                }
            }
    
            //Find maximum outgoing edge
            int outMax = 0;
            for (int i = 0; i < outgoing_edges[current].size(); ++i) {
                int v = outgoing_edges[current][i].vertex;
                if (vertices[v].contracted) continue;
                if (outgoing_edges[current][i].cost > outMax && v != inMaxV) 
                    outMax = outgoing_edges[current][i].cost;
            }
    
            
            //cout << "Vertex: " << current << " inMax, outMax: " << inMax << "," << outMax << endl;
    
            //Contract each of the incoming edges of current vertex;
            for (int i = 0; i < incoming_edges[current].size(); ++i) {
                int next = incoming_edges[current][i].vertex;
                if (vertices[next].contracted) continue;
                int nextDist = incoming_edges[current][i].cost;
                simulateContraction(next, nextDist, current, inMax + outMax);
                //vector<Shortcut> shortcuts = simulateContraction(next, nextDist, current, inMax + outMax);
                //do_shortcut(shortcuts);
            }
        }
    
        //printEdges();
        //cout << "----Incoming Edges----" << endl;
        //printEdgesR();
        
        finalize();
    }

/*##########################################*/
/*# Query shortest distance in the Graph   #*/
/*##########################################*/

    void clear() {
        for (int i = 0; i < N; ++i) {
            distF[i] = INF;
            distR[i] = INF;
            visitedF[i] =false;
            visitedR[i] = false;
        }
    
        minpqF.clear();
        minpqR.clear();
    }
    
    bool checkRelaxCond (int current, int next, const vector<Distance>& cost_so_far, intL new_cost) {
        bool nodeRank = vertices[next].rank > vertices[current].rank;
        bool cost = (cost_so_far[next] == INF) || (new_cost < cost_so_far[next]);
        //cout << "nodeRank: " << nodeRank << " cost: " << cost << endl;
        return nodeRank && cost;
    }
    
    void relaxEdge(int v, string direction) {
        if (direction == "forward") {
    
            for (int i = 0; i < outgoing_edges[v].size(); ++i) {
                int next = outgoing_edges[v][i].vertex;
                if (visitedF[next]) continue;
    
                intL edge_cost = outgoing_edges[v][i].cost;
                intL new_cost = distF[v] + edge_cost;
    
                if (checkRelaxCond(v, next, distF, new_cost)) {
                    distF[next] = new_cost;
                    minpqF.push(next, new_cost);
                }
            }
    
        } else if (direction == "reverse") {
    
            for (int i = 0; i < incoming_edges[v].size(); ++i) {
                int next = incoming_edges[v][i].vertex;
                if (visitedR[next]) continue;
    
                intL edge_cost = incoming_edges[v][i].cost;
                intL new_cost = distR[v] + edge_cost;
                if (checkRelaxCond(v, next, distR, new_cost)) {
                    distR[next] = new_cost;
                    minpqR.push(next, new_cost);
                    //cout << "Next: " << next << " new_cost: " << new_cost << endl;
                }
            }
    
        }
    }
    
    //Bidirectional Dijkstra
    int biDijkstra (int s, int t) {
        minpqF.push(s, 0);
        distF[s] = 0;
    
        minpqR.push(t, 0);
        distR[t] = 0;
        
        // Estimate of the distance from s to t
        intL estimate = INF;
        
        while (!minpqF.empty() || !minpqR.empty()) {
            if (!minpqF.empty()) { 
                int currentF = minpqF.pop().vertex;
                visitedF[currentF] = true;
                if (distF[currentF] <= estimate)
                    relaxEdge(currentF, "forward");
                if (visitedR[currentF] && distF[currentF] + distR[currentF] < estimate) {
                    estimate = distF[currentF] + distR[currentF];
                }
                //cout << "Front: " << currentF << " : " << estimate << endl;
            } 
    
            if (!minpqR.empty()) {
                int currentR = minpqR.pop().vertex;
                visitedR[currentR] = true;
                if (distR[currentR] <= estimate)
                    relaxEdge(currentR, "reverse");
                if (visitedF[currentR] && distF[currentR] + distR[currentR] < estimate) {
                    estimate = distF[currentR] + distR[currentR];
                }
                //cout << "Reverse: " << currentR << " : " << estimate << endl;
            }
        }
    
        if (estimate == INF) return -1;
        return estimate;
    }
    
    
    // Returns distance from s to t in the graph
    int query(int s, int t) {
        clear();
        return biDijkstra(s, t);
    }

/*##########################################*/
/*# Helper Functions                       #*/
/*##########################################*/

    void printEdges () {
        for (int i = 0; i < N; ++i) {
            cout << i << " :: " ;
            for (int j = 0; j < outgoing_edges[i].size(); ++j) 
                cout << i << "->" << outgoing_edges[i][j].vertex << ":" << outgoing_edges[i][j].cost << " , ";
            cout << "\n";
        }
    }
    
    void printEdgesR () {
        for (int i = 0; i < N; ++i) {
            cout << i << " :: " ;
            for (int j = 0; j < incoming_edges[i].size(); ++j) 
                cout << i << "->" << incoming_edges[i][j].vertex << ":" << incoming_edges[i][j].cost << " , ";
            cout << "\n";
        }
    }

    void printGraphStatistics() {
        int osum = 0;
        int isum = 0;
        for (const auto& e: outgoing_edges) {
            osum += e.size();
        }
        for (const auto& e: incoming_edges) {
            isum += e.size();
        }
        cout << "#Incoming Edges: " << isum << " #Outgoing Edges: " << osum << endl;
    }

};
/*##########################################*/


int main() {
    std::ios::sync_with_stdio(false);
    int u,v,c,n,m;
    cin >> n >> m;
    Graph g(n);
    for (int i = 0; i < m; ++i) {
        cin >> u >> v >> c;
        g.add_edge(u-1, v-1, c);
    }

    g.preprocess();
    cout << "Ready" << endl;

    int t;
    cin >> t;
    for (int i = 0; i < t; ++i) {
        int u, v;
        cin >> u >> v;
        cout << g.query(u-1, v-1) << "\n";
    }

    return 0;
}



int main2() {
    std::ios::sync_with_stdio(false);
     
    ifstream test("City_Maps/NYC.txt");
    //ifstream test("mytest.txt");
    auto t1 = high_resolution_clock::now();
    
    int u,v,c,n,m;
    test >> n >> m;
    Graph g(n);
    for (int i = 0; i < m; ++i) {
        test >> u >> v >> c;
        g.add_edge(u-1, v-1, c);
    }

    g.printGraphStatistics();
    g.preprocess();

    auto t2 = high_resolution_clock::now();
    
    cout << "Ready" << endl;

    cout << "Preprocess Time: " << (duration_cast<duration<double>>(t2 - t1)).count() << " seconds" << endl;
    g.printGraphStatistics();
    
    int t;
    test >> t;
    for (int i = 0; i < t; ++i) {
        int u, v;
        test >> u >> v;
        cout << g.query(u-1, v-1) << "\n";
    }

    return 0;
}


/* Learnings */
/* 1. Use visited bool array for Dijkstra, As we can have undirected edges or multiple edges to same node. It will avoid loop conditions.
 * 2. In 1-Dijkstra's, if any outgoing node's new distance is more than maxD, then don't explore it.
 * 3. While calculating maxD, ignore the same edge for both incoming/outgoing. This happens in case of undirected graphs. If we have an arc with huge distance, then both incoming and outgoing max becomes that. 
 *    So, we should be considering different edges for calculating maxD.
 * 4. Never recalculate stuffs for already contracted node.
 * 5. Have to avoid adding duplicate edges. That just exponentially expands the number of edges and runs infinitely. These gets added because of same contract edge getting added with different edge weights while contracting two different nodes. Eg: 1->2->3 and 1->4->3. While contracting node 2, we add a shortcut for 1->3 and while contracting node 4, we add a shortcut for 1->3 again. 
 * 6. Lazy update really works. Without this the number of nodes added are ~50X higher.
 *
 */
