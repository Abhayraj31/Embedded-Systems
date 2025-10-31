const int buttonPin = 3;
const int pirPin    = 2;
 const int led1Pin   = 7;
 const int led2Pin   = 8;
 //LED states
volatile bool led1State = LOW;
volatile bool led2State = LOW;

// Interrupt Service Routine for button
void handleButton() {
  led1State = !led1State;  // Toggle LED1 state
  digitalWrite(led1Pin, led1State);
  Serial.println("Button pressed! LED1 toggled.");
}

// Interrupt Service Routine for PIR sensor
void handlePIR() {
  led2State = !led2State;  // Toggle LED2 state
  digitalWrite(led2Pin, led2State);
  Serial.println("Motion detected! LED2 toggled.");
}

void setup() {
  Serial.begin(9600);

  // Setup LEDs
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);

  // Setup button with internal pull-up
  pinMode(buttonPin, INPUT_PULLUP);

  // Setup PIR sensor as input
  pinMode(pirPin, INPUT);

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButton, FALLING); // button press (goes LOW)
  attachInterrupt(digitalPinToInterrupt(pirPin), handlePIR, RISING);        // PIR goes HIGH when motion detected

  Serial.println("System ready. Waiting for events...");
}

void loop() {
  // Nothing here - everything handled by interrupts
}
