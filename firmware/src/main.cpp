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
    String response;
    
    if (SIMULATION_MODE) {
        // Mock Data for Demonstration
        response = "[{\"id\":1,\"name\":\"Atif's iPad\",\"type\":\"tablet\",\"status\":\"online\",\"blocked\":true,\"usage\":\"1.2 GB\"},{\"id\":2,\"name\":\"Dad's Laptop\",\"type\":\"laptop\",\"status\":\"online\",\"blocked\":true,\"usage\":\"4.5 GB\"},{\"id\":3,\"name\":\"Living Room TV\",\"type\":\"tv\",\"status\":\"offline\",\"blocked\":true,\"usage\":\"0 GB\"}]";
    } else {
        // Production: Return empty list until MikroTik Parser is implemented
        response = "[]";
    }
    
    request->send(200, "application/json", response);
  });

  server.on("/api/blocklist", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "application/json", "{\"apps\":{},\"custom\":[]}");
  });

  server.on("/api/allowlist", HTTP_GET, [](AsyncWebServerRequest *request){
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
