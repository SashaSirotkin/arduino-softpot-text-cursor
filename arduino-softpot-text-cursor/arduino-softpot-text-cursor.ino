#include <HID.h>

const int RED_LED_PIN = 9;    // Red LED Pin
const int GREEN_LED_PIN = 10; // Green LED Pin
const int BLUE_LED_PIN = 11;  // Blue LED Pin

const int SENSOR_PIN = 0;      // Analog input pin

void setup() {
  Serial.begin(SERIAL_HID_BAUD);
}

void loop() {
  // Read the voltage from the softpot (0-1023)
  int sensorValue = analogRead(SENSOR_PIN);
  gestureCheck(sensorValue);
  delay(20);
}

void sendKey(int key) {
  Keyboard.begin();
  Keyboard.press(key);
  Keyboard.release(key);
  Keyboard.end();
}

void leftKey() {
  analogWrite(RED_LED_PIN, 255);
  analogWrite(GREEN_LED_PIN, 0);
  analogWrite(BLUE_LED_PIN, 0);
  sendKey(KEY_LEFT_ARROW);
}

void rightKey() {
  analogWrite(RED_LED_PIN, 0);
  analogWrite(GREEN_LED_PIN, 255);
  analogWrite(BLUE_LED_PIN, 0);
  sendKey(KEY_RIGHT_ARROW);
}

void incomplete() {
  analogWrite(RED_LED_PIN, 0);
  analogWrite(GREEN_LED_PIN, 60);
  analogWrite(BLUE_LED_PIN, 60);
}

void idle() {
  analogWrite(RED_LED_PIN, 0);
  analogWrite(GREEN_LED_PIN, 0);
  analogWrite(BLUE_LED_PIN, 0);
}

void reset() {
  analogWrite(RED_LED_PIN, 0);
  analogWrite(GREEN_LED_PIN, 0);
  analogWrite(BLUE_LED_PIN, 255);
}

// The softpot is divided into 32 regions of 32 units each.
const int STEP_SIZE = 16;
// How many steps a motion need to reach to register as a swipe.
const int STEPS_THRESHOLD = 2;
// How many cycles of 0 voltage until the step count resets.
const int IDLE_CYCLE_TOLERANCE = 2;

// Starting point on the softpot. -1 is no gesture has started.
int gesture_start = -1;
// Distance from gesture_start measured in STEP_SIZE.
int gesture_steps = 0;
// How many cycles the gesture was idle
int cycles_idle = 0;

void gestureCheck(int value) {
  if (value <= 0) {
    // Given that the softpot can be finicky, we allow it to recieve a voltage of 0
    // for a few cycles before resetting the gesture.
    if (cycles_idle == IDLE_CYCLE_TOLERANCE && gesture_start == -1) {
      idle();
    } else if (cycles_idle == IDLE_CYCLE_TOLERANCE) {
      cycles_idle = 0;
      gesture_start = -1;
      gesture_steps = 0;
      reset();
    } else {
      cycles_idle++;
    }
    return;
  }

  // Start of a gesture.
  if (gesture_start == -1) {
    gesture_start = value;
    return;
  }

  // Add the distance travelled.
  gesture_steps = gesture_steps + ((gesture_start - value) / STEP_SIZE);

  // Do nothing if thresholds on either side are not reached.
  if (gesture_steps < STEPS_THRESHOLD && gesture_steps > -STEPS_THRESHOLD) {
    gesture_start = -1;
    incomplete();
    return;
  }

  // Trigger the left key based if the positve step threshold is reached.
  while (gesture_steps >= STEPS_THRESHOLD) {
    gesture_steps -= STEPS_THRESHOLD;
    leftKey();
  }

   // Trigger the right key based if the negative step threshold is reached.
  while (gesture_steps <= -STEPS_THRESHOLD) {
    gesture_steps += STEPS_THRESHOLD;
    rightKey();
  }

  // Begin the next gesture
  gesture_start = -1;
  gesture_steps = 0;
}


