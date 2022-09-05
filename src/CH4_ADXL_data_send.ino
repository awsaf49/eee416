const int AOUTpin0=0;//the AOUT pin of the CH4 sensor goes into analog pin A0 of the arduino
const int AOUTpin1=4;//the AOUT pin of the CO sensor goes into analog pin A4 of the arduino

float CH4_data = 0;
float CO_data = 0;

//Arduino to NodeMCU Lib
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

float t_g;
int xpin = A1;                
int ypin = A2;                
int zpin = A3;

float xvalue, x, xg;
float yvalue, y, yg;
float zvalue, z, zg;

//Initialise Arduino to NodeMCU (5=Rx & 6=Tx)
SoftwareSerial nodemcu(5, 6);

void setup() {
  Serial.begin(9600);//sets the baud rate
  pinMode(13, OUTPUT);
  nodemcu.begin(9600);

}

void loop()
{
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  
  MQ4_func();
  ADXL_func();
  MQ7_func();

   //Assign collected data to JSON Object
  data["CH4"] = CH4_data;
  data["Gravity"] = t_g;
  data["CO"] = CO_data;
   
  //Send data to NodeMCU
  data.printTo(nodemcu);
  jsonBuffer.clear();

  if(Serial.available()>0)
   {     
      char data= Serial.read(); // reading the data received from the bluetooth module
      switch(data)
      {
        case 'a': digitalWrite(13, HIGH);break; // when a is pressed on the app on your smart phone
        case 'd': digitalWrite(13, LOW);break; // when d is pressed on the app on your smart phone
        default : break;
      }
   }
  delay(50);
}

void ADXL_func(){

  float xvalue = analogRead(xpin);                              //reads values from x-pin & measures acceleration in X direction 
  float x = map(xvalue, 276, 416, -100, 100);               //maps the extreme ends analog values from -100 to 100 for our understanding
  float xg = (float)x/(-100.00)*9.8;                          //converts the mapped value into acceleration in terms of "g"
   
  float yvalue = analogRead(ypin);
  float y = map(yvalue, 275, 420, -100, 100);
  float yg = (float)y/(-100.00)*9.8;
 
  float zvalue = analogRead(zpin);
  float z = map(zvalue, 278, 420, -100, 100);
  float zg = (float)z/(100.00)*9.8;

  t_g = pow((pow(xg,2)+pow(yg,2)+pow(zg,2)),0.5);
  Serial.print("Free Fall Feedback:  ");
  Serial.println(t_g); 
}

void MQ4_func(){
  CH4_data = analogRead(AOUTpin0);//reads the analaog value from the CH4 sensor's AOUT pin
  
  Serial.print("CH4: ");
  Serial.println(CH4_data);//prints the CH4 value 
}

void MQ7_func(){
  CO_data = analogRead(AOUTpin1);//reads the analaog value from the CO sensor's AOUT pin
  
  Serial.print("CO: ");
  Serial.println(CO_data);//prints the CO value 
}
