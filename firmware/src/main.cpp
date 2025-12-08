#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "OpenWrtClient.h"
#include <esp_task_wdt.h>

// Config
const char* ssid = "OpenWrt";
const char* password = "";

// Router Credentials
const char* router_host = "192.168.10.100";
const char* router_user = "root";
const char* router_pass = ""; // Default, user should change this

AsyncWebServer server(80);
OpenWrtClient router(router_host, router_user, router_pass);

void setup() {
  Serial.begin(115200);

  // Initialize LittleFS
  if(!LittleFS.begin(true)){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  // Increase watchdog timeout to 30 seconds to allow dnsmasq restart
  esp_task_wdt_init(30, true);
  esp_task_wdt_add(NULL);


  // API: Get Stats
  server.on("/api/stats", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    
    // Get real data from router
    doc["connectedDevices"] = router.getConnectedDeviceCount();
    
    // Get full device list directly into the doc
    JsonArray devicesArray = doc["devices"].to<JsonArray>();
    router.getConnectedDevices(devicesArray);
    
    String total, down, up;
    unsigned long long rx = 0, tx = 0;
    
    // Get raw stats (Single Request)
    if(router.getTrafficStats(rx, tx)) {
        doc["traffic"]["rx"] = rx;
        doc["traffic"]["tx"] = tx;
        
        // Format for display using the public helper
        down = router.formatBytes(tx); // TX from router is Download for client
        up = router.formatBytes(rx);   // RX to router is Upload from client
        total = router.formatBytes(rx + tx);
    } else {
        total = "0 B";
        down = "0 B";
        up = "0 B";
    }
    
    doc["dataUsage"]["total"] = total;
    doc["dataUsage"]["download"] = down;
    doc["dataUsage"]["upload"] = up;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // API: Block Domain
  server.on("/api/block", HTTP_POST, [](AsyncWebServerRequest *request){
    if(request->hasParam("domain", true)){
        String domain = request->getParam("domain", true)->value();
        if(router.blockDomain(domain.c_str())){
            request->send(200, "text/plain", "Blocked");
        } else {
            request->send(500, "text/plain", "Failed to block");
        }
    } else {
        request->send(400, "text/plain", "Missing domain param");
    }
  });

  // API: Unblock Domain
  server.on("/api/blocklist/custom", HTTP_DELETE, [](AsyncWebServerRequest *request){
    if(request->hasParam("domain")){
        String domain = request->getParam("domain")->value();
        if(router.unblockDomain(domain.c_str())){
            request->send(200, "text/plain", "Unblocked");
        } else {
            request->send(500, "text/plain", "Failed to unblock");
        }
    } else {
        request->send(400, "text/plain", "Missing domain param");
    }
  });

  // API: Get Custom Blocklist
  server.on("/api/blocklist/custom", HTTP_GET, [](AsyncWebServerRequest *request){
    String blocklist = router.getBlocklist();
    if (blocklist.length() > 0) {
      // Parse blocklist and return as JSON array
      JsonDocument doc;
      JsonArray array = doc["blocklist"].to<JsonArray>();
      
      int startPos = 0;
      int newlinePos = 0;
      while ((newlinePos = blocklist.indexOf('\n', startPos)) != -1) {
        String line = blocklist.substring(startPos, newlinePos);
        line.trim();
        if (line.length() > 0) {
          array.add(line);
        }
        startPos = newlinePos + 1;
      }
      
      String response;
      serializeJson(doc, response);
      request->send(200, "application/json", response);
    } else {
      request->send(200, "application/json", "{\"blocklist\":[]}");
    }
  });


  // API: Apply Blocklist Changes (Batch)
  server.on("/api/blocklist/apply", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, 
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      JsonDocument doc;
      deserializeJson(doc, (const char*)data);
      
      if(!doc["changes"].is<JsonArray>()){
        request->send(400, "text/plain", "Invalid changes format");
        return;
      }
      
      JsonArray changes = doc["changes"];
      bool success = router.applyBlocklistChanges(changes);
      
      if(success){
        request->send(200, "text/plain", "Changes applied");
      } else {
        request->send(500, "text/plain", "Failed to apply changes");
      }
  });

  // API: Allow Domain
  server.on("/api/allow", HTTP_POST, [](AsyncWebServerRequest *request){
    if(request->hasParam("domain", true)){
        String domain = request->getParam("domain", true)->value();
        if(router.allowDomain(domain.c_str())){
            request->send(200, "text/plain", "Allowed");
        } else {
            request->send(500, "text/plain", "Failed to allow");
        }
    } else {
        request->send(400, "text/plain", "Missing domain param");
    }
  });

  // Serve Static Files (Moved to end to avoid capturing API requests)
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.begin();
}

void loop() {
    // Feed the watchdog timer to prevent timeout
    esp_task_wdt_reset();
    
    // Keep session alive
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 60000) {
        lastCheck = millis();
        router.checkSession();
    }
}
