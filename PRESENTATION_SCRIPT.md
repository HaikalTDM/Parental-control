# NetGuard: ESP32 DNS Blocking System
## Presentation Script for Supervisor

---

## 🎯 Project Overview

**NetGuard** is a smart home network security system that uses an **ESP32 microcontroller** to manage and control DNS-based content blocking on an **OpenWrt router**.

### The Problem We're Solving
- Traditional parental controls are difficult to configure
- Router interfaces are complex for average users
- No centralized, user-friendly way to block unwanted websites
- Existing solutions require technical knowledge

### Our Solution
A **mobile-friendly web interface** that allows anyone to:
- View connected devices on the network
- Block/unblock websites with a single tap
- See real-time network statistics
- Manage the entire home network from their phone

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     HOME NETWORK                            │
│                                                             │
│  ┌──────────┐       ┌──────────────┐       ┌─────────────┐ │
│  │  User's  │ WiFi  │    ESP32     │ UBUS  │   OpenWrt   │ │
│  │  Phone   │◄─────►│ (NetGuard)   │◄─────►│   Router    │ │
│  │          │       │   Web UI     │       │  (dnsmasq)  │ │
│  └──────────┘       └──────────────┘       └─────────────┘ │
│                                                  │         │
│                                             DNS Queries    │
│                                                  ▼         │
│                                          ┌─────────────┐   │
│                                          │  Internet   │   │
│                                          └─────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### Components

| Component | Technology | Purpose |
|-----------|------------|---------|
| **ESP32** | C++ / Arduino | Hosts web server, communicates with router |
| **Web UI** | React + Vite | Modern, responsive user interface |
| **OpenWrt** | Linux-based | Router OS with dnsmasq for DNS |
| **UBUS** | JSON-RPC | Secure API for router communication |

---

## 💡 Key Features

### 1. Real-Time Dashboard
- Connected device count
- Network traffic statistics (Upload/Download)
- Active blocking status

### 2. Device Management
- View all connected devices
- See device names, IP addresses, MAC addresses
- Online/offline status indicators

### 3. DNS Blocking
- Add domains to blocklist with one tap
- Block entire categories (Social Media, Videos, etc.)
- **Both IPv4 and IPv6 blocking** for complete coverage
- Changes apply instantly via dnsmasq restart

### 4. Master Switch
- One-tap internet kill switch
- Visual "breathing" animation for status
- Immediate feedback on state changes

---

## 🔧 Technical Implementation

### DNS Blocking Mechanism

When a domain is blocked:

```
User adds "facebook.com" to blocklist
         ↓
ESP32 writes to /etc/dnsmasq.d/custom_blocklist.conf:
    address=/facebook.com/0.0.0.0
    address=/facebook.com/::
         ↓
ESP32 restarts dnsmasq via UBUS:
    ubus call rc init '{"name":"dnsmasq","action":"restart"}'
         ↓
Any device trying to access facebook.com
gets redirected to 0.0.0.0 (blocked!)
```

### Security Features
- Authenticated UBUS sessions with ACL permissions
- No direct SSH/root access from ESP32
- Session timeout and re-authentication
- HTTPS-ready architecture

---

## 📱 User Interface Demo

### Home Screen
- Large circular "Master Switch" button
- Real-time network statistics cards
- Connected device count

### Blocklist Screen
- Add domain input field
- Toggle switches for each blocked domain
- One-click "Apply Changes" button
- Categories with pre-defined domains

### Device List Screen
- All connected devices with status
- Device type icons
- IP and MAC address display

---

## 🚀 Live Demonstration

### Demo 1: Blocking a Website
1. Open NetGuard on phone (192.168.10.193)
2. Navigate to Blocklist tab
3. Add domain: `example.com`
4. Click "Apply Changes"
5. Verify block: `nslookup example.com` → Returns `0.0.0.0`

### Demo 2: Unblocking
1. Toggle off the blocked domain
2. Click "Apply Changes"
3. Verify unblock: `nslookup example.com` → Returns real IP

### Demo 3: Master Switch
1. Click the main power button
2. Observe the visual state change
3. All internet access blocked/unblocked

---

## 📊 Performance Metrics

| Metric | Value |
|--------|-------|
| **ESP32 Response Time** | < 100ms |
| **DNS Block Application** | 2-3 seconds |
| **Web UI Load Time** | < 2 seconds |
| **Maximum Domains** | ~200 (current implementation) |
| **Memory Usage** | ~60% of ESP32 RAM |

---

## 🔮 Future Enhancements

1. **Scheduled Blocking** - Block sites during specific hours
2. **Per-Device Rules** - Different rules for each device
3. **Usage Analytics** - Track blocked requests
4. **Cloud Backup** - Sync settings across devices
5. **Ad Blocking Lists** - Import from Pi-hole/AdGuard

---

## 🎓 Learning Outcomes

Through this project, we:

- **Embedded Systems**: Programmed ESP32 with PlatformIO
- **Web Development**: Built React frontend with modern UI/UX
- **Networking**: Understood DNS, DHCP, and router internals
- **API Design**: Implemented RESTful endpoints
- **Security**: Managed authentication and permissions
- **IoT Integration**: Connected hardware with software systems

---

## ❓ Q&A

### Common Questions

**Q: Why ESP32 instead of Raspberry Pi?**
> A: Lower cost (~$5 vs ~$35), lower power consumption, sufficient for this use case.

**Q: Is this secure?**
> A: Yes, uses authenticated UBUS sessions with limited ACL permissions. No root access exposed.

**Q: Can this be bypassed?**
> A: A tech-savvy user could use a VPN or change their DNS. This is designed for household/family use, not enterprise security.

**Q: How is this different from OpenWrt's LuCI interface?**
> A: Much simpler, mobile-friendly, focused on DNS blocking only. No technical knowledge required.

---

## 📁 Project Structure

```
ESP/
├── firmware/              # ESP32 code
│   ├── src/
│   │   ├── main.cpp       # Web server & API endpoints
│   │   ├── OpenWrtClient.cpp  # Router communication
│   │   └── OpenWrtClient.h
│   └── data/              # Web UI files (LittleFS)
│
├── web-ui/                # React frontend
│   ├── src/
│   │   ├── App.jsx        # Main application
│   │   └── components/    # UI components
│   └── dist/              # Built files
│
└── Info/                  # Documentation
```

---

## 🙏 Thank You

**NetGuard** demonstrates how IoT devices can enhance home network security with a focus on user experience.

### Contact
- **Developer**: [Your Name]
- **Project**: ESP32 DNS Blocking System (NetGuard)
- **Repository**: [If applicable]

---

*Presentation Date: December 2025*
