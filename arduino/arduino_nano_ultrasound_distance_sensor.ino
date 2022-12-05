/* 
 *  Workshop_Nano_UltrasonicDistanceSensor
 *  Author: AdvantIoT Development Team
 *  Authored on: 11-Jan-2020
 *  Last Modified on: 07-Mar-2020
 *  
 *  This is the complete sketch required for the Workshop_Nano_UltrasonicDistanceSensor circuit 
 *  which has the following:
 *  - An ultrasonic distance centre
 *  - Three LEDs, all three lighting up when object is closer than a threshold, 
 *  reducing to two and one as object moves away
 *  
 */
  
//Define pins numbers
const int trigger_pin = 9;
const int echo_pin = 10;

const int led1_pin = 2;
const int led2_pin = 3;
const int led3_pin = 4;

// defines variables
long pulse_duration;
int distance_cm;
int distance_inches;

void setup() {
  // put your setup code here, to run once:

  //Turn the built in LED off (not usually required, mine staying on for some reason)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  pinMode(trigger_pin, OUTPUT); // Sets the trigger pin as an Output
  pinMode(echo_pin, INPUT); // Sets the echo pin as an Input

  pinMode(led1_pin, OUTPUT); // Sets the trigger pin as an Output
  pinMode(led2_pin, OUTPUT); // Sets the trigger pin as an Output
  pinMode(led3_pin, OUTPUT); // Sets the trigger pin as an Output
  
  Serial.begin(115200); // Starts the serial communication
}

void loop() {
  // put your main code here, to run repeatedly:

  // Clears the trigger pin
  digitalWrite(trigger_pin, LOW);
  delayMicroseconds(2);

  // Sets the trigger pin on HIGH state for 10 micro seconds
  //This pulse is sent out as a sound wave and reflected back if it encounters an object
  digitalWrite(trigger_pin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigger_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger_pin, LOW);

  // Reads the echo pin, returns the sound wave travel time in microseconds
  //pulseIn() measures the time from the signal going high to it going low again
  pulse_duration = pulseIn(echo_pin, HIGH);

  //Convert the time to distance
  distance_cm = (pulse_duration/2)*0.0343; //Or divide by 29.1
  distance_inches = (pulse_duration/2)*0.0135; //Or divide by 74
  
  /* Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.print(" cm or ");
  Serial.print(distance_inches);
  Serial.println(" inches.");
  */
  
  //Light up more LEDs as the object somes closer
  if(distance_inches > 15){
    digitalWrite(led1_pin, LOW);
    digitalWrite(led2_pin, LOW);
    digitalWrite(led3_pin, LOW);      
  }else if(distance_inches > 10 && distance_inches < 15){
    digitalWrite(led1_pin, HIGH);
    digitalWrite(led2_pin, LOW);
    digitalWrite(led3_pin, LOW);      
  }else if(distance_inches > 5 && distance_inches < 10){
    digitalWrite(led1_pin, HIGH);
    digitalWrite(led2_pin, HIGH);
    digitalWrite(led3_pin, LOW);      
  }else if(distance_inches < 5){
    digitalWrite(led1_pin, HIGH);
    digitalWrite(led2_pin, HIGH);
    digitalWrite(led3_pin, HIGH);      
  }
  
  delay(1000);
}
