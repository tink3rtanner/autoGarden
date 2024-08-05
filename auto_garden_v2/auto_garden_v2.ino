/*
  AUTO GARDEN

    Post data to Adafruit IO for moisture sensors
  V2
    HTTP GET for auto_water and water_now



*/

////////////////////////////////
// L  I  B  R  A  R  I  E  S  //
////////////////////////////////

#include "Wire.h"         //talk to relays and sensors
#include "U8glib.h"       //think this is the display driver
#include "RTClib.h"       //realtime clock
RTC_DS1307 RTC;

#include "WiFiEsp.h"

// Include the WiFi configuration header
#include "wifi_config.h"  // Add this line at the beginning of your includes

////////////////////////////////
// V  A  R  I  A  B  L  E  S  //
////////////////////////////////


// WIFI SETUP
// Remove or comment out the hardcoded WiFi credentials
// char ssid[] = "_Josh";            // your network SSID (name)
// char pass[] = "blueberries";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
// Initialize the Ethernet client object
WiFiEspClient client;

// IO POST SETUP
char server[] = "io.adafruit.com";

unsigned long lastConnectionTime = 0;         // last time you connected to the adafruit.io server, in milliseconds (GET AND POST) (0 ms = never connected)
unsigned long lastConnectionTimeIFTTT = 9999999;         // last time you connected to the ifttt server, in milliseconds (GET AND POST). 9999999 is ~2.8 hours, simulating "forever ago"
const unsigned long postingInterval = 10000L; // 1 hour in milliseconds - delay between http updates, in milliseconds. L forces long (3600000L ms = 1 hour (effectively stopping HTTP requests).  Typically 10000L ms = 10 seconds)
const unsigned long postingIntervalIFTTT = 100000L; // delay between http updates, in milliseconds. L forces long (100000 ms = 100 seconds or ~1.7 minutes)
static unsigned long currentMillis_send = 0;  // dont think this one is used (0 ms = start of program)
static unsigned long Lasttime_send = 0;       // last time you sent, used? (0 ms = never sent)



// set all moisture sensors PIN ID
int moisture1 = A0;
int moisture2 = A1;
int moisture3 = A2;
int moisture4 = A3;

// declare moisture values
int moisture1_value = 0 ;
int moisture2_value = 0;
int moisture3_value = 0;
int moisture4_value = 0;

// set water relays
int relay1 = 6;
int relay2 = 8;
int relay3 = 9;
int relay4 = 10;

// set water pump
int pump = 4;

// set button
int button = 12;

//pump state    1:open   0:close
int pump_state_flag = 0;

//relay1 state    1:open   0:close
int relay1_state_flag = 0;

//relay2 state   1:open   0:close
int relay2_state_flag = 0;

//relay3 state  1:open   0:close
int relay3_state_flag = 0;

//relay4 state   1:open   0:close
int relay4_state_flag = 0;


char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat",};
unsigned long nowtime;
unsigned long endtime;
unsigned long nowtimeNext;
unsigned long nowtime1;
unsigned long endtime1;
unsigned long nowtimeNext1;
unsigned long nowtime2;
unsigned long endtime2;
unsigned long nowtimeNext2;
unsigned long nowtime3;
unsigned long endtime3;
unsigned long nowtimeNext3;

int auto_water = 0;
int water_now = 0;
int moisture_setpoint = 0;
int io_auto_water = 0;
int io_water_now = 0;
int io_moisture_setpoint = 0;
static unsigned long lasttime_water = 0;      // last time you watered
const unsigned long wateringInterval = 600000L; //10 mins. 40000L;4000000L(1hr) - short for testing. First water waits interval //watering interval. 4M ms = 64 mins

// Add these function prototypes before setup()
void read_value();
void should_water();
void water_plants();
void httpPostRequestAdafeed(int value, String feed);
int httpGetRequest(String feed, int maxRetries = 3);
void httpPostRequestIFTTT(String event);
void printWifiStatus();
String formatTime(unsigned long milliseconds);

// At the top of your file, add these global variables
unsigned long lastWifiCheck = 0;
const unsigned long wifiCheckInterval = 30000; // Check WiFi every 30 seconds

////////////////////
// S  E  T  U  P  //
////////////////////

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  Serial.print("SETUP");
  // initialize serial for ESP module
  Serial1.begin(115200);
  // initialize ESP module
  WiFi.init(&Serial1);


  // check for the presence of the esp wifi chip
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi chip not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  int attempts = 0;
  while (status != WL_CONNECTED && attempts < 10) {
    Serial.print("Attempt ");
    Serial.print(attempts + 1);
    Serial.print(": Connecting to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Use variables from the header file.
    status = WiFi.begin(ssid, pass);
    delay(10000); // Wait 10 seconds between attempts
    attempts++;
  }

  if (status == WL_CONNECTED) {
    Serial.println("You're connected to the network");
    printWifiStatus();
  } else {
    Serial.println("Failed to connect to the network after 10 attempts");
  }

  //other stuff
  delay(2000);
  Wire.begin();
  RTC.begin();            // begin clock
  Serial.begin(9600);     // begin serial output for pc
  // declare relay as output
  pinMode(relay1, OUTPUT); //valve 1
  pinMode(relay2, OUTPUT); //valve 2
  pinMode(relay3, OUTPUT); //valve 3
  pinMode(relay4, OUTPUT); //valve 4
  // declare pump as output
  pinMode(pump, OUTPUT);
  // declare switch as input
  pinMode(button, INPUT);
  // water_flower();

}


//////////////////
//  L  O  O  P  //
//////////////////

void loop()   // MAIN LOOP
{
  // Check WiFi connection periodically
  if (millis() - lastWifiCheck > wifiCheckInterval) {
    checkWiFiConnection();
    lastWifiCheck = millis();
  }

  delay(2000);
  read_value();

  // Print sensor values in a table format
  Serial.print("LOOP | ");
  Serial.print(String(moisture1_value).length() < 2 ? " " : "");
  Serial.print(moisture1_value);
  Serial.print(" | ");
  Serial.print(String(moisture2_value).length() < 2 ? " " : "");
  Serial.print(moisture2_value);
  Serial.print(" | ");
  Serial.print(String(moisture3_value).length() < 2 ? " " : "");
  Serial.print(moisture3_value);
  Serial.print(" | ");
  Serial.print(String(moisture4_value).length() < 2 ? " " : "");
  Serial.println(moisture4_value);

  // HTTP GET/POST CONNECTIONS ADAFRUIT (Spam Protected, 10s posting time)
  if (millis() - lastConnectionTime > postingInterval) {
    // if 10 seconds have passed since your last connection,
    //  connect again and send data
    // AKA don't spam
    Serial.println("HTTP REQUESTS");

    //POST MOISTURE SENSOR DATA
    //POST Moisture 1
    httpPostRequestAdafeed(moisture1_value, "soil-moisture1");

    //POST Moisture 2
    httpPostRequestAdafeed(moisture2_value, "soil-moisture2");

    //POST Moisture 3
    httpPostRequestAdafeed(moisture3_value, "soil-moisture3");


    //POST Bucket Water Levle (Moisture 4)
    //httpPostRequestAdafeed(moisture4_value, "water-bucket-level");

    // GET AUTO GARDEN STATE
    delay(500);
    //io_auto_water = httpGetRequest("auto-water", 3);
    delay(500);
    //io_water_now = httpGetRequest("water-now", 3);
    delay(500);
    //io_moisture_setpoint = httpGetRequest("moisture-setpoint", 3);


    // LOG GONNECTION. Already doing this in http****request? no harm you're already here
    lastConnectionTime = millis();
  }


  // WATER PLANTS

  should_water();
  if (water_now == 1) {
    //Decided to water!
    Serial.println("Decided to water, water-now=");
    Serial.println(water_now);
    water_plants();
  }
  //    water_plants()


  // WATER BUCKET LEVEL

  if (moisture4_value > 10000) { //100 is better, 10000 to basically disable it.
    Serial.println("Water level high. Turning off dehumidifier");
    //Water Level High, turn of dehumidifier
    if (millis() - lastConnectionTimeIFTTT > postingIntervalIFTTT) {
      
      // httpPostRequestIFTTT("Dehumidifier_off");
      // HTTP call to IFTTT webhook
      Serial.println("Water level high. Here's where you would turn off dehumidifier via IFTTT webhook.");
    }
  }
  else {
    //Water level not high
    //Dehumidifier on, or nothing? how to get humidifier state? Might have to set up tuya API or spam ON's. Maybe day-limit ON calls.
  }


  //end loop
}


/////////////////////////////////
//  F  U  N  C  T  I  O  N  S  //
/////////////////////////////////



// READ MOISTURE VALUE
void read_value()
{
  // Hardware notes:
  // Each moisture sensor is connected via a 4-pin analog JST connector
  // PIN layout:
  // RED = GROUND
  // BLACK = VCC (5V) (a crime I KNOW)
  // WHITE = SENSOR SIGNAL (connected to analog pins A0, A1, A2, A3)
  // Note: The fourth pin is not used in this setup

  // ON New Capacitive Moisture Sensor: (THE RIGHT WAY)
  // RED = VCC
  // BLACK = GROUND
  // YELLOW = SENSOR SIGNAL


  /************These is for capacitive moisture sensor*********/
  float value1 = analogRead(A1);
  moisture1_value = map(value1, 600, 360, 0, 100); delay(20); //MAP probably normalizes it from sensor ticks to %, wait 20s
  if (moisture1_value < 0) {                               //avoid negative value cases
    moisture1_value = 0;
  }
  float value2 = analogRead(A0);
  moisture2_value = map(value2, 600, 360, 0, 100); delay(20);
  if (moisture2_value < 0) {
    moisture2_value = 0;
  }
  float value3 = analogRead(A2);
  moisture3_value = map(value3, 600, 360, 0, 100); delay(20);
  if (moisture3_value < 0) {
    moisture3_value = 0;
  }
  float value4 = analogRead(A3);
  moisture4_value = map(value4, 600, 360, 0, 100); delay(20);
  if (moisture4_value < 0) {
    moisture4_value = 0;
  }
  // A4 is also available
}


// SOULD WATER?

void should_water()
{
  // process server values
  if (io_auto_water == -1) {
    Serial.print("Bad io_auto_water return value from io: ");
    Serial.println(io_auto_water);
  }
  else {
    auto_water = io_auto_water;
  }
  if (io_water_now == -1) {
    Serial.print("Bad io_water_now return value from io: ");
    Serial.println(io_water_now);
  }
  else {
    water_now = io_water_now;
  }

  if (io_moisture_setpoint == -1) {
    Serial.print("Bad io_moisture_setpoint return value from io: ");
    Serial.println(io_moisture_setpoint);
  }
  else {
    moisture_setpoint = io_moisture_setpoint;
  }



  //get auto water status
  Serial.println("Should water?");
  Serial.print("Auto Water:");
  Serial.println(auto_water);
  Serial.print("Water Now:");
  Serial.println(water_now);
  Serial.print("Moisture Setpoint:");
  Serial.println(moisture_setpoint);


  //AUTO WATER

  unsigned long timeSinceLastWater = millis() - lasttime_water;
  String formattedTimeSinceLastWater = formatTime(timeSinceLastWater);
  String formattedWateringInterval = formatTime(wateringInterval);

  Serial.print("Time since last water: ");
  Serial.println(formattedTimeSinceLastWater);
  Serial.print("Watering Interval: ");
  Serial.println(formattedWateringInterval);

  //If (haven't watered recently) and (auto water on) and (mosture value below setpoint)
  if ((timeSinceLastWater > wateringInterval) && (auto_water == 1) && (moisture1_value < moisture_setpoint)) {
    Serial.println("Auto Water Triggered");
    water_now = 1;
    httpPostRequestAdafeed(water_now, "water-now"); //send to server

  }

  //check moisture state
  //water_now=1; //debug




}


// WATER PLANTS
void water_plants()
{
  //open relay and activate pump
  Serial.println("WATERING NOW!!");
  digitalWrite(relay1, HIGH);     //relay that's the gate for the water
  relay1_state_flag = 1;
  delay(50);
  //if (pump_state_flag == 0)       //if not pumping already
  //{
  digitalWrite(pump, HIGH);
  pump_state_flag = 1;
  delay(50);
  //}

  Serial.println("things are getting wet...");
  //else if (moisture1_value > 55)  //original code only stops watering once wet
  //Water Flow:
  delay(20000); //couple seconds

  //close the relay
  Serial.println("Water stopping.");
  digitalWrite(relay1, LOW);
  relay1_state_flag = 0;
  delay(50);

  //stop the pump
  digitalWrite(pump, LOW);
  pump_state_flag = 0;
  delay(50);

  //log the watering
  lasttime_water = millis();

  //set water now to 0 and send to io
  water_now = 0;
  io_water_now = 0;
  Serial.println("Setting water_now to 0 on IO");
  httpPostRequestAdafeed(water_now, "water-now");







  //
}


// HTTP POST REQUEST
// this method makes a HTTP connection to the server
// and POSTS to an IO feed
void httpPostRequestAdafeed(int value, String feed)
{
  client.stop();
  char server[] = "io.adafruit.com";

  String data = "{\n\"value\": " + String(value) + "\n}";

  Serial.println("Preparing to send POST request to Adafruit IO");
  Serial.print("Server: ");
  Serial.println(server);
  Serial.print("Feed: ");
  Serial.println(feed);
  Serial.print("Data to send: ");
  String printData = data.substring(1, data.length() - 1);
  printData.replace('\n', ' ');
  Serial.println(printData);

  if (client.connect(server, 80)) {
    Serial.println("Connected to server");

    String poststring = "POST /api/v2/jpriebe/feeds/" + feed + "/data HTTP/1.1";
    client.println(poststring);
    client.println(F("Host: io.adafruit.com"));
    client.println(F("X-AIO-Key: 8c039146eedd4d65a83333349cbdab74"));
    client.println(F("Connection: keep-alive"));
    client.println(F("Content-Type: application/json"));
    client.print("Content-Length: ");
    client.println(String(data.length()));
    client.println();
    client.println(data);

    Serial.println("POST request sent.");
    
    // Wait a short time for any immediate error response
    delay(100);

    if (client.available()) {
      Serial.println("Received immediate response:");
      while (client.available()) {
        Serial.write(client.read());
      }
    } else {
      Serial.println("No immediate response received. Assuming successful POST.");
    }

    lastConnectionTime = millis();
  }
  else {
    Serial.println("Connection failed");
  }

  client.stop();
  Serial.println("Connection closed");
}


// HTTP GET REQUEST
// this method makes a HTTP connection to the server
// and GETs an IO feed value
// returns: value(int) if found or -1 if not
int httpGetRequest(String feed, int maxRetries) {
  for (int retry = 0; retry < maxRetries; retry++) {
    client.stop();
    const char* server = "io.adafruit.com";

    Serial.println("Attempting to connect to Adafruit IO...");
    if (client.connect(server, 80)) {
      Serial.println("Connected to Adafruit IO.");
      
      String getRequest = "GET /api/v2/jpriebe/feeds/" + feed + "/data?limit=1 HTTP/1.1";
      client.println(getRequest);
      client.println("Host: io.adafruit.com");
      client.println("X-AIO-Key: 8c039146eedd4d65a83333349cbdab74");
      client.println("Connection: close");
      client.println();

      Serial.println("GET request sent. Waiting for response...");
      
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          delay(1000);
          break;
        }
      }
      
      if (client.available()) {
        String statusLine = client.readStringUntil('\r');
        Serial.println("Response status: " + statusLine);
        
        // Print the full response for debugging
        Serial.println("Full response:");
        while (client.available()) {
          String line = client.readStringUntil('\r');
          Serial.println(line);
        }
        
        if (statusLine.indexOf("200 OK") > 0) {
          Serial.println("Received 200 OK response");
          client.stop();
          return 0; // Success
        }
      }
    }
    Serial.println("Connection failed or invalid response. Retrying...");
    delay(1000);
  }
  return -1; // Failed after all retries
}


// IFTTT HTTP POST REQUEST
// this method makes a HTTP connection to the server
// and POSTS to an IO feed
void httpPostRequestIFTTT(String event)
{

  //https://ifttt.com/maker_webhooks/triggers/event

  // example in curl:
  // curl -X POST -H -d https://maker.ifttt.com/trigger/Dehumidifier_off/with/key/dZY0w4W9KOxljr_RT_9gmPpZQTVyJc6ZIt1AuroYAKr

  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();
  char server[] = "maker.ifttt.com";


  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    delay(2000);
    //send the HTTP POST request
    String poststring = "POST /trigger/" + event + "/with/key/dZY0w4W9KOxljr_RT_9gmPpZQTVyJc6ZIt1AuroYAKr HTTP/1.1";
    client.println(poststring);
    //client.println(" HTTP/1.1");
    client.println("Host: maker.ifttt.com");
    client.println("Connection: close");
    client.println(); // end http header

    // note the time that the connection was made
    lastConnectionTimeIFTTT = millis();

    // Wait for the response from the server
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
      }
    }

    Serial.println("\nRequest complete.");
    client.stop();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}







// PRINT WIFI STATUS

void printWifiStatus()
{
  // Print the current WiFi status
  Serial.print("WiFi status: ");
  Serial.println(WiFi.status());

  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

  // Print MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  // Print gateway IP
  IPAddress gateway = WiFi.gatewayIP();
  Serial.print("Gateway IP: ");
  Serial.println(gateway);

  // Print subnet mask
  IPAddress subnet = WiFi.subnetMask();
  Serial.print("Subnet mask: ");
  Serial.println(subnet);
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(ssid, pass);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnected to WiFi");
    } else {
      Serial.println("Failed to reconnect to WiFi");
    }
  }
}

String formatTime(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;

  String result = "";
  if (hours > 0) {
    result += String(hours) + "h ";
  }
  if (minutes > 0 || hours > 0) {
    result += String(minutes) + "m ";
  }
  result += String(seconds) + "s";
  return result;
}