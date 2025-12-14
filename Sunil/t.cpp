#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

#define SIGNALS 4
#define NEIGHBORS 4
#define MAX_GREEN 60
#define MIN_GREEN 15

struct Signal {
    int id;
    int vehicles;
    bool emergency;
    int greenTime;
};

struct Junction {
    int id;
    vector<Signal> signals;
    int totalVehicles;
};

/* ---------- Function Declarations ---------- */
void readSignalCSV(const string& filename, Junction& j);
void readNearbyCSV(const string& filename, vector<Junction>& neighbors);
int checkEmergency(const Junction& j);
void handleEmergency(Junction& j, vector<vector<int> >& graph, int source);
void optimizeTraffic(Junction& j, const vector<Junction>& neighbors);
void displayJunction(const Junction& j);
vector<int> dijkstra(vector<vector<int> >& graph, int src);
void heapify(vector<Signal>& s, int n, int i);
void heapSort(vector<Signal>& s);
void initGraph(vector<vector<int> >& graph);

/* ---------- MAIN ---------- */
int main() {
    Junction current;
    current.id = 0;
    current.signals.resize(SIGNALS);

    vector<Junction> neighbors(NEIGHBORS);

    vector<vector<int> > graph;
    initGraph(graph);

    cout << "SMART TRAFFIC MANAGEMENT SYSTEM (CSV INPUT)\n\n";

    readSignalCSV("traffic_data.csv", current);
    readNearbyCSV("nearby_junctions.csv", neighbors);

    int emergencyIndex = checkEmergency(current);

    if (emergencyIndex != -1) {
        handleEmergency(current, graph, emergencyIndex);
    } else {
        optimizeTraffic(current, neighbors);
    }

    displayJunction(current);
    return 0;
}

/* ---------- INITIALIZE GRAPH ---------- */
void initGraph(vector<vector<int> >& graph) {
    graph.resize(SIGNALS);
    for (int i=0;i<SIGNALS;i++)
        graph[i].resize(SIGNALS,0);

    graph[0][1] = 1; graph[0][2] = 1; graph[0][3] = 1;
    graph[1][0] = 1; graph[1][2] = 1; graph[1][3] = 1;
    graph[2][0] = 1; graph[2][1] = 1; graph[2][3] = 1;
    graph[3][0] = 1; graph[3][1] = 1; graph[3][2] = 1;
}

/* ---------- READ SIGNAL CSV ---------- */
void readSignalCSV(const string& filename, Junction& j) {
    ifstream file(filename);
    string line;

    if(!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    getline(file, line); // skip header
    j.totalVehicles = 0;
    int index = 0;

    while (getline(file, line) && index < SIGNALS) {
        stringstream ss(line);
        string temp;

        getline(ss, temp, ','); // signal id
        j.signals[index].id = index;

        getline(ss, temp, ',');
        j.signals[index].vehicles = atoi(temp.c_str());

        getline(ss, temp, ',');
        j.signals[index].emergency = atoi(temp.c_str());

        j.signals[index].greenTime = 0;
        j.totalVehicles += j.signals[index].vehicles;
        index++;
    }
    file.close();
}

/* ---------- READ NEARBY JUNCTION CSV ---------- */
void readNearbyCSV(const string& filename, vector<Junction>& neighbors) {
    ifstream file(filename);
    string line;

    if(!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    getline(file, line); // skip header
    int index = 0;

    while (getline(file, line) && index < NEIGHBORS) {
        stringstream ss(line);
        string temp;

        getline(ss, temp, ',');
        neighbors[index].id = atoi(temp.c_str());

        getline(ss, temp, ',');
        neighbors[index].totalVehicles = atoi(temp.c_str());

        index++;
    }
    file.close();
}

/* ---------- CHECK EMERGENCY ---------- */
int checkEmergency(const Junction& j) {
    for (int i = 0; i < SIGNALS; i++) {
        if (j.signals[i].emergency)
            return i;
    }
    return -1;
}

/* ---------- HANDLE EMERGENCY ---------- */
void handleEmergency(Junction& j, vector<vector<int> >& graph, int source) {
    cout << "🚑 Emergency detected at Signal " << source + 1 << endl;

    vector<int> dist = dijkstra(graph, source);

    cout << "Green Corridor Distances:\n";
    for (int i = 0; i < SIGNALS; i++) {
        cout << "Signal " << source + 1 << " → Signal " << i + 1
             << " Distance: " << dist[i] << endl;
    }

    for (int i = 0; i < SIGNALS; i++)
        j.signals[i].greenTime = (i == source) ? MAX_GREEN : 0;
}

/* ---------- NORMAL TRAFFIC OPTIMIZATION ---------- */
void optimizeTraffic(Junction& j, const vector<Junction>& neighbors) {
    cout << "No emergency detected.\n";

    heapSort(j.signals);

    int neighborLoad = 0;
    for (size_t i = 0; i < neighbors.size(); i++)
        neighborLoad += neighbors[i].totalVehicles;

    int adjustedLoad = j.totalVehicles + (neighborLoad / NEIGHBORS);

    for (size_t i = 0; i < j.signals.size(); i++) {
        j.signals[i].greenTime = (j.signals[i].vehicles * MAX_GREEN) / adjustedLoad;
        if (j.signals[i].greenTime < MIN_GREEN)
            j.signals[i].greenTime = MIN_GREEN;
    }

    // Restore original signal order
    sort(j.signals.begin(), j.signals.end(),
         [](const Signal& a, const Signal& b){ return a.id < b.id; });
}

/* ---------- HEAP SORT ---------- */
void heapify(vector<Signal>& s, int n, int i) {
    int largest = i;
    int left = 2*i + 1;
    int right = 2*i + 2;

    if (left < n && s[left].vehicles > s[largest].vehicles)
        largest = left;
    if (right < n && s[right].vehicles > s[largest].vehicles)
        largest = right;

    if (largest != i) {
        swap(s[i], s[largest]);
        heapify(s, n, largest);
    }
}

void heapSort(vector<Signal>& s) {
    int n = s.size();
    for (int i = n/2 -1; i >=0; i--)
        heapify(s,n,i);
    for (int i = n-1; i>0; i--) {
        swap(s[0], s[i]);
        heapify(s,i,0);
    }
}

/* ---------- DIJKSTRA ---------- */
vector<int> dijkstra(vector<vector<int> >& graph, int src) {
    vector<int> dist(SIGNALS, INT_MAX);
    priority_queue< pair<int,int>, vector< pair<int,int> >, greater< pair<int,int> > > pq;

    dist[src] = 0;
    pq.push(make_pair(0, src));

    while(!pq.empty()) {
        int u = pq.top().second; pq.pop();

        for(int v=0; v<SIGNALS; v++) {
            if(graph[u][v] && dist[v] > dist[u]+1) {
                dist[v] = dist[u]+1;
                pq.push(make_pair(dist[v], v));
            }
        }
    }
    return dist;
}

/* ---------- DISPLAY ---------- */
void displayJunction(const Junction& j) {
    cout << "\nFinal Signal Timings:\n";
    for(size_t i=0;i<j.signals.size();i++) {
        cout << "Signal " << j.signals[i].id+1
             << " | Vehicles: " << j.signals[i].vehicles
             << " | Green: " << j.signals[i].greenTime << " sec\n";
    }
}
