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
    char wind;
    int aqi;
    int anomaly; // Computed dynamically
};

struct Edge {
    int from;
    int to;
    int weight;
};

/* ===================== AQI CALCULATION ===================== */

int calculateAQI(int pm25, int pm10, int co) {
    return (pm25 * 3 + pm10 * 2 + co * 4) / 9;
}

/* ===================== REAL-TIME ANOMALY DETECTION ===================== */

int detectAnomaly(int aqi) {
    if (aqi < 100) return 0;
    else if (aqi < 180) return 1;
    else return 2;
}

/* ===================== CSV READING ===================== */

vector<AirSensor> readCSV(const string& file) {
    vector<AirSensor> data;
    ifstream fin(file);
    string line;

    getline(fin, line); // header

    while (getline(fin, line)) {
        stringstream ss(line);
        AirSensor s;
        string temp;

        getline(ss, s.zone, ',');
        getline(ss, temp, ','); s.pm25 = stoi(temp);
        getline(ss, temp, ','); s.pm10 = stoi(temp);
        getline(ss, temp, ','); s.co = stoi(temp);
        getline(ss, temp, ','); s.wind = temp[0];

        s.aqi = calculateAQI(s.pm25, s.pm10, s.co);
        s.anomaly = detectAnomaly(s.aqi);

        data.push_back(s);
    }
    return data;
}

/* ===================== SORTING (QUICK SORT) ===================== */

int partition(vector<AirSensor>& a, int low, int high) {
    int pivot = a[high].aqi;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (a[j].aqi > pivot) {
            swap(a[++i], a[j]);
        }
    }
    swap(a[i + 1], a[high]);
    return i + 1;
}

void quickSort(vector<AirSensor>& a, int low, int high) {
    if (low < high) {
        int pi = partition(a, low, high);
        quickSort(a, low, pi - 1);
        quickSort(a, pi + 1, high);
    }
}

/* ===================== WIND TRANSFER ===================== */

int windTransfer(char w1, char w2) {
    if (w1 == w2) return 100;
    if ((w1=='E'&&w2=='N')||(w1=='N'&&w2=='E')||
        (w1=='S'&&w2=='W')||(w1=='W'&&w2=='S'))
        return 50;
    return 20;
}

/* ===================== GRAPH ===================== */

class Graph {
    int V;
    vector<vector<int>> adj;

public:
    Graph(int v): V(v), adj(v) {}

    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    /* DFS – Source tracing */
    void DFSUtil(int u, int pollution,
                 vector<bool>& visited,
                 vector<AirSensor>& s) {

        visited[u] = true;
        cout << s[u].zone << " (" << pollution << "%) -> ";

        for (int v : adj[u]) {
            int transfer = windTransfer(s[u].wind, s[v].wind);
            int nextPollution = pollution * transfer / 100;

            if (!visited[v] && nextPollution > 30) {
                DFSUtil(v, nextPollution, visited, s);
            }
        }
    }

    void DFS(int src, vector<AirSensor>& s) {
        vector<bool> visited(V, false);
        cout << "\nDFS Pollution Chain:\n";
        DFSUtil(src, 100, visited, s);
        cout << "END\n";
    }

    /* BFS – Impact order */
    void BFS(int src, vector<AirSensor>& s) {
        vector<bool> visited(V, false);
        vector<int> pollution(V, 0);
        queue<int> q;

        visited[src] = true;
        pollution[src] = 100;
        q.push(src);

        cout << "\nBFS Pollution Spread:\n";

        while (!q.empty()) {
            int u = q.front(); q.pop();
            cout << s[u].zone << " (" << pollution[u] << "%) -> ";

            for (int v : adj[u]) {
                int transfer = windTransfer(s[u].wind, s[v].wind);
                int nextPollution = pollution[u] * transfer / 100;

                if (!visited[v] && nextPollution > 30) {
                    visited[v] = true;
                    pollution[v] = nextPollution;
                    q.push(v);
                }
            }
        }
        cout << "END\n";
    }
};

/* ===================== BELLMAN-FORD (SUDDEN SPIKES) ===================== */

void bellmanFord(int src,
                 int V,
                 vector<Edge>& edges,
                 vector<AirSensor>& s) {

    vector<int> dist(V, INT_MAX);
    dist[src] = 0;

    for (int i = 0; i < V - 1; i++) {
        for (auto& e : edges) {
            int penalty = 0;
            if (s[e.to].anomaly == 1) penalty = -20;
            if (s[e.to].anomaly == 2) penalty = -50;

            if (dist[e.from] != INT_MAX &&
                dist[e.from] + e.weight + penalty < dist[e.to]) {

                dist[e.to] = dist[e.from] + e.weight + penalty;
            }
        }
    }

    cout << "\nBellman-Ford (Anomaly Spread Analysis):\n";
    for (int i = 0; i < V; i++) {
        cout << s[src].zone << " -> " << s[i].zone
             << " = " << dist[i] << endl;
    }
}

/* ===================== DISPLAY ===================== */

void display(vector<AirSensor>& s) {
    cout << "\nCITY AIR STATUS:\n";
    for (auto& x : s) {
        cout << x.zone
             << " | AQI: " << x.aqi
             << " | AnomalyLevel: " << x.anomaly
             << " | Wind: " << x.wind << endl;
    }
}

/* ===================== MAIN ===================== */

int main() {

    vector<AirSensor> sensors = readCSV("air_sensors.csv");

    quickSort(sensors, 0, sensors.size() - 1);
    display(sensors);

    Graph city(sensors.size());
    city.addEdge(0,1);
    city.addEdge(1,2);
    city.addEdge(2,3);
    city.addEdge(0,3);

    city.DFS(0, sensors);
    city.BFS(0, sensors);

    vector<Edge> edges = {
        {0,1,40},{1,2,30},{2,3,20},{3,1,-10}
    };

    bellmanFord(0, sensors.size(), edges, sensors);

    return 0;
}
