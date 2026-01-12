#ifndef OPENWRT_CLIENT_H
#define OPENWRT_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class OpenWrtClient {
public:
    OpenWrtClient(const char* host, const char* username, const char* password);
    
    bool login();
    bool checkSession();
    
    // Telemetry
    int getConnectedDeviceCount();
    bool getConnectedDevices(JsonArray& targetArray); // Populates provided array
    bool getTrafficStats(unsigned long long& rx, unsigned long long& tx); // Raw bytes
    void getDataUsage(String& total, String& download, String& upload); // Returns formatted strings
    String formatBytes(unsigned long long bytes); // Helper
    
    // Control
    bool blockDomain(const char* domain);
    bool unblockDomain(const char* domain);
    bool applyBlocklistChanges(JsonArray& changes); // Batch apply
    String getBlocklist(); // Get current blocklist from router
    bool allowDomain(const char* domain);
    bool unallowDomain(const char* domain);

private:
    const char* _host;
    const char* _username;
    const char* _password;
    String _sid;
    unsigned long _lastLoginTime;
    
    String sendRequest(const char* object, const char* method, JsonDocument& params);
};

#endif
