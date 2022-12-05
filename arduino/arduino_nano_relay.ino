/* 
 *  HomeCentral_Circuit3_001_relay.ino
 *  Author: AdvantIoT Development Team
 *  License: MIT
 *  Board: Arduino Nano
 *  
 *  This sketch does the following:
 *  - Tests the board with a push button and LED.
 *  - Activates a relay with the pushbutton to turn electrical lightbulbs on and off.
 *  
 *  - Note that the Circuit 3 uses a Nano exactly as in Circuit Two so the basic sketch
 *    for connecting and testing a Nano will not be repeated. 
 *    You can use HomeCentral_Circuit2_001.ino
 */

/* LIBRARIES */

/* CONSTANT DEFINITIONS */
/*
 * To reference pins: 
 * Digital pins: either D1,D2,and so on  
 * or 1,2,and so on depending on the board.
 * Analog pins: Always A1,A2,and so on.
 */

#define LED_PIN 2
#define PUSHBUTTON_PIN 6
#define ONBOARD_LED 13 //inverted logic: HIGH=off, LOW=on

/* 2-Channel Relay */
#define CHANNEL_ONE 9 //Connected to relay 1 and used as motion indicator light
#define CHANNEL_TWO 10 //Connected to relay 2

/* 
 * GLOBAL VARIABLE DEFINITIONS
 * Use global variables only if needed across functions or
 * reused between function calls, else use local variables.
 * Read up on data types.
 * While Arduino support strings, they take up memory so
 * character arrays are used more often.
 * char* (char pointer) and char[] (array) are similar.
 * const char* points to an immutable string, cannot be interchanged with char*
 */

bool button_pressed = false;

/* LOCAL FUNCTIONS */

/* Switch a device attached to a relay channel on or off */
void switchState(int pin, char* state){
  if(strncmp(state, "on", 2) == 0){
    digitalWrite(pin, LOW); //switch on the device, pin is "active low" so LOW turns on
  }
  else if(strncmp(state, "off", 3) == 0){
    digitalWrite(pin, HIGH); //switch off the device, pin is "active low" so HIGH turns off
  }
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

  pinMode(CHANNEL_ONE, OUTPUT); // initialize relay control pin as OUTPUT
  pinMode(CHANNEL_TWO, OUTPUT); // initialize relay control pin as OUTPUT

  digitalWrite(CHANNEL_ONE, HIGH); //switch the device off, pin is active low so HIGH turns off
  digitalWrite(CHANNEL_TWO, HIGH); //switch the device off, pin is active low so HIGH turns off

  //Initialise LED to switched off state.
  digitalWrite(LED_PIN, LOW);
}

/* MAIN LOOP: runs repeatedly at a very high frequency (1000s of times a second) */
void loop() {

  int button_state = digitalRead(PUSHBUTTON_PIN);
  unsigned long current_millis = millis(); //Keep local since needs to be update every call
  
  //Read the state of the pushbutton value and take some action
  //The pushbutton state will be HIGH as long as it is pressed, and LOW when released.
  if (button_state == HIGH) {
    digitalWrite(LED_PIN, HIGH); //turn LED on
    //Also switch on the lightbulb attached to channel 2 of the relay
    switchState(CHANNEL_TWO, "on");

  } else {
    digitalWrite(LED_PIN, LOW); //turn LED off
    switchState(CHANNEL_TWO, "off");
  }

  //INFO: relay siganls on serial monitor
  Serial.print(digitalRead(CHANNEL_ONE));
  Serial.print(",");
  Serial.println(digitalRead(CHANNEL_TWO));

  delay(5000);
}
