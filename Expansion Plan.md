# Auto Garden System Expansion Plan

## Overview

This document outlines the plan to expand the Auto Garden system to control the watering of three separate plants, each with its own moisture sensor and relay gate. This expansion allows for individualized watering schedules based on the specific moisture needs of each plant.

## Hardware Requirements

### Additional Components

- **2 Moisture Sensors**: To measure the soil moisture levels of the additional plants.
- **2 Relay Modules**: To control the water flow to each of the new plants independently.

### Connections

- **Moisture Sensors**: Connect the new sensors to analog pins A4 and A5.
- **Relays**: Connect the new relays to digital pins 11 and 12.

## Software Modifications

### Variable Definitions

- Additional variables for the new moisture sensors and their readings:
  ```cpp
  int moisture5 = A4; // New sensor for Plant 2
  int moisture6 = A5; // New sensor for Plant 3
  int moisture5_value = 0;
  int moisture6_value = 0;
  ```

- Additional relay control pins:
  ```cpp
  int relay5 = 11; // Relay for Plant 2
  int relay6 = 12; // Relay for Plant 3
  ```

### Setup Function

- Initialize the new sensor and relay pins:
  ```cpp
  pinMode(relay5, OUTPUT); // Relay for Plant 2
  pinMode(relay6, OUTPUT); // Relay for Plant 3
  ```

### Main Logic

- Update the `read_value()` function to include readings from the new sensors:
  ```cpp
  float value5 = analogRead(A4);
  moisture5_value = map(value5, 600, 360, 0, 100); delay(20);
  if (moisture5_value < 0) {
    moisture5_value = 0;
  }
  float value6 = analogRead(A5);
  moisture6_value = map(value6, 600, 360, 0, 100); delay(20);
  if (moisture6_value < 0) {
    moisture6_value = 0;
  }
  ```

- Modify the `water_plants()` function to control watering based on the readings from all sensors:
  ```cpp
  if (moisture1_value < moisture_setpoint) {
    digitalWrite(relay1, HIGH);
    delay(20000); // Watering time
    digitalWrite(relay1, LOW);
  }
  if (moisture5_value < moisture_setpoint) {
    digitalWrite(relay5, HIGH);
    delay(20000); // Watering time
    digitalWrite(relay5, LOW);
  }
  if (moisture6_value < moisture_setpoint) {
    digitalWrite(relay6, HIGH);
    delay(20000); // Watering time
    digitalWrite(relay6, LOW);
  }
  ```

### README Update

Update the README.md to reflect these changes, including the new hardware connections and the updated function descriptions.
