#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Preferences.h>

#include "openwrt.h"
#include "app_domains.h"

// Configuration - Update these with your actual credentials
const char* ssid = "OpenWrt";
const char* password = "";
const char* routerHost = "192.168.10.100"; // OpenWRT IP
const char* routerUser = "root";           // OpenWRT User
const char* routerPass = "";       // OpenWRT Password

// Set to true for testing without a router, false for production
#define SIMULATION_MODE false 

AsyncWebServer server(80);
Preferences preferences;
OpenWRTClient router(routerHost, routerUser, routerPass);

// Blocklist storage
const int MAX_BLOCKED_DOMAINS = 50;
String blockedDomains[MAX_BLOCKED_DOMAINS];
int blockedDomainsCount = 0;

// Blocked apps storage (youtube, tiktok, facebook, roblox, instagram)
bool blockedApps[5] = {false, false, false, false, false};
const char* appIds[5] = {"youtube", "tiktok", "facebook", "roblox", "instagram"};

// Load blocked apps from Preferences
void loadBlockedApps() {
  preferences.begin("blockedapps", true);
  for (int i = 0; i < 5; i++) {
    blockedApps[i] = preferences.getBool(appIds[i], false);
  }
  preferences.end();
  Serial.println("Loaded blocked apps");
}

// Save blocked apps to Preferences
void saveBlockedApps() {
  preferences.begin("blockedapps", false);
  for (int i = 0; i < 5; i++) {
    preferences.putBool(appIds[i], blockedApps[i]);
  }
  preferences.end();
  Serial.println("Saved blocked apps");
}

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
  // Check if already exists locally
  for (int i = 0; i < blockedDomainsCount; i++) {
    if (blockedDomains[i] == domain) {
      Serial.println("Domain already blocked locally: " + domain);
      return false;
    }
  }
  
  // Add to array
  if (blockedDomainsCount < MAX_BLOCKED_DOMAINS) {
    // Try to add to OpenWRT FIRST
    if (router.blockDomain(domain)) {
      blockedDomains[blockedDomainsCount] = domain;
      blockedDomainsCount++;
      saveBlocklist();
      Serial.println("Domain added and blocked: " + domain);
      return true;
    } else {
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
      JsonArray result = doc["result"];
      JsonArray leases;
      
      // Handle different OpenWRT JSON-RPC response formats
      // Format 1: [0, {"dhcp_leases": [...]}]
      if (result.size() > 1 && result[1].is<JsonObject>() && result[1].containsKey("dhcp_leases")) {
          leases = result[1]["dhcp_leases"];
      } 
      // Format 2: [{"dhcp_leases": [...]}]
      else if (result.size() > 0 && result[0].is<JsonObject>() && result[0].containsKey("dhcp_leases")) {
          leases = result[0]["dhcp_leases"];
      }
      
      if (!leases.isNull()) {
        Serial.print("Found ");
        Serial.print(leases.size());
        Serial.println(" devices");
        
        int id = 1;
        for (JsonObject lease : leases) {
          JsonObject device = devices.createNestedObject();
          device["id"] = id++;
          // Use hostname if available, otherwise MAC address
          if (lease.containsKey("hostname") && lease["hostname"].as<String>().length() > 0) {
              device["name"] = lease["hostname"];
          } else {
              device["name"] = lease["macaddr"];
          }
          device["type"] = "device";
          device["status"] = "online"; // Assume online if in lease table (simplified)
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
      DynamicJsonDocument doc(4096);
      
      // Add blocked apps object
      JsonObject appsObj = doc.createNestedObject("apps");
      for (int i = 0; i < 5; i++) {
        appsObj[appIds[i]] = blockedApps[i];
      }
      
      // Add custom blocklist array
      JsonArray customArray = doc.createNestedArray("custom");
      
      // Fetch real blocklist from router to check active status
      String blacklistContent = router.readFile("/etc/hosts.user");
      
      // Migration: If local list is empty but router has domains, import them
      // (Skipping complex migration from hosts file for now to avoid parsing errors)

      for (int i = 0; i < blockedDomainsCount; i++) {
        JsonObject domainObj = customArray.createNestedObject();
        domainObj["id"] = i + 1; // 1-based ID
        domainObj["domain"] = blockedDomains[i];
        // Active if found in router file
        String blockEntry = "0.0.0.0 " + blockedDomains[i];
        domainObj["active"] = (blacklistContent.indexOf(blockEntry) >= 0);
      }
      
      String response;
      serializeJson(doc, response);
      request->send(200, "application/json", response);
  });

  server.on("/api/blocklist/app", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (!error && doc.containsKey("id")) {
        String appId = doc["id"].as<String>();
        Serial.println("Toggling app block: " + appId);
        
        // Find app index
        int appIndex = -1;
        for (int i = 0; i < 5; i++) {
          if (String(appIds[i]) == appId) {
            appIndex = i;
            break;
          }
        }
        
        if (appIndex >= 0) {
          // Toggle the app
          blockedApps[appIndex] = !blockedApps[appIndex];
          saveBlockedApps();
          
          // Get domains for this app
          const char** domains = getAppDomains(appId.c_str());
          if (domains) {
            // Block or unblock all domains for this app
            for (int i = 0; domains[i] != nullptr; i++) {
              if (blockedApps[appIndex]) {
                // Block the domain
                router.blockDomain(String(domains[i]));
              } else {
                // Unblock the domain
                router.unblockDomain(String(domains[i]));
              }
            }
          }
          
          // Return updated blockedApps state
          DynamicJsonDocument responseDoc(256);
          for (int i = 0; i < 5; i++) {
            responseDoc[appIds[i]] = blockedApps[i];
          }
          String response;
          serializeJson(responseDoc, response);
          request->send(200, "application/json", response);
        } else {
          request->send(400, "application/json", "{\"error\":\"Invalid app ID\"}");
        }
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid request\"}");
      }
  });

  server.on("/api/blocklist/custom/toggle", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (!error && doc.containsKey("id")) {
        int id = doc["id"].as<int>();
        
        if (id > 0 && id <= blockedDomainsCount) {
            String domain = blockedDomains[id - 1];
            // Read file ONCE before modification to avoid race condition with restartDnsmasq
            String blacklistContent = router.readFile("/etc/hosts.user");
            String blockEntry = "0.0.0.0 " + domain;
            bool wasActive = (blacklistContent.indexOf(blockEntry) >= 0);
            
            bool success = false;
            bool nowActive = false;
            
            if (wasActive) {
                success = router.unblockDomain(domain);
                nowActive = false;
            } else {
                success = router.blockDomain(domain);
                nowActive = true;
            }
            
            if (success) {
                // Return updated list
                DynamicJsonDocument responseDoc(4096);
                JsonArray customArray = responseDoc.to<JsonArray>();
                
                for (int i = 0; i < blockedDomainsCount; i++) {
                    JsonObject domainObj = customArray.createNestedObject();
                    domainObj["id"] = i + 1;
                    domainObj["domain"] = blockedDomains[i];
                    
                    if (blockedDomains[i] == domain) {
                        domainObj["active"] = nowActive;
                    } else {
                        String entry = "0.0.0.0 " + blockedDomains[i];
                        domainObj["active"] = (blacklistContent.indexOf(entry) >= 0);
                    }
                }
                
                String response;
                serializeJson(responseDoc, response);
                request->send(200, "application/json", response);
            } else {
                request->send(500, "application/json", "{\"error\":\"Failed to toggle domain\"}");
            }
        } else {
            request->send(400, "application/json", "{\"error\":\"Invalid ID\"}");
        }
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid request\"}");
      }
  });

  server.on("/api/blocklist/custom", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (!error && doc.containsKey("domain")) {
        String domain = doc["domain"].as<String>();
        Serial.println("Adding domain to blocklist: " + domain);
        
        if (addBlockedDomain(domain)) {
          // Return just the custom blocklist array
          DynamicJsonDocument responseDoc(2048);
          JsonArray customArray = responseDoc.to<JsonArray>();
          
          String blacklistContent = router.readFile("/etc/hosts.user");
          
          for (int i = 0; i < blockedDomainsCount; i++) {
            JsonObject domainObj = customArray.createNestedObject();
            domainObj["id"] = i + 1;
            domainObj["domain"] = blockedDomains[i];
            String blockEntry = "0.0.0.0 " + blockedDomains[i];
            domainObj["active"] = (blacklistContent.indexOf(blockEntry) >= 0);
          }
          
          String response;
          serializeJson(responseDoc, response);
          request->send(200, "application/json", response);
        } else {
          // Return empty array on error to prevent frontend crash
          request->send(200, "application/json", "[]");
        }
      } else {
        request->send(400, "application/json", "[]");
      }
  });

  server.on("/api/blocklist/custom", HTTP_DELETE, [](AsyncWebServerRequest *request){
      if (request->hasParam("id")) {
        int id = request->getParam("id")->value().toInt();
        
        if (id > 0 && id <= blockedDomainsCount) {
            String domain = blockedDomains[id - 1];
            
            // Remove from router
            router.unblockDomain(domain);
            
            // Remove from local array
            for (int i = id - 1; i < blockedDomainsCount - 1; i++) {
                blockedDomains[i] = blockedDomains[i + 1];
            }
            blockedDomainsCount--;
            saveBlocklist();
            
            // Return updated list
            DynamicJsonDocument responseDoc(4096);
            JsonArray customArray = responseDoc.to<JsonArray>();
            String blacklistContent = router.readFile("/etc/adblock/adblock.blacklist");
            
            for (int i = 0; i < blockedDomainsCount; i++) {
                JsonObject domainObj = customArray.createNestedObject();
                domainObj["id"] = i + 1;
                domainObj["domain"] = blockedDomains[i];
                domainObj["active"] = (blacklistContent.indexOf(blockedDomains[i]) >= 0);
            }
            
            String response;
            serializeJson(responseDoc, response);
            request->send(200, "application/json", response);
        } else {
            request->send(404, "application/json", "{\"error\":\"Domain not found\"}");
        }
      } else {
        request->send(400, "application/json", "{\"error\":\"Missing id parameter\"}");
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

  // Network statistics endpoint
  server.on("/api/stats", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = router.getNetworkStats();
    request->send(200, "application/json", response);
  });

  // Serve static files last to avoid checking for API paths as files
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");


  // Serve blocklist for adblock to fetch (in hosts file format)
  server.on("/blocklist.txt", HTTP_GET, [](AsyncWebServerRequest *request){
      String blocklist = "";
      for (int i = 0; i < blockedDomainsCount; i++) {
        blocklist += "0.0.0.0 " + blockedDomains[i] + "\n";
      }
      request->send(200, "text/plain", blocklist);
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
  
  // Load blocked apps from Preferences
  loadBlockedApps();
  
  // Ensure Adblock is configured correctly
  if (!SIMULATION_MODE) {
      router.configureAdblock();
      router.enforceStrictDNS();
  }
  
  setupRoutes();
  
  server.begin();
}

void loop() {
}
