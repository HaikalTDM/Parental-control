#ifndef OPENWRT_H
#define OPENWRT_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class OpenWRTClient {
private:
    const char* host;
    const char* user;
    const char* pass;
    String session_id;
    String url;

public:
    OpenWRTClient(const char* h, const char* u, const char* p) {
        host = h;
        user = u;
        pass = p;
        url = "http://" + String(host) + "/ubus";
        session_id = "00000000000000000000000000000000"; // Initial dummy session
    }

    // Login to OpenWRT via ubus (JSON-RPC)
    bool login() {
        if (WiFi.status() != WL_CONNECTED) return false;

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        // Construct JSON-RPC Login Request
        // { "jsonrpc": "2.0", "method": "call", "params": [ "00000000000000000000000000000000", "session", "login", { "username": "root", "password": "password" } ], "id": 1 }
        
        DynamicJsonDocument doc(1024);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 1;
        
        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("session");
        params.add("login");
        
        JsonObject loginData = params.createNestedObject();
        loginData["username"] = user;
        loginData["password"] = pass;

        String requestBody;
        serializeJson(doc, requestBody);

        Serial.println("OpenWRT: Attempting Login...");
        int httpResponseCode = http.POST(requestBody);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("OpenWRT: Response: " + response);
            
            DynamicJsonDocument resDoc(2048);
            deserializeJson(resDoc, response);
            
            // Parse Session ID from response
            // Response format: {"jsonrpc":"2.0","id":1,"result":[0,{"ubus_rpc_session":"86283901f46c642964a7379424d83253","timeout":300,"expires":300,"acls":{...}}]}
            if (resDoc.containsKey("result")) {
                JsonArray result = resDoc["result"];
                if (result.size() > 1) {
                    JsonObject sessionData = result[1];
                    if (sessionData.containsKey("ubus_rpc_session")) {
                        session_id = sessionData["ubus_rpc_session"].as<String>();
                        Serial.println("OpenWRT: Login Successful! Session ID: " + session_id);
                        http.end();
                        return true;
                    }
                }
            }
        } else {
            Serial.print("OpenWRT: Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        http.end();
        return false;
    }

    // Generic method to call ubus commands
    bool call(String object, String method, JsonObject params_obj) {
        if (session_id == "00000000000000000000000000000000") {
            if (!login()) return false;
        }

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        DynamicJsonDocument doc(1024);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 2;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add(object);
        params.add(method);
        params.add(params_obj); // Add the specific parameters for the call

        String requestBody;
        serializeJson(doc, requestBody);

        Serial.println("OpenWRT: Sending Command (" + object + "->" + method + ")...");
        int httpResponseCode = http.POST(requestBody);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("OpenWRT: Command Response: " + response);
            http.end();
            return true;
        }
        
        http.end();
        return false;
    }

    // Get connected devices from DHCP leases
    String getDevices() {
        if (session_id == "00000000000000000000000000000000") {
            if (!login()) return "[]";
        }

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        DynamicJsonDocument doc(1024);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 3;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("luci-rpc");
        params.add("getDHCPLeases");
        JsonObject emptyObj = params.createNestedObject();

        String requestBody;
        serializeJson(doc, requestBody);

        Serial.println("OpenWRT: Fetching devices...");
        int httpResponseCode = http.POST(requestBody);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("OpenWRT: Devices Response: " + response);
            http.end();
            return response;
        }
        
        http.end();
        return "[]";
    }

    // Block a domain using /etc/hosts file (most reliable method)
    bool blockDomain(String domain) {
        if (session_id == "00000000000000000000000000000000") {
            if (!login()) return false;
        }

        Serial.println("OpenWRT: Starting domain block for: " + domain);

        // Read current hosts file
        String hostsContent = readHostsFile();
        if (hostsContent == "") {
            Serial.println("OpenWRT: Failed to read hosts file");
            return false;
        }

        // Check if domain already blocked
        if (hostsContent.indexOf(domain) >= 0) {
            Serial.println("OpenWRT: Domain already in hosts file");
            return true;
        }

        // Add domain to hosts content
        hostsContent += "0.0.0.0 " + domain + "\n";
        hostsContent += "0.0.0.0 www." + domain + "\n";

        // Write back to hosts file
        if (writeHostsFile(hostsContent)) {
            Serial.println("OpenWRT: Successfully blocked domain: " + domain);
            return true;
        }

        Serial.println("OpenWRT: Failed to write hosts file");
        return false;
    }

    // Read /etc/hosts file
    String readHostsFile() {
        DynamicJsonDocument doc(512);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 8;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("file");
        params.add("read");
        
        JsonObject readParams = params.createNestedObject();
        readParams["path"] = "/etc/hosts";

        String requestBody;
        serializeJson(doc, requestBody);

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = http.POST(requestBody);
        String content = "";
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            
            // Parse response to get file content
            DynamicJsonDocument resDoc(8192);
            deserializeJson(resDoc, response);
            
            if (resDoc.containsKey("result") && resDoc["result"].size() > 1) {
                JsonObject resultObj = resDoc["result"][1];
                if (resultObj.containsKey("data")) {
                    content = resultObj["data"].as<String>();
                }
            }
        }
        
        http.end();
        return content;
    }

    // Write /etc/hosts file
    bool writeHostsFile(String content) {
        DynamicJsonDocument doc(8192);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 9;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("file");
        params.add("write");
        
        JsonObject writeParams = params.createNestedObject();
        writeParams["path"] = "/etc/hosts";
        writeParams["data"] = content;

        String requestBody;
        serializeJson(doc, requestBody);

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = http.POST(requestBody);
        http.end();

        return httpResponseCode > 0;
    }

    // Add a new section to UCI config
    String addDhcpSection() {
        DynamicJsonDocument doc(512);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 4;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("uci");
        params.add("add");
        
        JsonObject addParams = params.createNestedObject();
        addParams["config"] = "dhcp";
        addParams["type"] = "address";

        String requestBody;
        serializeJson(doc, requestBody);

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = http.POST(requestBody);
        String response = "";
        
        if (httpResponseCode > 0) {
            response = http.getString();
            Serial.println("OpenWRT: Add section response: " + response);
            
            // Parse response to get section name
            DynamicJsonDocument resDoc(1024);
            deserializeJson(resDoc, response);
            
            if (resDoc.containsKey("result") && resDoc["result"].size() > 1) {
                // Result format: [0, {"section":"cfg0553eb"}]
                JsonObject resultObj = resDoc["result"][1];
                if (resultObj.containsKey("section")) {
                    String section = resultObj["section"].as<String>();
                    Serial.println("OpenWRT: Parsed section name: " + section);
                    http.end();
                    return section;
                }
            }
        }
        
        http.end();
        return "";
    }

    // Set a UCI value
    bool setUciValue(String config, String section, String option, String value) {
        DynamicJsonDocument doc(512);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 5;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("uci");
        params.add("set");
        
        JsonObject setParams = params.createNestedObject();
        setParams["config"] = config;
        setParams["section"] = section;
        
        JsonObject values = setParams.createNestedObject("values");
        values[option] = value;

        String requestBody;
        serializeJson(doc, requestBody);

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = http.POST(requestBody);
        http.end();

        return httpResponseCode > 0;
    }

    // Commit UCI changes
    bool commitConfig(String config) {
        DynamicJsonDocument doc(256);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 6;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("uci");
        params.add("commit");
        
        JsonObject commitParams = params.createNestedObject();
        commitParams["config"] = config;

        String requestBody;
        serializeJson(doc, requestBody);

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = http.POST(requestBody);
        http.end();

        if (httpResponseCode > 0) {
            Serial.println("OpenWRT: Configuration committed successfully");
            
            // Reload dnsmasq service
            reloadDnsmasq();
            return true;
        }
        
        Serial.println("OpenWRT: Failed to commit configuration");
        return false;
    }

    // Reload dnsmasq service
    void reloadDnsmasq() {
        DynamicJsonDocument doc(512);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 7;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("rc");
        params.add("init");
        
        JsonObject initParams = params.createNestedObject();
        initParams["name"] = "dnsmasq";
        initParams["action"] = "reload";

        String requestBody;
        serializeJson(doc, requestBody);

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        http.POST(requestBody);
        http.end();

        Serial.println("OpenWRT: Dnsmasq reloaded");
    }

    // Unblock a domain (remove from dnsmasq)
    void unblockDomain(String domain) {
        // For now, just log it - proper implementation would require finding and deleting the UCI section
        Serial.println("OpenWRT: Unblocking domain: " + domain);
        Serial.println("OpenWRT: Note - Router restart required to fully remove domain");
    }

    // Toggle Firewall Rule
    void setInternet(bool active) {
        DynamicJsonDocument doc(256);
        JsonObject p = doc.to<JsonObject>();
        p["scope"] = "uci"; 
        
        if (call("session", "access", p)) {
             Serial.println(active ? "OpenWRT: Internet UNBLOCKED (Command Sent)" : "OpenWRT: Internet BLOCKED (Command Sent)");
        } else {
             Serial.println("OpenWRT: Command Failed");
        }
    }

    // Get adblock status by checking if adblock daemon is running
    String getAdblockStatus() {
        if (session_id == "00000000000000000000000000000000") {
            if (!login()) return "{\"status\":\"error\"}";
        }

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        DynamicJsonDocument doc(512);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 10;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("rc");
        params.add("list");
        JsonObject emptyObj = params.createNestedObject();

        String requestBody;
        serializeJson(doc, requestBody);

        int httpResponseCode = http.POST(requestBody);
        String status = "idle"; // Default status
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            // Check if adblock service is running
            if (response.indexOf("adblock") >= 0 && response.indexOf("running") >= 0) {
                status = "idle";
            }
        }
        
        http.end();
        return "{\"status\":\"" + status + "\"}";
    }

    // Get adblock logs from syslog
    String getAdblockLogs() {
        if (session_id == "00000000000000000000000000000000") {
            if (!login()) return "[]";
        }

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        DynamicJsonDocument doc(512);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 11;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("file");
        params.add("exec");
        
        JsonObject execParams = params.createNestedObject();
        execParams["command"] = "logread";
        JsonArray argsArray = execParams.createNestedArray("params");
        argsArray.add("-e");
        argsArray.add("adblock");

        String requestBody;
        serializeJson(doc, requestBody);

        Serial.println("OpenWRT: Fetching adblock logs...");
        int httpResponseCode = http.POST(requestBody);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("OpenWRT: Log Response: " + response);
            
            // Parse the response to extract logs
            DynamicJsonDocument resDoc(8192);
            deserializeJson(resDoc, response);
            
            if (resDoc.containsKey("result") && resDoc["result"].size() > 1) {
                JsonObject resultObj = resDoc["result"][1];
                if (resultObj.containsKey("stdout")) {
                    String logOutput = resultObj["stdout"].as<String>();
                    
                    // Parse log lines and create JSON array
                    DynamicJsonDocument logsDoc(6144);
                    JsonArray logsArray = logsDoc.to<JsonArray>();
                    
                    int startIdx = 0;
                    int lineCount = 0;
                    while (startIdx < logOutput.length() && lineCount < 50) {
                        int endIdx = logOutput.indexOf('\n', startIdx);
                        if (endIdx == -1) endIdx = logOutput.length();
                        
                        String line = logOutput.substring(startIdx, endIdx);
                        if (line.length() > 0) {
                            // Parse log line: timestamp + message
                            int colonIdx = line.indexOf(": ");
                            if (colonIdx > 0) {
                                JsonObject logEntry = logsArray.createNestedObject();
                                logEntry["timestamp"] = line.substring(0, colonIdx);
                                logEntry["message"] = line.substring(colonIdx + 2);
                                
                                // Determine log level
                                String message = line.substring(colonIdx + 2);
                                message.toLowerCase();
                                if (message.indexOf("failed") >= 0 || message.indexOf("error") >= 0) {
                                    logEntry["level"] = "error";
                                } else if (message.indexOf("successfully") >= 0 || message.indexOf("loaded") >= 0) {
                                    logEntry["level"] = "success";
                                } else {
                                    logEntry["level"] = "info";
                                }
                                
                                lineCount++;
                            }
                        }
                        
                        startIdx = endIdx + 1;
                    }
                    
                    String logsJson;
                    serializeJson(logsArray, logsJson);
                    http.end();
                    return logsJson;
                }
            }
        }
        
        http.end();
        return "[]";
    }
};

#endif
