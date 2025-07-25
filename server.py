from flask import Flask, request, jsonify, render_template
import csv
from collections import defaultdict
import heapq

network_graph = defaultdict(list)
ip_coordinates = {}

def load_graph_from_csv(filepath):
    with open(filepath, "r") as file:
        reader = csv.reader(file)
        header = next(reader, None) 

        for row in reader:
            try:
                src_ip = row[0].strip()
                dst_ip = row[1].strip()
                src_lng = float(row[2])
                src_lat = float(row[3])
                cost = float(row[4].strip())
                city = row[5].strip() if len(row) > 5 else "Unknown"

                network_graph[src_ip].append((dst_ip, cost))
                network_graph[dst_ip].append((src_ip, cost))

                if src_ip not in ip_coordinates:
                    ip_coordinates[src_ip] = {"lat": src_lat, "lng": src_lng, "city": city}

            except (IndexError, ValueError) as e:
                print(f"Skipping row due to error: {e}")

load_graph_from_csv("Updated_DSADataset.csv")

def dijkstra(start_ip, end_ip="1.108.102.183"):

    queue = [(0, start_ip, [])]
    visited = set()

    while queue:
        cost, current, path = heapq.heappop(queue)

        if current in visited:
            continue

        path = path.copy()
        path.append(current)

        if current == end_ip:
            return path

        visited.add(current)

        for neighbor, edge_cost in network_graph.get(current, []):
            if neighbor not in visited:
                heapq.heappush(queue, (cost + edge_cost, neighbor, path))

    return []


app = Flask(__name__)

@app.route("/")
def index():
    return render_template("Deployment.html")

@app.route("/map-data")
def map_data():
    return jsonify({
        "nodes": ip_coordinates,
        "edges": {
            src: [[dst, cost] for dst, cost in neighbors]
            for src, neighbors in network_graph.items()
        }
    })

@app.route("/path", methods=["POST"])
def get_path():
    try:
        data = request.get_json()
        start_ip = data.get("startIP", "").strip()

        if not start_ip or start_ip not in network_graph:
            return jsonify({"error": "Invalid start IP"}), 400

        path_ips = dijkstra(start_ip)

        if not path_ips:
            return jsonify({"error": "No path to gateway found"}), 404

        # Check for missing coordinates
        missing_coords = [ip for ip in path_ips if ip not in ip_coordinates or
                          ip_coordinates[ip]["lat"] is None or
                          ip_coordinates[ip]["lng"] is None]

        if missing_coords:
            return jsonify({
                "error": f"Missing coordinates for IPs: {', '.join(missing_coords)}"
            }), 500

        # Build enhanced path with city + ip
        path_coords = [
            {
                "ip": ip,
                "lat": ip_coordinates[ip]["lat"],
                "lng": ip_coordinates[ip]["lng"],
                "city": ip_coordinates[ip].get("city", "Unknown")
            }
            for ip in path_ips
        ]

        return jsonify(path_coords)

    except Exception as e:
        return jsonify({"error1": str(e)}), 500

if __name__ == "__main__":
    app.run(debug=True)
