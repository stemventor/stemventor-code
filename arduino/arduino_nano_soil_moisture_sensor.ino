/* 
 *  HomeCentral_Circuit2_004.ino
 *  Author: AdvantIoT Development Team
 *  License: MIT
 *  Board: Arduino Nano
 *  
 *  This sketch does the following:
 *  - Tests the board with a push button and LED.
 *  - Reads data from the connected Gas sensor and flashes the LED as an alert. 
 *  - Reads data from a Proximity/Distance sensor. 
 *  - Reads data from a Soil Moisture sensor.
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

//The MQ-series Gas sensor sends both an analog and a digital signal
#define MQX_PIN_ANALOG A1 
#define MQX_PIN_DIGITAL 8 

//For the ultrasound distance sensor
#define ECHO_PIN 10
#define TRIGGER_PIN 11
#define BUZZER_PIN 12

//For LM393 Soil Moisture sensor sends both an analog and a digital signal
#define LM393_PIN_ANALOG A2
#define LM393_PIN_DIGITAL 7

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
unsigned long started_buzzing_millis;
unsigned long stopped_buzzing_millis;
boolean buzzing = false;
boolean quiet = true;
int urgency = 0;

//Timer variables
unsigned long last_mqx_reading_millis;
const unsigned long mqx_reading_freq = 25*1000;
unsigned long last_lm393_reading_millis;
const unsigned long lm393_reading_freq = 30*1000;

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

/*
 * Function to sound buzzer intermittently based on urgency.
 * The change is achieved by keeping the same buzz time but
 * reducing the time between buzzing depending on urgency.
 */
void soundBuzzer(int urgency){

  unsigned long current_millis; //Keep local since needs to be update every call
  int buzz_time = 500; //milliseconds, same for all urgencies
  int quiet_time = urgency*1000; //milliseconds, lower for high urgencies (1 = highest)
  
  current_millis = millis();
  
  if(urgency == 0 ||
     buzzing && current_millis - started_buzzing_millis > buzz_time){
      digitalWrite(BUZZER_PIN, LOW);
      quiet = true;
      buzzing = false;
      stopped_buzzing_millis = millis();
  }
  if(urgency != 0 &&
     quiet && current_millis - stopped_buzzing_millis > quiet_time){
      digitalWrite(BUZZER_PIN, HIGH);
      buzzing = true;
      quiet = false;
      started_buzzing_millis = millis();
  }
}

/*
 * Function to get gas sensor reading and send it to
 * Circuit One via I2C for onward publishing to the MQTT topic.
 */

void getGasReading(){

  //Read the gas digital and analog values using the Arduino read functions
  int mqx_digital_value = digitalRead(MQX_PIN_DIGITAL);
  float mqx_analog_value = analogRead(MQX_PIN_ANALOG);

  //The float value needs to be converted to a string for display
  char gas_value_string[8];
  //Message string (to Circuit One and onwards to the MQTT broker)
  char gas_message[32];

  //First check the digital signal, if LOW, don't read the analog value
  //The threshold for the module can be configured by checking the analog value
  //at which the digital turns HIGH
  if(mqx_digital_value == LOW)
    return; //Exit the function, no need to do anything further

  /*
   * This is a system function which will convert a float to a char array.
   * This results in a leading space if the number is less than width. 
   * This can be handled by the app that will read the data so this code is kept simple.
   */
  dtostrf(mqx_analog_value, 3, 0, gas_value_string);

  /*
   * The message format defined here is a pipe-separated string: parameter|value|timestamp.
   * Construct the parameter|value part here,
   * the timestamp is inserted by the publishToMQTT(...) function in the Circuit One sketch.
   */
  gas_message[0] = '\0';
  strcat(gas_message, "G|");
  strcat(gas_message, gas_value_string);
  
  //INFO: on the serial monitor
  Serial.println(gas_message);

  //The message also needs to be sent to Circuit One over I2C
  //and to the Node-RED server over serial communication.
  
  //On Circuit Two, light up the LED red, orange or green
  //depending on the concentration of the gas detected.
  if(mqx_analog_value >= 400){
    lightRGBLED(255,0,0);
  }
  else if(mqx_analog_value >= 200 && mqx_analog_value < 400){
    lightRGBLED(255,165,0);
  }
  else{
    lightRGBLED(0,255,0);
  }
}

/*
 * Function to get ultrasound distance sensor reading
 */
 
void getDistanceReading(){

  //LOCAL VARIABLES
  unsigned long pulse_duration;
  int distance_cm;
  int distance_inches;

  //Clears the trigger pin
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  //Sets the trigger pin on HIGH state for 10 micro seconds
  //This pulse is sent out as a sound wave and reflected back if it encounters an object
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  //Reads the echo pin, returns the sound wave travel time in microseconds
  //pulseIn() is a system function measures the time 
  //from the signal going high to it going low again
  pulse_duration = pulseIn(ECHO_PIN, HIGH);

  //Convert the time to distance with a formula from the sensor calibration/specification
  distance_cm = (pulse_duration/2)*0.0343; //Or divide by 29.1
  distance_inches = (pulse_duration/2)*0.0135; //Or divide by 74
  
  //INFO: on the serial monitor
  //Serial.print("Distance: ");
  //Serial.print(distance_cm);
  //Serial.println(" cm or ");
  //Serial.print(distance_inches);
  //Serial.println(" inches.");

  //Sound buzzer with an increasing frequency as the distance reduces
  //Uses an urgency value to determine seconds between buzzing
  if(distance_inches >= 24 || distance_inches == 0) urgency = 0; //Stop sounding the buzzer
  if(distance_inches < 24  && distance_inches >= 18) urgency = 6; 
  if(distance_inches < 18 && distance_inches >= 6) urgency = 3;
  if(distance_inches < 6) urgency = 1;

  soundBuzzer(urgency);
}

/*
 * Function to get soil moisture sensor reading and send it to
 * Circuit One via I2C for onward publishing to the MQTT topic.
 */
 
void getSoilMoistureReading(){

  //Read the gas digital and analog values using the Arduino read functions
  int lm393_digital_value = digitalRead(LM393_PIN_DIGITAL);
  float lm393_analog_value = analogRead(LM393_PIN_ANALOG);

  //The float value needs to be converted to a string for display
  char moisture_value_string[8];
  //Message string (to Circuit One and onwards to the MQTT broker)
  char moisture_message[32];
  
  //First check the digital signal, if LOW, don't read the analog value
  //The threshold for the module can be configured by checking the analog value
  //at which the digital turns HIGH
  if(lm393_digital_value == LOW)
    return; //Exit the function, no need to do anything further

  /*
   * This is a system function which will convert a float to a char array.
   * This results in a leading space if the number is less than width. 
   * This can be handled by the app that will read the data so this code is kept simple.
   */
  dtostrf(lm393_analog_value, 3, 0, moisture_value_string);

  /*
   * The message format defined here is a pipe-separated string: parameter|value|timestamp.
   * Construct the parameter|value part here,
   * the timestamp is inserted by the publishToMQTT(...) function in the Circuit One sketch.
   */
  moisture_message[0] = '\0';
  strcat(moisture_message, "M|");
  strcat(moisture_message, moisture_value_string);
  
  //INFO: on the serial monitor
  Serial.println(moisture_message);

  //The message also needs to be sent to Circuit One over I2C
  //and to the Node-RED server over serial communication.
  
  //On Circuit Two, light up the LED red, orange or green
  //depending on the concentration of the gas detected.
  if(lm393_analog_value >= 1000){
    lightRGBLED(255,0,0);
  }
  else if(lm393_analog_value >= 500 && lm393_analog_value < 1000){
    lightRGBLED(255,165,0);
  }
  else{
    lightRGBLED(0,255,0);
  }
}

/* SETUP CODE: runs once when the board starts up or resets */ 

void setup() {

  //Start the serial communication with baud rate suitable for your components.
  Serial.begin(115200);

  //Set the pin modes, since all pins are capable of both input and output.
  pinMode(RGB_LED_GPIN, OUTPUT); //will get an analog output so needs to be a PWM pin
  pinMode(RGB_LED_RPIN, OUTPUT); //will get an analog output so needs to be a PWM pin
  
  pinMode(PUSHBUTTON_PIN, INPUT);
  
  pinMode(MQX_PIN_ANALOG, INPUT);
  pinMode(MQX_PIN_DIGITAL, INPUT);
  
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(LM393_PIN_ANALOG, INPUT);
  pinMode(LM393_PIN_DIGITAL, INPUT);

  //Initialise LED to switched off state.
  lightRGBLED(0,0,0);
}

/* 
 * MAIN LOOP: runs repeatedly at a very high frequency (1000s of times a second) 
 * Avoid sending data over serial or any other communication channel in every loop.  
 * Either add a delay to the loop or use some timing logic.
 */

void loop() {
  static unsigned long current_millis; //local scope but persistent across function calls
  int button_state = digitalRead(PUSHBUTTON_PIN);
  
  //Read the state of the pushbutton value and light the RGB LED
  //Green when pressed, off when released.
  //The pushbutton state will be HIGH as long as it is pressed, and LOW when released.
  if (button_state == HIGH) {
    lightRGBLED(255,0,0); //flash LED red to confirm button pressed
    digitalWrite(BUZZER_PIN, HIGH);
    button_pressed = true;
    
    //Get soil moisture sensor reading
    //Since this is not a time-critical measure, 
    //get this reading when the pushbutton is pressed
    getSoilMoistureReading();
  }
  else {
    //if turned on with button press take it back to green
    //to act as the gas sensor indicator
    if(button_pressed){
      lightRGBLED(0,255,0);
      digitalWrite(BUZZER_PIN, LOW);
      button_pressed = false;
    }
  }

  //Get gas sensor reading
  //Need not read in every loop, can be spaced out as required
  current_millis = millis();
  //If time since last reading is greater than desired frequency
  if(current_millis - last_mqx_reading_millis > mqx_reading_freq){
    getGasReading();
    last_mqx_reading_millis = current_millis;
  }

  //Get distance reading, must run without a delay for the buzzing frequency logic
  getDistanceReading();
}
