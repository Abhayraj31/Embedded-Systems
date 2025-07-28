#include "arduino_secrets.h"
#include "thingProperties.h"

// === Global variables ===
const int ledPin = 13;

const int dotDuration = 200;
const int dashDuration = dotDuration * 3;

const int symbolSpace = dotDuration;
const int letterSpace = dotDuration * 3;
const int wordSpace = dotDuration * 7;

void setup() {
  Serial.begin(9600);
  delay(1500);
  
  pinMode(ledPin, OUTPUT);
  
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
  blinkMorseCode();
}

void blinkDot() {
  digitalWrite(ledPin, HIGH);
  delay(dotDuration);
  digitalWrite(ledPin, LOW);
  delay(symbolSpace);
}

void blinkDash() {
  digitalWrite(ledPin, HIGH);
  delay(dashDuration);
  digitalWrite(ledPin, LOW);
  delay(symbolSpace);
}

void blinkLetter(char letter) {
  switch (letter) {
    case 'A': // .-
      blinkDot();
      blinkDash();
      break;
    case 'B': // -...
      blinkDash();
      blinkDot();
      blinkDot();
      blinkDot();
      break;
    case 'H': // ....
      blinkDot();
      blinkDot();
      blinkDot();
      blinkDot();
      break;
    case 'Y': // -.--
      blinkDash();
      blinkDot();
      blinkDash();
      blinkDash();
      break;
    case 'R': // .-.
      blinkDot();
      blinkDash();
      blinkDot();
      break;
    case 'J': // .---
      blinkDot();
      blinkDash();
      blinkDash();
      blinkDash();
      break;
    default:
      break;
  }
  delay(letterSpace - symbolSpace); // Inter-letter space correction
}

void blinkMorseCode() {
  blinkLetter('A');
  blinkLetter('B');
  blinkLetter('H');
  blinkLetter('A');
  blinkLetter('Y');
  blinkLetter('R');
  blinkLetter('A');
  blinkLetter('J');
  delay(wordSpace);
}

