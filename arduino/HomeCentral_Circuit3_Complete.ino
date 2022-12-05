/* 
 *  HomeCentral_Circuit3_Complete.ino
 *  Author: AdvantIoT Development Team
 *  License: MIT
 *  Board: Arduino Nano
 *  
 *  This sketch is exactly the same as HomeCentral_Circuit3_003_HC05.ino
 *  without detailed comments.
 */

/* LIBRARIES */
#include <SoftwareSerial.h>
#include <Wire.h> 

/* CONSTANT DEFINITIONS */
#define LED_PIN 2
#define PUSHBUTTON_PIN 6
#define ONBOARD_LED 13

#define CHANNEL_ONE 9
#define CHANNEL_TWO 10

#define PIR_PIN_ISR 3

#define RX_pin 7
#define TX_pin 8

#define I2C_ADDRESS 0x9

/* GLOBAL VARIABLE DEFINITIONS */
bool button_pressed = false;
const unsigned long motion_indicator_duration = 15*1000;
unsigned long last_motion_detected;
boolean motion_detected = false;

char control_signal_bt[8];
char control_signal_i2c[8];

/* INITIALIZE OBJECTS */
 SoftwareSerial BTSerial(RX_pin, TX_pin);

/* LOCAL FUNCTIONS */
void switchState(int pin, char* state){
  if(strncmp(state, "on", 2) == 0){
    digitalWrite(pin, LOW);
  }
  else if(strncmp(state, "off", 3) == 0){
    digitalWrite(pin, HIGH);
  }
}

/* Function to execute when an interrupt is triggered by the motion sensor  */  
void motionDetected() {
  unsigned long current_millis = millis();
  unsigned int motion_threshold = 15*1000;
  
  if(current_millis - last_motion_detected > motion_threshold) {
    Serial.println("Motion detected!");  
    last_motion_detected = millis();
    motion_detected = true;

    switchState(CHANNEL_ONE, "on");
  }
}

void motionStopped() {
  Serial.println("Motion stopped!");
}

/*
 * Function to process a control signal */
void processControlSignal(char control_signal[]){
  Serial.println(control_signal);

  char* device_id = strtok(control_signal, "|");
  char* state = strtok(NULL, "|");

  switch(atoi(device_id)){
    case 1: 
      switchState(CHANNEL_ONE, state);
      break;
    case 2: 
      switchState(CHANNEL_TWO, state);
      break;
  }
}

/* The function to be called when data is received over I2C comm. */
void onReceiveI2CData() {
  int i = 0;
  while (Wire.available() > 0) {
    control_signal_i2c[i] = Wire.read();
    i++;
  }
  control_signal_i2c[i] = '\0';
  processControlSignal(control_signal_i2c[i]);
}

/* SETUP CODE: runs once when the board starts up or resets */ 
void setup() {

  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUSHBUTTON_PIN, INPUT);

  pinMode(CHANNEL_ONE, OUTPUT);
  pinMode(CHANNEL_TWO, OUTPUT);

  digitalWrite(CHANNEL_ONE, HIGH);
  digitalWrite(CHANNEL_TWO, HIGH);

  digitalWrite(LED_PIN, LOW);

  pinMode(PIR_PIN_ISR, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(PIR_PIN_ISR), motionDetected, RISING);

  Serial.println("HC-05 Bluetooth initializing...");
  BTSerial.begin(9600);
  Serial.println("HC-05 Bluetooth initialized!");

  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(onReceiveI2CData);
}

/* MAIN LOOP: runs repeatedly at a very high frequency (1000s of times a second) */
void loop() {

  int button_state = digitalRead(PUSHBUTTON_PIN);
  unsigned long current_millis = millis();
  
  if (button_state == HIGH) {
    digitalWrite(LED_PIN, HIGH);
    switchState(CHANNEL_TWO, "on");

  } else {
    digitalWrite(LED_PIN, LOW);
    switchState(CHANNEL_TWO, "off");
  }

  delay(5000);
  
  if(current_millis - last_motion_detected > motion_indicator_duration) {
    switchState(CHANNEL_ONE, "off");
  }

  for( int i = 0; i < sizeof(control_signal_bt);  i++ )
   control_signal_bt[i] = (char)0;
   
  if(BTSerial.available()){ //Use if() instead of while() [why?]   
    BTSerial.readBytesUntil('\0', control_signal_bt, 128);
    processControlSignal(control_signal_bt);
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
