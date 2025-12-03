#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Preferences.h>

#include "openwrt.h"

// Configuration - Update these with your actual credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* routerHost = "192.168.10.100"; // OpenWRT IP
const char* routerUser = "root";           // OpenWRT User
const char* routerPass = "password";       // OpenWRT Password

// Set to true for testing without a router, false for production
#define SIMULATION_MODE false 

AsyncWebServer server(80);
Preferences preferences;
OpenWRTClient router(routerHost, routerUser, routerPass);

// Blocklist storage
const int MAX_BLOCKED_DOMAINS = 50;
String blockedDomains[MAX_BLOCKED_DOMAINS];
int blockedDomainsCount = 0;

// Load blocklist from Preferences
void loadBlocklist() {
  preferences.begin("blocklist", true); // Read-only
  blockedDomainsCount = preferences.getInt("count", 0);
  
  for (int i = 0; i < blockedDomainsCount && i < MAX_BLOCKED_DOMAINS; i++) {
    String key = "domain" + String(i);
    blockedDomains[i] = preferences.getString(key.c_str(), "");
  }
  
  preferences.end();
  Serial.print("Loaded ");
  Serial.print(blockedDomainsCount);
  Serial.println(" blocked domains");
}

// Save blocklist to Preferences
void saveBlocklist() {
  preferences.begin("blocklist", false); // Read-write
  preferences.putInt("count", blockedDomainsCount);
  
  for (int i = 0; i < blockedDomainsCount; i++) {
    String key = "domain" + String(i);
    preferences.putString(key.c_str(), blockedDomains[i]);
  }
  
  preferences.end();
  Serial.print("Saved ");
  Serial.print(blockedDomainsCount);
  Serial.println(" blocked domains");
}

// Add domain to blocklist
bool addBlockedDomain(String domain) {
  // Check if already exists
  for (int i = 0; i < blockedDomainsCount; i++) {
    if (blockedDomains[i] == domain) {
      Serial.println("Domain already blocked: " + domain);
      return false;
    }
  }
  
  // Add to array
  if (blockedDomainsCount < MAX_BLOCKED_DOMAINS) {
    blockedDomains[blockedDomainsCount] = domain;
    blockedDomainsCount++;
    saveBlocklist();
    
    // Add to OpenWRT
    if (router.blockDomain(domain)) {
      Serial.println("Domain added and blocked: " + domain);
      return true;
    } else {
      // Rollback if OpenWRT fails
      blockedDomainsCount--;
      saveBlocklist();
      Serial.println("Failed to block domain on router: " + domain);
      return false;
    }
  }
  
  Serial.println("Blocklist full!");
  return false;
}

// Remove domain from blocklist
bool removeBlockedDomain(String domain) {
  for (int i = 0; i < blockedDomainsCount; i++) {
    if (blockedDomains[i] == domain) {
      // Shift array left
      for (int j = i; j < blockedDomainsCount - 1; j++) {
        blockedDomains[j] = blockedDomains[j + 1];
      }
      blockedDomainsCount--;
      saveBlocklist();
      
      // Remove from OpenWRT
      router.unblockDomain(domain);
      
      Serial.println("Domain removed: " + domain);
      return true;
    }
  }
  
  Serial.println("Domain not found: " + domain);
  return false;
}

// Get blocklist as JSON array
String getBlocklistJSON() {
  DynamicJsonDocument doc(2048);
  JsonArray customArray = doc.createNestedArray("custom");
  
  for (int i = 0; i < blockedDomainsCount; i++) {
    JsonObject domainObj = customArray.createNestedObject();
    domainObj["id"] = i + 1;
    domainObj["domain"] = blockedDomains[i];
    domainObj["active"] = true;
  }
  
  String response;
  serializeJson(doc, response);
  return response;
}

struct Device {
  int id;
  String name;
  String type;
  String status;
  bool blocked;
  String usage;
};

bool internetActive = true;

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());
}

void setupRoutes() {
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(1024);
    doc["internet_active"] = internetActive;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.on("/api/toggle-internet", HTTP_POST, [](AsyncWebServerRequest *request){
    internetActive = !internetActive;
    
    if (!SIMULATION_MODE) {
        // Attempt to login and send command to OpenWRT
        if (router.login()) {
            router.setInternet(internetActive);
        }
    }
    
    DynamicJsonDocument doc(1024);
    doc["internet_active"] = internetActive;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.on("/api/devices", HTTP_GET, [](AsyncWebServerRequest *request){
    String rawResponse = router.getDevices();
    Serial.println("Raw device response: " + rawResponse);
    
    // Create response document
    DynamicJsonDocument responseDoc(4096);
    JsonArray devices = responseDoc.to<JsonArray>();
    
    // Parse the JSON-RPC response
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, rawResponse);
    
    if (!error && doc.containsKey("result")) {
      Serial.println("Parsing OpenWRT result...");
      JsonArray result = doc["result"];
      
      if (result.size() > 1 && result[1].containsKey("dhcp_leases")) {
        JsonArray leases = result[1]["dhcp_leases"];
        Serial.print("Found ");
        Serial.print(leases.size());
        Serial.println(" devices");
        
        int id = 1;
        for (JsonObject lease : leases) {
          JsonObject device = devices.createNestedObject();
          device["id"] = id++;
          device["name"] = lease["hostname"] | "Unknown";
          device["type"] = "device";
          device["status"] = "online";
          device["blocked"] = false;
          device["usage"] = "0 GB";
        }
      } else {
        Serial.println("No dhcp_leases found in result");
      }
    } else {
      Serial.println("Failed to parse JSON or no result");
    }
    
    String response;
    serializeJson(devices, response);
    Serial.println("Sending devices: " + response);
    
    request->send(200, "application/json", response);
  });

  server.on("/api/blocklist", HTTP_GET, [](AsyncWebServerRequest *request){
      String response = getBlocklistJSON();
      request->send(200, "application/json", response);
  });

  server.on("/api/blocklist/app", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      request->send(200, "application/json", "{}");
  });

  server.on("/api/blocklist/custom", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (!error && doc.containsKey("domain")) {
        String domain = doc["domain"].as<String>();
        Serial.println("Adding domain to blocklist: " + domain);
        
        if (addBlockedDomain(domain)) {
          // Return full blocklist
          String response = getBlocklistJSON();
          request->send(200, "application/json", response);
        } else {
          request->send(500, "application/json", "{\"error\":\"Failed to block domain\"}");
        }
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid request\"}");
      }
  });

  server.on("/api/allowlist", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "application/json", "[]");
  });

  server.on("/api/allowlist", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      request->send(200, "application/json", "[]");
  });

  // New Adblock endpoints
  server.on("/api/adblock/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = router.getAdblockStatus();
    request->send(200, "application/json", response);
  });

  server.on("/api/adblock/logs", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = router.getAdblockLogs();
    request->send(200, "application/json", response);
  });

  server.on("/api/adblock/reload", HTTP_POST, [](AsyncWebServerRequest *request){
    // Trigger adblock reload by reloading dnsmasq
    router.reloadDnsmasq();
    request->send(200, "application/json", "{\"status\":\"reload triggered\"}");
  });


  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
  
  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else if (request->url().startsWith("/api/")) {
      request->send(404, "application/json", "{\"error\":\"Not found\"}");
    } else {
      request->send(LittleFS, "/index.html");
    }
  });
}

void setup() {
  Serial.begin(115200);
  
  if(!LittleFS.begin(true)){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  setupWiFi();
  
  // Load blocked domains from Preferences
  loadBlocklist();
  
  setupRoutes();
  
  server.begin();
}

void loop() {
}
