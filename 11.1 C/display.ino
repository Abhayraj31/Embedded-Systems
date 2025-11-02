// =======================================================
// === 1. INCLUDE YOUR DISPLAY LIBRARY(IES) HERE ===
#include <Wire.h>             // Needed for I2C communication
#include <LiquidCrystal_I2C.h> // Include the I2C LCD library
// =======================================================


// =======================================================
// === 2. DEFINE DISPLAY OBJECT AND SETTINGS HERE ===
// --- IMPORTANT: Change 0x27 to YOUR LCD's I2C address found by the scanner ---
// Common addresses: 0x27, 0x3F
// Format: LiquidCrystal_I2C lcd(I2C_ADDRESS, COLUMNS, ROWS);
LiquidCrystal_I2C lcd(0x27, 16, 2); // <-- CHANGE 0x27 if needed! (16 columns, 2 rows)
// =======================================================


String receivedGesture = "";      // Stores the incoming data from the Pi
String lastDisplayedGesture = ""; // Stores the last gesture shown

void setup() {
  // Start serial communication with Raspberry Pi (MUST match Python script)
  Serial.begin(9600);
  // while (!Serial); // Optional: Wait for serial connection

  // =======================================================
  // === 3. INITIALIZE YOUR DISPLAY HERE ===
  lcd.init();           // Initialize the LCD
  lcd.backlight();      // Turn on the backlight
  lcd.setCursor(0, 0);  // Set cursor to the first column, first row
  lcd.print("Waiting for Pi.."); // Initial message
  delay(1000);        // Display message briefly
  // =======================================================
}

void loop() {
  // Checks if data is available from the Raspberry Pi
  if (Serial.available() > 0) {
    // Reads the incoming string until newline character '\n'
    receivedGesture = Serial.readStringUntil('\n');
    receivedGesture.trim(); // Remove any extra whitespace

    // Only updates if a valid gesture was received AND it's different
    if (receivedGesture.length() > 0 && receivedGesture != lastDisplayedGesture) {

      // =======================================================
      // === 4. UPDATE YOUR DISPLAY HERE ===
      lcd.clear();              // Clear the LCD
      lcd.setCursor(0, 0);      // Set cursor to top-left
      lcd.print(receivedGesture); // Print the received gesture
      // =======================================================

      lastDisplayedGesture = receivedGesture; // Remember what was displayed
    }
  }
}
