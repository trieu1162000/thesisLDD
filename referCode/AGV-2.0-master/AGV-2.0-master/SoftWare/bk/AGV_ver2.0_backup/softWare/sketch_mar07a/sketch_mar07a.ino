
// Set GPIOs for LED and PIR Motion Sensor
const int motionSensor = 10;

// Checks if motion was detected, sets LED HIGH and starts a timer
  ICACHE_RAM_ATTR void detectsMovement() {
    Serial.println("MOTION DETECTED!!!");
  }

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, FALLING);

}

void loop() {

  }
