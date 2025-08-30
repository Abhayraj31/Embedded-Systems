#include <Arduino.h>

// Pin assignments
const int buttonPin = 3;   // Push button
const int pirPin    = 2;   // PIR motion sensor
const int ledPin    = 6;   // Single LED controlled by both

// State for LED
volatile bool ledState = LOW;

// Event flags (set in ISR, cleared in loop)
volatile bool buttonEvent = false;
volatile bool motionEvent = false;

// ISR for button press
void handleButton() {
  ledState = !ledState;           // Toggle LED state
  digitalWrite(ledPin, ledState);
  buttonEvent = true;             // Flag event for loop()
}

// ISR for PIR motion detection
void handleMotion() {
  ledState = !ledState;           // Toggle LED state
  digitalWrite(ledPin, ledState);
  motionEvent = true;             // Flag event for loop()
}

void setup() {
  // Setup LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Setup input pins
  pinMode(buttonPin, INPUT_PULLUP);  // button: pressed → LOW
  pinMode(pirPin, INPUT);            // PIR digital output

  // Serial for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("System Ready...");

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(pirPin), handleMotion, RISING);
}

void loop() {
  // Check if button interrupt occurred
  if (buttonEvent) {
    buttonEvent = false;
    Serial.println("Button Interrupt: LED Toggled");
  }

  // Check if motion interrupt occurred
  if (motionEvent) {
    motionEvent = false;
    Serial.println("Motion Interrupt: LED Toggled");
  }
}
