# NetGuard - ESP32 DNS Blocking System

![NetGuard](web-ui/public/icon-192.png)

A smart home network security system that uses an ESP32 microcontroller to manage DNS-based content blocking on an OpenWrt router through an intuitive Progressive Web App.

## 🌟 Features

- **📱 Progressive Web App** - Install on your phone like a native app
- **🛡️ DNS Blocking** - Block unwanted websites at the network level
- **📊 Real-time Dashboard** - Monitor connected devices and network stats
- **🎯 One-Tap Control** - Master switch for instant internet control
- **🌐 IPv4 & IPv6 Support** - Complete blocking coverage
- **⚡ Instant Updates** - Changes apply immediately via dnsmasq

## 🏗️ Architecture

```
User's Phone (PWA) <--WiFi--> ESP32 (Web Server) <--UBUS--> OpenWrt Router (dnsmasq)
```

## 🚀 Quick Start

### Prerequisites

- ESP32 development board
- OpenWrt router with UBUS support
- PlatformIO IDE
- Node.js & npm

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/HaikalTDM/Parental-control.git
   cd Parental-control
   ```

2. **Configure WiFi credentials**
   Edit `firmware/src/main.cpp`:
   ```cpp
   const char* ssid = "YourWiFiSSID";
   const char* password = "YourPassword";
   const char* router_host = "192.168.x.x";  // Your router IP
   ```

3. **Build the web UI**
   ```bash
   cd web-ui
   npm install
   npm run build
   ```

4. **Copy web files to ESP32**
   ```bash
   xcopy /E /I /Y dist\* ..\firmware\data\
   ```

5. **Upload to ESP32**
   ```bash
   cd ../firmware
   pio run --target uploadfs  # Upload filesystem
   pio run --target upload     # Upload firmware
   ```

6. **Access NetGuard**
   - Open browser to `http://[ESP32-IP]`
   - Install as PWA from browser menu

## 📱 PWA Installation

### Android/Chrome
1. Open NetGuard in Chrome
2. Tap menu (⋮) → "Install app" or "Add to Home screen"
3. Confirm installation

### iOS/Safari
1. Open NetGuard in Safari
2. Tap Share button → "Add to Home Screen"
3. Confirm

## 🛠️ Tech Stack

| Component | Technology |
|-----------|------------|
| **Microcontroller** | ESP32 |
| **Framework** | Arduino / PlatformIO |
| **Web UI** | React + Vite |
| **Router OS** | OpenWrt |
| **DNS** | dnsmasq |
| **API** | UBUS (JSON-RPC) |

## 📂 Project Structure

```
Parental-control/
├── firmware/              # ESP32 firmware
│   ├── src/
│   │   ├── main.cpp       # Web server & API
│   │   ├── OpenWrtClient.cpp
│   │   └── OpenWrtClient.h
│   └── data/              # Web UI files (LittleFS)
├── web-ui/                # React PWA
│   ├── src/
│   │   ├── App.jsx
│   │   └── components/
│   └── public/
│       ├── manifest.json  # PWA manifest
│       └── sw.js          # Service worker
└── Info/                  # Documentation
```

## 🔧 Configuration

### OpenWrt Setup

1. **Install required packages**
   ```bash
   opkg update
   opkg install rpcd luci-mod-rpc
   ```

2. **Configure UBUS permissions**
   Create `/usr/share/rpcd/acl.d/custom-dnsmasq.json`:
   ```json
   {
     "custom-dnsmasq": {
       "description": "Custom DNS blocking permissions",
       "read": {
         "file": {
           "/etc/adblock/*": ["read"],
           "/etc/dnsmasq.d/*": ["read"]
         }
       },
       "write": {
         "file": {
           "/etc/adblock/*": ["write"],
           "/etc/dnsmasq.d/*": ["write"]
         },
         "ubus": {
           "rc": ["init"]
         }
       }
     }
   }
   ```

3. **Restart rpcd**
   ```bash
   /etc/init.d/rpcd restart
   ```

## 📊 Performance

- **Response Time**: < 100ms
- **DNS Block Application**: 2-3 seconds
- **Web UI Load**: < 2 seconds
- **Max Domains**: ~200 (current implementation)
- **Memory Usage**: ~60% ESP32 RAM

## 🔮 Roadmap

- [ ] Scheduled blocking (time-based rules)
- [ ] Per-device blocking rules
- [ ] Usage analytics dashboard
- [ ] Cloud sync for settings
- [ ] Import Pi-hole/AdGuard lists
- [ ] HTTPS support

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📄 License

This project is licensed under the MIT License.

## 👨‍💻 Author

**Atif**

## 🙏 Acknowledgments

- OpenWrt community
- ESP32 Arduino core developers
- React and Vite teams

---

**Made with ❤️ for safer home networks**
