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

    // Block a domain using dnsmasq
    bool blockDomain(String domain) {
        if (session_id == "00000000000000000000000000000000") {
            if (!login()) return false;
        }

        // Add dnsmasq address rule to return 0.0.0.0 for blocked domain
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
        addParams["type"] = "domain";
        
        JsonObject values = addParams.createNestedObject("values");
        values["name"] = domain;
        values["ip"] = "0.0.0.0";

        String requestBody;
        serializeJson(doc, requestBody);

        Serial.println("OpenWRT: Blocking domain: " + domain);
        
        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = http.POST(requestBody);
        http.end();

        if (httpResponseCode > 0) {
            // Commit and reload dnsmasq
            commitAndReload();
            Serial.println("OpenWRT: Domain blocked: " + domain);
            return true;
        }
        
        return false;
    }

    // Commit UCI changes and reload dnsmasq
    void commitAndReload() {
        // Commit changes
        DynamicJsonDocument doc(256);
        doc["jsonrpc"] = "2.0";
        doc["method"] = "call";
        doc["id"] = 5;

        JsonArray params = doc.createNestedArray("params");
        params.add(session_id);
        params.add("uci");
        params.add("commit");
        
        JsonObject commitParams = params.createNestedObject();
        commitParams["config"] = "dhcp";

        String requestBody;
        serializeJson(doc, requestBody);

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        http.POST(requestBody);
        http.end();

        Serial.println("OpenWRT: Configuration committed, reloading dnsmasq...");
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
};

#endif
