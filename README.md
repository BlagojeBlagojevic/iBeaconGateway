# iBeacon Gateway with ESP32 for Position Estimation and Smart Camera Data Parsing

This repository features an **iBeacon Gateway** implemented using **ESP32** and **ESP-IDF**, primarily designed for position estimation using **BLE**. Additionally, it includes parsing data from a **Milesight smart camera**. The project uses MQTT to transmit data and integrates position estimation calculations based on RSSI values with Kalman filtering for improved accuracy.

---

## 1. **Features**

- **BLE Scanning and Filtering:**
  - Captures BLE advertisements and filters data based on UUIDs.

- **Position Estimation with Kalman Filter:**
  - Calculates distances using RSSI values with the formula:  
    ```c
    RSSI = -10.0f * N * log10(distance) + RSSI(Distance.1m);
    Distance = 10^((RSSI(Distance.1m) - RSSI)/(10 * N)).
    ```
  - Implements Kalman filtering for smoothing RSSI values in streaming mode

- **Smart Camera Integration:**
  - Parses data from Milesight smart cameras, including parking space detection and occupancy information.

- **MQTT Communication:**
  - Publishes iBeacon and camera data to an MQTT broker over secure connections (MQTT over TLS).

- **Web Server:**
  - Provides an asynchronous server for managing configurations, including:
    - UUID filters
    - Environmental factors for position estimation
    - Coordinate and tag management.

- **Wi-Fi and Ethernet Support:**
  - Supports both Wi-Fi and ENC28J60 Ethernet connectivity for flexible deployment.

- **Persistent Storage:**
  - Stores parking and tag data in SPIFFS for long-term availability.

- **Scalable BLE and Camera Support:**
  - Supports up to 100 parking areas and multiple BLE tags.

---

## 2. **How It Works**

1. **BLE Scanning and UUID Filtering:**
   - The ESP32 scans for nearby BLE iBeacons and filters data by UUID before processing
   - RSSI values are processed through a Kalman filter for noise reduction

2. **Position Estimation:**
   - Calculates distance from the beacon using filtered RSSI values and predefined calibration parameters

3. **Smart Camera Data Integration:**
   - Parses real-time data from a Milesight smart camera for parking space monitoring

4. **Data Transmission via MQTT:**
   - Sends formatted JSON data to the MQTT broker for iBeacon and camera data

---

## 3. **Repository Structure**

```
├── components
│   ├── bt # Blueroid component
│   ├── eth_enc28j60 # Ethernet component for enc28j60 
│   ├── json # cJSON lib

├── main/
│   ├── CMakeLists.txt        # Build instructions 
│   ├── async_server_utils.h  # Generics for async webserver
│   ├── ble.c                 # BLE init, scanning, filtering
│   ├── cert.h                # Cert and credentials for MQTTs
│   ├── defs.h                # Common includes, topics, generics for sending MQTT, utility  
│   ├── kalman.c              # Kalman filter implementation for RSSI smoothing
│   ├── kalman.h              # Kalman filter header
│   ├── main.c                # Main interface for calling, task creation, initializing SPIFFS,
│                               initializing Ethernet/WiFi, initializing MQTT, BLE
│   ├── mqtt.h                # MQTT init, publishing data, logic for changing
│                               common params (x, y, uuid allowed list, env factor, RSSI)
│   ├── parking.c             # Parking parsing saving, logic for saving into SPIFFS
│   ├── parking.h             # Parking struct defs of func global parkings(100 max)
│   ├── server.c              # Server implementation, saving parkings
│   ├── server.h              # Server header
│   ├── tag.c                 # Tag implementation, saving tags
│   ├── tag.h                 # Tag header
│   └── wifi.h                # WiFi implementation
├── ESP32_Guide.md            # Detailed description of used libs and ESP32 overview 
└── README.md                 # Project overview (this file)
```

---

## 4. **Kalman Filter Implementation**

The system implements a Kalman filter to improve the accuracy of RSSI measurements:

```c
// Kalman filter structure;
#ifndef _KALMAN_H
#define _KALMAN_F

#define PROCES_VAR     9.0f
#define VAR0          10.0f
#define MESURMENT_VAR  0.1f

typedef struct {
    float x;   // procjena RSSI (dB)
    float P;   // varijanca
    float Q;   // procesna varijanca
    float R;   // mjerenje varijanca
} Kalman1D;

void  kalman1d_init(Kalman1D *kf, float x0, float P0, float Q, float R);
float kalman1d_update(Kalman1D *kf, float z);

#endif
void kalman1d_init(Kalman1D *kf, float x0, float P0, float Q, float R) {
    kf->x = x0;
    kf->P = P0;
    kf->Q = Q;
    kf->R = R;
}

float kalman1d_update(Kalman1D *kf, float z) {
    // predict
    float x_pred = kf->x;
    float P_pred = kf->P + kf->Q;

    // update
    float K = P_pred / (P_pred + kf->R);
    kf->x = x_pred + K * (z - x_pred);
    kf->P = (1.0f - K) * P_pred;

    return kf->x; // filtrirani RSSI
}
```

The filter is applied to incoming RSSI values before distance calculation, significantly reducing noise and improving position estimation accuracy.

---

## 5. **Hardware Requirements**

- **ESP32 Development Board** (e.g., ESP32-WROOM-32 or similar)
- One or more **iBeacon Devices**
- **Milesight Smart Camera** (optional)
- Wi-Fi network for data transmission
- Optional: ENC28J60 Ethernet module for wired connectivity
- Optional: Power supply, enclosures, and mounting equipment

---

## 6. **Software Requirements**

- **ESP-IDF Framework** (v4.x or later recommended)
- **MQTT Broker** (e.g., Mosquitto, AWS IoT, or HiveMQ)
- **Python 3.x** (for testing MQTT data or server-side applications)

---

## 7. **Setup and Installation**

### ESP32 Firmware
1. Clone this repository:
   ```bash
   git clone https://github.com/BlagojeBlagojevic/iBeaconGateway.git
   cd iBeaconGateway-main
   ```
2. Install and configure ESP-IDF:
   - Follow the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).
3. Configure `defs.h` and `cert.h` with:
   - Wi-Fi/Ethernet credentials
   - MQTT broker details
   - UUID filter list
   - Kalman filter parameters (process noise and measurement noise)

4. Build and flash the firmware:
   ```bash
   idf.py build
   idf.py flash
   ```
---

## 8. **JSON Data Formats**

### iBeacon Data (with Kalman Filtering)
Data is sent to the MQTT broker in the following JSON format:
```json
{
  "refpower": 0,
  "uuid": "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX",
  "majorID": 0,
  "rssi": -65,
  "rssi_filtered": -64.2,
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

## 9. **Testing**

- Power on the ESP32, iBeacon devices, and the Milesight camera.
- Monitor MQTT topics for iBeacon and camera data.
- Verify position estimation outputs match expected distances based on known RSSI values.
- Compare filtered vs. unfiltered RSSI values to observe Kalman filter improvement.

---

## 10. **Contributing**

Contributions are welcome! Feel free to submit a pull request or open an issue to suggest improvements or new features.

---

## 11. **License**

This project is licensed under the [MIT License](LICENSE).

---
