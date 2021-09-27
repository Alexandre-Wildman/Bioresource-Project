// Code for the TEMP and RH mapping with moving sensors

 
// Include Libraries
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include "DHT.h"
// RK previous version would #include <dht.h>

// Sampling Frequency
// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
unsigned long LOG_INTERVAL = 7000; //mills between entries (reduce to take more/faster data)
unsigned long LOG_INTERVAL_1 = 5000;
// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest and fastest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to
// the last 10 reads if power is lost but it uses less power and is much faster!
unsigned long SYNC_INTERVAL = 10*LOG_INTERVAL; // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

//const int DECISION_INTERVAL = 1000; // subject to change, Interval between checking conditions

#define PRINT_LABEL 0 // Print labels in serial output
#define LOG_DATA 1 //
#define POSITION_CHECK 0 //

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// define position values at start
int x_position=0;
int y_position=0;


// define pins
const int DHTPIN1 = 2;
const int DHTPIN2 = 3;
const int DHTPIN3 = 4;
const int START = 5;
const int RESET = 6;
const int greenLEDpin = 7;
const int startpin = 8;
const int chipSelect = 9;
const int stepPin = 10;
const int dirPin = 11;

// define the number of DHT to use
const int dhtcount = 3;
boolean TempState = true;
boolean HumdState = true;

// define x_count for stepper movement 500 is 10 cm,
int x_1=250;
int x_2=500;
int x_restart=1250;

// define state to operate
int state=0; // 0 not runing, 0 is runing


int TempInReading;
int HumdInReading;
// Define the sensors output, each sensor is unique
float TEMP0;
float TEMP1;
float TEMP2;
float RH0;
float RH1;
float RH2;


// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht0(DHTPIN1, DHTTYPE);
DHT dht1(DHTPIN2, DHTTYPE);
DHT dht2(DHTPIN3, DHTTYPE);



void setup() {
   // Define Pin type
  Serial.begin(9600);
 pinMode(DHTPIN1,INPUT);
 pinMode(DHTPIN2,INPUT);
 pinMode(DHTPIN3,INPUT);
 pinMode(startpin, INPUT);
 // use debugging LEDs if needed, could be readings indicator
 pinMode(greenLEDpin, OUTPUT);
  pinMode(stepPin,OUTPUT);
  pinMode(dirPin,OUTPUT);
  digitalWrite(stepPin,LOW);
  digitalWrite(dirPin,LOW);




  // connect to RTC, if needed
  Wire.begin();

#if PRINT_LABEL
  Serial.println("temp_0,temp_1,temp_2,rh_0,rh_1,rh_2");
#endif
// DL made 3 DHT, 1 for each sensor
  dht0.begin();
  dht1.begin();
  dht2.begin();
}
  // dht1.begin(); this was included in void set up at one point...

//// STUFF TO ENTER POSITION
//char rx_byte = 0;
//String rx_str = "";
//boolean not_number = false;
//int result;

void loop() {

  //  // delay for the amount of time we want between readings

  digitalWrite(greenLEDpin, HIGH);
  // Button for start signal
  if (digitalRead(startpin)== HIGH){
  state=1;
  }

  // Start the brooming
  while (state == 1){
  MOVE_1();
	x_position=0;

	//    if ((millis() - syncTime) < SYNC_INTERVAL){
	//  syncTime = millis();
	delay(LOG_INTERVAL_1);
	READ_SENSORS();
	LOG_WRITE();
	// }
	MOVE_2();
	x_position=1;
	//    if ((millis() - syncTime) < SYNC_INTERVAL){
	//  syncTime = millis();
	delay(LOG_INTERVAL);
	READ_SENSORS();
	LOG_WRITE();
	//}
	MOVE_2();
	x_position=2;
	//       if ((millis() - syncTime) < SYNC_INTERVAL){
	//  syncTime = millis();
	delay(LOG_INTERVAL);
	READ_SENSORS();
	LOG_WRITE();
	// }

	RESTART ();
	state=0;
	break;
	}

  #if POSITION_CHECK
  Serial.print(x_position);
  Serial.print(", ");
  Serial.print(y_position);
  #endif
  // Now we write data to computer via serial ! Don't sync too often, let time for sensor response to be stable


}
// Read Sensors loop
void READ_SENSORS() {

  // Get DHT22 Info
  // First Sensor

  TEMP0 = dht0.readTemperature();
  RH0 = dht0.readHumidity();

  // Second Sensor

  TEMP1 = dht1.readTemperature();
  RH1 = dht1.readHumidity();

  // Third Sensor

  TEMP2 = dht2.readTemperature();
  RH2 = dht2.readHumidity();
}


// Log Loop
void LOG_WRITE() {

  y_position=0;
  Serial.print(x_position);
  Serial.print(", ");
  Serial.print(y_position);
  Serial.print(", ");
  Serial.print(TEMP0);
  Serial.print(", ");
  Serial.print(RH0);
  Serial.println();
  y_position++;
  Serial.print(x_position);
  Serial.print(", ");
  Serial.print(y_position);
  Serial.print(", ");
  Serial.print(TEMP1);
  Serial.print(", ");
  Serial.print(RH1);
  Serial.println();
  y_position++;
  Serial.print(x_position);
  Serial.print(", ");
  Serial.print(y_position);
  Serial.print(", ");
  Serial.print(TEMP2);
  Serial.print(", ");
  Serial.print(RH2);
  Serial.println();

}

void ENTER_STATE (){
  if (Serial.available() > 0) {	// is a character available?
	state = Serial.parseInt();
	Serial.println(x_2);
  }
}
void MOVE_1 (){
  digitalWrite(dirPin,HIGH); // Enables the motor to move in a particular direction
  for(int x = 0; x < x_1; x++) {
	digitalWrite(stepPin,HIGH);
	delayMicroseconds(500);
	digitalWrite(stepPin,LOW);
	delayMicroseconds(500);
  }
  delay(1000); // One second delay
}
void MOVE_2 (){
  digitalWrite(dirPin,HIGH); // Enables the motor to move in a particular direction
  for(int x = 0; x < x_2; x++) {
	digitalWrite(stepPin,HIGH);
	delayMicroseconds(500);
	digitalWrite(stepPin,LOW);
	delayMicroseconds(500);
  }
  delay(1000); // One second delay
}
void RESTART (){
	digitalWrite(dirPin,LOW); //Changes the rotations direction
  // Makes 400 pulses for making two full cycle rotation
  for(int x = 0; x < x_restart; x++) {
	digitalWrite(stepPin,HIGH);
	delayMicroseconds(500);
	digitalWrite(stepPin,LOW);
	delayMicroseconds(500);
  }
  delay(1000);
}
