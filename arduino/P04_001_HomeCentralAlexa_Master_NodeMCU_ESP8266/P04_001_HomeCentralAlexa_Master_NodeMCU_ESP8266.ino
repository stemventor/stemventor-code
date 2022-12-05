/* 
 *  P04_001_HomeCentralAlexa_Master_NodeMCU_ESP8266
 *  Author: AdvantIoT Development Team
 *  Authored on: 04-Dec-2019
 *  Last Modified on: 17-Jan-2020
 *  
 *  This is the complete sketch required for the HomeCentralAlexa_Master circuit. Use NodeMCU ESP8266 as the microcontroller.
 *  The Sketch connects to a WiFi network and then to the MQTT server on AWS IoT. It sends data from sensors and receives control signals from MQTT.
 *  It has a relay that controls lights via Alexa commands and a PIR motion sensor.
 *  It also has a DHT11 temperature and humidity sensor that reads and sends data to the AWS IoT MQTT queue
 *  It has a 20x4 LCD connected to it to display status and other data
 */
  
/* REFERENCES:  
 *  
 * Arduino with PIR motion sensor
 * For complete project details, visit: http://RandomNerdTutorials.com/pirsensor
 * https://randomnerdtutorials.com/esp32-pir-motion-sensor-interrupts-timers/
 * Modified by Rui Santos based on PIR sensor by Limor Fried
 *  
 * For connecting a Relay
 * https://randomnerdtutorials.com/guide-for-relay-module-with-arduino/
 * 
 * Base code from ESP8266 AWS IoT example by Evandro Luis Copercini
 *  
 * Three specific libraries/plugins required for this project: 
 * 1. The IDE needs to detect the board:
   Copy the below code in the Additional Boards Manager URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
   Then in Tools->Board->Boards Manager: Find esp8266 by esp8266 community and install the software for Arduino.
   See: http://arduino.esp8266.com/Arduino/versions/2.0.0/doc/installing.html This installs the core ESP8266 libraries as well.
   The board shows up as NodeMCU 1.0 (ESP-12E Module)
 * 2. The NodeMCU needs a UART driver for the port to be detected. Download from:
   https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver
   The port will show up as /dev/cu.wchusbserial1410
 *
 * 3. A tool to upload the AWS certificates into SPIFFS: https://github.com/esp8266/arduino-esp8266fs-plugin
 * Serial Peripheral Interface Flash File System (SPIFFS) https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
 * Once installed to the Arduino IDE it gives you a menu option Tools -> ESP8266 Sketch Data Upload
 * Files to be uploaded must be saved in a folder named 'data' in the folder where your sketch is
 * 
 * To connect to AWS IoT you need your account certificate and private key and a CA certficate. These are provided by AWS IoT when you create an account.
 * Three certificates need to be uploaded:
   - thing/device certificate (download from link provided after creating a certificate using the one-click option on AWS IoT Secure page)
   - private key (download from link provided after creating a certificate using the one-click option on AWS IoT Secure page)
   - Amazon CA trust certificate (https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html#server-authentication-certs)
 * 
 * The files need to be converted from .pem to .der files. They are saved in the data folder and are named cert.der, key.der and ca.der in this code.
 * The commands below do the conversion:
 * openssl x509 -in xxx-certificate.pem.crt -out cert.der -outform DER
 * openssl rsa -in xxx-private.pem.key -out private.der -outform DER
 * openssl x509 -in AmazonRootCA1.pem -out ca.der -outform DER
   
 * Also read: 
    https://medium.com/@jgillard/marrying-esp8266-aws-iot-69f1ab219c2
    https://medium.com/@jgillard/aws-iot-rules-with-the-esp8266-e8aab5a8a7f9

  * AWS IoT Information:
    User name,Password,Access key ID,Secret access key,Console login link
    HomeCentral_Node,,AKIA46GUJJ2MMOJOJKVE,W73F4NdB0N1wfj2yJjYd6ZcQllmHGKCOrBtwb3mj,https://889503436440.signin.aws.amazon.com/console
    ARN:
    arn:aws:iot:us-west-2:889503436440:thing/HomeCentral_AWS_Node1
    REST API Endpoint: a3jb74f354h65r-ats.iot.us-west-2.amazonaws.com
*/

/* I2C LCD Display
 *  
 * This is the library that works with an I2C LCD: 
 * https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home#!downloading-and-installation
 * The folder extracted after unzipping the downloaded zip file is named NewLiquidCrystal_lib and should be copied into the Arduino/libraries folder.
 * You have to delete all other LCD libraries to avoid a conflict.
 * TODO: Find a way to reduce the brightness. Use on the on-baord pot to control the contrast. 
 * 
 * IMPORTANT NOTE FOR NODE MCU SDA/SCL PINS:
 * The code cannot be uploaded if thses pins are connected to a sensor or device. Disconnect upload and reconnect.
 * 
 * PIN Connections:
 * From I2C Backpack connect the SDA and SCL to the corresponding SDA and SCL on the board as per the board's pinout.
 * VCC and GND are obvious.
 * 
 */


/*
 *  LIBRARIES
 *  Libraries need to be installed using the Tools -> Manage Libraries option and then installed
 *  Some libraries are available with the IDE while others may need to be downloaded and installed form a .zip file 
 *  When including header files on most compilers: 
 *  using the "" first checks your local directory, and if it doesn't find a match then moves on to check the system paths. 
 *  using <> starts the search with system headers
 * 
 *  Core libraries are in the Arduino installation folder, other third-party libraries may be in your Sketches/Library folder
 *  as configured in IDE Preferences
 *  
 *  The client library to publish to an MQTT broker is from: https://pubsubclient.knolleary.net/
 */

#include "FS.h" //file system wrapper
#include <ESP8266WiFi.h> //The WiFi library
#include <ESP8266WiFiMulti.h> // The Wi-Fi-Multi library - to connect to the strongest network
#include "PubSubClient.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Time.h>

/* I2C LCD Display */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "DHT.h" //For DHT11 functions needed to get the humidity and temperature readings from the sensor. Using the Adafruit library, needs its sensor-master library as well.
#define DHTTYPE DHT11 // DHT 11

/* PIN DEFINITIONS */
//4-Channel Relay Channels (using only 2)
#define CHANNEL_ONE D3 //Connected to relay
#define CHANNEL_TWO D4 //Connected to relay
#define DHT_PIN D6

/* Initialize objects */
DHT dht(DHT_PIN, DHTTYPE);

/*
 * All I2C modules come with a default address which can be discovered by running an I2C Scanner sketch
 * https://playground.arduino.cc/Main/I2cScanner/
 * 0x27 is a common default
 */
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//LiquidCrystal_I2C lcd(0x3F, 4, 5, 6, 0, 1, 2, 3, 7, POSITIVE);

//Note that char* cannot be modified, adding a const prefix is just making that explicit
//Use char[] when you need to modify

// Update these with values suitable for the WiFI networks available.
//Network 1
const char* ssid2 = "muzzu";
const char* password2 = "qwertyui";
//Network 2
const char* ssid1 = "MarioKart"; 
const char* password1 = "Boeing747";
//Network 3
const char* ssid3 = "MomPhone";
const char* password3 = "Boeing747";

//This is your custom endpoint that allows you to connect to AWS IoT. 
//Each of your Things has a REST API available at this endpoint. 
//This is also an important property to insert when using an MQTT client or the AWS IoT Device SDK.
const char* AWS_endpoint = "a3jb74f354h65r-ats.iot.us-west-2.amazonaws.com";

//For HiveMQ MQTT
const char* HiveMQ_endpoint = "broker.hivemq.com";

//AWS IoT MQTT Queues
const char* homecentral_sensors = "homecentral/sensors"; //Sensor data will be received on this topic
const char* homecentral_devices = "homecentral/devices"; //Device control signals via the app or Alexa or sensors will be received on this topic

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


//LCD Text buffers, 20 characters is max display anyway
char lcd_text1[20] = "";
char lcd_text2[20] = "";
char lcd_text3[20] = "";
char lcd_text4[20] = "";

//Timer variables
unsigned long current_millis;
unsigned long last_dht_reading_millis;

//Pre-defined time/frequency periods
const unsigned long dht_reading_freq = 60*1000; //60 seconds
boolean start_timer = false;

/* ALL FUNCTIONS HERE */

/*
 * Function to print to an LCD, a bit of optimization
 * Pass the string as a reference to save heap space
 */
void printToLCD(uint8_t row, String text, uint8_t clear_row = 0){
  //Clear all (clear_row == 5) or any one row, 0 = don't clear
  if(clear_row == 5){
    lcd.clear();
  }
  else if(clear_row == 1){
    lcd.setCursor(0,0);
    lcd.print("                    ");//print 20 spaces, no elegant solution
  }
  else if(clear_row == 2){
    lcd.setCursor(0,1);
    lcd.print("                    ");//print 20 spaces, no elegant solution
  }
  else if(clear_row == 3){
    lcd.setCursor(0,3);
    lcd.print("                    ");//print 20 spaces, no elegant solution
  }
  else if(clear_row == 4){
    lcd.setCursor(0,4);
    lcd.print("                    ");//print 20 spaces, no elegant solution
  }

  //Set cursor to row 1 or 2
  if(row == 1){
    lcd.setCursor(0,0);
  }
  else if(row == 2){
    lcd.setCursor(0,1);
  }
  else if(row == 3){
    lcd.setCursor(0,2);
  }
  else if(row == 4){
    lcd.setCursor(0,3);
  }

  lcd.print(text);
}

//Switch a device attached to a relay channel on or off
void switchState(int pin, char* state){
  if(strncmp(state, "on", 2) == 0){
    //digitalWrite(LED_BUILTIN, LOW); //Built-in LED is active low so LOW turns on
    digitalWrite(pin, LOW); //switch on the device, pin is active low so HIGH turns off
  }
  else if(strncmp(state, "off", 3) == 0){
    //digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(pin, HIGH); //switch off the device, pin is active low so HIGH turns off
  }
}

void callOnMQTTMsgRcvd(char* topic, byte* payload, unsigned int length) {
  char message[64]; //convert the byte* payload to char* for further processing
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    message[i] = (char)payload[i];
  }
  message[length] = '\0'; //null terminate the string so string operations work well
  Serial.println(message);
  /*
   * Message will be in the format: room|device|state
   * NOTE: strtok() inserts a '\0' after each token found thus the original string
   * if accessed, only shows the first token. Reconstruct the data from tokens.
   */
  //char *room = strtok ((char *)payload,"|"); //The first call to strtok takes the string to parse
  char *room = strtok (message,"|"); //The first call to strtok takes the string to parse
  char *device = strtok (NULL,"|"); //Subsequent calls take NULL
  char *state = strtok (NULL,"|");

  lcd_text4[0] = '\0';
  strcat(lcd_text4, room);
  strcat(lcd_text4, " ");
  strcat(lcd_text4, device);
  strcat(lcd_text4, " ");
  strcat(lcd_text4, state);

  printToLCD(4, String(lcd_text4), 4);

  //Turn selected device on or off
  if (strcmp(room,"kitchen") == 0 && strcmp(device,"lights") == 0){
    switchState(CHANNEL_ONE, state);
  }else if (strcmp(room,"bedroom") == 0 && strcmp(device,"lights") == 0){
    switchState(CHANNEL_TWO, state);
  }else if (strcmp(room,"all") == 0 && strcmp(device,"lights") == 0){
    //Have an array of devices and then loop through and switch states for all devices
    switchState(CHANNEL_ONE, state);
    switchState(CHANNEL_TWO, state);
  }  
} //END function callOnMQTTMsgRcvd(...)

WiFiClientSecure espClient;
ESP8266WiFiMulti wifiMulti; // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
//This is for AWS IoT MOTT
//PubSubClient pubsub_client(AWS_endpoint, 8883, callOnMQTTMsgRcvd, espClient); //secure MQTT port
//Publish to Hive MQTT instead so the app is reading from one broker
PubSubClient pubsub_client(HiveMQ_endpoint, 1883, callOnMQTTMsgRcvd, espClient); //non-secure MQTT port
//Can try to figure out how to have app send certificates and connect to AWS IoT MQTT - maybe later.
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network - pick the strongest one from those configured
  
  /* Code below is for a fixed network
  espClient.setBufferSizes(512, 512);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  */
  
  wifiMulti.addAP(ssid1, password1); // add Wi-Fi networks you want to connect to
  wifiMulti.addAP(ssid2, password2); // add Wi-Fi networks you want to connect to
  wifiMulti.addAP(ssid3, password2); // add Wi-Fi networks you want to connect to

  Serial.println("Attempting a connection to the strongest network...");
  int i = 0;
  //int wifi_connect_attempts = 5; //If not connect in specified number of attempts go into deep sleep until woken up
  while (wifiMulti.run() != WL_CONNECTED) { // Scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(1000);
    /*
    Serial.print("Attempt ");
    Serial.println(i);
    i=i+1;
    */
    /*
    if(i >= wifi_connect_attempts){
      //Go into deep sleep
      // Deep sleep mode for 30 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
      //Serial.println("Going into deep sleep mode for 60 seconds");
 ca.     //ESP.deepSleep(30e6); 
  
      // Deep sleep mode until RESET pin is connected to a LOW signal (for example pushbutton or magnetic reed switch)
      //Serial.println("Going into deep sleep mode for 30 seconds or until RESET pin is connected to a LOW signal");
      //30e6 = 30000000 microseconds = 30 seconds
      //ESP.deepSleep(30e6);   
    }
    */
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID()); // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer

  printToLCD(1, WiFi.SSID(), 1);
  printToLCD(2, WiFi.localIP().toString(), 2); //returns 4 bytes so the need to convert to string

  timeClient.begin();
  while(!timeClient.update()){
    timeClient.forceUpdate();
  }

  espClient.setX509Time(timeClient.getEpochTime());
}


void reconnect() {
  // Loop until we're reconnected
  while (!pubsub_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (pubsub_client.connect("ESPthing")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      pubsub_client.publish(homecentral_sensors, "Ready to send sensor data...");
      // ... and resubscribe
      pubsub_client.subscribe(homecentral_devices);
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubsub_client.state());
      Serial.println(" try again in 5 seconds");

      char buf[256];
      espClient.getLastSSLError(buf,256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


/* This function loads the certificates required by AWS IoT from the SPIFFS file system into the espClient object */ 
void loadCertsFromSPIFFS() {
  
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());

  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
  if (!cert) {
    Serial.println("Failed to open cert file");
  }
  else
    Serial.println("Success opening cert file");

  delay(1000);

  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");

  // Load private key file
  File private_key = SPIFFS.open("/key.der", "r"); //replace private eith your uploaded file name
  if (!private_key) {
    Serial.println("Failed to open private key file.");
  }
  else
    Serial.println("Success opening private key file.");

  delay(1000);

  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded.");
  else
    Serial.println("private key not loaded.");

    // Load CA file
    File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
    if (!ca) {
      Serial.println("Failed to open ca.");
    }
    else
    Serial.println("Success opening ca.");

    delay(1000);

    if(espClient.loadCACert(ca))
    Serial.println("ca loaded");
    else
    Serial.println("ca failed");

  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());  
}


void getTempAndHumidity(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();         

  //Print to LCD
  char t_string[8]; //To store float value converted to string
  char h_string[8]; //To store float value converted to string
  dtostrf(t, 6, 2, t_string); //(float source, char length, decimal places, char[] target)
  dtostrf(h, 3, 0, h_string);

  lcd_text3[0] = '\0';
  strcat(lcd_text3, "Temp:");
  strcat(lcd_text3, t_string);
  strcat(lcd_text3, "C");

  lcd_text4[0] = '\0';
  strcat(lcd_text4, "Humidity:");
  strcat(lcd_text4, h_string);
  strcat(lcd_text4, "%");

  Serial.println(lcd_text3);
  Serial.println(lcd_text4);
  printToLCD(3, lcd_text3, 3);
  printToLCD(4, lcd_text4, 4);

  //Publish message to MQTT
  //Each sensor reading should go as a separate message so parsing logic is the same for each sensor data message
  char mqtt_string_t[10];
  strcat(mqtt_string_t, "T|");
  strcat(mqtt_string_t, t_string);
  pubsub_client.publish(homecentral_sensors, mqtt_string_t);

  char mqtt_string_h[10];
  strcat(mqtt_string_h, "H|");
  strcat(mqtt_string_h, h_string);
  pubsub_client.publish(homecentral_sensors, mqtt_string_h);

  /*
  char mqtt_string_t[10];
  strcat(mqtt_string_t, "T|");
  strcat(mqtt_string_t, t_string);

  strcat(mqtt_string, "|H|");
  strcat(mqtt_string, h_string);
  pubsub_client.publish(homecentral_sensors, mqtt_string);
  */
 
  //Serial.print("Heap: "); Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
  //delay(30000); //Need a good delay or the buffer is getting flooded. If using a timer (millis) delay is not needed.
}

void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  dht.begin();
  
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, HIGH); //LED is active low on the ESP8266 so LOW turns it on

  pinMode(CHANNEL_ONE, OUTPUT); // initialize pin as OUTPUT
  pinMode(CHANNEL_TWO, OUTPUT); // initialize pin as OUTPUT

  digitalWrite(CHANNEL_ONE, HIGH); //switch on the device, pin is active low so HIGH turns off
  digitalWrite(CHANNEL_TWO, HIGH); //switch on the device, pin is active low so HIGH turns off

  pinMode(DHT_PIN, INPUT);
  
  //initialize the LCD
  //lcd.begin(16,2);
  lcd.begin(20,4); //Using a 20x4 LCD
  lcd.clear();
  
  // Wait for serial to initialize.
  //while(!Serial) {}

  // Deep sleep mode for 30 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
  //Serial.println("I'm awake, but I'm going into deep sleep mode for 30 seconds");
  //ESP.deepSleep(30e6); 
  
  // Deep sleep mode until RESET pin is connected to a LOW signal (for example pushbutton or magnetic reed switch)
  //Serial.println("I'm awake, but I'm going into deep sleep mode until RESET pin is connected to a LOW signal");
  //ESP.deepSleep(0); 
  
  setup_wifi();
    
  delay(1000);

  loadCertsFromSPIFFS();
}

void loop() {
  //Call all required functions
  if (!pubsub_client.connected()) {
    reconnect();
  }
  pubsub_client.loop(); //Needed to keep polling the MQTT queues and keep the connection alive

  //Get temperature and humidity readings at a defined frequency
  current_millis = millis();
  if(current_millis - last_dht_reading_millis > dht_reading_freq){
    getTempAndHumidity();
    last_dht_reading_millis = millis();
  }

  //Serial.print("Heap: "); Serial.println(ESP.getFreeHeap()); //FOR DEBUGGING: Low heap can cause problems
}
