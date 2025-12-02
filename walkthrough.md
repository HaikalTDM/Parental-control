# IoT OpenWRT Parental Control System

This project is a complete IoT-enabled Parental Control System designed for **OpenWRT routers**. It features a modern React-based mobile dashboard and an ESP32 firmware implementation that communicates directly with the router via JSON-RPC.

## Project Architecture
1.  **PC Simulation (Localhost)**:
    *   **Frontend**: React App (`npm run dev`) on your computer.
    *   **Backend**: Python Script (`app.py`) on your computer.
    *   **Purpose**: Testing UI without hardware.

2.  **ESP32 Production (Hardware)**:
    *   **Frontend**: React App (Built & Uploaded) running on the ESP32.
    *   **Backend**: C++ Firmware (`main.cpp`) running on the ESP32.
    *   **Purpose**: Connects to OpenWRT WiFi and sends commands via HTTP.

## Prerequisites (OpenWRT Router)
Your OpenWRT router must have the `ubus` service enabled (standard on most builds).
1.  **Login**: Ensure you know the router IP (e.g., `192.168.10.100`), username (usually `root`), and password.
2.  **WiFi**: The router must have a 2.4GHz WiFi network enabled.

## Setup & Installation

### 1. Configure Firmware
1.  Open `firmware/src/main.cpp`.
2.  Update the **Configuration** section:
    ```cpp
    const char* ssid = "Your_OpenWRT_WiFi_Name";
    const char* password = "Your_WiFi_Password";
    const char* routerHost = "192.168.10.100"; // Your Router IP
    const char* routerUser = "root";
    const char* routerPass = "your_router_password";
    ```
3.  Ensure `#define SIMULATION_MODE false` is set.

### 2. Upload to ESP32
1.  **Connect**: Plug ESP32 into USB.
2.  **Upload Filesystem**:
    *   Click **PlatformIO Icon** (Alien head).
    *   **Project Tasks** -> **esp32dev** -> **Platform** -> **Upload Filesystem Image**.
3.  **Upload Firmware**:
    *   **Project Tasks** -> **esp32dev** -> **General** -> **Upload and Monitor**.

### 3. Verify
1.  Watch the **Terminal** output in VS Code.
2.  You should see:
    ```
    Connecting to WiFi...
    Connected! IP Address: 192.168.10.xxx
    OpenWRT: Attempting Login...
    OpenWRT: Login Successful! Session ID: ...
    ```
3.  Open the IP address (`http://192.168.10.xxx`) on your phone.
4.  Press the **Toggle Internet** button.
5.  Check the terminal for: `OpenWRT: Internet BLOCKED (Command Sent)`.

## Troubleshooting
*   **Login Failed**: Check your username/password and ensure you can log in to the router's web interface from your browser.
*   **WiFi Stuck**: Ensure the router is broadcasting 2.4GHz WiFi.
*   **Upload Error**: Close Serial Monitor (trash can icon) and try again.
