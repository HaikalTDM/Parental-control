from flask import Flask, jsonify, request
from flask_cors import CORS
import json
import os
import subprocess
import threading
import time
from datetime import datetime

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
    ],
    "adblock_status": "idle",  # idle, loading, error
    "adblock_logs": []
}

# OpenWRT Configuration
# Update these with your router details
OPENWRT_HOST = os.getenv('OPENWRT_HOST', '192.168.1.1')
OPENWRT_USER = os.getenv('OPENWRT_USER', 'root')
OPENWRT_PASSWORD = os.getenv('OPENWRT_PASSWORD', '')
OPENWRT_KEY_FILE = os.getenv('OPENWRT_KEY_FILE', None)

# SSH Helper Functions
def execute_ssh_command(command):
    """Execute command on OpenWRT router via SSH"""
    try:
        # Try paramiko if available
        try:
            import paramiko
            
            ssh = paramiko.SSHClient()
            ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            
            if OPENWRT_KEY_FILE:
                ssh.connect(OPENWRT_HOST, username=OPENWRT_USER, key_filename=OPENWRT_KEY_FILE)
            else:
                ssh.connect(OPENWRT_HOST, username=OPENWRT_USER, password=OPENWRT_PASSWORD)
            
            stdin, stdout, stderr = ssh.exec_command(command)
            output = stdout.read().decode('utf-8')
            error = stderr.read().decode('utf-8')
            ssh.close()
            
            return output if not error else None
        except ImportError:
            # Fallback to ssh command if paramiko not available
            cmd = f'ssh {OPENWRT_USER}@{OPENWRT_HOST} "{command}"'
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=10)
            return result.stdout if result.returncode == 0 else None
    except Exception as e:
        print(f"SSH Error: {e}")
        return None

def get_adblock_logs():
    """Fetch adblock logs from OpenWRT router"""
    output = execute_ssh_command("logread | grep adblock | tail -n 50")
    if not output:
        return []
    
    logs = []
    for line in output.strip().split('\n'):
        if line:
            # Parse log line format: "Wed Dec  3 04:03:17 2025 user.info adblock-4.4.2-r3[1732]: message"
            parts = line.split(': ', 1)
            if len(parts) == 2:
                timestamp = parts[0]
                message = parts[1]
                
                # Determine log level
                level = 'info'
                if 'failed' in message.lower() or 'error' in message.lower():
                    level = 'error'
                elif 'successfully' in message.lower() or 'loaded' in message.lower():
                    level = 'success'
                
                logs.append({
                    'timestamp': timestamp,
                    'message': message,
                    'level': level
                })
    
    return logs

def reload_adblock_async():
    """Reload adblock in background thread"""
    def reload():
        data = load_data()
        data['adblock_status'] = 'loading'
        save_data(data)
        
        # Execute reload command
        result = execute_ssh_command('/etc/init.d/adblock reload')
        
        # Wait a bit for logs to populate
        time.sleep(2)
        
        # Fetch updated logs
        logs = get_adblock_logs()
        
        data = load_data()
        data['adblock_logs'] = logs
        
        # Check if reload was successful by looking at logs
        if logs and any('successfully' in log['message'].lower() for log in logs[-5:]):
            data['adblock_status'] = 'idle'
        else:
            data['adblock_status'] = 'error'
        
        save_data(data)
    
    thread = threading.Thread(target=reload)
    thread.daemon = True
    thread.start()

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
        # Trigger adblock reload
        reload_adblock_async()
    return jsonify(data["blocked_apps"])

@app.route('/api/blocklist/custom', methods=['POST'])
def add_custom_block():
    domain = request.json.get('domain')
    data = load_data()
    new_id = int(max([i['id'] for i in data["custom_blocklist"]] or [0])) + 1
    data["custom_blocklist"].append({"id": new_id, "domain": domain, "active": True})
    save_data(data)
    # Trigger adblock reload
    reload_adblock_async()
    return jsonify(data["custom_blocklist"])

@app.route('/api/blocklist/custom/<int:block_id>', methods=['DELETE'])
def remove_custom_block(block_id):
    data = load_data()
    data["custom_blocklist"] = [d for d in data["custom_blocklist"] if d["id"] != block_id]
    save_data(data)
    # Trigger adblock reload
    reload_adblock_async()
    return jsonify(data["custom_blocklist"])

@app.route('/api/blocklist/custom/<int:block_id>/toggle', methods=['POST'])
def toggle_custom_block(block_id):
    data = load_data()
    for item in data["custom_blocklist"]:
        if item["id"] == block_id:
            item["active"] = not item["active"]
            break
    save_data(data)
    # Trigger adblock reload
    reload_adblock_async()
    return jsonify(data["custom_blocklist"])

# New Adblock Endpoints

@app.route('/api/adblock/status', methods=['GET'])
def get_adblock_status():
    """Get current adblock status (idle, loading, error)"""
    data = load_data()
    return jsonify({
        "status": data.get("adblock_status", "idle")
    })

@app.route('/api/adblock/logs', methods=['GET'])
def get_logs():
    """Get recent adblock logs from router"""
    # Try to fetch fresh logs from router
    logs = get_adblock_logs()
    
    # If we got logs, update data
    if logs:
        data = load_data()
        data["adblock_logs"] = logs
        save_data(data)
    else:
        # Return cached logs if SSH fails
        data = load_data()
        logs = data.get("adblock_logs", [])
    
    return jsonify(logs)

@app.route('/api/adblock/reload', methods=['POST'])
def trigger_reload():
    """Manually trigger adblock reload"""
    reload_adblock_async()
    return jsonify({"status": "reload triggered"})

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
