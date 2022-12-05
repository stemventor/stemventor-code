/* 
 * HomeCentral_Circuit1_001.ino
 * Author: AdvantIoT Development Team
 * License: MIT
 * Board: NodeMCU Development Board
 *  
 * IMPORTANT NOTE FOR NODE MCU SDA/SCL PINS:
 * A sketch cannot be uploaded if these pins are connected to a module.
 * You will have to disconnect, upload sketch and reconnect.
 *  
 * This sketch does the following:
 *  - Tests the board with a push button and LED.
 *  - Connects the board to a WiFi access point.
 */

/* LIBRARIES */

#include <ESP8266WiFi.h> //installed when you install the ESP8266 board

/* CONSTANT DEFINITIONS */

#define LED_PIN D3
#define ONBOARD_LED D4 //inverted logic: HIGH=off, LOW=on
#define PUSHBUTTON_PIN D5

/* 
 * GLOBAL VARIABLE DEFINITIONS 
 * Read up on data types. 
 * While Arduino support strings, they take up memory so
 * character arrays are used more often. 
 * char* (char pointer) and char[] (array) are similar.
 * const char* points to an immutable string, cannot be interchanged with char*
 */

//The WiFi network name and password configured on your router
//SSID = ServiceSet ID, which is the technical term for a WiFi network name.
const char* ssid = "your_ssid"; 
const char* password = "your_password";

/* INITIALIZE OBJECTS
 * Libraries usually follow an object-oriented approach that requires
 * an instance of the class to call its methods.
 */
WiFiClient wifiClient;

/* LOCAL FUNCTIONS */

/*
 * Function to connect to a WiFi network which should have Internet access.
 */
void connectWiFi() {

  delay(10); //a brief pause to allow the connection to stabilise

  //In a WiFi network one device (the router) is the Access Point
  //and the other devices are referred to to as stations.
  //The ESP32 can act as an access point or a station.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //initiates the connection
  
  //INFO: on the serial monitor
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  //Keep trying to connect in a loop, until the status changes to WL_CONNECTED
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000); //with a small delay between connection attempts
    Serial.print("."); //INFO: increasing dots to show progress 
  }

  //INFO: on the serial monitor
  Serial.println('\n');
  Serial.println("Connected to WiFi!");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP().toString());
}

/* SETUP CODE: runs once when the board starts up or resets */ 
void setup() {

  //Start the serial communication with baud rate suitable for your components.
  Serial.begin(115200);
  //This is additional diagnostics for the WiFi, not required if WiFi is working reliably.
  //Serial.setDebugOutput(true); 

  //Set the pin modes, since all pins are capable of both input and output.
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUSHBUTTON_PIN, INPUT);

  //Initialise LED to switched off state.
  digitalWrite(LED_PIN, LOW);

  //Call the function to connect to WiFi.
  //Keep this commented while you test the board and the LED and Pushbutton connections.
  //When ready, uncommment and compile and upload again.
  //connectWiFi();
}

/* MAIN LOOP: runs repeatedly at a very high frequency (1000s of times a second) */
void loop() {

  int button_state = digitalRead(PUSHBUTTON_PIN);
  
  //Read the state of the pushbutton value and turn the LED on or off.
  //The pushbutton state will be HIGH as long as it is pressed, and LOW when released.
  if (button_state == HIGH) {
    digitalWrite(LED_PIN, HIGH); //turn LED on
  } else {
    digitalWrite(LED_PIN, LOW); //turn LED off
  }
}
