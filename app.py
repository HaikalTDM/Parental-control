from flask import Flask, jsonify, request
from flask_cors import CORS
import json
import os

app = Flask(__name__)
CORS(app)

# Data & Persistence
DATA_FILE = 'data.json'

DEFAULT_DATA = {
    "internet_active": True,
    "devices": [
        {"id": 1, "name": "Atif's iPad", "type": "tablet", "status": "online", "blocked": False, "usage": "1.2 GB"},
        {"id": 2, "name": "Dad's Laptop", "type": "laptop", "status": "online", "blocked": False, "usage": "4.5 GB"},
        {"id": 3, "name": "Living Room TV", "type": "tv", "status": "offline", "blocked": False, "usage": "0 GB"},
    ],
    "blocked_apps": {
        "youtube": False,
        "tiktok": True,
        "roblox": False,
        "instagram": False
    },
    "custom_blocklist": [
        {"id": 1, "domain": "gambling.com", "active": True},
        {"id": 2, "domain": "adult-site.com", "active": True}
    ],
    "allowlist": [
        {"id": 1, "domain": "google.com", "active": True},
        {"id": 2, "domain": "myschool.edu", "active": True}
    ]
}

def load_data():
    if not os.path.exists(DATA_FILE):
        save_data(DEFAULT_DATA)
        return DEFAULT_DATA
    try:
        with open(DATA_FILE, 'r') as f:
            return json.load(f)
    except:
        return DEFAULT_DATA

def save_data(data):
    with open(DATA_FILE, 'w') as f:
        json.dump(data, f, indent=2)

# Routes

@app.route('/api/status', methods=['GET'])
def get_status():
    data = load_data()
    return jsonify({"internet_active": data["internet_active"]})

@app.route('/api/toggle-internet', methods=['POST'])
def toggle_internet():
    data = load_data()
    data["internet_active"] = not data["internet_active"]
    save_data(data)
    return jsonify({"internet_active": data["internet_active"]})

@app.route('/api/devices', methods=['GET'])
def get_devices():
    data = load_data()
    return jsonify(data["devices"])

@app.route('/api/device/<int:device_id>/block', methods=['POST'])
def toggle_device_block(device_id):
    data = load_data()
    for device in data["devices"]:
        if device["id"] == device_id:
            device["blocked"] = not device["blocked"]
            break
    save_data(data)
    return jsonify(data["devices"])

@app.route('/api/blocklist', methods=['GET'])
def get_blocklist():
    data = load_data()
    return jsonify({
        "apps": data["blocked_apps"],
        "custom": data["custom_blocklist"]
    })

@app.route('/api/blocklist/app', methods=['POST'])
def toggle_app_block():
    app_id = request.json.get('id')
    data = load_data()
    if app_id in data["blocked_apps"]:
        data["blocked_apps"][app_id] = not data["blocked_apps"][app_id]
        save_data(data)
    return jsonify(data["blocked_apps"])

@app.route('/api/blocklist/custom', methods=['POST'])
def add_custom_block():
    domain = request.json.get('domain')
    data = load_data()
    new_id = int(max([i['id'] for i in data["custom_blocklist"]] or [0])) + 1
    data["custom_blocklist"].append({"id": new_id, "domain": domain, "active": True})
    save_data(data)
    return jsonify(data["custom_blocklist"])

@app.route('/api/blocklist/custom/<int:block_id>', methods=['DELETE'])
def remove_custom_block(block_id):
    data = load_data()
    data["custom_blocklist"] = [d for d in data["custom_blocklist"] if d["id"] != block_id]
    save_data(data)
    return jsonify(data["custom_blocklist"])

@app.route('/api/blocklist/custom/<int:block_id>/toggle', methods=['POST'])
def toggle_custom_block(block_id):
    data = load_data()
    for item in data["custom_blocklist"]:
        if item["id"] == block_id:
            item["active"] = not item["active"]
            break
    save_data(data)
    return jsonify(data["custom_blocklist"])

@app.route('/api/allowlist', methods=['GET'])
def get_allowlist():
    data = load_data()
    return jsonify(data["allowlist"])

@app.route('/api/allowlist', methods=['POST'])
def add_allow_domain():
    domain = request.json.get('domain')
    data = load_data()
    new_id = int(max([i['id'] for i in data["allowlist"]] or [0])) + 1
    data["allowlist"].append({"id": new_id, "domain": domain, "active": True})
    save_data(data)
    return jsonify(data["allowlist"])

@app.route('/api/allowlist/<int:allow_id>', methods=['DELETE'])
def remove_allow_domain(allow_id):
    data = load_data()
    data["allowlist"] = [d for d in data["allowlist"] if d["id"] != allow_id]
    save_data(data)
    return jsonify(data["allowlist"])

@app.route('/api/allowlist/<int:allow_id>/toggle', methods=['POST'])
def toggle_allow_domain(allow_id):
    data = load_data()
    for item in data["allowlist"]:
        if item["id"] == allow_id:
            item["active"] = not item["active"]
            break
    save_data(data)
    return jsonify(data["allowlist"])

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5050)
