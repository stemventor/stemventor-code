/* 
 *  P02_001_HomeCentral_Slave1_Nano
 *  Author: AdvantIoT Development Team
 *  Authored on: 11-Jan-2019
 *  Last Modified on: 23-Mar-2019
 *  
 *  This is the complete sketch required for the HomeCentral_Slave1 circuit and is assembled in the following steps:
 *  - P02_001_HomeCentral_Slave1_Nano.fzz: Getting the Nano setup and buzzing a buzzer.
 *  - P02_002_HomeCentral_Slave1_NRF24L01.fzz: Adding a radio module and sending and receiving data to the master and other slaves.
 *  - P02_003_HomeCentral_Slave1_GasSensor.fzz: Adding a gas sensor to buzz when gas is detected and send data to the master.
 *  - P02_004_HomeCentral_Slave1_TemperatureSensor.fzz: Adding a temperature sensor to read temperature data and send it to the master. 
 *  
 *  Refer to the comments to see which sections of code are required at each step.
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

/*
 * PIN DEFINITIONS
 */
 
//For nRF24L01
#define CE_PIN 9
#define CSN_PIN 10

// For LM35
#define TEMPERATURE_PIN A7

/*
 * VARIABLE DEFINITIONS
 */

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

/*
 * INSTANCE/OBJECT DEFINITIONS
 */

RF24 radio(CE_PIN,CSN_PIN); //The client object for RF communication

/*
 * Function to initialize radio communication.
 */
 
void initRadio() {
  // Initialize RF communidation
  radio.begin();
  
  // Set power level of the radio
  radio.setPALevel(RF24_PA_LOW);
  //radio.setPALevel(RF24_PA_MIN);
  //Set RF data rate - lowest rate for longest range capability
  radio.setDataRate(RF24_250KBPS);
  // Set radio channel to use - ensure all slaves match this
  radio.setChannel(0x66);
  // Set time between retries and max retries
  radio.setRetries(4, 10);
  // Enable ackpayload - enables each slave to reply with data. Not used.
  //radio.enableAckPayload();

  // Open reading pipes and start listening
  radio.openReadingPipe(1, reading_pipes[0]); //Listen to Master node
  radio.openReadingPipe(2, reading_pipes[1]); //Listen to Slave node 2
  radio.startListening();
}

/*
 * Function to send a radio signal.
 */

void sendToRFNode(uint8_t node_id, char node_data[32]) {
  
  //For some reason we cannot use the char[] passed as a parameter
  //It must be copied into a local char[] 
  char node_data_copy[32] = "";
  strcpy(node_data_copy, node_data);

  //Since we have a dedicated writing pipe, no need to call stopListening().
  //Calling it anyway to be safe, ensure startListening() is called as required. 
  radio.stopListening();
  
  // For now, slaves nodes will only send data to the Master node
  Serial.print(F("Sending to node: "));
  Serial.println(node_id);
  
  radio.openWritingPipe(writing_pipes[node_id-1]); //Subtract 1 from the node_id to get the right array index
  
  // Send RF data
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
  
   
   //No need to open the reading pipe everytime
   //Unless you have switched to writing on the reading pipes no need to start listening. 
   //However, since, to be safe, stopListening() is being called when writing data
   //need to call startListening() here.
  radio.startListening();
  
   //Set up a timeout period, and break after the period...
   //so other listeners can be polled in the loop()  

  //Get the current time in microseconds
  unsigned long started_waiting_at = micros();               
  //Variable to indicate if a response was received or not
  boolean timeout = false;                                   

  //While nothing is received
  while ( !radio.available(&pipeNum) ){                             
    //If waited longer than 200ms, indicate timeout and exit while loop
    if (micros() - started_waiting_at > 200000 ){            
        timeout = true;
        break;
    }      
  }
  // DEBUG: Print data to serial monitor
  if ( timeout ){                                             
      //Serial.print(F("."));
  }else{
      // Grab the response, compare, and send to debugging spew
      char node_data[32];                                 
      radio.read( &node_data, sizeof(node_data) );
      
      unsigned long recv_time = micros();
      
      // Spew it to console for debugging
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
 * Function to read Temperature Sensor (LM35) data.
 * Returns temperature value in C or F as type float
 */ 

float readTemperature(uint8_t unit) {
  // read the input on analog pin 7
  int8_t sensor_value = analogRead(TEMPERATURE_PIN);

  //Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V): 
  //The LM35 formula return temp in Celcius
  float temperature_value = sensor_value * (5.0 / 1023.0) * 100; 
    
  // If temperature required in Farenheit
  if(unit == 'F'){
    temperature_value = (9/5)*temperature_value + 32;  
  } 
    
  return temperature_value;
}

/*
 * Function to send temprature data to Master node, calls readTemperature()
 */
void readAndSendTemperature() {
   //Get temperature in Celcius, pass 'F' if required in Farenheit 
   temperature_value = readTemperature('C');
   
   //Use millis() instead of delay() to get data at a defined frequency
   //Also check if the current and last readings are close, don't bother to send then
   unsigned long current_time = millis();

   float reading_difference = last_temperature_value - temperature_value;
     
   if (current_time - temperature_last_read >= temperature_read_delay
       &&  abs(reading_difference) > 0.5) {
    // Update the last time the temperature was read and its value
    temperature_last_read = current_time;
    last_temperature_value = temperature_value;
    
    //Add topic and parameter code information and send the data to the Master Node 
    //Format will be topic|parameter|value
    //For now, only one parameter is supported.
    //This will be parsed by the Master sketch and published to the MQTT broker
    //A datetime stamp may be added by the Master if a Real Time Clock (RTC) module is used
    
    char temperature_data[32] = "";

    //Construct a pipe separated value string
    strcat(temperature_data, "hc/st1|"); //hc/st1 is the MQTT broker queue name
    strcat(temperature_data, "T|"); //T is the parameter

    //Need to use dtostrf(float, width, precision, buffer) to convert float to char[]
    char buffer[6]; //width is minimum width for display, 0 for dynamic, precision=1 for one digit after decimal, 
    dtostrf(temperature_value, 0, 1, buffer);
    //Code to trim a char[]

    //char *p = strchr(buffer, ' ');  // search for space
    //if (p) // if found truncate at space
    //  *p = '\0';

    strcat(temperature_data, buffer);
    // DEBUG: print value to serial monitor
    Serial.println(temperature_data);

    //Send data to Master Node (id = 1, array offset = 0)
    sendToRFNode(1, temperature_data);
  }
}

/*
 * Setup
 * The setup function runs once when you press reset or power the board.
 */ 

void setup() {
  //Initialize digital pin LED_BUILTIN as an output. Can be used to indicate connection status.
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(TEMPERATURE_PIN, INPUT);

  Serial.begin(115200); //The default serial port for communication with serial monitor.

  //Initialize radio communication
  initRadio();
}


/*
 * Loop
 * The loop function runs over and over again forever with a delay of 10 seconds
 */ 
void loop() {
   //Open the RF listening port and listen for a while then break 
   //so other things can be done in the loop()
  readFromRFNode();

  //Read temperature, construct the data string and send to master node, at a defined frequency
  readAndSendTemperature();
}

/*
 * END of sketch
 */
