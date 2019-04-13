/*  Heart sensor code is based on: Pulse Sensor Amped 1.4 by Joel Murphy and Yury Gitman of http://www.pulsesensor.com
 *  For the original Pulse Sensor Amped code and license of said code, please see: https://github.com/WorldFamousElectronics/PulseSensor_Amped_Arduino/blob/master/LICENSE
 */

 /*
  * This edited and extended version of code was written by Gerke van Garderen, https://gerkevangarderen.nl/
  * It was written as an extension of beformentioned code, to add stress sensing through monitoring of Heart Rate Variability (HRV)
  * 
  * Note: because the sensor code makes use of Timer2, pins 3 and 11 lose PWM-functionality.
  * 
  * This code was modified by Leigh Yeh for SELF-. 
  */

#define LOG_OUT 1 // use the log output function
#define FHT_N 32 // set to 256 point fht

#include <FHT.h> // include the library for Fourier transform
#include <Ethernet.h>
#include <SPI.h>
#define vibrationPin 5

long startTime;
// CHANGE IF YOU WANT DIFFERENT TIME INTERVAL 
long   minutes = 60000 * 5;

boolean vibing = true; //checks whether vibration is currently on
boolean silence = false;
long vibeTimeSet = 500;
long vibeTimer = 0;

float HF=0;
float LF=0;
float LFHF,LFHFOld;
float P = 0;

//  Variables
//int sensorVcc = 12;
int pulsePin = A0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

int data[32];
int dataCount = 0;
char test[2];
boolean once = false;


/******* USER RELATED VARS ***********/
// HRV variables (numbers are LF/HF)
// min variables are mean - (2 * SD)
// max variables are mean + (2 * SD)
float YM_min = -3.61;
float YM_max = 10.27;
float YM_avg = 3.33;

float YF_min = -2.01;
float YF_max = 6.19;
float YF_avg = 2.09;

float EM_min = -3.83;
float EM_max = 12.41;
float EM_avg = 4.29;

float EF_min = -3.11;
float EF_max = 8.61;
float EF_avg = 4.29;

int age = 0;
// zero is female, one is male
int sex = 0;
bool athlete = false;


/*********** ETHERNET *************/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 0, 0, 20); 
//byte ip[] = { 192, 168, 2, 201 };   // static ip of Arduino
//byte gateway[] = { 192, 168, 2, 254 };  // gateway address
//byte subnet[] = { 255, 255, 255, 0 };  //subnet mask
EthernetServer server(80);   //web server port
String HTTP_req;

//  Decides How To OutPut BPM and IBI Data
void serialOutputWhenBeatHappens();
void interruptSetup();
void getInfo();

void setup(){
 // pinMode(sensorVcc,OUTPUT);
 // digitalWrite(sensorVcc,HIGH);
  
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(4, OUTPUT);         // disable SD card
  digitalWrite(4, HIGH);     // disable SD card

  Serial.begin(115200);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
  Ethernet.begin(mac);
  
  // users age
  Serial.println("What is your age?");
  while (Serial.available() == 0) { }
  age = Serial.parseInt();

  // users sex
  Serial.println("What is your sex? Input 1 for \"Female\", 2 for \"Male\", or 3 for \"I'd rather not say\". ");
  while (Serial.available() == 0) { }
  sex = Serial.parseInt();

  startTime = millis();
  
}

void runEthernet() {
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
         if (QS == true)  { // A Heartbeat Was Found, BPM and IBI have been Determined. Quantified Self "QS" is set to true when arduino finds a heartbeat
          // serialOutputWhenBeatHappens();  // A Beat Happened, Output that to serial. 
          Fourier();                      // Run the Fourier code to calculate HF/LF    
          QS = false;                     // reset the Quantified Self flag for next time    
         }
         delay(10);
         client.print(F("<br>LF/HF ratio "));
         client.println(LFHF);
      }
    }
  }
}

//  Where the Magic Happens
void loop(){
  // can change - set at 5 minutes 
  if (millis() - startTime <= minutes) {
    fourier_score();
    startTime = millis();
  }
  else {
    runFourier();
  }
}
