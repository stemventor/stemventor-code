/* 
 *  P01_004_HomeCentral_Master_NRF24L01
 *  Author: AdvantIoT Development Team
 *  Authored on: 11-Jan-2019
 *  Last Modified on: 06-Apr-2019
 *  
 *  This sketch is primarily responsible for handling the communication between various devices
 *  and via various protocols as below:
 *  1. ESP32 Master to Arduino Nano Slaves (bi-directional) via RF (using the NRF24L01 breakout board)
 *  2. Android App on phone to ESP32 Master via Bluetooth (using the HC-05 breakout board)
 *  3. ESP32 Master to MQTT Broker via WiFi (using the on-board WiFi of the ESP32)
 *  
 *  This is the complete sketch required for the HomeCentral_Master circuit and is assembled in the following steps:
 *  - P01_001_HomeCentral_Master_ESP32.fzz: Getting the ESP32 setup with a WiFi connection and blinking its on-board LED.
 *  - P01_002_HomeCentral_Master_I2CLCD.fzz: Adding an I2C LCD and displaying status information.
 *  - P01_003_HomeCentral_Master_HC05.fzz: Adding an external bluetooth module and reading data sent from the app.
 *  - P01_004_HomeCentral_Master_NRF24L01.fzz: Adding a radio module and sending and receiving data to a slave 
 *    (needs P02_002_HomeCentral_Slave1_NRF24L01.fzz and P02_002_HomeCentral_Slave2_NRF24L01.fzz, which are exactly the same circuits)
 *  
 *  Refer to the comments to see which sections of code are required at each step.
 */

/*
 *  LIBRARIES  
 */

/* For RF communication using the nRF24L01: 
 * TMRh20/RF24, https://github.com/tmrh20/RF24/
 * 
 * For ESP32 the RF24 library needs a small modification:
 * https://github.com/nRF24/RF24/issues/393
 * 
 * In file RF24_config.h add the following code just after line 135:
 * 
 * #elif defined (ESP32)
 * #include <pgmspace.h>
 * #define PRIPSTR "%s"
 * 
 */
 
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/* To use the ESP32 on-board WiFi */ 
#include <WiFi.h>

/* 
 *  For writing to an MQTT broker 
 *  https://pubsubclient.knolleary.net/index.html
 */
#include <PubSubClient.h>

/* For Classic Bluetooth 
 * The ESP32 on-board WiFi and Bluetooth are not working well simultaneously, so using HC-05 for Bluetooth
 * This library is required if using the ESP32 on-board Bluetooth
 */
/*#include <BluetoothSerial.h>*/

/* For JSON parsing - DEPRECATED as a simple pipe separated string format is used*/
/*#include <libraries/ArduinoJson-v5.13.3.h>*/
/*#include <ArduinoJson.h>*/

/* For the I2C LCD display
 * https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home#!downloading-and-installation 
 * Note that this has its own LiquidCrystal.h which may conflict with the default LiquidCrystal.h that comes with the Arduino IDE
 * Rename the default to ensure the correct library is used.
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/*
 * PIN DEFINITIONS
 */
 
/* For nRF24L01 */
#define CE_PIN 26
#define CSN_PIN 27

/* For HC-05 use hardware serial interface UART2
 * On the ESP32, you can configure any GPIO pin to act as hardware serial interface pins.
 * There are however three pre-defined pin pairs: 
 * 
 * UART0 (RX: 3, TX: 1) is used to communicate with the ESP32 for programming and during reset/boot.
 * UART1 (RX: 9, TX: 10) is not exposed on some boards.
 * UART2 (RX: 16, TX: 17) is unused and can be used for your projects.
 */
#define RXD2 16
#define TXD2 17

/*
 * Constant definitions
 */

#define BTSerial Serial2 //Makes it clear what Serial2 is used for

/*
 * VARIABLE DEFINITIONS
 */

/* 
 * RF device pipe addresses:
 * Read http://tmrh20.blogspot.com/2016/08/rf24-addressing-review-of-nrf24l01.html
 * Up to 6 pipes can be open for reading at once. 
 * Open all the reading pipes, and then call startListening().
 * Pipe 0 doubles as the default writing pipe. 
 * So if you open pipe 0 for reading, do an openWritingPipe() again before write().
 * To be safe, use only pipes 1-5 for reading.
 * 
 * Addresses are assigned via a 5-byte array
 * For a radio, pipess 1-5 should have addresses that differ only in the least significant (right-most) byte. 
 *
 * READING PIPES:
 * Radio 1 (Master): F0F0F0A1A1, F0F0F0A1A2
 * Radio 2 (Slave 1): F0F0F0A2A1, F0F0F0A2A2 
 * Radio 3 (Slave 2): F0F0F0A3A1, F0F0F0A3A2
 * 
 * WRITING PIPES:
 * Radio 1 (Master) to Radio 2 (Slave 1): F0F0F0A2A1
 * Radio 1 (Master) to Radio 3 (Slave 2): F0F0F0A3A1
 * Radio 2 (Slave 1) to Radio 1 (Master): F0F0F0A1A1
 * Radio 3 (Slave 2) to Radio 1 (Master): F0F0F0A1A2
 * 
 * This can continue from any radio to any other radio
 * Prefix HEX values with 0x and suffix with LL to store as unit64_t
 */

/* To write to Slave 1 and Slave 2 */
const uint64_t writing_pipes[2] = { 0xF0F0F0A2A1LL, 0xF0F0F0A3A1LL };
/* To read from Slave 1 and Slave 2 */
const uint64_t reading_pipes[2] = { 0xF0F0F0A1A1LL, 0xF0F0F0A1A2LL };
  
/* 
 * SERVICE ACCESS CONFIG AND CREDS
 * 
 * Would be good if this could be made configurable using the app...
 * but will hardcode to keep things simple for now
 * /
 
/* WiFi access credentials */
const char* ssid = "MarioKart";
const char* password = "Boeing747";

/* MQTT broker access endpoint and credentials */

/*
// Cloud MQTT
const char* mqtt_server = "m13.cloudmqtt.com";
const int mqtt_port = 19034; //Use the TCP port, not the WebSocket port which is used in the app
const char* mqtt_user = "tfzpyakl";
const char* mqtt_password = "9-McjdgBOLnB";
*/

// HiveMQ
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883; //Use the TCP port, not the WebSocket port which is used in the app
const char* mqtt_user = "";
const char* mqtt_password = "";

/* Allocate space for incoming and outgoing data */
char BTSerial_data_in[128]; //For data received via Bluetooth from app
char RFSerial_data_in[32]; //For data received via RF from slaves (max 32 bytes)
char RFSerial_data_out[32]; //For data to be sent via RF to slaves (max 32 bytes)

/*
 * INSTANCE/OBJECT DEFINITIONS
 */

RF24 radio(CE_PIN,CSN_PIN); //The client object for RF communication
WiFiClient espClient; //The client object for WiFi communication
PubSubClient client(espClient); //The client object for communication with the MQTT broker
//BluetoothSerial BTSerial; //If using the ESP32 on-board Bluetooth

/*
 * All I2C modules come with a deafult address which can be discovered by running an I2C Scanner sketch
 * https://playground.arduino.cc/Main/I2cScanner/
 * 0x27 is a common default
 */
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//LiquidCrystal_I2C lcd(0x3F, 4, 5, 6, 0, 1, 2, 3, 7, POSITIVE); //Alternative pin connections between the I2C backpack and the LCD module


/*
 * Function to print to an LCD, a bit of optimization
 */
void printToLCD(uint8_t row, String text, uint8_t clear_row = 0){
  //Clear both or either row, 0 = don't clear
  if(clear_row == 3){
    lcd.clear();
  }
  else if(clear_row == 1){
    lcd.setCursor(0,0);
    lcd.print("                ");//print 16 spaces, no elegant solution
  }
  else if(clear_row == 2){
    lcd.setCursor(0,1);
    lcd.print("                ");//print 16 spaces, no elegant solution
  }

  //Set cursor to row 1 or 2
  if(row == 1){
    lcd.setCursor(0,0);
  }
  else if(row > 1){
    lcd.setCursor(0,1);
  }

  lcd.print(text);
}

/*
 * Function to decode the WiFi encryption type code
 * Not really necessary since the access point to connect to is hardcoded.
 * Needed if offering a choice of networks to connect to.
 */
 
String translateEncryptionType(wifi_auth_mode_t encryptionType) {
 
  switch (encryptionType) {
    case (WIFI_AUTH_OPEN):
      return "Open";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2_ENTERPRISE";
  }
}

/*
 * Function to scan for available WiFi networks and print their name, signal strength and MAC address.
 * Not really necessary since the access point to connect to is hardcoded.
 * Needed if offering a choice of networks to connect to.
 */
 
void scanNetworks() {
 
  int numberOfNetworks = WiFi.scanNetworks();
 
  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);
 
  for (int i = 0; i < numberOfNetworks; i++) {
 
    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));
 
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
 
    Serial.print("MAC address: ");
    Serial.println(WiFi.BSSIDstr(i));
 
    Serial.print("Encryption type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("-----------------------");
 
  }
}

/*
 * Function to connect to specific network based on the ssid and password define in the config variables.
 */
 
void connectToNetwork() {
  /* Delay needed before calling the WiFi.begin for the hardware to start up. Not always needed. */
  //delay(4000);

  WiFi.begin(ssid, password); //Initialize the WiFi connection.

  /* Keep printing an update message while the WiFi connection is initializing. */
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    printToLCD(1, "Connecting to", 3);
    printToLCD(2, (String)ssid + "...");

    //DEBUG
    Serial.println("Establishing connection to " + (String)ssid + "...");
  }

  printToLCD(1, "Connected to:", 3);
  printToLCD(2, (String)ssid + "!");
  
  //DEBUG
  Serial.println("Connected to" + (String)ssid + "!");
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
}


/*
 * Function to disconnect from the connected network. 
 * Not used for now. If used, call connectToNetwork as required to reconnect.
 */
 
void disconnectFromNetwork() {
  WiFi.disconnect(true);
  Serial.println(WiFi.localIP());
  Serial.println("WiFi disconnected.");
}

/*
 * Function to initialize radio communication.
 */
 
void initRadio() {

  printToLCD(1, "Initializing", 3);
  printToLCD(2, "Radio...");
  
  /* Initialize RF communidation */
  radio.begin();

  printToLCD(1, "Radio", 3);
  printToLCD(2, "Initialized!");
  
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
  radio.openReadingPipe(1, reading_pipes[0]); //Listen to Slave node 1
  radio.openReadingPipe(2, reading_pipes[1]); //Listen to Slave node 2
  radio.startListening();

  printToLCD(1, "Listening...", 3);
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

  /* Since we have a dedicated writing pipe, no need to call stopListening().
   * Calling it anyway to be safe, ensure startListening() is called as required. 
   */
  radio.stopListening();

  //Display status on the LCD
  printToLCD(1, "Sending: " + (String)node_id + "|" + (String)node_data_copy, 3);

  //Decode the node address based on the node value received over Bluetooth
  //DEBUG
  Serial.print(F("Sending to node: "));
  Serial.println(node_id);
  
  radio.openWritingPipe(writing_pipes[node_id-1]); //Subtract 1 from the node_id since array index start from 0
  
  /* Send RF data */
  if (!radio.write( &node_data_copy, sizeof(node_data_copy) )){
    //Display status on the LCD
    printToLCD(2, "Failed!", 2);
    
    //DEBUG
    Serial.print(F("Failed: "));
    Serial.println(node_data_copy);
  }
  else{
    //Display status on the LCD
    printToLCD(2, "Sent!", 2);

    //DEBUG
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
  
  //Display status on the LCD
  //printToLCD(1, "Radio listening...", 3);

  /* 
   * Set up a timeout period, and break after the period...
   * so other listeners can be polled in the loop()  
   */
   
  /* Get the current time in microseconds */
  unsigned long started_waiting_at = micros();               
  /* Variable to indicate if a response was received or not */
  boolean timeout = false;                                   

  /* While nothing is received */
  while ( !radio.available(&pipeNum) ){                             
    /* If waited longer than 200ms, indicate timeout and exit while loop */
    if (micros() - started_waiting_at > 200000 ){
        timeout = true;
        break;
    }      
  }
  // Describe the results    
  if ( timeout ){                                             
      Serial.print(F("."));
  }else{
      /* Grab the response, compare, and send to debugging spew */
      char node_data[32];                                 
      radio.read( &node_data, sizeof(node_data) );
      
      unsigned long recv_time = micros();
      
      /* Spew it to console for debugging */
      Serial.println(F("."));
      Serial.print(F("Got message: "));
      Serial.print(node_data);
      Serial.print(F(", From pipe: "));
      Serial.print(pipeNum);
      Serial.print(F(", Received at: "));
      Serial.print(recv_time);
      Serial.println(F(" microseconds"));

      //Display data on the LCD
      printToLCD(1, "Data received:", 3);
      printToLCD(2, (String)node_data);
      
      /* TODO: Publish to MQTT broker */
      publishToMQTT(node_data);
  }
}
/*
 * Function to connect and publish a message to an MQTT broker.
 */
 
void publishToMQTT(char message[]) {

 /* Get the current time in microseconds */
 unsigned long started_trying_at = micros();               
 boolean timeout = false;

  /*
   * Message will be in the format: topic|parameter|value
   * Extract node to determine where to send the data
   * The data itself needs to include on pin|value (device is redundant for now)
   * NOTE: strtok() inserts a '\0' after each token found thus the original string
   * if accessed, only shows the first token. Reconstruct the data from tokens.
   */

  char *topic = strtok (message,"|"); //The first call to strtok takes the string to parse
  char *parameter = strtok (NULL,"|"); //Subsequent calls take NULL
  char *value = strtok (NULL,"|");

  char data_to_publish[32];
  data_to_publish[0] = '\0';

  strcpy(data_to_publish, parameter);
  strcat(data_to_publish, ":");
  strcat(data_to_publish, value);
  //Add date, will need an RTC for accurate date time
  strcat(data_to_publish, ",date:");
  strcat(data_to_publish, "1553343966"); //random fixed epoch for now

  //Connect to the MQTT broker
  client.setServer(mqtt_server, mqtt_port);

  //Keep printing an update message while the MQTT broker connection is being established.
  //while (!client.connected()) {
    
    //Display status on the LCD
    printToLCD(1,"Connecting to:", 3);
    printToLCD(2, (String)mqtt_server + ":" + (String)mqtt_port );

    //DEBUG
    Serial.println("Connecting to MQTT broker...");
 
    //if (client.connect("HomeCentral_Master", mqttUser, mqttPassword )) { //HomeCentral_Master is the client id
    if (client.connect("HomeCentral_Master")) { //HomeCentral_Master is the client id

      //Display status on the LCD
      printToLCD(1, "Connected to:");

      //DEBUG
      Serial.println("Connected to MQTT broker!");
      
      //Publish the message to the MQTT broker (a quick confirmation print to the Serial Monitor as well)
      Serial.print(data_to_publish);
      client.publish(topic, data_to_publish); //(queue_name, message)
    } else {
      //Display status on the LCD
      printToLCD(1, "Conn. Failed:");

      //DEBUG
      Serial.print("Connection to MQTT broker failed with state: ");
      Serial.print(client.state());
      
      /* If been trying longer than 1s, indicate timeout and exit while loop */
      /*
      if (micros() - started_trying_at > 1000000 ){            
          timeout = true;
          break;
      }
      else{
        Serial.println("Trying to connect again...");
        delay(200); //Try again after a small delay
      }
      */
    } //END else
  //} //END while()
}//END function publishToMQTT()
 
/*
 * Setup
 * The setup function runs once when you press reset or power the board.
 */ 

void setup() {
  //Initialize digital pin LED_BUILTIN as an output. Can be used to indicate connection status.
  pinMode(LED_BUILTIN, OUTPUT);

  //Initialise the LCD
  lcd.begin(16,2);
  lcd.clear();

  Serial.begin(115200); //The default serial port for communication with serial monitor.

  //Initialize radio communication
  initRadio();
  
  /* Native ESP32 Bluetooth not working simultaneously with WiFi */
  /*
  if(!SerialBT.begin("ESP32")){ //the parameter is the name other devices will see the ESP32 as when pairing
      Serial.println("An error occurred initializing Bluetooth");
    }
    else{
      Serial.println("Bluetooth initialized!");
    }
  */

  /*
   * Start the HC-05 Bluetooth connection on the hardware serial interface Serial2 defined as BTSerial
   * The function call for initializing a hardware serial port is as follows: 
   * Serial1.begin(baud-rate, protocol, RX pin, TX pin);
   */

  printToLCD(1, "Initializing", 3);
  printToLCD(2, "Bluetooth...");
  
  BTSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //Display confirmation on the LCD
  printToLCD(1, "Bluetooth", 3);
  printToLCD(2, "Initialized!");
  
  //DEBUG
  Serial.println("HC-05 Bluetooth initialized!");
  
  // Connect to WiFi
  //scanNetworks(); //Not required
  connectToNetwork();
} /* END of setup() */


/*
 * Loop
 * The loop function runs over and over again forever with a delay of 10 seconds
 */ 

void loop() {
  
  /* Send RF data */
  /* Basic test: Get system time and send it. This will block until complete */
  /*
  unsigned long start_time = micros();                             
  if (!radio.write( &start_time, sizeof(unsigned long) )){
    Serial.println(F("Sending signal failed."));
  }
  else{
    Serial.println(F("Sent signal successfully."));
  }
  */

  /* 
   * Open the RF listening port and listen for a while then break 
   * so other listeners can be polled in the loop()
   */
  readFromRFNode();

  client.loop(); //This is required to keep the WiFi connection on

  //Empty the receiving char array
  for( int i = 0; i < sizeof(BTSerial_data_in);  i++ )
   BTSerial_data_in[i] = (char)0;
  
  /* Data coming from the app is control data and needs to be sent to slave node(s) */

  //while(BTSerial.available()){   
  if(BTSerial.available()){ //Use if() instead of while()   
    BTSerial.readBytesUntil('\0', BTSerial_data_in, 128);
    Serial.println(BTSerial_data_in); 
    /*
     * Scrapping JSON in favor of a simple pipe separated string to keep the data light
     * String will be in the format: node|device|pin|value
     * Extract node to determine where to send the data
     * The data itself needs to include on pin|value (device is redundant for now)
     * NOTE: strtok() inserts a '\0' after each token found thus the original string
     * if accessed, only shows the first token. Reconstruct the data from tokens.
     */

    char *node = strtok (BTSerial_data_in,"|"); //The first call to strtok takes the string to parse
    char *pin = strtok (NULL,"|"); //Subsequent calls take NULL
    char *value = strtok (NULL,"|");

    /* Spew to the console for debugging */
    
    Serial.print(F("Got Bluetooth message: "));
    Serial.print(F("For node: "));
    Serial.print(node);
    Serial.print(F(", For pin: "));
    Serial.print(pin);
    Serial.print(F(", Value: "));
    Serial.println(value);
    
    /* Get the node_id of the node to which the data needs to be sent */
    uint8_t node_id = atoi(node);

    /* Construct the data string that needs to be send to the node with node_id */
    /* First reset the string variable */
    RFSerial_data_out[0] = '\0';
    
    strcat(RFSerial_data_out, pin);
    strcat(RFSerial_data_out, "|");
    strcat(RFSerial_data_out, value);
    
    /* Original approach but since strtok() modifies original string, this won't work */
    /*   
    strncpy(RFSerial_data_out, BTSerial_data_in, sizeof(RFSerial_data_out));
    RFSerial_data_out[sizeof(RFSerial_data_out)-1] = '\0';
    */
    
    /* Send data over RF to the slave node depending on the incoming data values */
    sendToRFNode(node_id, RFSerial_data_out);
  }
}

/*
 * END OF SKETCH
 */
