
# Auto Garden README

## Overview

The Auto Garden system automates watering plants by monitoring soil moisture levels and controlling water relays and pumps. It communicates with Adafruit IO to send sensor data and receive control commands.

## Libraries

- **Wire.h**: Communicates with relays and sensors.
- **U8glib.h**: Display driver.
- **RTClib.h**: Real-time clock.
- **WiFiEsp.h**: Handles WiFi communication with the ESP module.

## Setup

### Hardware Connections

- **Moisture Sensors**: A0, A1, A2, A3
- **Relays**: 6, 8, 9, 10
- **Pump**: 4
- **Button**: 12

### WiFi Configuration

Update the following variables with your network credentials:
```cpp
char ssid[] = "_Josh";  // Your network SSID
char pass[] = "blueberries";  // Your network password
```

### Real-Time Clock Initialization

```cpp
RTC_DS1307 RTC;
```

### Variables

- **Moisture Values**: `moisture1_value`, `moisture2_value`, `moisture3_value`, `moisture4_value`
- **Relay States**: `relay1_state_flag`, `relay2_state_flag`, `relay3_state_flag`, `relay4_state_flag`
- **Pump State**: `pump_state_flag`
- **Watering Control**: `auto_water`, `water_now`, `moisture_setpoint`, `io_auto_water`, `io_water_now`, `io_moisture_setpoint`
- **Timing**: `lastConnectionTime`, `lastConnectionTimeIFTTT`, `postingInterval`, `postingIntervalIFTTT`, `lasttime_water`, `wateringInterval`

## Functions

### Setup Function

Initializes serial communication, WiFi connection, RTC, and sets pin modes for relays, pump, and button.

### Loop Function

Main logic loop, includes:

- Reading moisture sensor values.
- Posting sensor data to Adafruit IO.
- Getting control values from Adafruit IO.
- Checking if watering is needed and executing watering if necessary.
- Monitoring water bucket level and sending notifications to IFTTT if needed.

### Auxiliary Functions

- **read_value()**: Reads and maps moisture sensor values.
- **should_water()**: Determines if plants should be watered based on sensor data and control values.
- **water_plants()**: Activates relays and pump to water plants.
- **httpPostRequestAdafeed(int value, String feed)**: Posts data to Adafruit IO.
- **httpGetRequest(String feed)**: Gets control values from Adafruit IO.
- **httpPostRequestIFTTT(String event)**: Posts notifications to IFTTT.
- **printWifiStatus()**: Prints WiFi connection status.

## References

- [Auto Garden User Manual](https://www.elecrow.com/download/product/AAK90039K/Automatic_Smart_Plant_Watering_Kit_User%20Manual_v2.2.pdf)
