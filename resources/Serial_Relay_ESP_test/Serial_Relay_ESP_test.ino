
//#include <SoftwareSerial.h>

//SoftwareSerial mySerial = SoftwareSerial(0,1);  //RX,TX

//  Arduino pin 2 (RX) to ESP8266 TX
//  Arduino pin 3 to voltage divider then to ESP8266 RX
//  Connect GND from the Arduiono to GND on the ESP8266
//  Pull ESP8266 CH_PD HIGH

// When a command is entered in to the serial monitor on the computer 
// the Arduino will relay it to the ESP8266




void setup() 
{
    Serial.begin(9600);     // communication with the host computer
    while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
    }
    Serial.println("Setup");
    
    Serial1.begin(115200);
    delay(1000);
    Serial1.println("AT+CWMODE=1");
    delay(1000);
    Serial1.println("AT+CWJAP=\"PieFi\",\"candamir\"");
    delay(1000);
    

    char input;
    while(Serial1.available() > 0)
    {
      input = Serial1.read();
      Serial.print(input);
    }    
}
 
void loop() 
{
    Serial.println("LOOP");
    
    
    Serial1.println("AT+CWJAP?");
    
    delay(5000);

    char input;
    while(Serial1.available() > 0)
    {
      input = Serial1.read();
      Serial.print(input);
      //Serial1.print(input);
      //if(input='/n'){
      //  Serial.println();
      //
      //}
    }
//    Serial.println(Serial1.available());
//    delay(500);
//     = Serial1.read();
//    Serial.println(c);
//    c = Serial1.read();
//    
//    c = Serial1.read();
//    Serial.println(c);
//    c = Serial1.read();
//    Serial.println(c);
//    c = Serial1.read();
//    Serial.println(c);
//    c = Serial1.read();
//    Serial.println(c);
//    c = Serial1.read();
//    Serial.println(c);
//    c = Serial1.read();
//    Serial.println(c);
//    
    
    // listen for communication from the ESP8266 and then write it to the serial monitor
    //if ( Serial1.available() )   {  Serial.write(Serial1.read() );  }
    
    
    
}
