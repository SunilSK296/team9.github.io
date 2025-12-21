#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <climits>

using namespace std;

/* ===================== DATA STRUCTURES ===================== */
struct AirSensor {
    string zone;
    int pm25, pm10, co;
    char wind;     // N, S, E, W
    int aqi;
};

/* Graph Edge */
struct Edge {
    int to;
    int weight; // pollution cost or distance
};

/* ===================== AQI FUNCTION ===================== */
int calculateAQI(int pm25, int pm10, int co) {
    return (pm25 * 3 + pm10 * 2 + co * 4) / 9;
}

/* ===================== CSV READING ===================== */
vector<AirSensor> readCSV(const string& file) {
    vector<AirSensor> data;
    ifstream fin(file);
    string line;

    if(!fin.is_open()) {
        cout << "Error: Cannot open file " << file << endl;
        return data;
    }

    getline(fin, line); // skip header

    while (getline(fin, line)) {
        stringstream ss(line);
        AirSensor s;
        string temp;

        getline(ss, s.zone, ',');
        getline(ss, temp, ','); s.pm25 = stoi(temp);
        getline(ss, temp, ','); s.pm10 = stoi(temp);
        getline(ss, temp, ','); s.co   = stoi(temp);
        getline(ss, temp, ','); s.wind = temp[0];

        s.aqi = calculateAQI(s.pm25, s.pm10, s.co);
        data.push_back(s);
    }
    return data;
}

/* ===================== SORTING ALGORITHMS ===================== */
/* Quick Sort */
int partition(vector<AirSensor>& a, int low, int high) {
    int pivot = a[high].aqi;
    int i = low - 1;
    for(int j = low; j < high; j++) {
        if(a[j].aqi > pivot) { // descending AQI
            swap(a[++i], a[j]);
        }
    }
    swap(a[i+1], a[high]);
    return i+1;
}

void quickSort(vector<AirSensor>& a, int low, int high) {
    if(low < high) {
        int pi = partition(a, low, high);
        quickSort(a, low, pi-1);
        quickSort(a, pi+1, high);
    }
}

/* Insertion Sort (for small updates) */
void insertionSort(vector<AirSensor>& a) {
    for(int i = 1; i < a.size(); i++) {
        AirSensor key = a[i];
        int j = i-1;
        while(j >=0 && a[j].aqi < key.aqi) {
            a[j+1] = a[j];
            j--;
        }
        a[j+1] = key;
    }
}

/* ===================== POLLUTION TRANSFER (WIND) ===================== */
int windTransfer(char w1, char w2) {
    if(w1 == w2) return 100;               // same direction
    if((w1=='E' && w2=='N') || (w1=='N' && w2=='E') ||
       (w1=='S' && w2=='W') || (w1=='W' && w2=='S')) return 50; // perpendicular
    return 20;                             // opposite
}

/* ===================== GRAPH CLASS ===================== */
class Graph {
    int V;
    vector<vector<Edge>> adj;

public:
    Graph(int v) : V(v), adj(v) {}

    void addEdge(int u, int v, int w) {
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
    }

    /* DFS with pollution strength */
    void DFSUtil(int u, int pollution, vector<bool>& visited, vector<AirSensor>& sensors) {
        visited[u] = true;
        cout << sensors[u].zone << " (Pollution: " << pollution << "%) -> ";

        for(auto e : adj[u]) {
            int v = e.to;
            int transfer = windTransfer(sensors[u].wind, sensors[v].wind);
            int newPollution = pollution * transfer / 100;

            if(!visited[v] && newPollution > 30) {
                DFSUtil(v, newPollution, visited, sensors);
            }
        }
    }

    void DFS(int src, vector<AirSensor>& sensors) {
        vector<bool> visited(V,false);
        cout << "\nDFS Pollution Source Trace:\n";
        DFSUtil(src,100,visited,sensors);
        cout << "END\n";
    }

    /* BFS with pollution decay */
    void BFS(int src, vector<AirSensor>& sensors) {
        vector<bool> visited(V,false);
        vector<int> pollution(V,0);
        queue<int> q;

        visited[src] = true;
        pollution[src] = 100;
        q.push(src);

        cout << "\nBFS Pollution Spread Level-wise:\n";

        while(!q.empty()) {
            int u = q.front(); q.pop();
            cout << sensors[u].zone << " (Pollution: " << pollution[u] << "%) -> ";

            for(auto e : adj[u]) {
                int v = e.to;
                int transfer = windTransfer(sensors[u].wind, sensors[v].wind);
                int newPollution = pollution[u] * transfer / 100;

                if(!visited[v] && newPollution > 30) {
                    visited[v] = true;
                    pollution[v] = newPollution;
                    q.push(v);
                }
            }
        }
        cout << "END\n";
    }

    /* Dijkstra for least polluted path */
    void dijkstra(int src, vector<AirSensor>& sensors) {
        vector<int> dist(V, INT_MAX);
        dist[src] = 0;

        for(int i = 0; i < V-1; i++) {
            int u = -1;
            for(int j=0;j<V;j++)
                if(u==-1 || dist[j]<dist[u]) u=j;

            for(auto e: adj[u]) {
                if(dist[e.to] > dist[u]+e.weight)
                    dist[e.to] = dist[u]+e.weight;
            }
        }

        cout << "\nLeast Polluted Routes (Dijkstra):\n";
        for(int i=0;i<V;i++)
            cout << sensors[src].zone << " -> " << sensors[i].zone 
                 << " = " << dist[i] << endl;
    }
};

/* ===================== DISPLAY FUNCTION ===================== */
void display(vector<AirSensor>& sensors) {
    cout << "\nAIR POLLUTION STATUS:\n";
    for(auto& x : sensors)
        cout << x.zone << " | AQI: " << x.aqi << " | Wind: " << x.wind << endl;
}

/* ===================== MAIN ===================== */
int main() {

    vector<AirSensor> sensors = readCSV("air_sensors.csv");

    if(sensors.empty()) return 0;

    // Sort by AQI descending
    quickSort(sensors,0,sensors.size()-1);
    display(sensors);

    // Create city graph
    Graph city(sensors.size());
    city.addEdge(0,1,40);
    city.addEdge(1,2,30);
    city.addEdge(2,3,20);
    city.addEdge(0,3,50);

    // Run algorithms
    city.DFS(0, sensors);    // Trace source
    city.BFS(0, sensors);    // Spread level-wise
    city.dijkstra(0, sensors); // Least polluted routes

    return 0;
}

