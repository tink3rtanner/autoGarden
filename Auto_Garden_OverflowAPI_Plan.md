# Auto Garden API Integration Updates

## Overview

This document outlines the updates to the Auto Garden system to integrate Google HomeGraph API calls. These calls will be used to control a dehumidifier device based on the water tank level measured by an additional moisture sensor. When the tank is full, the system will set the dehumidifier state to OFF, stopping new water from coming in.

## New Components

- **Water Tank Sensor**: Measures when the water tank is full.
- **Google Cloud Service Account**: For making authenticated API calls to the HomeGraph API.

## Libraries

Ensure the following libraries are installed for API calls and JWT generation:
- **WiFi.h**: For handling WiFi communication.
- **HTTPClient.h**: For making HTTP requests.
- **ArduinoJson**: For JSON parsing and creation.
- **Arduino-JWT**: For generating JSON Web Tokens.

## Setup

### Google Cloud Configuration

1. **Create a Google Cloud Project**:
   - Go to the [Google Cloud Console](https://console.cloud.google.com/).
   - Create a new project or select an existing project.

2. **Enable HomeGraph API**:
   - In the Google Cloud Console, go to "APIs & Services" > "Library".
   - Search for "HomeGraph API" and enable it for your project.

3. **Create a Service Account**:
   - Navigate to "IAM & Admin" > "Service Accounts".
   - Click "Create Service Account".
   - Provide a name and description for the service account.
   - Assign the necessary roles (e.g., "HomeGraph API Service Agent").
   - Click "Continue" and then "Done".

4. **Create and Download a Key**:
   - After creating the service account, click on it to open the details.
   - Go to the "Keys" tab and click "Add Key" > "Create New Key".
   - Choose JSON format and download the key file. Keep this file secure.

## Arduino Code Changes

### Include Necessary Libraries

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <jwt.h>
```

### Global Variables

Add these global variables at the top of your Arduino sketch:

```cpp
const char* PROJECT_ID = "your_project_id";
const char* PRIVATE_KEY = "-----BEGIN PRIVATE KEY-----\n...\n-----END PRIVATE KEY-----\n";
const char* PRIVATE_KEY_ID = "your_private_key_id";
String accessToken = "";
unsigned long lastTokenRefresh = 0;
const unsigned long TOKEN_REFRESH_INTERVAL = 3540000; // Refresh token every 59 minutes
```

### Add JWT Generation Function

Add this function to generate a JWT token:

```cpp
String createJWT(const char* project_id, const char* private_key, const char* private_key_id) {
  // Current time and expiration time
  time_t now = time(nullptr);
  time_t exp = now + 3600;  // Token valid for 1 hour

  // Create JWT header
  JsonDocument jwtHeader;
  jwtHeader["alg"] = "RS256";
  jwtHeader["typ"] = "JWT";
  jwtHeader["kid"] = private_key_id;

  // Create JWT payload
  JsonDocument jwtPayload;
  jwtPayload["iat"] = now;
  jwtPayload["exp"] = exp;
  jwtPayload["aud"] = "https://homegraph.googleapis.com/";

  // Serialize header and payload
  String headerStr, payloadStr;
  serializeJson(jwtHeader, headerStr);
  serializeJson(jwtPayload, payloadStr);

  // Create JWT
  String jwt = jwt_encode(headerStr, payloadStr, private_key);
  return jwt;
}
```

### Add Function to Get Access Token

```cpp
String getAccessToken(const char* project_id, const char* private_key, const char* private_key_id) {
  String jwt = createJWT(project_id, private_key, private_key_id);
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://oauth2.googleapis.com/token";
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String requestBody = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + jwt;
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      JsonDocument doc;
      deserializeJson(doc, response);
      return doc["access_token"].as<String>();
    }
    http.end();
  }
  return "";
}
```

### Update Loop Function

Modify the loop function to refresh the access token as needed:

```cpp
void loop() {
  unsigned long currentMillis = millis();

  // Refresh access token if needed
  if (currentMillis - lastTokenRefresh >= TOKEN_REFRESH_INTERVAL || accessToken.isEmpty()) {
    accessToken = getAccessToken(PROJECT_ID, PRIVATE_KEY, PRIVATE_KEY_ID);
    lastTokenRefresh = currentMillis;
  }

  // Read moisture sensor value for water tank
  int waterTankLevel = analogRead(A4);  // Example analog pin A4 for new sensor

  // Check if water tank is full (example threshold value)
  if (waterTankLevel > 800) {
    // Water tank is full, turn off the dehumidifier
    setDeviceState(accessToken, "your_device_id", "action.devices.commands.OnOff", false);
  }

  delay(60000);  // Delay to avoid continuous requests, adjust as needed
}
```

### New Functions

#### Function to Get Device State

```cpp
void getDeviceState(String accessToken, String deviceId) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://homegraph.googleapis.com/v1/devices/" + deviceId;
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + accessToken);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  }
}
```

#### Function to Set Device State

```cpp
void setDeviceState(String accessToken, String deviceId, String command, bool state) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://homegraph.googleapis.com/v1/devices:executeCommand";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + accessToken);

    String requestBody = "{"command":"" + command + "","params":{"on":" + String(state ? "true" : "false") + "},"deviceIds":["" + deviceId + ""]}";
    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  }
}
```

## Summary

1. **Google Cloud Setup**:
   - Create a Google Cloud Project and enable HomeGraph API.
   - Create a service account and download the JSON key file.

2. **Arduino Code Updates**:
   - Include necessary libraries (`WiFi.h`, `HTTPClient.h`, `ArduinoJson`, and `Arduino-JWT`).
   - Add functions to generate JWT and get access token.
   - Update the loop function to refresh the access token as needed and control the dehumidifier based on the water tank level.

These updates integrate the HomeGraph API to control a dehumidifier based on the water tank level, enhancing the functionality of the Auto Garden system. The Arduino now generates its own JWT and access tokens as needed, making the system more self-sufficient and suitable for long-term operation.