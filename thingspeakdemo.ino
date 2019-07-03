#include <SoftwareSerial.h>

#define USE_ARDUINO_INTERRUPTS true    
#define DEBUG true
#define RX 10
#define TX 11
#include "Timer.h"
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.   
Timer t;
PulseSensorPlayground pulseSensor;

String AP = "J P R NET";       // CHANGE ME
String PASS = "8454069811"; // CHANGE ME
String API = "4PPFBWSD253J5G14";   // CHANGE ME
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field2";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
float valSensor = 1;
const int PulseWire = A0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;           //for heart rate sensor

float myTemp;
int myBPM;
String BPM;
String temp;
int error;
int panic=0;
int raw_myTemp;
float Voltage;
float tempC;

SoftwareSerial esp8266(RX,TX); 
 
  
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);   

  // Double-check the "pulseSensor" object was created and "began" seeing a signal. 
   if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }
  
  sendCommand("AT",5,"OK");
  sendCommand("AT",5,"OK");
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
}

void loop()
{
  getSensorData();

    if(digitalRead(8) == HIGH){
    Serial.println("Panic");
    panic=1;
    String getData = "GET /update?api_key="+ API +"&field3="+panic;
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),10,">");
    esp8266.println(getData);delay(1500);countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");

    panic=0;      
}
}

void getSensorData(){

  raw_myTemp = analogRead(A1);
  Voltage = (raw_myTemp / 1023.0) * 5000; // 5000 to get millivots.
  tempC = Voltage * 0.1; 
  myTemp = tempC;
  Serial.println(myTemp);
  

 int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened". 
Serial.println(myBPM);                        // Print the value inside of myBPM. 
}

  delay(20);            
   
  char buffer1[10];
  char buffer2[10];
  BPM = dtostrf(myBPM, 4, 1, buffer1);
  temp = dtostrf(myTemp, 4, 1, buffer2); 


  String getData = "GET /update?api_key="+ API +"&field1="+BPM+"&field2="+temp;
  sendCommand("AT+CIPMUX=1",5,"OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),10,">");
  esp8266.println(getData);delay(1500);countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0",5,"OK");

  
  

  
}


void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
