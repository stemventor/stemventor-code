/* 
 *  P03_001_HomeCentral_Slave1_Nano
 *  Author: AdvantIoT Development Team
 *  Authored on: 11-Jan-2019
 *  Last Modified on: 06-Apr-2019
 *  
 *  This is the complete sketch required for the HomeCentral_Slave2 circuit and is assembled in the following steps:
 *  - First assemble the Nano and an NRF24L01 radio module exactly as in P02_001 and P02_002. Then:
 *  - P03_001_HomeCentral_Slave2_Relay.fzz: Adding a 4-channel relay with connected devices to be turned on and off.
 *  - P03_002_HomeCentral_Slave2_MotionSensor.fzz: Adding a motion sensor to turn on a light when motion is detected and send data to the master.
 *  - P03_003_HomeCentral_Slave2_SoundSensor.fzz: Adding a sound sensor to turn on a light when sound is detected and send data to the master.
 *  
 *  Refer to the comments to see which sections of code are required at each step.
 *  
 */
  
/*  
 * The Nano needs additional drivers as it has an FTDI chip:
 * https://www.ftdichip.com/Drivers/VCP.htm
 * 
 * Select Board: Arduino Nano
 * Select Processor: ATmega328P (Old Bootloader)
 * Select Port: /dev/cu.wchusbserial1420 
 * 
 * Arduino with PIR motion sensor
 * For complete project details, visit: http://RandomNerdTutorials.com/pirsensor
 * https://randomnerdtutorials.com/esp32-pir-motion-sensor-interrupts-timers/
 * Modified by Rui Santos based on PIR sensor by Limor Fried
 *  
 * For connecting a Relay
 * https://randomnerdtutorials.com/guide-for-relay-module-with-arduino/
 * 
 */

/*
 *  LIBRARIES
 *  LIbraries need to be installed using the Tools -> Manage Libraries option and then installed
 *  Some libraries are available with the IDE while others may need to be downloaded and installed form a .zip file 
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/*
 * PIN DEFINITIONS
 */
 
/* For nRF24L01 */
#define CE_PIN 9
#define CSN_PIN 10

#define MOTION_SENSOR 2
#define SOUND_SENSOR 3

//4-Channel Relay Channels
#define CHANNEL_ONE 2 //Light controlled by Motion sensor
#define CHANNEL_TWO 9 //Fan controlled by Sound sensor
#define CHANNEL_THREE 10 //Something controlled by user inpput from app
#define CHANNEL_FOUR 11 //Something controlled by user inpput from app

/*
 * VARIABLE DEFINITIONS
 */ 

/* 
 * RF device pipe addresses:
 * Please see P01_001_HomeCentral_Master_ESP32.ino for a detailed explanation
 */

// To write to Master node and Slave 1
const uint64_t writing_pipes[2] = { 0xF0F0F0A1A2LL, 0xF0F0F0A2A2LL };
// To read from Master node and Slave 2 */
const uint64_t reading_pipes[2] = { 0xF0F0F0A3A1LL, 0xF0F0F0A3A2LL };

// Allocate space for incoming and outgoing data
char RFSerial_data_in[32]; //For data received via RF from master and other slaves (max 32 bytes)
char RFSerial_data_out[32]; //For data to be sent via RF to master and other slaves (max 32 bytes)

// To track the on/off status of the device connected to each channel
boolean channel1_on = false;
boolean channel2_on = false;
boolean channel3_on = false;
boolean channel4_on = false;

// For each channel track last change, do not allow changes in a time period less than the threshold
unsigned long channel1_last_change = 0;
unsigned long channel2_last_change = 0;
unsigned long channel3_last_change = 0;
unsigned long channel4_last_change = 0;

//Thresholds for signals define for how long a signal should be acted upon.
//Frequent repeated motion or sound can be ignored to prevent rapid actions.
//These can be used in conjunction with channel thresholds.
//These thresholds can be used to send data to the master and the app as user information
//And the channel thresholds can be used to decice whether or not to change the device state
const unsigned long motion_threshold = 60000; //in millis so 60 seconds
const unsigned long sound_threshold = 60000; //in millis so 60 seconds

//Thresholds for each channel will allow control on how frequently the connected device can be turned on or off
//For devices that have a high power consumption during starting or have parts that need time between restarts, should be switched on and off less frequently
const unsigned long channel1_threshold = 600000; //in millis so 10 minutes
const unsigned long channel2_threshold = 600000; //in millis so 10 minutes
const unsigned long channel3_threshold = 600000; //in millis so 10 minutes
const unsigned long channel4_threshold = 600000; //in millis so 10 minutes

//Timer
unsigned long now = millis();
unsigned long last_motion_detected = 0;
unsigned long last_sound_detected = 0;

/*
 * INSTANCE/OBJECT DEFINITIONS
 */

RF24 radio(CE_PIN,CSN_PIN); //The client object for RF communication

/*
 * Function to initialize radio communication.
 */
 
void initRadio() {
  /* Initialize RF communidation */
  radio.begin();

  /* Set power level of the radio */
  radio.setPALevel(RF24_PA_LOW);
  //radio.setPALevel(RF24_PA_MIN);
  /* Set RF data rate - lowest rate for longest range capability */
  radio.setDataRate(RF24_250KBPS);
  /* Set radio channel to use - ensure all slaves match this */
  radio.setChannel(0x66);
  /* Set time between retries and max retries */
  radio.setRetries(4, 10);
  /* Enable ackpayload - enables each slave to reply with data. Not used. */
  //radio.enableAckPayload();

  /* Open reading pipes and start listening */
  radio.openReadingPipe(1, reading_pipes[0]); //Listen to Master node
  radio.startListening();
}

/*
 * Function to send a radio signal.
 */

void sendToRFNode(uint8_t node_id, char node_data[32]) {

  /* 
   * For some reason we cannot use the char[] passed as a parameter
   * It must be copied into a local char[] 
   */
  char node_data_copy[32] = "";
  strcpy(node_data_copy, node_data);

  /* 
   * Since we have a dedicated writing pipe, no need to call stopListening().
   * Calling it anyway to be safe, ensure startListening() is called as required. 
   */
  radio.stopListening();

  /* For now, slaves nodes will only send data to the Master node */
  Serial.print(F("Sending to node: "));
  Serial.println(node_id);
  
  radio.openWritingPipe(writing_pipes[node_id-1]); //Subtract 1 from the node_id to get the right array index
  
  /* Send RF data */
  if (!radio.write( &node_data_copy, sizeof(node_data_copy) )){
    Serial.print(F("Failed: "));
    Serial.println(node_data_copy);
  }
  else{
    Serial.print(F("Sent: "));
    Serial.println(node_data_copy);
  }
}

/*
 * Function to receive a radio signal.
 */ 
void readFromRFNode() {

  uint8_t pipeNum;
  
  /* 
   * No need to open the reading pipe everytime
   * Unless you have switched to writing on the reading pipes no need to start listening. 
   * However, since, to be safe, stopListening() is being called when writing data
   * need to call startListening() here.
   */
  radio.startListening();
    
  /* 
   * Set up a timeout period, and break after the period...
   * so other listeners can be polled in the loop()  
   */

  /* Get the current time in microseconds */ 
  unsigned long started_waiting_at = micros();               
  /* Set up a variable to indicate if a response was received or not */
  boolean timeout = false;                                   

  /* While nothing is received */
  while ( !radio.available(&pipeNum) ){                             
    /* If waited longer than 200ms, indicate timeout and exit while loop */
    if (micros() - started_waiting_at > 200000 ){            
        timeout = true;
        break;
    }      
  }
  /* DEBUG: Print data to serial monitor */    
  if ( timeout ){                                             
      Serial.print(F("."));
  }else{
      /* Grab the response, compare, and send to debugging spew */
      char node_data[32];                                 
      radio.read( &node_data, sizeof(node_data) );

     /*
      * Node data will be in the format: pin|value
      * Since pin is configurable in the app, set it to match connections on the board
      * NOTE: strtok() inserts a '\0' after each token found thus the original string
      * if accessed, only shows the first token. Reconstruct the data from tokens.
      */
    
      char *pin = strtok (node_data,"|"); //The first call to strtok takes the string to parse
      char *value = strtok (NULL,"|"); //Subsequent calls take NULL

      //Since there is no feedback from the circuit to the app the signal will have to be actioned without any controls
      //If at all, thresholds or other controls may be maintained in the app
      digitalWrite(atoi(pin), atoi(value));
      
      //DEBUG
      unsigned long recv_time = micros();
      // Spew it
      Serial.println(F("."));
      Serial.print(F("Got message: "));
      Serial.print(node_data);
      Serial.print(F(", From pipe: "));
      Serial.print(pipeNum);
      Serial.print(F(", Received at: "));
      Serial.print(recv_time);
      Serial.println(F(" microseconds"));
  }
}

/*
 * Function to execute when an interrupt is triggered by the motion sensor
 * Checks if motion was detected, sets relay channel to HIGH and starts a timer
 * Keep the light on until motion is detected or for 5 minutes, manage that in loop()
 * Controls device attached to channel 1
 * 
 * Note: IRAM_ATTR is used to run the interrupt code in RAM, otherwise code is stored in flash and it’s slower. Only for ESP32
 */

void motionDetected() {
  // Current time
  now = millis();
  //Process signal only if apart more than threshold to filter continuous motion
  //Else ignore the signal, do nothing.
  if(now - last_motion_detected > motion_threshold) {
    //DEBUG
    Serial.println("Motion Detected.");
    last_motion_detected = millis();  

    //If time elapsed since last state change for the channel being controlled is more than defined threshold, change device state
    //Else do nothing to the channel but still send the signal to the master and the app
    if(now - channel1_last_change > channel1_threshold){
      //Send signal to channel
      //DEBUG
      Serial.println("Changing channel state to on");

      //If the device on channel_1 is not on, switch it on, else do nothing
      //It will be switched off in the loop() if no motion is detected for x minutes
      if(!channel1_on){
        Serial.println("Switching bulb on!");
        digitalWrite(CHANNEL_ONE, HIGH); //switch on the light bulb
        channel1_on = true;
      }
      channel1_last_change = millis();
    }
    else{
      //Send signal to master node
      //DEBUG
      Serial.println("Sending data to master.");
    } //End if-else channel change state threshold
  }//End if signal threshold
}

/*
 * Function to execute when an interrupt is triggered by the sound sensor
 * If a sharp sound is detected, relay channel 2 is toggled HIGH/LOW
 * Every sharp sound switches the fan on or off.
 * Controls device attached to channel 2
 * 
 * Note: IRAM_ATTR is used to run the interrupt code in RAM, otherwise code is stored in flash and it’s slower. Only for ESP32
 */

void soundDetected() {
  // Current time
  now = millis();
  //Process signal only if apart more than threshold to filter continuous sound
  //Else ignore the signal, do nothing.
  if(now - last_sound_detected > sound_threshold) {
    //DEBUG
    Serial.println("Sound Detected.");
    last_sound_detected = millis();  

    //If time elapsed since last state change for the channel being controlled is more than defined threshold, change device state
    //Else do nothing
    if(now - channel2_last_change > channel2_threshold){
      //Send signal to channel
      //DEBUG
      Serial.println("Changing channel 2 state.");

      uint8_t state = channel2_on?LOW:HIGH;
      digitalWrite(CHANNEL_TWO, state); //switch the light bulb on or off
      channel2_on = !channel2_on; //Toggle channel_2 state indicator
      
      channel2_last_change = millis();
    } //End if channel threshold
  }//End if signal threshold
}

/*
 * Setup
 * The setup function runs once when you press reset or power the board.
 */ 

void setup() {
  /* Initialize digital pin LED_BUILTIN as an output. Can be used to indicate connection status.*/
  pinMode(LED_BUILTIN, OUTPUT);

  //MOTION SENSOR
  //When the PIR senses activity in its viewing area, it pulls the signal pin low. 
  //But when the sensor is inactive, the pin is basically floating. 
  //To avoid any false-positives, the alarm output should be pulled high to 5V.
  pinMode(MOTION_SENSOR, INPUT_PULLUP);
  
  //Set MOTION_SENSOR pin as interrupt, assign interrupt function and set RISING mode, 
  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR), motionDetected, RISING);

  //SOUND SENSOR
  //Sound sensors are available in 3- and 4-pin configurations
  //A 3-pin outputs a digital signal, a 4-pin outputs a digital and analog signal
  //This code is for a 3-pin sensor

  //Not sure if SOUND_SENSOR also needs to be pulled up
  //pinMode(SOUND_SENSOR, INPUT_PULLUP);
  pinMode(SOUND_SENSOR, INPUT);

  //Set SOUND_SENSOR pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(SOUND_SENSOR), soundDetected, HIGH);

  
  Serial.begin(115200); //The default serial port for communication with serial monitor.

  //Initialize radio communication
  initRadio();
}


/*
 * Loop
 * The loop function runs over and over again forever with a delay of 10 seconds
 */ 
void loop() {
  /* 
   * Open the RF listening port and listen for a while then break 
   * so other things can be done in the loop()
   */
  readFromRFNode();

  //Switch off relay_channel1 if some time passes with no motion detected
  // Current time
  now = millis();

  if(now - last_motion_detected > (10*1000) && channel1_on) {
    digitalWrite(CHANNEL_ONE, LOW); //switch off the light bulb
    channel1_on = false;
    Serial.println("Motion stopped, switching bulb off.");
  }
}

/*
 * END of sketch
 */
