
/*
>>> Air Quality Egg v01 <<<
 Nanode based environmental sensor ethernet connected unit. 
 Measures NO2 (ppb) , CO (ppm) , air quality (?), humidity (%), temperature (C).
*/

 
#include <aJSON.h>
#include <SPI.h>         
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EEPROM.h> 
#include <DHT.h>
#include <WConstants.h>
#include <Wire.h>
#include <CS_MQ7.h>

#define FEED    "48091" //unique feed id -- Egg feeds are below:
//unit01: 48091 // unit02: 48306 // unit03: 48307 // unit04: 48308 // unit05: 48309 // unit06: 48310 //

#define DHTPIN 17 //analog pin 3
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);


//timer vars
const int transmitFrequency = 12; //time to wait between sending data in seconds
unsigned long currTime; //holds ms passed since board started

// analog sensor input pins
const int No2SensorPin = A0;
const int CoSensorPin = A1;
const int qualitySensorPin = A2;
const int humiditySensorPin = A3;
const int buttonPin = 7;



//___________________________________JUST ADDED__________________________________________________________________
byte mac[] = { 0x90 , 0xA2, 0xDA, 0x00, 0x00, 0x12 }; // Must be unique on local network

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "TYL0ZA5ED3BQBQIN";
const int updateThingSpeakInterval = 10 * 1000;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

// Variable Setup
long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;

EthernetClient client2;
//______________________________________END______________________________________


//sensor value vars
//int currNo2, currCo, currQuality, currHumidity, currTemp, currButton = 0;
int currNo2, currCo, currQuality,currButton = 0;
float  currHumidity, currTemp = 0.0;
boolean debug = true;

//reset for when ethernet times out and never comes back
const int resetterPin = 3; //when pulled low, will reset board.


byte server[] = { 130,102,129,175}; //winter.ceit.uq.edu.au
char IPstring[25];
int greenLEDpin = 13;

EthernetClient ethClient;







PubSubClient client(server, 1883, callback, ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
}

void setup_MQTT() 
{
  //_________________________________JUST ADDED  ________________________________________________________________
  startEthernet();
  //_________________________________END_________________________________________________________________________
  
  
  
  // EEPROM locations:
  // Unique MAC address can be stored in EEPROM using 'setMACaddress' sketch, which sets the last 3 bytes of this address.
 // const int eepromMacFlag = 0; // 1 byte
  //int eepromMacAddress = 1; // 3 bytes
  // Default MAC address for the ethernet controller.
  //static byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x00, 0x12  }; 

  // Retrieve MAC address from EEPROM if present
  //if (EEPROM.read(eepromMacFlag) == '#') {
    //Serial.println("Retrieving MAC address from EEPROM");
    //for (int i = 3; i < 6; i++)
      //mac[i] = EEPROM.read(eepromMacAddress++);
  //} 
  //else {
    //Serial.println("No MAC address stored in EEPROM");
    //Serial.println("Using default MAC address");
  //}  

  // Publish the MAC address to serial port
  //{
    //const int outputStringLen = 25;
    //static char outputString[outputStringLen];  // buffer used for constructing output strings

    //snprintf(outputString, outputStringLen, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    //Serial.print("MAC Address: "); 
    //Serial.println(outputString);
 // }

  // Use DHCP to determine my IP address
  Serial.println("DHCP...");
  while (Ethernet.begin(mac) == 0) {
    digitalWrite(greenLEDpin, HIGH);         
    //Serial.println("No DHCP");
    delay(1000);
    digitalWrite(greenLEDpin, LOW);         
    //Serial.println("DHCP...");
  }  
  //Serial.println("IP address: ");
  sprintf(IPstring, "%u.%u.%u.%u", Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], Ethernet.localIP()[3]);
  //Serial.println(IPstring);
}









void setup () 
{
  Serial.begin(9600);
//____________________________JUST ADDED___________________________________________________________
  startEthernet();
//_____________________________END_________________________________________________________________
  setup_MQTT();
  pinMode(resetterPin, OUTPUT);  
  digitalWrite(resetterPin, HIGH); //this is a hack! --> ???
  pinMode(buttonPin, INPUT);  
  ledSetup();
  Wire.begin(); 

  
  
}   

void loop ()
{  
  if (!client.connected()) {
    client.connect("AQE000");
    client.publish("AQE/log","(Re)Connecting to MQTT");
    
    return;
  }
  client.loop(); 
  currTime = millis();
  nanodeUpdate(); //checking for received data
  ledUpdate();

  //note: transmitTime() contains sending function 
  if( !transmitTime() ){
    //if we are not transmitting
    if(currTime%2000 == 0){  //print the currTime every second
      //JUST CHANGED !!!!!!
      //Serial.print("currTime: ");  
      //Serial.println(currTime/1000);  
      
      readSensors(); //update sensor values every second
    }
  } //else we are transmitting!

//__________________________________JUST ADDED______________________________
  // Read value from Analog Input Pin 0
  //String analogValue0 = String(analogRead(A0), DEC);
  // Print Update Response to Serial Monitor
  
  if (client2.available())
  {
    char c = client2.read();
    Serial.print(c);
  }

  // Disconnect from ThingSpeak
  if (!client2.connected() && lastConnected)
  {
    //Serial.println("...disconnected");
    //Serial.println();
    
    client2.stop();
  }
  
  // Update ThingSpeak
  if(!client2.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {
     updateThingSpeak("field1="+ (String) currNo2 + "&field2="+ (String) currCo);
     //updateThingSpeak("field2="+ (String) currQuality);
     //Serial.println(currQuality);
    
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) {startEthernet();}
  
  lastConnected = client2.connected();}

//_____________________________________END____________________________________

}

//_________________________JUST ADDED________________________________________
void updateThingSpeak(String tsData)
{
  if (client2.connect(thingSpeakAddress, 80))
  {         
    client2.print("POST /update HTTP/1.1\n");
    client2.print("Host: api.thingspeak.com\n");
    client2.print("Connection: close\n");
    client2.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client2.print("Content-Type: application/x-www-form-urlencoded\n");
    client2.print("Content-Length: ");
    client2.print(tsData.length());
    client2.print("\n\n");

    client2.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client2.connected())
    {
      //Serial.println("Connecting to ThingSpeak...");
      //Serial.println();
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      //Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
      //Serial.println();
    }
    
  }
  else
  {
    failedCounter++;
    
    //Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    //Serial.println();
    
    lastConnectionTime = millis(); 
  }
}









void startEthernet()
{
  
  client2.stop();

  Serial.println("Connecting Arduino to network...");
  Serial.println();  

  delay(1000);
  
  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  }
  else
  {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
  }
  
  delay(1000);
}
//___________________________END___________________________________________
