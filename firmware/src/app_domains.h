#ifndef APP_DOMAINS_H
#define APP_DOMAINS_H

#include <Arduino.h>

// Map of popular app IDs to their primary domains
struct AppDomain {
    const char* appId;
    const char* domains[10]; // Up to 10 domains per app
};

const AppDomain APP_DOMAINS[] = {
    {"youtube", {"youtube.com", "youtu.be", "ytimg.com", "googlevideo.com", nullptr}},
    {"tiktok", {"tiktok.com", "tiktokcdn.com", "musical.ly", nullptr}},
    {"facebook", {"facebook.com", "fb.com", "fbcdn.net", nullptr}},
    {"roblox", {"roblox.com", "rbxcdn.com", nullptr}},
    {"instagram", {"instagram.com", "cdninstagram.com", nullptr}}
};

const int APP_DOMAINS_COUNT = 5;

// Get domains for a specific app ID
const char** getAppDomains(const char* appId) {
    for (int i = 0; i < APP_DOMAINS_COUNT; i++) {
        if (strcmp(APP_DOMAINS[i].appId, appId) == 0) {
            return (const char**)APP_DOMAINS[i].domains;
        }
    }
    return nullptr;
}

#endif
