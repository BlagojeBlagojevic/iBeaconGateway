# iBeacon Gateway with ESP32 for Position Estimation and Smart Camera Data Parsing

This repository features an **iBeacon Gateway** implemented using **ESP32** and **ESP-IDF**, primarily designed for position estimation using **BLE**. Additionally, it includes parsing data from a **Milesight smart camera**. The project uses MQTT to transmit data and integrates position estimation calculations based on RSSI values.

---

## 📖 **Features**

- **BLE Scanning and Position Estimation:**
  - Scans for BLE iBeacon advertisements and calculates distances using RSSI values.
  - Filters UUIDs to ensure only relevant data is sent to the MQTT broker.
- **Position Estimation Formula:**
  - Implements distance estimation with the formula:  
    ```
    RSSI = -10.0f * N * log10(distance) + RSSI(Distance.1m);
    Distance = 10^((Measured Power - Instant RSSI)/(10 * N)).
    ```
- **Smart Camera Data Parsing:**
  - Parses event data from a Milesight smart camera for parking space monitoring and sends it to the MQTT broker.
- **JSON Data Formats:**
  - iBeacon data: Includes RSSI, distance, and spatial coordinates.
  - Camera data: Contains event type, timestamp, and occupancy information.

---

## 🚀 **How It Works**

1. **BLE Scanning and UUID Filtering:**
   - The ESP32 scans for nearby BLE iBeacons and filters data by UUID before processing and rssi.
   
2. **Position Estimation:**
   - Calculates distance from the beacon using RSSI values and predefined calibration parameters.


3. **Smart Camera Data Integration:**
   - Parses real-time data from a Milesight smart camera for parking space monitoring.

4. **Data Transmission via MQTT:**
   - Sends formatted JSON data to the MQTT broker for iBeacon and camera data.

---

## 📦 **Repository Structure**

```
├── components
│   ├── bt # Blueroid component
│   ├── eth_enc28j60 # Etherent component for enc28j60 
│   ├── json # cJSON lib

├── main/
|   ├── CMakeLists.txt        # Build instructions 
│   ├── async_server_utils.h  # Generics for async webserver
│   ├── ble.c                 # BLE init, scanning, filtering
│   ├── cert.h                # Cert and credintials for mqtts
│   ├── defs.h                # Common includes, topic, generics for sending mqtt, utility  
│   ├── main.c                # Main interface for calling, task creation initing spiff,
│                               initing etherent/wifi, init mqtt, ble
│   ├── mqtt.h                # Mqtt init, publising data, logic for changing
│                               comon params (x, y, uuid allowed list, env factor, rssi)
│   ├── parking.c             # Parking parsing saving, logic for saving into a spiff,
│   ├── parking.h             # Parking struct defs of func global parkings(100 max)
│   ├── server.c              # Server implementation, saving parkings
│   ├── server.h              # Server header
│   ├── tag.c                 # Tag implementation, saving tags
│   ├── tag.h                 # Tag header
│   └── wifi.h                # WIFI implementation
├── ESP32_Guide.md            # Detail description of used libs and esp32 overview 
└── README.md              # Project overview (this file)
```

---

## 🛠️ **Hardware Requirements**

- **ESP32 Development Board** (e.g., ESP32-WROOM-32 or similar)
- One or more **iBeacon Devices**
- **Milesight Smart Camera** (optional)
- Wi-Fi network for data transmission
- Optional: Power supply, enclosures, and mounting equipment

---

## 🖥️ **Software Requirements**

- **ESP-IDF Framework** (v4.x or later recommended)
- **MQTT Broker** (e.g., Mosquitto, AWS IoT, or HiveMQ)
- **Python 3.x** (for testing MQTT data or server-side applications)

---

## 📋 **Setup and Installation**

### ESP32 Firmware
1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/ibeacon-gateway-esp32.git
   cd ibeacon-gateway-esp32
   ```
2. Install and configure ESP-IDF:
   - Follow the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).
3. Configure `config.h` with:
   - Wi-Fi credentials
   - MQTT broker details
   - UUID filter list
4. Build and flash the firmware:
   ```bash
   idf.py build
   idf.py flash
   ```

---

## 📊 **JSON Data Formats**

### iBeacon Data
Data is sent to the MQTT broker in the following JSON format:
```json
{
  "refpower": 0,
  "uuid": "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX",
  "majorID": 0,
  "rssi": -65,
  "minorID": 0,
  "time": "2024-12-09T12:00:00Z",
  "distance": 1.5,
  "x": 0.0,
  "y": 0.0
}
```

### Milesight Camera Data
Camera events are parsed and sent to the MQTT broker in this format:
```json
{
  "event": "Parking Space Detection",
  "device": "Network Camera",
  "time": "2024-12-09T12:00:00Z",
  "parking_area": "Area A",
  "occupancy": 5
}
```

---

## 🧪 **Testing**

- Power on the ESP32, iBeacon devices, and the Milesight camera.
- Monitor MQTT topics for iBeacon and camera data.
- Verify position estimation outputs match expected distances based on known RSSI values.

---

## 🌟 **Contributing**

Contributions are welcome! Feel free to submit a pull request or open an issue to suggest improvements or new features.

---

## 📜 **License**

This project is licensed under the [MIT License](LICENSE).

---
