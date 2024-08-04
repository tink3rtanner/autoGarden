/*
  AUTO GARDEN V1

  * Post data to Adafruit IO for 1 moisture sensor

 
*/



#include "Wire.h"         //talk to relays and sensors
#include "U8glib.h"       //think this is the display driver
#include "RTClib.h"       //realtime clock? Idk how it works
RTC_DS1307 RTC;

#include "WiFiEsp.h"


// WIFI SETUP
char ssid[] = "PieFi";            // your network SSID (name)
char pass[] = "candamir";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
// Initialize the Ethernet client object
WiFiEspClient client;

// IO POST SETUP
char server[] = "io.adafruit.com";

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds

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

static unsigned long currentMillis_send = 0;
static unsigned long  Lasttime_send = 0;

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

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  Serial.print("SETUP");
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


  //other stuff
  //draw_elecrow();
  delay(2000);
  Wire.begin();
  RTC.begin();
  Serial.begin(9600);
  // declare relay as output
  pinMode(relay1, OUTPUT); //valve 1
  pinMode(relay2, OUTPUT); //valve 2
  pinMode(relay3, OUTPUT); //valve 3
  pinMode(relay4, OUTPUT); //valve 4
  // declare pump as output
  pinMode(pump, OUTPUT);
  // declare switch as input
  pinMode(button, INPUT);
  //pinMode(ROTARY_ANGLE_SENSOR, INPUT); //??
  // water_flower();
  
}





void loop()   // MAIN LOOP
{
  delay(2000);
  Serial.println("LOOP");
  read_value();

 //serial print sensor values
  Serial.println(moisture1_value);
  Serial.println(moisture2_value);
  Serial.println(moisture3_value);
  Serial.println(moisture4_value);

  
  
  
  
  
  
  
  // HTTP STUFF
  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if 10 seconds have passed since your last connection,
  // then connect again and send data
  // AKA don't spam
  if (millis() - lastConnectionTime > postingInterval) {

    //Send Moisture 1
    httpRequest(moisture1_value,"soil-moisture1");

    //Send Moisture 2
    httpRequest(moisture2_value,"soil-moisture2");
    
    
    Serial.println("HTTP REQUEST");
    lastConnectionTime = millis();
  }








  
}


//CALLED TO READ MOISTURE VALUE
void read_value() 
{
/**************These is for resistor moisture sensor***********
 float value1 = analogRead(A0);
  moisture1_value = (value1 * 120) / 1023; delay(20);
  float value2 = analogRead(A1);
  moisture2_value = (value2 * 120) / 1023; delay(20);
  float value3 = analogRead(A2);
  moisture3_value = (value3 * 120) / 1023; delay(20);
  float value4 = analogRead(A3);
  moisture4_value = (value4 * 120) / 1023; delay(20);
 **********************************************************/
/************These is for capacity moisture sensor*********/
 float value1 = analogRead(A0);
  moisture1_value =map(value1,590,360,0,100); delay(20);   //MAP probably normalizes it from sensor ticks to %, wait 20s
  if(moisture1_value<0){                                   //avoid negative value cases
    moisture1_value=0;
  }
  float value2 = analogRead(A1);
  moisture2_value =map(value2,600,360,0,100); delay(20);
  if(moisture2_value<0) {
    moisture2_value=0;
  }
  float value3 = analogRead(A2);
  moisture3_value =map(value3,600,360,0,100); delay(20);
  if(moisture3_value<0){
    moisture3_value=0;
  }
  float value4 = analogRead(A3);
  moisture4_value =map(value4,600,360,0,100); delay(20);
  if(moisture4_value<0) {
    moisture4_value=0;
  }
}


// this method makes a HTTP connection to the server
void httpRequest(int value, String feed)
{
  Serial.println(value);
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();
  Serial.println(value);
  String data="{\n\"value\": " + String(value) + "\n}";
  Serial.println(data);
  

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
     String poststring = "POST /api/v2/jpriebe/feeds/" + feed + "/data HTTP/1.1";
     client.println(poststring);
     client.println(F("Host: io.adafruit.com"));
     client.println(F("X-AIO-Key: 8c039146eedd4d65a83333349cbdab74"));
     client.println(F("Connection: keep-alive"));
     client.println(F("Content-Type: application/json"));
     client.print("Content-Length: ");
     client.println(String(data.length()));
     client.println(); // end http header
     client.println(data);

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
