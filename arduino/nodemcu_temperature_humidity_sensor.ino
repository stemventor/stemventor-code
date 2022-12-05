/* 
 * HomeCentral_Circuit1_003.ino
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
 *  - Writes data to the connected I2C LCD
 *  - Reads data from the DHT11 Temperature and Humdity sensor
 *    and displays it on the LCD.
 */

/* LIBRARIES */

#include <ESP8266WiFi.h> //installed when you install the ESP8266 board
#include <Wire.h> //used for communicating with I2C devices
/*
 * The library for using an I2C LCD needs a driver to be downloaded and installed.
 * Download the .zip file from: https://github.com/lucasmaziero/LiquidCrystal_I2C (Click on the Code button).
 * To install: With this sketch open, select the Menu option Sketch->Include Library->Add .ZIP libary...
 * Select the downloaded .zip file.
 * The library should now show up in Sketch > Include Library and can be selected for inclusion.
 */
 #include <LiquidCrystal_I2C.h>
 /*
  * The library for reading data from the DHT11 sensor
  * is bundled with the IDE but needs to be installed.
  * Go to Tools->Manage Libraries and search for 
  * the DHT sensor library from Adafruit and install it.
  * It needs an other library as a dependency, install that
  * as well when prompted.
  */
 #include "DHT.h"

/* CONSTANT DEFINITIONS */

#define LED_PIN D3
#define ONBOARD_LED D4 //inverted logic: HIGH=off, LOW=on
#define PUSHBUTTON_PIN D5
#define DHT11_PIN D6

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

/*
 * Time variables are used to take actions
 * at a defined frequency, not necessarily in every loop.
 */
unsigned long current_millis; //current time in milliseconds
unsigned long last_dht_reading_millis; //updated every time sensor is read
const unsigned long dht_reading_freq = 15*1000; //15 seconds

/* INITIALIZE OBJECTS
 * Libraries usually follow an object-oriented approach that requires
 * an instance of the class to call its methods.
 */
WiFiClient wifiClient;
/* 
 * All I2C components have an address, the default is usually 0x27
 * If that doesn't work, see this:https://playground.arduino.cc/Main/I2cScanner/
 * The init statement accepts the address and the number of columns and rows.
 */
LiquidCrystal_I2C lcd(0x27, 20, 4);
/*
 * The first parameter is the sensor pin and the second specifies the sensor type
 * since this library supports all the DHT-series sensors.
 */
DHT dht(DHT11_PIN, DHT11);

/* LOCAL FUNCTIONS */

/*
 * Function to print to LCD, a convenience function.
 * Takes the row number and the text to display on that row.
 * The entire display is cleared if the clear_all flag is true,
 * else only the row is cleared.
 */
void printToLCD(int row, String text, bool clear_all = false){

  const char* twenty_spaces = "                    ";

  if(clear_all){ 
    lcd.clear(); //clears the entire display
  } 

  lcd.setCursor(0,row-1);
  lcd.print(twenty_spaces); //clears the row
  lcd.setCursor(0,row-1); //cursor has to be set again after printing spaces
  lcd.print(text);
}

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
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  //INFO: on the LCD
  printToLCD(1, "Connecting to WiFi", true);
  printToLCD(2, ssid, false);

  //Keep trying to connect in a loop, until the status changes to WL_CONNECTED
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000); //with a small delay between connection attempts
    Serial.print("."); //INFO: increasing dots to show progress
  }

  //INFO: on the serial monitor
  Serial.println('\n');
  Serial.println("Connected to WiFi!");  
  Serial.print("IP address: ");
  //returns the IP as an IPAddress data type, so convert to String
  Serial.println(WiFi.localIP().toString());

  //INFO: on the LCD
  printToLCD(1, "Connected to WiFi!", true);
  printToLCD(2, WiFi.localIP().toString(), false);
}

/*
 * Function to read data from the DHT11 sensor
 */
void getTemperatureAndHumidity(){

  //Read the temperature and humidity using the library functions
  float temperature = dht.readTemperature(); 
  float humidity = dht.readHumidity();

  //The float value needs to be converted to a string for display
  char temperature_value_string[8];
  char humidity_value_string[8];

  //Message strings
  char *temperature_message;
  char *humidity_message;

  /*
   * This is a system function which will convert a float to a char array.
   * This results in a leading space if the number is less than width. 
   * This can be handled by the app that will read the data so this code is kept simple.
   */
  dtostrf(temperature, 5, 1, temperature_value_string); //5 is total width and 1 is the decimal places (100.0)
  dtostrf(humidity, 3, 0, humidity_value_string); //3 is total width and 0 decimal places (100)

  /*
   * The message format defined here is a pipe-separated string: parameter|value|timestamp.
   * Construct the parameter|value part here,
   * the timestamp is inserted by the publishToMQTT(...) function.
   */
  temperature_message[0] = '\0';
  strcat(temperature_message, "T|");
  strcat(temperature_message, temperature_value_string);
  
  humidity_message[0] = '\0';
  strcat(humidity_message, "H|");
  strcat(humidity_message, humidity_value_string);
  
  //DEBUG:
  //Serial.print("Heap: "); Serial.println(ESP.getFreeHeap()); //low heap can cause problems
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
  pinMode(DHT11_PIN, INPUT);

  //Initialise LED to switched off state.
  digitalWrite(LED_PIN, LOW);

  //Initialize the LCD.
  lcd.begin();
  //Turn on the blacklight and print a message.
  lcd.backlight();
  //Use the local function to display text on the LCD.
  printToLCD(1, "I2C LCD Ready", true);

  //Call the function to connect to WiFi.
  //Keep this commented while you test the board and the LED and Pushbutton connections.
  //When ready, uncommment and compile and upload again.
  connectWiFi();
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

  //Get temperature and humidity readings from DHT11
  //Need not read in every loop, can be spaced out as required
  current_millis = millis();
  //If time since last reading is greater than desired frequency
  if(current_millis - last_dht_reading_millis > dht_reading_freq){
    getTemperatureAndHumidity();
    last_dht_reading_millis = millis();
  }
}
