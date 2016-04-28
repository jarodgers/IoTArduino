#include <eHealth.h>
#include <SoftwareSerial.h>
#include "HardwareSerial.h"


void sendBPM(int rate);
void sendBloodPressure(int systolic, int diastolic);
void sendSweat(int sweat);
boolean checkForString(char * string);
SoftwareSerial serialBT(10, 11);

//  Variables
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat

char buf[30];

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

void setup() {
  
 pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
 Serial.begin(9600);             // we agree to talk fast!
 serialBT.begin(9600);
 //Serial.begin(115200);
 interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
  // IF YOU ARE POWERING The Pulse Sensor AT VOLTAGE LESS THAN THE BOARD VOLTAGE, 
  // UN-COMMENT THE NEXT LINE AND APPLY THAT VOLTAGE TO THE A-REF PIN
 //   analogReference(EXTERNAL);  
  
}

void loop() {
  // look for start command
  if (checkForNewInput(buf) && !strcmp(buf,"START")) {
    eHealth.readBloodPressureSensor();
    delay(100);
    sendBloodPressure(eHealth.bloodPressureDataVector[0].systolic, eHealth.bloodPressureDataVector[0].diastolic); 
   
    while(1) {
      if (checkForNewInput(buf) && !strcmp(buf,"STOP")) {
        // if stop string was found, send final blood pressure, and end the data gathering
        eHealth.readBloodPressureSensor();
        delay(100);
        sendBloodPressure(eHealth.bloodPressureDataVector[0].systolic, eHealth.bloodPressureDataVector[0].diastolic);
        break;
      }
      
      if (QS == true){     // A Heartbeat Was Found
        sendBPM(BPM);
        QS = false; // reset the Quantified Self flag for next time    
      }
      
      delay(100); //  take a break
    }
  }
  else
    delay(100);
}

void sendBPM(int rate) {
  char str[30];
  sprintf(str,"NHR,%d\n",rate);
  serialBT.print(str);
  //Serial.print(str);
  /*
  Serial.print("NHR,");
  Serial.print(rate);
  Serial.print("\n");
  Serial.print("NHR,");
  Serial.println(rate);
  */
}

void sendBloodPressure(int systolic, int diastolic) {
  char str[30];
  sprintf(str,"NBP,%d,%d\n",30+systolic,diastolic);
  serialBT.print(str);
  //Serial.print(str);
  /*
  Serial.print("NBP,");
  Serial.print(30+systolic);
  Serial.print(",");
  Serial.print(diastolic);
  Serial.print("\n");
  Serial.print("NBP,");
  Serial.print(30+systolic);
  Serial.print(",");
  Serial.println(diastolic);
  */
}

void sendSweat(int sweat) {
  serialBT.print("NS,");
  serialBT.print(sweat);
  serialBT.print("\n");
}

boolean checkForNewInput(char * string) {
  if (serialBT.available() > 0) {
    int index = 0;
    char inChar;
    while (serialBT.available() > 0) {
      inChar = serialBT.read();
      if (inChar == '\n' || inChar == '\r' || index >= 29)
        break;
      string[index++] = inChar;
    }
    string[index] = '\0';
    while (serialBT.available() > 0)
      serialBT.read();
    return 1;
  }
  else {
    return 0;
  }
}

