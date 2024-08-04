/*
 WiFiEsp example: WebClientRepeating

 This sketch connects to a web server and makes an HTTP request
 using an Arduino ESP8266 module.
 It repeats the HTTP call each 10 seconds.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

#include "WiFiEsp.h"

// Emulate Serial1 on pins 6/7 if not present
//#ifndef HAVE_HWSERIAL1
//#include "SoftwareSerial.h"
//SoftwareSerial Serial1(6, 7); // RX, TX
//#endif

char ssid[] = "PieFi";            // your network SSID (name)
char pass[] = "candamir";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "io.adafruit.com";

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(115200);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  
  printWifiStatus();
}

void loop()
{
  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if 10 seconds have passed since your last connection,
  // then connect again and send data
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}

// this method makes a HTTP connection to the server
void httpRequest()
{
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();
  String data="{\n{\"value\": 42}}";
  

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
//    // send the HTTP GET request
//    client.println(F("GET /api/v2/jpriebe/feeds/soil-moisture/data?limit=1 HTTP/1.1"));
//    client.println(F("X-AIO-Key: 8c039146eedd4d65a83333349cbdab74"));
//    client.println(F("Host: io.adafruit.com"));
//    client.println("Connection: close");
//    client.println(); // end http header

     //send the HTTP POST request
    client.println(F("POST /api/v2/jpriebe/feeds/soil-moisture/data HTTP/1.1"));
    client.println(F("X-AIO-Key: 8c039146eedd4d65a83333349cbdab74"));
    client.println(F("Host: io.adafruit.com"));
    client.println(F("Content-Type: application/json"));
    client.print("Content-Length: ");
    client.print(String(data.length()));
    client.println("; charset=utf-8");
    client.println("Accept: */*");
    client.println("Connection: keep-alive");
    client.println(); // end http header
    client.println(data);
    //client.println(); // end http header

//    //test length    
    Serial.println(F("POST /api/v2/jpriebe/feeds/soil-moisture/data HTTP/1.1"));
    Serial.println(F("X-AIO-Key: 8c039146eedd4d65a83333349cbdab74"));
    Serial.println(F("Host: io.adafruit.com"));
    Serial.println(F("Content-Type: application/json"));
    Serial.print("Content-Length: ");
    Serial.print(String(data.length()));
    Serial.println("; charset=utf-8");
    Serial.println("Accept: */*");
    Serial.println("Connection: keep-alive");
    Serial.println(); // end http header
    Serial.println(data);

    // note the time that the connection was made
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
