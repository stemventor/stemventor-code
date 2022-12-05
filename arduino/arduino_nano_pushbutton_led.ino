/* 
 *  HomeCentral_Circuit2_001.ino
 *  Author: AdvantIoT Development Team
 *  License: MIT
 *  Board: Arduino Nano
 *  
 *  This sketch does the following:
 *  - Tests the board with a push button an RGB LED.
 */

/* LIBRARIES */

/* CONSTANT DEFINITIONS */

/*
 * To reference pins: 
 * Digital pins: either D1,D2,and so on  
 * or 1,2,and so on depending on the board.
 * Analog pins: Always A1,A2,and so on.
 */

#define PUSHBUTTON_PIN 2

/* AN RGB LED needs three signals, one for each color
 * To save a pin, we will only connect Red and Green to a digital out  
 * while Blue will be conected to GND (always 0). 
 * This will reduce the number of color combinations.
 */
#define RGB_LED_RPIN 3 //red pin, needs to be a PWM pin
#define RGB_LED_GPIN 6 //green pin, needs to be a PWM pin

#define ONBOARD_LED 13 //inverted logic: HIGH=off, LOW=on

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

/* INITIALIZE OBJECTS
 * Libraries usually follow an object-oriented approach that requires
 * an instance of the class to call its methods.
 */

/* LOCAL FUNCTIONS */

/*
 * Function to light up RGB LED with RGB values
 */
void lightRGBLED(int red_value, int green_value, int blue_value)
 {
  //This is writing an analog value to a digital pin
  //that is PWM capable. RGB colors and analogWrite take values from 0 to 255
  analogWrite(RGB_LED_RPIN, red_value);
  analogWrite(RGB_LED_GPIN, green_value);
  
  //In this case the blue_value will have no impact
  //since the blue pin is connected to GND (blue_value = 0)
  //If you connect the blue pin, uncomment the following line
  //analogWrite(RGB_LED_BPIN, blue_value);
}

/* SETUP CODE: runs once when the board starts up or resets */ 

void setup() {

  //Start the serial communication with baud rate suitable for your components.
  Serial.begin(115200);

  //Set the pin modes, since all pins are capable of both input and output.
  pinMode(RGB_LED_GPIN, OUTPUT); //will get an analog output so needs to be a PWM pin
  pinMode(RGB_LED_RPIN, OUTPUT); //will get an analog output so needs to be a PWM pin
  
  pinMode(PUSHBUTTON_PIN, INPUT);

  //Initialise LED to switched off state.
  lightRGBLED(0,0,0);
}

/* 
 * MAIN LOOP: runs repeatedly at a very high frequency (1000s of times a second) 
 * Avoid sending data over serial or any other communication channel in every loop.  
 * Either add a delay to the loop or use some timing logic.
 */

void loop() {
  int button_state = digitalRead(PUSHBUTTON_PIN);
  
  //Read the state of the pushbutton value and light the RGB LED
  //Green when pressed, off when released.
  //The pushbutton state will be HIGH as long as it is pressed, and LOW when released.
  if (button_state == HIGH) {
    lightRGBLED(255,0,0); //flash LED red to confirm button pressed
    button_pressed = true;
  }
  else {
    //if turned on with button press take it back to green
    //to act as the gas sensor indicator
    if(button_pressed){
      lightRGBLED(0,255,0);
      button_pressed = false;
    }
  }
}
