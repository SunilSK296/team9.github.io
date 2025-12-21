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
    int anomaly; // Real-time computed
};

struct Edge {
    int from;
    int to;
    int weight; // normal pollution cost
};

/* ===================== AQI & ANOMALY ===================== */

int calculateAQI(int pm25, int pm10, int co) {
    return (pm25*3 + pm10*2 + co*4)/9;
}

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
    getline(fin,line); // skip header
    while(getline(fin,line)) {
        stringstream ss(line);
        AirSensor s;
        string temp;
        getline(ss,s.zone,',');
        getline(ss,temp,','); s.pm25 = stoi(temp);
        getline(ss,temp,','); s.pm10 = stoi(temp);
        getline(ss,temp,','); s.co = stoi(temp);
        getline(ss,temp,','); s.wind = temp[0];
        s.aqi = calculateAQI(s.pm25,s.pm10,s.co);
        s.anomaly = detectAnomaly(s.aqi);
        data.push_back(s);
    }
    return data;
}

/* ===================== SORTING ===================== */

int partition(vector<AirSensor>& a,int low,int high){
    int pivot = a[high].aqi;
    int i = low-1;
    for(int j=low;j<high;j++){
        if(a[j].aqi>pivot){
            swap(a[++i],a[j]);
        }
    }
    swap(a[i+1],a[high]);
    return i+1;
}

void quickSort(vector<AirSensor>& a,int low,int high){
    if(low<high){
        int pi = partition(a,low,high);
        quickSort(a,low,pi-1);
        quickSort(a,pi+1,high);
    }
}

/* ===================== WIND TRANSFER ===================== */

int windTransfer(char w1,char w2){
    if(w1==w2) return 100;
    if((w1=='E'&&w2=='N')||(w1=='N'&&w2=='E')||(w1=='S'&&w2=='W')||(w1=='W'&&w2=='S')) return 50;
    return 20;
}

/* ===================== GRAPH ===================== */

class Graph{
public:
    int V;
    vector<vector<int>> adj;
    Graph(int v): V(v), adj(v) {}
    void addEdge(int u,int v){adj[u].push_back(v); adj[v].push_back(u);}

    void DFSUtil(int u,int pollution, vector<bool>& visited, vector<AirSensor>& s){
        visited[u]=true;
        cout<<s[u].zone<<" ("<<pollution<<"%) -> ";
        for(int v:adj[u]){
            int nextPollution = pollution*windTransfer(s[u].wind,s[v].wind)/100;
            if(!visited[v] && nextPollution>30)
                DFSUtil(v,nextPollution,visited,s);
        }
    }

    void DFS(int src, vector<AirSensor>& s){
        vector<bool> visited(V,false);
        cout<<"\nDFS Pollution Chain:\n";
        DFSUtil(src,100,visited,s);
        cout<<"END\n";
    }

    void BFS(int src, vector<AirSensor>& s){
        vector<bool> visited(V,false);
        vector<int> pollution(V,0);
        queue<int> q;
        visited[src]=true; pollution[src]=100;
        q.push(src);
        cout<<"\nBFS Pollution Spread:\n";
        while(!q.empty()){
            int u = q.front(); q.pop();
            cout<<s[u].zone<<" ("<<pollution[u]<<"%) -> ";
            for(int v:adj[u]){
                int nextPollution = pollution[u]*windTransfer(s[u].wind,s[v].wind)/100;
                if(!visited[v] && nextPollution>30){
                    visited[v]=true;
                    pollution[v]=nextPollution;
                    q.push(v);
                }
            }
        }
        cout<<"END\n";
    }
};

/* ===================== BELLMAN-FORD ===================== */

void bellmanFord(int src,int V,vector<Edge>& edges,vector<AirSensor>& s,vector<bool>& blocked){
    vector<int> dist(V,INT_MAX);
    dist[src]=0;

    for(int i=0;i<V-1;i++){
        for(auto &e:edges){
            int penalty=0;
            if(s[e.to].anomaly==1) penalty=-20;
            if(s[e.to].anomaly==2) { penalty=-50; blocked[e.to]=true;}
            if(dist[e.from]!=INT_MAX && dist[e.from]+e.weight+penalty<dist[e.to])
                dist[e.to]=dist[e.from]+e.weight+penalty;
        }
    }

    cout<<"\nBellman-Ford (Anomaly Spread Analysis):\n";
    for(int i=0;i<V;i++){
        if(blocked[i]) cout<<s[i].zone<<" = BLOCKED\n";
        else cout<<s[src].zone<<" -> "<<s[i].zone<<" = "<<dist[i]<<endl;
    }
}

/* ===================== DIJKSTRA AVOIDING BLOCKED ZONES ===================== */

void dijkstraAvoidingZones(int src,int n,vector<vector<pair<int,int>>>& adj,vector<bool>& blocked,vector<string>& zones){
    vector<int> dist(n,INT_MAX);
    priority_queue<pair<int,int>,vector<pair<int,int>>,greater<>> pq;
    if(blocked[src]) {cout<<"Source zone blocked. Routing aborted.\n"; return;}
    dist[src]=0;
    pq.push({0,src});

    while(!pq.empty()){
        int u=pq.top().second; pq.pop();
        for(auto edge:adj[u]){
            int v=edge.first; int w=edge.second;
            if(blocked[v]) continue;
            if(dist[u]+w<dist[v]) { dist[v]=dist[u]+w; pq.push({dist[v],v}); }
        }
    }

    cout<<"\nDijkstra (Avoiding Polluted Zones):\n";
    for(int i=0;i<n;i++){
        if(blocked[i]) cout<<zones[i]<<" = BLOCKED\n";
        else if(dist[i]==INT_MAX) cout<<zones[i]<<" = UNREACHABLE\n";
        else cout<<zones[src]<<" -> "<<zones[i]<<" = "<<dist[i]<<endl;
    }
}

/* ===================== DISPLAY ===================== */

void display(vector<AirSensor>& s){
    cout<<"\nCITY AIR STATUS:\n";
    for(auto& x:s)
        cout<<x.zone<<" | AQI: "<<x.aqi<<" | AnomalyLevel: "<<x.anomaly<<" | Wind: "<<x.wind<<endl;
}

/* ===================== MAIN ===================== */

int main(){
    vector<AirSensor> sensors = readCSV("air_sensors.csv");

    quickSort(sensors,0,sensors.size()-1);
    display(sensors);

    // Graph for BFS/DFS
    Graph city(sensors.size());
    city.addEdge(0,1);
    city.addEdge(1,2);
    city.addEdge(2,3);
    city.addEdge(0,3);

    city.DFS(0,sensors);
    city.BFS(0,sensors);

    // Bellman-Ford edges
    vector<Edge> edges = { {0,1,40},{1,2,30},{2,3,20},{3,1,-10} };
    vector<bool> blocked(sensors.size(),false);

    bellmanFord(0,sensors.size(),edges,sensors,blocked);

    // Dijkstra avoiding blocked zones
    vector<vector<pair<int,int>>> adj(sensors.size());
    adj[0].push_back({1,40}); adj[1].push_back({0,40});
    adj[1].push_back({2,30}); adj[2].push_back({1,30});
    adj[2].push_back({3,20}); adj[3].push_back({2,20});
    adj[3].push_back({1,-10}); adj[1].push_back({3,-10});
    vector<string> zones;
    for(auto &x:sensors) zones.push_back(x.zone);

    dijkstraAvoidingZones(0,sensors.size(),adj,blocked,zones);

    return 0;
}
