#include "OpenWrtClient.h"

OpenWrtClient::OpenWrtClient(const char* host, const char* username, const char* password) {
    _host = host;
    _username = username;
    _password = password;
    _sid = "00000000000000000000000000000000";
    _lastLoginTime = 0;
}

bool OpenWrtClient::login() {
    HTTPClient http;
    String url = String("http://") + _host + "/ubus";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    JsonDocument doc;
    doc["jsonrpc"] = "2.0";
    doc["id"] = 1;
    doc["method"] = "call";
    doc["params"][0] = "00000000000000000000000000000000";
    doc["params"][1] = "session";
    doc["params"][2] = "login";
    doc["params"][3]["username"] = _username;
    doc["params"][3]["password"] = _password;
    
    String requestBody;
    serializeJson(doc, requestBody);
    
    int httpResponseCode = http.POST(requestBody);
    
    if (httpResponseCode == 200) {
        String response = http.getString();
        Serial.println("Login Response: " + response); // DEBUG
        JsonDocument responseDoc;
        deserializeJson(responseDoc, response);
        
        if (!responseDoc["result"].isNull() && !responseDoc["result"][1]["ubus_rpc_session"].isNull()) {
            _sid = responseDoc["result"][1]["ubus_rpc_session"].as<String>();
            _lastLoginTime = millis();
            Serial.println("Login Success. SID: " + _sid);
            http.end();
            return true;
        }
    } else {
        Serial.print("Login HTTP Error: ");
        Serial.println(httpResponseCode);
    }
    
    http.end();
    return false;
}

bool OpenWrtClient::checkSession() {
    if (_sid == "00000000000000000000000000000000" || millis() - _lastLoginTime > 250000) { // Refresh if dummy SID or timeout
        return login();
    }
    return true;
}

String OpenWrtClient::sendRequest(const char* object, const char* method, JsonDocument& params) {
    if (!checkSession()) return "";

    HTTPClient http;
    String url = String("http://") + _host + "/ubus";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    JsonDocument doc;
    doc["jsonrpc"] = "2.0";
    doc["id"] = 1;
    doc["method"] = "call";
    doc["params"][0] = _sid;
    doc["params"][1] = object;
    doc["params"][2] = method;
    doc["params"][3] = params;
    
    String requestBody;
    serializeJson(doc, requestBody);
    Serial.println("Request Body: " + requestBody); // DEBUG
    
    int httpResponseCode = http.POST(requestBody);
    String result = "";
    
    if (httpResponseCode == 200) {
        result = http.getString();
    } else {
        Serial.print("HTTP Error: ");
        Serial.println(httpResponseCode);
        Serial.println(http.getString());
    }
    
    http.end();
    return result;
}

int OpenWrtClient::getConnectedDeviceCount() {
    JsonDocument params; 
    params.to<JsonObject>(); 
    String response = sendRequest("luci-rpc", "getDHCPLeases", params);
    
    if (response == "") return 0;
    
    JsonDocument doc;
    deserializeJson(doc, response);
    
    if (!doc["result"].isNull() && !doc["result"][1]["dhcp_leases"].isNull()) {
        return doc["result"][1]["dhcp_leases"].size();
    }
    
    return 0;
}

bool OpenWrtClient::getConnectedDevices(JsonArray& targetArray) {
    JsonDocument params; 
    params.to<JsonObject>(); 
    String response = sendRequest("luci-rpc", "getDHCPLeases", params);
    
    Serial.println("DHCP Response: " + response); // DEBUG
    
    if (response == "") return false;
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return false;
    }
    
    if (!doc["result"].isNull() && !doc["result"][1]["dhcp_leases"].isNull()) {
         JsonArray leases = doc["result"][1]["dhcp_leases"];
         for(JsonVariant v : leases) {
             targetArray.add(v);
         }
         return true;
    }
    
    return false;
}

bool OpenWrtClient::getTrafficStats(unsigned long long& rx, unsigned long long& tx) {
    JsonDocument params;
    params.to<JsonObject>(); 
    String response = sendRequest("luci-rpc", "getNetworkDevices", params);
    
    Serial.println("Network Response: " + response); // DEBUG
    
    if (response != "") {
        JsonDocument doc;
        deserializeJson(doc, response);
        
        if (!doc["result"].isNull() && !doc["result"][1].isNull()) {
            JsonVariant devices = doc["result"][1];
            if (devices["br-lan"].is<JsonObject>()) {
                 JsonVariant brLan = devices["br-lan"];
                 
                 if (!brLan["stats"].isNull()) {
                     rx = brLan["stats"]["rx_bytes"];
                     tx = brLan["stats"]["tx_bytes"];
                     return true;
                 } else if (!brLan["rx_bytes"].isNull()) {
                     rx = brLan["rx_bytes"];
                     tx = brLan["tx_bytes"];
                     return true;
                 }
            }
        }
    }
    return false;
}

void OpenWrtClient::getDataUsage(String& total, String& download, String& upload) {
    unsigned long long rx = 0;
    unsigned long long tx = 0;
    
    if (getTrafficStats(rx, tx)) {
        download = formatBytes(tx); // TX from router is Download for client
        upload = formatBytes(rx);   // RX to router is Upload from client
        total = formatBytes(rx + tx);
    } else {
        total = "0 B";
        download = "0 B";
        upload = "0 B";
    }
}

String OpenWrtClient::formatBytes(unsigned long long bytes) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double dblBytes = bytes;
    
    if (bytes > 1024) {
        for (i = 0; (bytes / 1024) > 0 && i < 4; i++, bytes /= 1024)
            dblBytes = bytes / 1024.0;
    }
    
    char buf[20];
    sprintf(buf, "%.2f %s", dblBytes, suffixes[i]);
    return String(buf);
}

bool OpenWrtClient::blockDomain(const char* domain) {
    // According to the doc, we need to write to /etc/adblock/adblock.blocklist
    // and then trigger adblock reload
    
    // 1. Read current blocklist
    JsonDocument params;
    params["path"] = "/etc/adblock/adblock.blocklist";
    String response = sendRequest("file", "read", params);
    
    // 2. Append the domain
    params.clear();
    params["path"] = "/etc/adblock/adblock.blocklist";
    
    // Create the data to append (domain + newline)
    String blocklistContent = "";
    if (response != "") {
        // Parse existing content if any
        JsonDocument doc;
        deserializeJson(doc, response);
        if (!doc["result"].isNull() && !doc["result"][1]["data"].isNull()) {
            blocklistContent = doc["result"][1]["data"].as<String>();
        }
    }
    
    // Append new domain
    blocklistContent += String(domain) + "\n";
    params["data"] = blocklistContent;
    
    response = sendRequest("file", "write", params);
    if (response == "") return false;

    // 3. Trigger adblock reload using file exec
    params.clear();
    params["command"] = "/etc/init.d/adblock";
    params["params"][0] = "reload";
    
    response = sendRequest("file", "exec", params);
    
    return true;
}

bool OpenWrtClient::unblockDomain(const char* domain) {
    // 1. Read current blocklist
    JsonDocument params;
    params["path"] = "/etc/adblock/adblock.blocklist";
    String response = sendRequest("file", "read", params);
    
    if (response == "") return false;
    
    // 2. Parse and remove the domain
    JsonDocument doc;
    deserializeJson(doc, response);
    
    String blocklistContent = "";
    if (!doc["result"].isNull() && !doc["result"][1]["data"].isNull()) {
        String currentContent = doc["result"][1]["data"].as<String>();
        
        // Split by newlines and rebuild without the target domain
        int startPos = 0;
        int newlinePos = 0;
        while ((newlinePos = currentContent.indexOf('\n', startPos)) != -1) {
            String line = currentContent.substring(startPos, newlinePos);
            line.trim();
            if (line != String(domain) && line.length() > 0) {
                blocklistContent += line + "\n";
            }
            startPos = newlinePos + 1;
        }
    }
    
    // 3. Write back the modified blocklist
    params.clear();
    params["path"] = "/etc/adblock/adblock.blocklist";
    params["data"] = blocklistContent;
    
    response = sendRequest("file", "write", params);
    if (response == "") return false;

    // 4. Trigger adblock reload
    params.clear();
    params["command"] = "/etc/init.d/adblock";
    params["params"][0] = "reload";
    
    response = sendRequest("file", "exec", params);
    
    return true;
}

bool OpenWrtClient::applyBlocklistChanges(JsonArray& changes) {
    // 1. Read current blocklist
    JsonDocument params;
    params["path"] = "/etc/adblock/adblock.blocklist";
    String response = sendRequest("file", "read", params);
    
    String blocklistContent = "";
    if (response != "") {
        JsonDocument doc;
        deserializeJson(doc, response);
        if (!doc["result"].isNull() && !doc["result"][1]["data"].isNull()) {
            blocklistContent = doc["result"][1]["data"].as<String>();
        }
    }
    
    // 2. Apply all changes
    for (JsonVariant change : changes) {
        String action = change["action"].as<String>();
        String domain = change["domain"].as<String>();
        
        if (action == "add" || action == "enable") {
            // Add domain if not already present
            if (blocklistContent.indexOf(domain) == -1) {
                blocklistContent += domain + "\n";
            }
        } else if (action == "remove" || action == "disable") {
            // Remove domain
            String newContent = "";
            int startPos = 0;
            int newlinePos = 0;
            while ((newlinePos = blocklistContent.indexOf('\n', startPos)) != -1) {
                String line = blocklistContent.substring(startPos, newlinePos);
                line.trim();
                if (line != domain && line.length() > 0) {
                    newContent += line + "\n";
                }
                startPos = newlinePos + 1;
            }
            blocklistContent = newContent;
        }
    }
    
    
    // 3. Write updated blocklist back to /etc/adblock/adblock.blocklist
    params.clear();
    params["path"] = "/etc/adblock/adblock.blocklist";
    params["data"] = blocklistContent;
    
    Serial.println("Saving blocklist to /etc/adblock/adblock.blocklist");
    response = sendRequest("file", "write", params);
    if (response == "") {
        Serial.println("ERROR: Failed to save blocklist");
        return false;
    }
    Serial.println("Blocklist saved successfully");
    
    // 4. Build dnsmasq configuration file content
    // Format: address=/domain/0.0.0.0 (IPv4) and address=/domain/:: (IPv6)
    String dnsmasqConfig = "";
    int startPos = 0;
    int newlinePos = 0;
    while ((newlinePos = blocklistContent.indexOf('\n', startPos)) != -1) {
        String line = blocklistContent.substring(startPos, newlinePos);
        line.trim();
        if (line.length() > 0) {
            dnsmasqConfig += "address=/" + line + "/0.0.0.0\n";
            dnsmasqConfig += "address=/" + line + "/::\n";
        }
        startPos = newlinePos + 1;
    }
    
    // 5. Write to /etc/dnsmasq.d/custom_blocklist.conf
    params.clear();
    params["path"] = "/etc/dnsmasq.d/custom_blocklist.conf";
    params["data"] = dnsmasqConfig;
    
    Serial.println("Writing to /etc/dnsmasq.d/custom_blocklist.conf");
    Serial.println("Content: " + dnsmasqConfig);
    
    response = sendRequest("file", "write", params);
    if (response == "") {
        Serial.println("ERROR: Failed to write dnsmasq config");
        return false;
    }
    Serial.println("Write Response: " + response);
    
    // 6. Restart dnsmasq using rc.init
    params.clear();
    params["name"] = "dnsmasq";
    params["action"] = "restart";
    
    Serial.println("Restarting dnsmasq via rc.init...");
    response = sendRequest("rc", "init", params);
    Serial.println("Dnsmasq Restart Response: " + response);
    
    return true;
}

String OpenWrtClient::getBlocklist() {
    // Read the blocklist file from the router
    JsonDocument params;
    params["path"] = "/etc/adblock/adblock.blocklist";
    
    String response = sendRequest("file", "read", params);
    
    // Parse the response to extract the file content
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        Serial.println("Failed to parse blocklist response");
        return "";
    }
    
    // Extract the data field from the response
    if (doc["result"][1]["data"].is<const char*>()) {
        return String(doc["result"][1]["data"].as<const char*>());
    }
    
    return "";
}


bool OpenWrtClient::allowDomain(const char* domain) {
    // Similar to block, but maybe different list?
    // Doc says "allowlist and blacklist". Usually adblock has a whitelist option.
    // Assuming 'whitelist_domains' option exists in adblock config.
    
    JsonDocument params;
    params["config"] = "adblock";
    params["section"] = "global";
    params["option"] = "whitelist_domains"; // Standard adblock option
    params["values"][0] = domain;
    
    String response = sendRequest("uci", "add_list", params);
    if (response == "") return false;

    params.clear();
    params["config"] = "adblock";
    sendRequest("uci", "commit", params);

    params.clear();
    params["rollback"] = true;
    sendRequest("uci", "apply", params);
    
    return true;
}

// Implement unblock/unallow similarly using 'del_list' if needed, 
// but for now the UI only requested 'block' and 'allow'. 
// The doc mentions "active security management via domain allowlisting and blacklisting".
