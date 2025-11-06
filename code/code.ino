#include <LiquidCrystal.h>

// Sriram Yerramsetty, EGN2020C Final Project

// PIN ASSIGMENTS
#define LIGHT_PIN A0 // photoresistor
#define BUZZ_PIN     // buzzer
#define BUTTON_PIN   // button

#define TRIG_PIN
#define ECHO_PIN 

// VARIABLES
#define MOTION_THRESHOLD 15 // min change to register motion
#define LIGHT_THRESHOLD 0.5 // min light to activate
long previousDistance = 0;

LiquidCrystal lcd(); // put lcd pins here

// Function to read the distance from the Ultrasonic sensor
long readUltrasonicDistance() {
  // Clears the TRIG_PIN 
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Sets the TRIG_PIN HIGH for 10 microseconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Reads the ECHO_PIN, returns the sound wave travel time in microseconds
  long duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculating the distance (cm): Duration * Speed of Sound / 2
  // Speed of sound is approx 0.034 cm/microsecond. 0.034 / 2 = 0.017
  long distance = duration * 0.034 / 2;
  
  return distance;
}


// Function to detect whether theres motion from the ultrasonic sensor
bool detectMotion() {
  long currentDistance = readUltrasonicDistance();
  
  long distChange = abs(currentDistance - previousDistance);

  bool motionDetected = 0;

  // ignore readings that r too crazy
  if (currentDistance > 10 && currentDistance < 400) {
    
    if (distChange > MOTION_THRESHOLD) {
      motionDetected = 1;
    } 
  } 

  previousDistance = currentDistance;

  return motionDetected;
}


// FUNCTIONS FOR COMPONENTS

void lcdDisplay(bool state) {
  if (state) {
    lcd.setCursor(0,0);
    lcd.print("Did you unlock the door?");
  } else {
    lcd.setCursor(0,0);
    lcd.print("    ");
  }
}

void buzzer(bool state) {
  if (state) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}


void setup() {

  Serial.begin(9600);

  // setup 
  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.begin(16,2); // 16 cols, 2 rows
  lcd.print("Setup Complete");

  previousDistance = readUltrasonicDistance(); 
}

void loop() {
  
  int lightLevel = analogRead(LIGHT_PIN); // read the light level

  lcdDisplay(false); // turn off the lcd screen
  buzzer(false); // turn off the buzzer

  if (detectMotion()) {
    // MOTION DETECTED
    
    // check is light detected by photoresistor
    if (lightLevel < LIGHT_THRESHOLD) {
      // NO LIGHT & MOTION
      // ACTIVATE MAIN SEQUENCE
      lcdDisplay(true); // turn on the lcd screen
      buzzer(true); // turn on the buzzer

      if (digitalRead(BUTTON_PIN) == HIGH) { // check if button pressed
        // BUTTON PRESSED
        // turn off everything
        lcdDisplay(false); 
        buzzer(false);
        continue; 
      }
    }
  }
}
