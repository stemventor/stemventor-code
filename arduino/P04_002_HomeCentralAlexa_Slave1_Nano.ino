/* 
 *  P04_002_HomeCentralAlexa_Slave1_Nano
 *  Author: AdvantIoT Development Team
 *  Authored on: 11-Jan-2019
 *  Last Modified on: 17-Jan-2020
 *  
 *  This is the complete sketch required for the HomeCentralAlexa_Slave1_Nano circuit which has the following:
 *  - A MQ35 gas sensor
 *  - A buzzer that sounds when the gas sensor detects gas
 *  - A soil moiture sensor
 *  - Green and Red LEDs that light up depending on the soil moisuture reading
 *  
 *  Since serial output is what the RPi will push to the MQTT broker, restrict the output to data in expected format
 *  Don't do any pre-processing here since the app will have the threhsolds configured, send sensor values as read
 *  Prefix debug or info messages with DEBUG: or INFO: - A Node-RED function will parse this and display them in the Debug panel
 *  while the rest will be sent to the MQTT queue assuming they are sensor values.
 */
  
/*  
 * The Nano needs additional drivers as it has an FTDI chip:
 * https://www.ftdichip.com/Drivers/VCP.htm
 * 
 * Select Board: Arduino Nano
 * Select Processor: ATmega328P (Old Bootloader)
 * Select Port: /dev/cu.wchusbserial1420
 */

/*
 *  LIBRARIES
 *  LIbraries need to be installed using the Tools -> Manage Libraries option and then installed
 *  Some libraries are available with the IDE while others may need to be downloaded and installed form a .zip file 
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "DHT.h" //For DHT11 functions needed to get the humidity and temperature readings from the sensor. 
//Using the Adafruit library, needs its sensor-master library as well. Install using the IDE Manage Libraries... option.

/*
 * PIN DEFINITIONS
 */

//For a Nano, only pins 2 and 3 support interrupts
//For Motion Sensor using an ISR (Interrup Service Request)
#define PIR_PIN_DIGITAL 2
//#define UNUSED_ISR_PIN 3

//For MQ2 or MQ# Gas Sensor
#define MQx_PIN_ANALOG A4 //Needs an analog pin
//Switched on or off based on a threshold defined by pot adjustment on the sensor - not used, 
//thresholds and actions are controlled in software based on analog signal values
//#define MQx_PIN_DIGITAL 4 //Not required

//For LM393 Soil Moisture Sensor
#define LM393_PIN_ANALOG A5
//#define LM393_PIN_DIGITAL 5 //Not required

//For Buzzer and LEDs
#define BUZZER_PIN 6
//Use PWM pins for LEDS
//PWM Pins: 3, 5, 6, 9, 10, 11, PWM Frequency: 490 Hz (pins 5 and 6: 980 Hz)

#define LED_RED_PIN 9
#define LED_YELLOW_PIN 10
#define LED_GREEN_PIN 11
 
//For nRF24L01 Radio Module - NOT USED FOR NOW
//#define CE_PIN 9
//#define CSN_PIN 10

//For LM35 Temperature Sensor - NOT USED FOER NOW
//#define LM35_PIN_ANALOG A0 //Needs an analog pin

float lm393_value; //LM393 digital data pin value
//float dht_value; //DHT11 digital data pin value


/*
 * VARIABLE DEFINITIONS
 */
//Keeping all variables global so they are not reset on every function call
float mqx_value; //MQx analog data pin value
//Timer variables
unsigned long last_gas_reading_millis;
unsigned long last_soilmoisture_reading_millis;
const unsigned long gas_reading_freq = 15*1000;
const unsigned long soilmoisture_reading_freq = 15*1000;
int urgency;  
//Thresholds
int gas_threshold = 300;
int soilmoisture_threshold = 500;
boolean start_motion_hold = false;

//Time for which motion sensor actiavted device is kept on
const unsigned long motion_sensor_hold = 15*1000;
unsigned long last_motion_trigger;
/* 
 * RF device pipe addresses:
 * Please see P01_001_HomeCentral_Master_ESP32.ino for a detailed explanation
 */

// To write to Master node and Slave 2
const uint64_t writing_pipes[2] = { 0xF0F0F0A1A1LL, 0xF0F0F0A3A2LL };
// To read from Master node and Slave 2
const uint64_t reading_pipes[2] = { 0xF0F0F0A2A1LL, 0xF0F0F0A2A2LL };

// Allocate space for incoming and outgoing data
char RFSerial_data_in[32]; //For data received via RF from master and other slaves (max 32 bytes)
char RFSerial_data_out[32]; //For data to be sent via RF to master and other slaves (max 32 bytes)

// Temperature reading
float temperature_value = 0;
float last_temperature_value = 0; //Use to check if there is a change in reading

// Use "unsigned long" for variables that hold time as the value can become too large for an int
unsigned long temperature_last_read = 0; // The last time the temperature was read
const uint32_t temperature_read_delay = 30000; //5 minutes   

//LCD Text buffers, 20 characters is max display anyway
char lcd_text1[20] = "";
char lcd_text2[20] = "";
char lcd_text3[20] = "";
char lcd_text4[20] = "";

/*
 * INSTANCE/OBJECT DEFINITIONS
 */

/*
 * FUNCTIONS
 */

//Sound buzzer intermittently based on urgency which is determined by the calling function
//As that is where you can determine urgency better. 
//Urgency is a value for seconds between buzzing
//Timer variables
unsigned long start_buzzing_millis;
unsigned long stop_buzzing_millis;
boolean started_buzzing = false;
boolean stopped_buzzing = false;

void soundBuzzer(int urgency){
  unsigned long current_millis; //Keep local since needs to be update every call
  int buzz_time = 1*1000; //same for all urgencies
  int quiet_time = urgency*1000; //higher for low urgencies
  
  if(urgency == 0){ //if urgency == 0 stop buzzer
    digitalWrite(BUZZER_PIN, LOW);
  }
  else{
    current_millis = millis();
    if(current_millis - start_buzzing_millis > buzz_time){
      digitalWrite(BUZZER_PIN, LOW);
      if(!stopped_buzzing) stop_buzzing_millis = millis();
      stopped_buzzing = true;
      started_buzzing = false;
    }
    //Do not put an else, both conditions need to be checked every time to enable the toggle
    if(current_millis - stop_buzzing_millis > quiet_time){
      digitalWrite(BUZZER_PIN, HIGH); //Keep buzzing for the urgency interval
      if(!started_buzzing) start_buzzing_millis = millis();
      started_buzzing = true;
      stopped_buzzing = false;
    }
  }
}

//Get reading at a set frequency, not every time called
void getGasReading(){
  unsigned long current_millis; //Keep local since needs to be update every call
  char string_g[8] = "\0"; //To store gas reading float value converted to string, init to null
  char mqtt_string_g[10] = "\0"; //For message to be sent to MQTT broker, init to null
  char debug_string[50] = "\0"; //For debug serial print as one string
  
  current_millis = millis();
  if(current_millis - last_gas_reading_millis > gas_reading_freq){
    //Do everything for gas reading, LED lights and buzzer    
    mqx_value = analogRead(MQx_PIN_ANALOG); // read analog input pin A0
    dtostrf(mqx_value, -7, 2, string_g); //(float source, char length including point - negative left aligns the string, decimal places, char[] target)

    //Since serial output is what the Node-RED on RPi will push to the MQTT broker, restrict the output to data in expected format
    //Don't do any pre-processing here since the app will have the threhsolds configured, send sensor values as read
    //Construct and print the MQTT message - goes to broker via Raspberry Pi serial input
    strcat(mqtt_string_g, "G|");
    strcat(mqtt_string_g, string_g);  
    Serial.println(mqtt_string_g);

    //Print to serial with INFO, DEBUG or ERROR prefix, Node-RED will only print these to Debug console 
    strcat(debug_string, "INFO:Gas Sensor Value: ");
    strcat(debug_string, string_g);
    
    if(mqx_value > gas_threshold)
    {
      strcat(debug_string, "|Gas detected!");
      digitalWrite(LED_RED_PIN, HIGH);  
    }else{
      strcat(debug_string, "|Clear");
      digitalWrite(LED_RED_PIN, LOW);
    }
    Serial.println(debug_string);
    //Done everything for gas reading
    
    last_gas_reading_millis = millis();
  }

  //Determine buzzer urgency (seconds between buzzing) based on how higher than threshold the value is
  if(mqx_value < gas_threshold) urgency = 0; //Stop sounding the buzzer
  if(mqx_value > gas_threshold && mqx_value < 400) urgency = 3; 
  if(mqx_value > 400 && mqx_value < 500) urgency = 2;
  if(mqx_value > 500) urgency = 1;
  soundBuzzer(urgency);
}

//Get soil moisture reading at a set frequency, not every time called
//LM393 is a soil moisture sensor
void getSoilMoistureReading(){
  unsigned long current_millis; //Keep local since needs to be update every call
  char string_s[8] = "\0"; //To store soil moisture float value converted to string
  char mqtt_string_s[10] = "\0";
  char debug_string[50] = "\0"; //For debug serial print as one string
  
  current_millis = millis();
  if(current_millis - last_soilmoisture_reading_millis > soilmoisture_reading_freq){
    //Do everything for gas reading, LED lights and buzzer    
    lm393_value = analogRead(LM393_PIN_ANALOG); // read analog input pin A0
    dtostrf(lm393_value, -7, 2, string_s); // //(float source, char length including point - negative left aligns the string, decimal places, char[] target)

    //Since serial output is what the RPi will push to the MQTT broker, restrict the output to data in expected format
    //Don't do any pre-processing here since the app will have the threhsolds configured, send sensor values as read
    //Construct and print the MQTT message - goes to broker via Raspberry Pi serial input
    strcat(mqtt_string_s, "S|");
    strcat(mqtt_string_s, string_s);  
    Serial.println(mqtt_string_s);

    //Print to serial with INFO, DEBUG or ERROR prefix, Node-RED will only print these to Debug console 
    strcat(debug_string, "INFO:Soil Moisture Sensor Value: ");
    strcat(debug_string, string_s);
    
    if(lm393_value > soilmoisture_threshold)
    {
      strcat(debug_string, "|Plants need water!");
      digitalWrite(LED_YELLOW_PIN, HIGH);  
    }else{
      strcat(debug_string, "|Plants are ok!");
      digitalWrite(LED_YELLOW_PIN, LOW);
    }
    Serial.println(debug_string);
    //Done everything for soil moisture reading
    
    last_soilmoisture_reading_millis = millis();
  }

  //Determine buzzer urgency (seconds between buzzing) based on how higher than threshold the value is
  if(mqx_value < gas_threshold) urgency = 0; //Stop sounding the buzzer
  if(mqx_value > gas_threshold && mqx_value < 400) urgency = 3; 
  if(mqx_value > 400 && mqx_value < 500) urgency = 2;
  if(mqx_value > 500) urgency = 1;
  soundBuzzer(urgency);
}

/* For PIR Motion Sensor */
// Checks if motion was detected, sets LED HIGH and starts a timer
//ICACHE_RAM_ATTR void motionDetected() { //Use ICACHE_RAM_ATTR for NodeMCU/ESP8266, not required for Nano
void motionDetected() {
  //Since serial output is what the RPi will push to the MQTT broker, restrict the output to data in expected format
  //Don't do any pre-processing here since the app will have the threhsolds configured, send sensor values as read
  char mqtt_string_m[10] = "\0";
  strcat(mqtt_string_m, "M|");
  strcat(mqtt_string_m, "1"); //And send the 0 in motionStopped()
  Serial.println(mqtt_string_m);

  //Print to serial with INFO, DEBUG or ERROR prefix, Node-RED will only print these to Debug console 
  Serial.println("INFO:Motion detected!");
  digitalWrite(LED_GREEN_PIN, HIGH);  // turn LED ON
  
  start_motion_hold = true;
  last_motion_trigger = millis();
}

void motionStopped() {
  //Since serial output is what the RPi will push to the MQTT broker, restrict the output to data in expected format
  //Don't do any pre-processing here since the app will have the threhsolds configured, send sensor values as read
  char mqtt_string_m[10] = "\0";
  strcat(mqtt_string_m, "M|");
  strcat(mqtt_string_m, "0"); //And send the 1 in motionDetected()
  Serial.println(mqtt_string_m);

  //Print to serial with INFO, DEBUG or ERROR prefix, Node-RED will only print these to Debug console 
  Serial.println("INFO:Motion stopped!");
  digitalWrite(LED_GREEN_PIN, LOW);  // turn LED OFF
}

/*
 * Setup
 * The setup function runs once when you press reset or power the board.
 */ 

void setup() {
  Serial.begin(115200); //The default serial port for communication with serial monitor.
  analogReference(DEFAULT); 
  
  //Initialize digital pin LED_BUILTIN as an output. Can be used to indicate connection status.
  pinMode(LED_BUILTIN, OUTPUT);
  
  //pinMode(MQx_PIN_ANALOG, INPUT);
  pinMode(LM393_PIN_ANALOG, INPUT);

  //Set Buzzer and LED Pins for OUTPUT
  pinMode(PIR_PIN_DIGITAL, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN_DIGITAL), motionDetected, RISING);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  
  
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  
  Serial.println("DEBUG: Gas sensor warming up!");
  //delay(20000); // allow the MQ-6 to warm up - since a lot of delay is due to other setup, this may not be needed
}


/*
 * Loop
 * The loop function runs over and over again forever, 30 times a minute (check!)
 */ 
void loop() {  
  //The frequency for each reading is managed within the function so each can have its own
  getGasReading();
  getSoilMoistureReading();

  unsigned long current_millis = millis(); //Keep local since needs to be update every call
  //Turn off the motion sensor LED after the number of seconds defined in the motion_sensor_hold variable
  if(start_motion_hold && (current_millis - last_motion_trigger > (motion_sensor_hold))) {
    //Serial.println(digitalRead(PIR_PIN_DIGITAL));
    if(digitalRead(PIR_PIN_DIGITAL) == LOW){
      motionStopped();
      start_motion_hold = false;
    }
  }
}

/*
 * END of sketch
 */
