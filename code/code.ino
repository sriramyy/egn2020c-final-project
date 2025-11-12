#include <LiquidCrystal.h>

// Sriram Yerramsetty, EGN2020C Final Project

// PIN ASSIGNMENTS
#define LIGHT_PIN A0    // photoresistor
#define BUZZ_PIN 13     // buzzer 
#define BUTTON_PIN 10   // button

#define TRIG_PIN 9
#define ECHO_PIN 8

// VARIABLES
#define MOTION_THRESHOLD 15 // min change to register motion
#define LIGHT_THRESHOLD 900 // min light to activate
long previousDistance = 0;
bool alarmActive = false; // State variable to keep the alarm on

// LCD PINS: RS=12, EN=11, D4=5, D5=4, D6=3, D7=2
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); 

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
  
  Serial.print("[Ultrasonic] Raw Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  return distance;
}


// Function to detect whether theres motion from the ultrasonic sensor
bool detectMotion() {
  // Do not read distance if the alarm is already active (saves time)
  if (alarmActive) return false; 
  
  long currentDistance = readUltrasonicDistance();
  
  long distChange = abs(currentDistance - previousDistance);

  Serial.print("[Motion] Previous Distance: ");
  Serial.print(previousDistance);
  Serial.print(" cm, Change: ");
  Serial.print(distChange);
  Serial.println(" cm");

  bool motionDetected = false; 

  // ignore readings that r too crazy
  if (currentDistance > 10 && currentDistance < 400) {
    
    if (distChange > MOTION_THRESHOLD) {
      motionDetected = true;
    } 
  } 

  previousDistance = currentDistance;

  if (motionDetected) {
    Serial.println(">>> MOTION DETECTED! <<<");
  } else {
    Serial.println("--- No Motion Detected. ---");
  }

  return motionDetected;
}


// FUNCTIONS FOR COMPONENTS

void lcdDisplay(bool state) {
  if (state) {
    lcd.setCursor(0,0);
    lcd.print("Did you unlock?");
    Serial.println("[LCD] Displaying: 'Did you unlock the door?'");
  } else {
    lcd.clear(); 
  }
}

void buzzer(bool state) {
  if (state) {
    digitalWrite(BUZZ_PIN, HIGH);
    Serial.println("[Buzzer] ALARM ON!");
  } else {
    digitalWrite(BUZZ_PIN, LOW);
  }
}


void setup() {

  Serial.begin(9600);
  Serial.println("--- System Setup Started ---");

  // setup 
  pinMode(BUZZ_PIN, OUTPUT);
  // Using INPUT assuming external pull-down resistor (HIGH when pressed)
  pinMode(BUTTON_PIN, INPUT); 
  pinMode(LIGHT_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.begin(16,2); // 16 cols, 2 rows
  lcd.print("Setup Complete");

  // Initial read to set the baseline distance
  previousDistance = readUltrasonicDistance(); 
  
  Serial.println("--- Setup Complete ---");
}

void loop() {
  
  // Read the button state. HIGH means pressed.
  int buttonState = digitalRead(BUTTON_PIN); 

  if (alarmActive) {
    // STATE 1: ALARM ACTIVE (Stay on until button press)

    // Ensure the alarm components remain ON
    lcdDisplay(true); 
    buzzer(true);
    
    Serial.print("[LOOP START] Button State (D10): ");
    Serial.println(buttonState == HIGH ? "HIGH (Pressed)" : "LOW (Released)");

    // Check for button press to deactivate.
    if (buttonState == HIGH) { 
      alarmActive = false; // Turn off alarm state
      Serial.println(">>> ALARM DEACTIVATED by Button Press <<<");
      lcdDisplay(false); // Turn off peripherals 
      buzzer(false);
    } else {
      Serial.println("*** Alarm Active, Awaiting Button Press ***");
    }

  } else {
    // STATE 2: MONITORING (Check for trigger conditions)
    
    int lightLevel = analogRead(LIGHT_PIN); // read the light level
    
    Serial.print("\n[MONITORING] Light Level (A0): ");
    Serial.print(lightLevel);
    Serial.print(" (Threshold: ");
    Serial.print(LIGHT_THRESHOLD);
    Serial.println(")");

    // Ensure peripherals are off during monitoring
    lcdDisplay(false);
    buzzer(false);
    
    if (detectMotion()) {
      
      // check if light is NOT detected by photoresistor (it is dark)
      if (lightLevel < LIGHT_THRESHOLD) {
        // NO LIGHT & MOTION -> TRIGGER ALARM
        alarmActive = true; 
        Serial.println(">> ALARM TRIGGERED: DARKNESS + MOTION <<");
        // Activate alalrm
        lcdDisplay(true); 
        buzzer(true);     
      } else {
        Serial.println("Motion Detected, but light level is high (ABORT alarm).");
      }
    }
  }
  
  delay(500); 
}