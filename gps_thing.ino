#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
//#include "ThingSpeak.h"
static const int RXPin = D2, TXPin = D1;
static const uint32_t GPSBaud = 9600;
float latitude , longitude;
int hour,sec,minn;
String  lat_str , lng_str,hour_str,sec_str,minn_str;
// repace your wifi username and password
const char* ssid     = "D-Link_DIR-600M";
const char* password = "23310452";
unsigned long myChannelNumber = 1830178;
const char * myWriteAPIKey = "GJ4BKZG8QGIH7LMJ";
// The TinyGPS++ object
TinyGPSPlus gps;
WiFiClient  client;
// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
   Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  ThingSpeak.begin(client);
}
void loop()
{
  while (ss.available() > 0){
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
        Serial.print("Latitude = ");
        Serial.println(lat_str);
        Serial.print("Longitude = ");
        Serial.println(lng_str);
        ThingSpeak.setField(1, lat_str);
        ThingSpeak.setField(2, lng_str);
        ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); 
      }
    if (gps.time.isValid())
    {
      if (gps.time.hour() < 10) Serial.print(F("0"));
      Serial.print(gps.time.hour());
      //latitude = gps.location.lat();
      //lat_str = String(latitude , 6);
      hour=gps.time.hour()+6;
      //hour_str = String(hour , 6);
      ThingSpeak.setField(3, hour);

      Serial.print(F(":"));
      if (gps.time.minute() < 10) Serial.print(F("0"));
      Serial.print(gps.time.minute());
      minn=gps.time.minute();
      minn_str = String(minn, 6);
      ThingSpeak.setField(4, minn_str);
      Serial.print(F(":"));
      if (gps.time.second() < 10) Serial.print(F("0"));
      Serial.print(gps.time.second());
      sec=gps.time.second();
      sec_str = String(sec , 6);
      ThingSpeak.setField(5, sec_str);
      Serial.print(F("."));
      if (gps.time.centisecond() < 10) Serial.print(F("0"));
      Serial.print(gps.time.centisecond());
     ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); 
    }     
     delay(1000);
     Serial.println();  
    }
  }
}