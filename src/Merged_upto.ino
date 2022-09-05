//Include library for wifi connection
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SoftwareSerial.h>

//connection to LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // library for I2C LCD
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display  

//Include Lib for Arduino to Nodemcu
#include <ArduinoJson.h>

//wifi connection setup
const char* ssid = "Mi Casa";
const char* password = "01757818150";

//connection to ThingSpeak
String apiKey = "5TESQGNU34Q0C57F"; // Enter API key from ThingSpeak
const char* server = "api.thingspeak.com";
//WiFiClient client;

//D6 = Rx & D5 = Tx;
SoftwareSerial nodemcu(D6, D5);

//connection for sending sms
const char* host = "maker.ifttt.com";
const int httpsPort = 443;
const char* ch = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* fingerprint = "616275faea5f64954af6090f59c90de71e6d66a3"; //public key

//initialize pin for temperature measurement
#define sensorPin A0

//variables for fall detection
int flag = 0;
int count = 0;

void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  nodemcu.begin(9600);
  delay(10);
  Serial.print("connecting to ");
  Serial.println(ssid);
  
  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //lcd setup
  lcd.begin();    // initialize the lcd   
  lcd.backlight(); // backlight ON

  //if receive data from Arduino
  while (!Serial) continue;
}

void loop() {
   //LM35 sensor data
   int reading = analogRead(sensorPin); // Get the voltage reading from the LM35
   float voltage = reading * (5.0 / 2047.0); // Convert that reading into voltage
   float temperature = voltage * 100; // Convert the voltage into the temperature in Celsius

   //ADXL, MQ4 & MQ7 receive code from arduino
   StaticJsonBuffer<1000> jsonBuffer;
   JsonObject& data = jsonBuffer.parseObject(nodemcu);

   if (data == JsonObject::invalid()) {
     //Serial.println("Invalid Json Object");
     jsonBuffer.clear();
     return;
   }

   //Serial.println("JSON Object Recieved");
   //CH4 sensor Data
   float CH4_data = data["CH4"];
   CH4_data = (CH4_data*3)/1023;

   //adxl sensor data
   float t_g = data["Gravity"];

   //MQ7 sensor data
   float CO_data = data["CO"];
   CO_data = (CO_data*50)/1023;

   //upload to thingspeak
   WiFiClient client;
   if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
    {             
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(CO_data);
    postStr +="&field2=";
    postStr += String(CH4_data);
    postStr +="&field3=";
    postStr += String(temperature);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("CO level:  ");
    Serial.println(CO_data);
    Serial.print("Temperature:  ");
    Serial.println(temperature, 1);
    Serial.print("Recieved CH4 level:  ");
    Serial.println(CH4_data);
    Serial.print("Recieved Accelaration Feedback level:  ");
    Serial.println(t_g);
    Serial.println("Data Send to Thingspeak");
    Serial.println();
    }

  client.stop();
  //Serial.println("Waiting...");
  // thingspeak needs minimum 15 sec delay between updates.

  //fall_detection calculation and sending sms
  if(flag == 0 && t_g<=6.5){
    flag = 1;
  }
  
  if(flag==1){
    count++;
  }
  
  if(flag==1 && count>=3 && count<=7 && t_g>=11){
    flag=0;
    count=0;

    Serial.println("Fall Detected");

    WiFiClientSecure client;
        Serial.print("connecting to ");
        Serial.println(host);
        Serial.print("Using fingerprint: ");
        Serial.println(fingerprint);
        client.setFingerprint(fingerprint);
    
    if (!client.connect(host, httpsPort))
        {
          Serial.println("connection failed");
          return;
        }

    String url = "/trigger/fall_detected/with/key/n8_V5oWLpLFMX6sglMoSvRBfoF5N5IKT471y6RKynsd";
        Serial.print("requesting URL: ");
        Serial.println(url);

        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "User-Agent: BuildFailureDetectorESP8266\r\n" +
                     "Connection: close\r\n\r\n");

        Serial.println("request sent");
        
    delay(1000);
  }

  if(CO_data<=30 && CH4_data<=10){
    lcd.setCursor(0,0);
    lcd.print("Gas Level safe");
  }
  else {
    lcd.setCursor(0,0);
    lcd.print("Gas Level Unsafe");
  }

  if(temperature<=50){
    lcd.setCursor(1,1);
    lcd.print("E: Suitable");
  }
  else{
    lcd.setCursor(1,1);
    lcd.print("E: Unsuitable");
  }
//  delay(1500);
}
