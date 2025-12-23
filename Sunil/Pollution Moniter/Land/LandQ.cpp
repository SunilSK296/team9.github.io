#include <bits/stdc++.h>
using namespace std;

// Structure to store subzone information
struct Subzone {
    string zone;
    string name;
    double pollution;
    double soilToxicity;
    double moisture;
    double heavyMetal;
    double landQuality;
    int index; // for graph
};

// Priority queue comparator (max-heap) based on land quality and pollution
struct ComparePriority {
    bool operator()(Subzone const &a, Subzone const &b) {
        if(a.landQuality == b.landQuality)
            return a.pollution < b.pollution; // higher pollution first if same quality
        return a.landQuality > b.landQuality; // lower land quality first
    }
};

// Bellman-Ford for spike detection in pollution or toxicity
bool bellmanFord(vector<vector<double>>& graph, vector<Subzone>& subzones, int start) {
    int n = subzones.size();
    vector<double> dist(n, 1e9);
    dist[start] = 0;
    
    for(int i=0;i<n-1;i++){
        for(int u=0;u<n;u++){
            for(int v=0;v<n;v++){
                if(dist[u] + graph[u][v] < dist[v]){
                    dist[v] = dist[u] + graph[u][v];
                }
            }
        }
    }
    
    // Detect negative cycles or spikes
    for(int u=0;u<n;u++){
        for(int v=0;v<n;v++){
            if(dist[u] + graph[u][v] < dist[v]){
                return true; // spike detected
            }
        }
    }
    return false;
}

// Dijkstra for shortest path for waste collection
vector<int> dijkstra(vector<vector<double>>& graph, int start) {
    int n = graph.size();
    vector<double> dist(n, 1e9);
    vector<int> parent(n, -1);
    dist[start] = 0;
    
    set<pair<double,int>> s;
    s.insert({0,start});
    
    while(!s.empty()){
        auto it = s.begin();
        int u = it->second;
        s.erase(it);
        
        for(int v=0;v<n;v++){
            if(graph[u][v] > 0 && dist[u] + graph[u][v] < dist[v]){
                s.erase({dist[v],v});
                dist[v] = dist[u] + graph[u][v];
                parent[v] = u;
                s.insert({dist[v],v});
            }
        }
    }
    return parent;
}

// Compute Land Quality
double computeLandQuality(double pollution, double soilTox, double heavyMetal){
    return 100 - (0.5*pollution + 0.3*soilTox + 0.2*heavyMetal);
}

int main() {
    ifstream file("land_pollution_data.csv");
    string line;
    vector<Subzone> subzones;
    
    getline(file, line); // skip header
    int index = 0;
    
    // Read CSV data
    while(getline(file, line)){
        stringstream ss(line);
        string zone, name;
        double pollution, soil, moisture, heavy;
        getline(ss, zone, ',');
        getline(ss, name, ',');
        ss >> pollution;
        ss.ignore();
        ss >> soil;
        ss.ignore();
        ss >> moisture;
        ss.ignore();
        ss >> heavy;
        ss.ignore();
        Subzone sz;
        sz.zone = zone;
        sz.name = name;
        sz.pollution = pollution;
        sz.soilToxicity = soil;
        sz.moisture = moisture;
        sz.heavyMetal = heavy;
        sz.landQuality = computeLandQuality(pollution, soil, heavy);
        sz.index = index++;
        subzones.push_back(sz);
    }
    
    // Build adjacency matrix for distances (example, replace with actual distances)
    int n = subzones.size();
    vector<vector<double>> graph(n, vector<double>(n, 1e9));
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(i==j) graph[i][j] = 0;
            else graph[i][j] = 1; // placeholder distance, replace with actual
        }
    }
    
    // Detect sudden spikes using Bellman-Ford
    bool spikeDetected = bellmanFord(graph, subzones, 0);
    if(spikeDetected) cout << "⚠️ Spike detected in pollution or soil toxicity!" << endl;
    
    // Use priority queue to select subzones for waste collection
    priority_queue<Subzone, vector<Subzone>, ComparePriority> pq;
    for(auto &sz: subzones){
        pq.push(sz);
    }
    
    cout << "\nWaste Collection Priority:\n";
    while(!pq.empty()){
        Subzone sz = pq.top(); pq.pop();
        cout << sz.zone << " - " << sz.name 
             << " | LandQuality: " << sz.landQuality 
             << " | Pollution: " << sz.pollution << endl;
    }
    
    // Compute shortest path from depot (index 0) using Dijkstra
    vector<int> parent = dijkstra(graph, 0);
    cout << "\nShortest paths from Depot:\n";
    for(int i=0;i<n;i++){
        cout << subzones[i].name << " <- ";
        int cur = i;
        while(parent[cur] != -1){
            cout << subzones[parent[cur]].name << " <- ";
            cur = parent[cur];
        }
        cout << "Depot" << endl;
    }
    
    return 0;
}
