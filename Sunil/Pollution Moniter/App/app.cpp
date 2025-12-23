#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>

using namespace std;

/*
 Prototype:
 - Area search (Trie concept simulated using string matching)
 - Hash table for fast data lookup
 - CSV-based real-time data simulation
*/

struct PollutionData {
    int airAQI;
    int waterPollution;
    int landQuality;
    string floodRisk;
    string industrialRisk;
};

unordered_map<string, PollutionData> pollutionDB;

// Convert string to lowercase (for flexible search)
string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Load CSV data into hash table
void loadCSV(const string& filename) {
    ifstream file(filename);
    string line;

    getline(file, line); // skip header

    while (getline(file, line)) {
        stringstream ss(line);
        string area, temp;

        PollutionData data;

        getline(ss, area, ',');
        getline(ss, temp, ','); data.airAQI = stoi(temp);
        getline(ss, temp, ','); data.waterPollution = stoi(temp);
        getline(ss, temp, ','); data.landQuality = stoi(temp);
        getline(ss, data.floodRisk, ',');
        getline(ss, data.industrialRisk, ',');

        pollutionDB[toLower(area)] = data;
    }
}

// Display pollution information
void displayInfo(const string& area) {
    string key = toLower(area);

    if (pollutionDB.find(key) == pollutionDB.end()) {
        cout << "\n❌ Area not found in database.\n";
        return;
    }

    PollutionData d = pollutionDB[key];

    cout << "\n📍 Area: " << area << endl;
    cout << "---------------------------------\n";
    cout << "🌫 Air Quality Index: " << d.airAQI << endl;
    cout << "💧 Water Pollution Index: " << d.waterPollution << endl;
    cout << "🌱 Land Quality Index: " << d.landQuality << endl;
    cout << "🌊 Flood Risk: " << d.floodRisk << endl;
    cout << "🏭 Industrial Risk: " << d.industrialRisk << endl;

    cout << "\n🔔 Alerts & Suggestions:\n";

    if (d.airAQI > 150)
        cout << "⚠ High air pollution – Wear masks & reduce outdoor activity.\n";

    if (d.waterPollution > 50)
        cout << "⚠ Water not safe for drinking – Treatment required.\n";

    if (d.landQuality < 50)
        cout << "⚠ Soil degradation detected – Avoid agriculture.\n";

    if (d.floodRisk == "High")
        cout << "🚨 Flood alert – Stay cautious near water bodies.\n";

    if (d.industrialRisk == "High")
        cout << "🚨 Industrial pollution alert – Authorities notified.\n";
}

int main() {
    loadCSV("city_pollution_data.csv");

    cout << "🌍 Smart City Pollution Monitoring System\n";
    cout << "-----------------------------------------\n";

    string area;
    cout << "Enter your area name: ";
    getline(cin, area);

    displayInfo(area);

    return 0;
}
