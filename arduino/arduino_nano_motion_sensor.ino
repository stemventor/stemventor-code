/* 
 *  HomeCentral_Circuit3_002_MotionSensor.ino
 *  Author: AdvantIoT Development Team
 *  License: MIT
 *  Board: Arduino Nano
 *  
 *  This sketch does the following:
 *  - Tests the board with a push button and LED.
 *  - Activates a relay with the pushbutton to turn electrical lightbulbs on and off.
 *  - Reads data from the connected Motion sensor and switches lightbulbs on or off.
 *  
 *  - Note that the Circuit 3 uses a Nano exactly as in Circuit Two so the basic sketch
 *    for connecting and testing a Nano will not be repeated. 
 *    You can use HomeCentral_Circuit2_001.ino
 */

/* LIBRARIES */

//Arduino boards have built-in support for serial communication on pins RX and TX 
//and via the USB connection)
//Alternate GPIO pins can be used for RX/TX with other modules (such as the HC-05) 
//using the SoftwareSerial library
#include <SoftwareSerial.h>

//Required for I2C communication
#include <Wire.h> 

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
 * The PIR motion sensor should be connected to send an interrupt
 * when motion is detected.
 * For a Nano, only pins 2 and 3 support interrupt service requests
 */
#define PIR_PIN_ISR 3

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
//Time for which motion sensor activated device is kept on
const unsigned long motion_indicator_duration = 15*1000;
unsigned long last_motion_detected;
boolean motion_detected = false;

/* LOCAL FUNCTIONS */

/*
 * Switch a device attached to a relay channel on or off
 */
void switchState(int pin, char* state){
  if(strncmp(state, "on", 2) == 0){
    digitalWrite(pin, LOW); //switch on the device, pin is "active low" so LOW turns on
  }
  else if(strncmp(state, "off", 3) == 0){
    digitalWrite(pin, HIGH); //switch off the device, pin is "active low" so HIGH turns off
  }
}

/* 
 * Function to execute when an interrupt is triggered by the motion sensor
 * Interrupt Service Requests (ISRs) are special functions 
 * that get called on an interrupt signal.
 * An ISR cannot have any parameters, and they shouldn’t return anything.
 * Generally, an ISR should be as short and fast as possible. 
 * If your sketch uses multiple ISRs, only one can run at a time, 
 * other interrupts will be executed after the current one finishes.
 */  

//IRAM_ATTR is used to run the interrupt code in RAM, otherwise code is stored in flash and it’s slower.
//ICACHE_RAM_ATTR void motionDetected() { //Use ICACHE_RAM_ATTR for NodeMCU/ESP8266, not required for Nano

void motionDetected() {
  unsigned long current_millis = millis();
  unsigned int motion_threshold = 15*1000;
  
  //Process signal only if apart more than threshold to filter continuous motion
  if(current_millis - last_motion_detected > motion_threshold) {
    //INFO: on the serial monitor
    Serial.println("Motion detected!");  
    last_motion_detected = millis();
    motion_detected = true;

    //Switch on the light bulb on channel 1
    switchState(CHANNEL_ONE, "on");
  }
}

void motionStopped() {
  //INFO: on the serial monitor
  Serial.println("Motion stopped!");
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

  //When the PIR senses activity in its viewing area, it pulls the signal pin low. 
  //But when the sensor is inactive, the pin is basically floating. 
  //To avoid any false-positives, the signal pin should be pulled high to 5V.
  pinMode(PIR_PIN_ISR, INPUT_PULLUP);
  
  //Set PIR_PIN_ISR pin as interrupt, assign interrupt function and set RISING mode, 
  attachInterrupt(digitalPinToInterrupt(PIR_PIN_ISR), motionDetected, RISING);
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
  
  //Turn off the light after the number of seconds defined in the motion_sensor_hold variable
  //If motion is detected again the last_motion_detected will be a moving target and light will stay on
  if(current_millis - last_motion_detected > motion_indicator_duration) {
    switchState(CHANNEL_ONE, "off");
  }
  
  //Sending an artificial interrupt to simulate motion
  long rand_num = random(30000, 120000);
  if(current_millis - last_motion_detected > rand_num ) {
    if(digitalRead(PIR_PIN_ISR) == HIGH){
      digitalWrite(PIR_PIN_ISR, LOW);
    }
    else if(digitalRead(PIR_PIN_ISR) == LOW){
      digitalWrite(PIR_PIN_ISR, HIGH);
    }
  }
}
