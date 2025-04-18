#include <bits/stdc++.h>
#include <vector>
#include <chrono>

using namespace std;

struct Edge {
    string destIP;
    int latency; 
};

unordered_map<string, vector<Edge>> networkGraph;
unordered_map<string, string> ipToCity;


void addConnection(const string& from, const string& to, int latency) {
    networkGraph[from].push_back({to, latency});
    networkGraph[to].push_back({from, latency}); 
}

int heuristic(const string& a, const string& b) {
    // Simple heuristic: count difference in IP octets
    istringstream sa(a), sb(b);
    string oa, ob;
    int score = 0;
    while (getline(sa, oa, '.') && getline(sb, ob, '.')) {
        score += abs(stoi(oa) - stoi(ob));
    }
    return score;
}


pair<int, vector<string>> AStar(const string& startIP, const string& gatewayIP) {
    unordered_map<string, int> gScore; // cost from start to node
    unordered_map<string, int> fScore; // estimated cost from start to goal through node
    unordered_map<string, string> previous;

    for (const auto& node : networkGraph) {
        gScore[node.first] = INT_MAX;
        fScore[node.first] = INT_MAX;
    }

    gScore[startIP] = 0;
    fScore[startIP] = heuristic(startIP, gatewayIP);

    // Min-heap based on fScore
    priority_queue<pair<int, string>, vector<pair<int, string>>, greater<>> pq;
    pq.push({fScore[startIP], startIP});

    while (!pq.empty()) {
        string current = pq.top().second;
        pq.pop();

        if (current == gatewayIP) break;

        for (const auto& edge : networkGraph[current]) {
            int tentativeGScore = gScore[current] + edge.latency;

            if (tentativeGScore < gScore[edge.destIP]) {
                previous[edge.destIP] = current;
                gScore[edge.destIP] = tentativeGScore;
                fScore[edge.destIP] = tentativeGScore + heuristic(edge.destIP, gatewayIP);
                pq.push({fScore[edge.destIP], edge.destIP});
            }
        }
    }

    // Reconstruct path
    vector<string> path;
    string at = gatewayIP;
    if (previous.find(at) == previous.end()) {
        return {-1, {}}; // No path found
    }
    while (previous.find(at) != previous.end()) {
        path.push_back(at);
        at = previous[at];
    }
    path.push_back(startIP);
    reverse(path.begin(), path.end());

    return {gScore[gatewayIP], path};
}


pair<int, vector<string>> bellmanFord(const string& startIP, const string& gatewayIP) {
    unordered_map<string, int> distances;
    unordered_map<string, string> predecessor;
    
    for (const auto& node : networkGraph) {
        distances[node.first] = INT_MAX;
    }
    distances[startIP] = 0;

    int V = networkGraph.size();

    for (int i = 0; i < V - 1; ++i) {
        for (const auto& u : networkGraph) {
            for (const auto& edge : u.second) {
                if (distances[u.first] != INT_MAX &&
                    distances[u.first] + edge.latency < distances[edge.destIP]) {
                    distances[edge.destIP] = distances[u.first] + edge.latency;
                    predecessor[edge.destIP] = u.first;
                }
            }
        }
    }

    vector<string> path;
    string at = gatewayIP;

    if (distances[at] == INT_MAX) {
        return {INT_MAX, {}};
    }

    while (predecessor.find(at) != predecessor.end()) {
        path.push_back(at);
        at = predecessor[at];
    }
    path.push_back(startIP);
    reverse(path.begin(), path.end());

    return {distances[gatewayIP], path};
}

pair<int, vector<string>> Dijkstra(const string& startIP, const string& gatewayIP) {
    unordered_map<string, int> distances;
    unordered_map<string, string> previous;
    for (const auto& node : networkGraph) {
        distances[node.first] =INT_MAX;
    }

    distances[startIP] = 0;
    priority_queue<pair<int, string>, vector<pair<int, string>>, greater<>> pq;
    pq.push({0, startIP});

    while (!pq.empty()) {
        int dist;
        string current;
        dist= pq.top().first;
        current=pq.top().second;
        pq.pop();

        if (current == gatewayIP) break;

        for (const auto& edge : networkGraph[current]) {
            int newDist = dist + edge.latency;
            if (newDist < distances[edge.destIP]) {
                distances[edge.destIP] = newDist;
                previous[edge.destIP] = current;
                pq.push({newDist, edge.destIP});
            }
        }
    }

    vector<string> path;
    string at = gatewayIP;
    while (previous.find(at) != previous.end()) {
        path.push_back(at);
        at = previous[at];
    }
    path.push_back(startIP);
    reverse(path.begin(), path.end());

    return {distances[gatewayIP], path};
}

void generateDotFile(const vector<string>& path, const string& fileName) {
    ofstream dotFile(fileName);
    
    dotFile << "graph network {\n";
    dotFile << "    node [shape=circle];\n";  

    for (const auto& node : networkGraph) {
        dotFile << "    \"" << node.first << "\";\n";  
    }

    if (!path.empty()) {
        const string& startNode = path.front();
        string label = startNode;
        if (ipToCity.count(startNode)) {
            label += "\\n" + ipToCity.at(startNode);
        }
        dotFile << "    \"" << startNode << "\" [shape=doublecircle, style=filled, fillcolor=lightblue, label=\"" << label << "\"];\n";
        dotFile << "    { rank=source; \"" << startNode << "\"; }\n";
    }

    
    if (path.size() > 1) {
        const string& gatewayNode = path.back();
        string label = gatewayNode;
        if (ipToCity.count(gatewayNode)) {
            label += "\\n" + ipToCity.at(gatewayNode);
        }
        dotFile << "    \"" << gatewayNode << "\" [shape=doubleoctagon, style=filled, fillcolor=lightgreen, label=\"" << label << "\"];\n";
    }

    for (const auto& node : networkGraph) {
        for (const auto& edge : node.second) {
            if (node.first < edge.destIP) { 
                dotFile << "    \"" << node.first << "\" -- \"" << edge.destIP << "\" [label=\"" 
                        << edge.latency << " ms\"];\n";
            }
        }
    }

    for (size_t i = 0; i < path.size() - 1; ++i) {
        dotFile << "    \"" << path[i] << "\" -- \"" << path[i + 1] << "\" [color=red, penwidth=2];\n";
    }

    dotFile << "}\n";
    dotFile.close();
}


int main() {
    ifstream file("DSADataset.csv");

    if (!file.is_open()) {
        cerr << "Error opening file!" << std::endl;
        return 1;
    }
    vector<string> ip_address_1, ip_address_2, city;
    vector<double> longitude, latitude, latency;

    string line, value;

    getline(file, line);
    while (getline(file, line)) {
        stringstream ss(line);

        getline(ss, value, ','); ip_address_1.push_back(value);
        getline(ss, value, ','); ip_address_2.push_back(value);
        getline(ss, value, ','); longitude.push_back(stod(value));
        getline(ss, value, ','); latitude.push_back(stod(value));
        getline(ss, value, ','); latency.push_back(stoi(value));
        getline(ss, value, ','); city.push_back(value);
        ipToCity[ip_address_1.back()] = city.back();
        ipToCity[ip_address_2.back()] = city.back();  

    
    }

    file.close();
    int n = ip_address_1.size();
    for(int i=0;i<n;i++){
        addConnection(ip_address_1[i], ip_address_2[i],latency[i]);
    }

    string startRouterIP;
    string gatewayIP = "1.99.185.142";

    cout << "Enter the IP address of the router: ";
    cin >> startRouterIP;

    if (networkGraph.find(startRouterIP) == networkGraph.end()) {
        cout << "\nRouter IP not found in network.\n";
        return 1;
    }

    
    auto startTime = chrono::high_resolution_clock::now();
    auto result = Dijkstra(startRouterIP, gatewayIP);
    auto endTime = chrono::high_resolution_clock::now();
    auto durationDijkstra = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

    
    startTime = chrono::high_resolution_clock::now();
    auto bellmanResult = bellmanFord(startRouterIP, gatewayIP);
    endTime = chrono::high_resolution_clock::now();
    auto durationBellman = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

    startTime = chrono::high_resolution_clock::now();
    auto astarResult = AStar(startRouterIP, gatewayIP);
    endTime = chrono::high_resolution_clock::now();
    auto durationAStar = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();


    cout << "\n--- Dijkstra's Algorithm ---\n";
    if (result.first == INT_MAX) {
        cout << "No path to gateway found using Dijkstra.\n";
    } else {
        cout << "Path: ";
        for (const auto& ip : result.second) {
            cout << ip;
            if (ipToCity.find(ip) != ipToCity.end()) cout << " (" << ipToCity[ip] << ")";
            if (ip != result.second.back()) cout << " -> ";
        }
        cout << "\nLatency: " << result.first << " ms\n";
        cout << "Time taken: " << durationDijkstra << " milliseconds\n";
    }

    cout << "\n--- Bellman-Ford Algorithm ---\n";
    if (bellmanResult.first == INT_MAX) {
        cout << "No path to gateway found using Bellman-Ford.\n";
    } else {
        cout << "Path: ";
        for (const auto& ip : bellmanResult.second) {
            cout << ip;
            if (ipToCity.find(ip) != ipToCity.end()) cout << " (" << ipToCity[ip] << ")";
            if (ip != bellmanResult.second.back()) cout << " -> ";
        }
        cout << "\nLatency: " << bellmanResult.first << " ms\n";
        cout << "Time taken: " << durationBellman << " milliseconds\n";
    }
    cout << "\n--- A* Algorithm ---\n";
    if (astarResult.first == -1) {
        cout << "No path to gateway found using A*.\n";
    } else {
        cout << "Path: ";
        for (const auto& ip : astarResult.second) {
            cout << ip;
            if (ipToCity.find(ip) != ipToCity.end()) cout << " (" << ipToCity[ip] << ")";
            if (ip != astarResult.second.back()) cout << " -> ";
        }
        cout << "\nLatency: " << astarResult.first << " ms\n";
        cout << "Time taken: " << durationAStar << " milliseconds\n";
    }


    generateDotFile(result.second, "graph.dot");
    cout << "\nDOT file generated as 'graph.dot'.\n";
    return 0;
}
