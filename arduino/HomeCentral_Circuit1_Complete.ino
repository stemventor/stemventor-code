/* 
 *  HomeCentral_Circuit1_Complete.ino
 *  Author: AdvantIoT Development Team
 *  License: MIT
 *  Board: NodeMCU Development Board
 *  
 *  This sketch is exactly the same as HomeCentral_Circuit1_004.ino
 *  without detailed comments.
 */

/* LIBRARIES */

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <PubSubClient.h>  

/* CONSTANT DEFINITIONS */

#define LED_PIN D3
#define ONBOARD_LED D4
#define PUSHBUTTON_PIN D5
#define DHT11_PIN D6

/* GLOBAL VARIABLE DEFINITIONS */

const char* ssid = "your_ssid";
const char* password = "your_password";

unsigned long current_millis;
unsigned long last_dht_reading_millis;
const unsigned long dht_reading_freq = 15*1000;
unsigned long lastMQTTConnectAttempt = 0;

const char* mqtt_server = "broker.mqttdashboard.com";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";
char* publish_topic = "homecentral/sensors";
char* subscribe_topic = "homecentral/controlsignals";

/* INITIALIZE OBJECTS */

WiFiClient wifiClient;
LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT dht(DHT11_PIN, DHT11);

/* LOCAL FUNCTIONS */

void printToLCD(int row, String text, bool clear_all = false){
  const char* twenty_spaces = "                    ";

  if(clear_all){ 
    lcd.clear();
  } 

  lcd.setCursor(0,row-1);
  lcd.print(twenty_spaces);
  lcd.setCursor(0,row-1);
  lcd.print(text);
}

/* 
 * Function to connect to a WiFi network and therefore to the Internet.
 */
void connectWiFi() {
  delay(10);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  printToLCD(1, "Connecting to WiFi:", true);
  printToLCD(2, ssid, false);

  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);
    Serial.print(".");
  }

  Serial.println('\n');
  Serial.println("Connected to WiFi!");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP().toString());

  printToLCD(1, "Connected to WiFi!", true);
  printToLCD(2, WiFi.localIP().toString(), false);
}

/*
 * Callback function invoked when a message is published on the topic
 * subscribed to by this client.
 */
void callOnMQTTMsgRcvd(char* topic, byte* payload, unsigned int length) {
  char message[64];

  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  printToLCD(1, "Message received:", true);
  printToLCD(2, message, false);
}

/* INITIALIZE OBJECTS */
PubSubClient pubsub_client(mqtt_server, mqtt_port, callOnMQTTMsgRcvd, wifiClient);

/*
 * Function to connect to the MQTT broker, returns true if connected, else returns false.
 * Your MQTT broker must be set up and configured with the topics before running this.
 */
boolean connectMQTT() {
  if (pubsub_client.connect("HomeCentral")) {
    
    pubsub_client.publish(publish_topic,"Ready to send sensor data.");

    pubsub_client.subscribe(subscribe_topic);
  }
  return pubsub_client.connected();
}

/*
 * Function to publish a message to a topic on the MQTT broker.
 * A format needs to be agreed upon between the sender and receiver,
 * the broker will store and forward whatever is sent.
 * This uses a pipe-separated format: parameter|value|timestamp
 * Maximum message size the MQTT library supports is 128 bytes.
 * NRF24L01 (if used) supports only 32 bytes.
 */
void publishToMQTT(char *message, char *topic) {
 
  char millis_string[20] = "";
  sprintf(millis_string, "%lu", millis());
  strcat(message, "|");
  strcat(message, millis_string);

  Serial.print("Publishing to: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(message);

  printToLCD(1, "Publishing to:", true);
  printToLCD(2, topic, false);
  printToLCD(3, "Message:", false);
  printToLCD(4, message, false);

  if (pubsub_client.connected()) {
    pubsub_client.publish(topic, message);
  } else {
    Serial.print("Could not publish data, not connected to broker.");
  }
}

/*
 * Function to read data from the DHT11 sensor and publish the data
 * to the MQTT broker by calling publishMQTT(...)
 */
void getTemperatureAndHumidity(){

  float temperature = dht.readTemperature(); 
  float humidity = dht.readHumidity();

  char temperature_string[8];
  char humidity_string[8];

  char temperature_message[32];
  char humidity_message[32];

  dtostrf(temperature, 5, 1, temperature_string);
  dtostrf(humidity, 3, 0, humidity_string);

  temperature_message[0] = '\0';
  strcat(temperature_message, "T|");
  strcat(temperature_message, temperature_string);
  
  publishToMQTT(temperature_message, publish_topic);
 
  humidity_message[0] = '\0';
  strcat(humidity_message, "H|");
  strcat(humidity_message, humidity_string);
  
  publishToMQTT(humidity_message, publish_topic);
}

/* SETUP CODE: runs once when the board starts up or resets */ 
void setup() {

  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUSHBUTTON_PIN, INPUT);
  pinMode(DHT11_PIN, INPUT);

  digitalWrite(LED_PIN, LOW);

  lcd.begin();
  lcd.backlight();
  printToLCD(1, "I2C LCD Ready", true);

  connectWiFi();
}

/* MAIN LOOP: runs repeatedly at a very high frequency (1000s of times a second) */
void loop() {

  int button_state = digitalRead(PUSHBUTTON_PIN);
  
  if (button_state == HIGH) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  if (!pubsub_client.connected()) {
    long now = millis();
    if (now - lastMQTTConnectAttempt > 5000) {
      lastMQTTConnectAttempt = now;
      if (connectMQTT()) {
        lastMQTTConnectAttempt = 0;
      }
    }
  } else {
    pubsub_client.loop();
  }
  
  current_millis = millis();
  if(current_millis - last_dht_reading_millis > dht_reading_freq){
    getTemperatureAndHumidity();
    last_dht_reading_millis = millis();
  }
}
