/* 
 * HomeCentral_Circuit1_002.ino
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
/* 
 * All I2C components have an address, the default is usually 0x27
 * If that doesn't work, see this:https://playground.arduino.cc/Main/I2cScanner/
 * The init statement accepts the address and the number of columns and rows.
 */
LiquidCrystal_I2C lcd(0x27, 20, 4);

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
  Serial.println(WiFi.localIP().toString());

  //INFO: on the LCD
  printToLCD(1, "Connected to WiFi!", true);
  printToLCD(2, WiFi.localIP().toString(), false);
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

  //Initialize the LCD.
  lcd.begin();
  //Turn on the blacklight and print a message.
  lcd.backlight();
  //Use the local function to display text on the LCD.
  printToLCD(1, "I2C LCD Ready", true);

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
