#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
using namespace std;

/* ===================== DATA STRUCTURES ===================== */

struct SubZone {
    string zone;
    string name;
    float waterLevel;
    float pollution;
};

struct Edge {
    int u, v, weight;
};

struct ZonePriority {
    string zone;
    string subzone;
    float score;

    bool operator<(const ZonePriority& other) const {
        return score < other.score; // Max-Heap
    }
};

/* ===================== CSV READING ===================== */

vector<SubZone> readCSV(string filename) {
    vector<SubZone> data;
    ifstream file(filename);
    string line;

    getline(file, line); // skip header

    while (getline(file, line)) {
        stringstream ss(line);
        string zone, sub, wl, pol;

        getline(ss, zone, ',');
        getline(ss, sub, ',');
        getline(ss, wl, ',');
        getline(ss, pol, ',');

        data.push_back({zone, sub, stof(wl), stof(pol)});
    }
    return data;
}

vector<SubZone> getZone(vector<SubZone>& all, string zoneName) {
    vector<SubZone> z;
    for (auto& s : all)
        if (s.zone == zoneName)
            z.push_back(s);
    return z;
}

/* ===================== BELLMAN–FORD (SPIKE DETECTION) ===================== */

bool bellmanFordSpike(int V, vector<Edge>& edges) {
    vector<int> dist(V, 0);

    for (int i = 1; i <= V - 1; i++)
        for (auto& e : edges)
            if (dist[e.u] + e.weight < dist[e.v])
                dist[e.v] = dist[e.u] + e.weight;

    for (auto& e : edges)
        if (dist[e.u] + e.weight < dist[e.v])
            return true; // spike detected

    return false;
}

/* ===================== FLOOD DETECTION ===================== */

void floodCheck(vector<SubZone>& upstream) {
    int flooded = 0;
    for (auto& z : upstream)
        if (z.waterLevel > 80)
            flooded++;

    if (flooded >= 3)
        cout << "🚨 FLOOD ALERT: Upstream water rising rapidly\n";
}

/* ===================== DAM CONTROL ===================== */

void damControl(vector<SubZone>& dam) {
    float avg = 0;
    for (auto& z : dam)
        avg += z.pollution;
    avg /= dam.size();

    if (avg > 30)
        cout << "⚠️ DAM ALERT: Water polluted, releasing water to refresh reservoir\n";
}

/* ===================== INDUSTRIAL ALERT ===================== */

void industrialAlert(vector<SubZone>& downstream2) {
    for (auto& z : downstream2) {
        if (z.pollution > 60) {
            cout << "🚨 INDUSTRIAL ALERT: Pollution spike at " << z.name << "\n";
            return;
        }
    }
}

/* ===================== FISHING ADVISORY ===================== */

void fishingCheck(vector<SubZone>& downstream1) {
    int safe = 0;
    for (auto& z : downstream1)
        if (z.pollution < 40)
            safe++;

    if (safe >= 3)
        cout << "🎣 Fishing Allowed in Downstream-1\n";
    else
        cout << "❌ Fishing Banned in Downstream-1\n";
}

/* ===================== PRIORITY QUEUE (HEAP) ===================== */

priority_queue<ZonePriority> buildPriorityQueue(vector<SubZone>& allData) {
    priority_queue<ZonePriority> pq;

    for (auto& z : allData) {
        float score = z.pollution * 2;

        if (z.waterLevel > 80)
            score += 50; // flood priority

        pq.push({z.zone, z.name, score});
    }
    return pq;
}

void processPriorities(priority_queue<ZonePriority>& pq) {
    cout << "\n--- PRIORITY ACTION ORDER (Heap Based) ---\n";
    while (!pq.empty()) {
        auto top = pq.top();
        pq.pop();
        cout << "Zone: " << top.zone
             << ", SubZone: " << top.subzone
             << ", Priority Score: " << top.score << endl;
    }
}

/* ===================== MAIN ===================== */

int main() {

    vector<SubZone> allData = readCSV("water_zones_data.csv");

    vector<SubZone> upstream     = getZone(allData, "Upstream");
    vector<SubZone> dam          = getZone(allData, "Dam");
    vector<SubZone> downstream1  = getZone(allData, "Downstream1");
    vector<SubZone> downstream2  = getZone(allData, "Downstream2");

    vector<Edge> upstreamEdges = {
        {0,1,-5}, {1,2,-6}, {2,3,-8}, {3,4,-7}, {4,1,-10}
    };

    cout << "=== SMART WATER POLLUTION MONITORING SYSTEM ===\n\n";

    if (bellmanFordSpike(5, upstreamEdges))
        cout << "⚠️ Sudden Spike Detected in Upstream Subzones\n";

    floodCheck(upstream);
    damControl(dam);
    industrialAlert(downstream2);
    fishingCheck(downstream1);

    auto pq = buildPriorityQueue(allData);
    processPriorities(pq);

    return 0;
}

