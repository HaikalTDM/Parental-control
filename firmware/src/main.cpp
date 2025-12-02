#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Preferences.h>

#include "openwrt.h"

// Configuration
// INSTRUCTIONS: Update these with your actual credentials
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
    
    // Parse the JSON-RPC response
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, rawResponse);
    
    String response = "[]";
    if (!error && doc.containsKey("result")) {
      JsonArray result = doc["result"];
      if (result.size() > 1 && result[1].containsKey("dhcp_leases")) {
        // Extract the dhcp_leases array
        JsonArray leases = result[1]["dhcp_leases"];
        
        // Format into our expected device format
        DynamicJsonDocument devicesDoc(4096);
        JsonArray devices = devicesDoc.to<JsonArray>();
        
        int id = 1;
        for (JsonObject lease : leases) {
          JsonObject device = devices.createNestedObject();
          device["id"] = id++;
          device["name"] = lease["hostname"].as<String>();
          device["type"] = "device";
          device["status"] = "online";
          device["blocked"] = false;
          device["usage"] = "0 GB";
          device["ip"] = lease["ipaddr"].as<String>();
          device["mac"] = lease["macaddr"].as<String>();
        }
        
        serializeJson(devices, response);
      }
    }
    
    request->send(200, "application/json", response);
  });

  server.on("/api/blocklist", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "application/json", "{\"apps\":{},\"custom\":[]}");
  });

  server.on("/api/blocklist/app", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      request->send(200, "application/json", "{}");
  });

  server.on("/api/blocklist/custom", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      request->send(200, "application/json", "[]");
  });

  server.on("/api/allowlist", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "application/json", "[]");
  });

  server.on("/api/allowlist", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      request->send(200, "application/json", "[]");
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
  setupRoutes();
  
  server.begin();
}

void loop() {
}
