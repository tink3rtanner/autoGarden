
# Auto Garden API Integration Updates

## Overview

This document outlines the updates to the Auto Garden system to integrate Google HomeGraph API calls. These calls will be used to control a dehumidifier device based on the water tank level measured by an additional moisture sensor. When the tank is full, the system will set the dehumidifier state to OFF, stopping new water from coming in.

## New Components

- **Water Tank Sensor**: Measures when the water tank is full.
- **Google Cloud Service Account**: For making authenticated API calls to the HomeGraph API.

## Libraries

Ensure the following libraries are installed for API calls:
- **WiFi.h**: For handling WiFi communication.
- **HTTPClient.h**: For making HTTP requests.

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

### Generate Access Token (Python Script)

Use the following Python script to generate an access token from the service account key:

```python
import jwt
import time
import requests
import json

def create_jwt(project_id, private_key, private_key_id):
    token = {
        "iat": int(time.time()),
        "exp": int(time.time()) + 3600,  # Token valid for 1 hour
        "aud": "https://homegraph.googleapis.com/",
    }
    
    additional_headers = {
        "kid": private_key_id,
        "alg": "RS256",
        "typ": "JWT",
    }

    signed_jwt = jwt.encode(token, private_key, headers=additional_headers, algorithm="RS256")

    return signed_jwt

def get_access_token(jwt_token):
    url = "https://oauth2.googleapis.com/token"
    headers = {
        "Content-Type": "application/x-www-form-urlencoded",
    }
    body = {
        "grant_type": "urn:ietf:params:oauth:grant-type:jwt-bearer",
        "assertion": jwt_token,
    }

    response = requests.post(url, headers=headers, data=body)
    return response.json().get("access_token")

# Load your service account JSON key file
with open("service_account_key.json") as f:
    key_data = json.load(f)

private_key = key_data["private_key"]
private_key_id = key_data["private_key_id"]
project_id = key_data["project_id"]

jwt_token = create_jwt(project_id, private_key, private_key_id)
access_token = get_access_token(jwt_token)
print(access_token)
```

Run this script to obtain the access token and note it down.

## Arduino Code Changes

### Include Necessary Libraries

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
```

### Update WiFi Configuration

Ensure your `wifi_config.h` is updated with your network credentials.

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

### Update Loop Function

Add logic to read the new moisture sensor and control the dehumidifier based on the water tank level:

```cpp
void loop() {
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

## Summary

1. **Google Cloud Setup**:
   - Create a Google Cloud Project and enable HomeGraph API.
   - Create a service account and download the JSON key file.
   - Generate an access token using the provided Python script.

2. **Arduino Code Updates**:
   - Include necessary libraries (`WiFi.h` and `HTTPClient.h`).
   - Add functions to get and set device state using HomeGraph API.
   - Update the loop function to read the water tank sensor and control the dehumidifier.

These updates integrate the HomeGraph API to control a dehumidifier based on the water tank level, enhancing the functionality of the Auto Garden system.
