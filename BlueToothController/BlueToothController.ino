#include <Arduino.h>
#include <BleGamepad.h>

// Thumbstick Pins
#define THUMBSTICK_X 4
#define THUMBSTICK_Y 2
#define THUMBSTICK_BTN 16

// Button Pins
#define BTN1_PIN 34
#define BTN2_PIN 27
#define BTN3_PIN 14
#define BTN4_PIN 13

// TILT SWITCH AND SENSORS
#define SWITCH_PIN 21
#define DO_TILT digitalRead(SWITCH_PIN)

#define TILT_SENSOR_LEFT 25
#define TILT_SENSOR_RIGHT 26
#define TILT_SENSOR_LEFT_LED_PIN 19
#define TILT_SENSOR_RIGHT_LED_PIN 18
#define TILT_VALUE_LEFT analogRead(TILT_SENSOR_LEFT)
#define TILT_VALUE_RIGHT analogRead(TILT_SENSOR_RIGHT)

// Globals
BleGamepad bleGamepad;

struct Thumbstick{
  int X;
  int Y;
  int SW;
};

Thumbstick thumbstick_values;

struct GamepadButton {
  uint8_t pin;
  uint8_t buttonID;
};

GamepadButton gamepadButtons[] {
  {BTN1_PIN, BUTTON_1},
  {BTN2_PIN, BUTTON_2},
  {BTN3_PIN, BUTTON_3},
  {BTN4_PIN, BUTTON_4}
};

// Read Thumbstick values
Thumbstick getThumbstickValues(int Xpin, int Ypin, int SWpin){
  int XVal = analogRead(Xpin);
  int YVal = analogRead(Ypin);
  int SWVal = digitalRead(SWpin);
  Thumbstick returnVal = {XVal, YVal, SWVal};
  return returnVal;
}

// Update BLE Button States
void update_buttons(){
  for (int i = 0; i < 4; i++) {
    bool pressed = digitalRead(gamepadButtons[i].pin) == LOW;
    if (pressed) {
      bleGamepad.press(gamepadButtons[i].buttonID);
    } else {
      bleGamepad.release(gamepadButtons[i].buttonID);
    }
  }
}

// Update Thumbstick based on tilt sensors
void update_thumbstick_tilt(){
  int yVal = (thumbstick_values.Y*32767L)/4095;
  if(TILT_VALUE_LEFT > 1500){
    bleGamepad.setAxes(0, yVal, 0, 0, 0, 0, 0, 0);  
    digitalWrite(TILT_SENSOR_LEFT_LED_PIN, HIGH);
    digitalWrite(TILT_SENSOR_RIGHT_LED_PIN, LOW);
  }
  else if (TILT_VALUE_RIGHT > 1500){
    bleGamepad.setAxes(32767L, yVal, 0, 0, 0, 0, 0, 0);  
    digitalWrite(TILT_SENSOR_LEFT_LED_PIN, LOW);
    digitalWrite(TILT_SENSOR_RIGHT_LED_PIN, HIGH);
  }
  else{
    bleGamepad.setAxes(16383, yVal, 0, 0, 0, 0, 0, 0);  
    digitalWrite(TILT_SENSOR_LEFT_LED_PIN, LOW);
    digitalWrite(TILT_SENSOR_RIGHT_LED_PIN, LOW);
  }
}


void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  // Configure input pins
  pinMode(THUMBSTICK_BTN, INPUT_PULLUP);
  pinMode(BTN1_PIN, INPUT);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);
  pinMode(BTN4_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  // Configure output pins
  pinMode(TILT_SENSOR_LEFT_LED_PIN, OUTPUT);
  pinMode(TILT_SENSOR_RIGHT_LED_PIN, OUTPUT);

  // Start BLE gamepad
  bleGamepad.begin();
}

void loop() 
{
  // Check for BLE connection
  if (bleGamepad.isConnected())
  {
    // Get inputs
    thumbstick_values = getThumbstickValues(THUMBSTICK_X, THUMBSTICK_Y, THUMBSTICK_BTN);
    update_buttons();

    // If the tilt sensor is enabled, look at that, otherwise use the full joystick for controls
    if(!DO_TILT){
      digitalWrite(TILT_SENSOR_LEFT_LED_PIN, LOW);
      digitalWrite(TILT_SENSOR_RIGHT_LED_PIN, LOW);
      bleGamepad.setAxes((thumbstick_values.X*32767L)/4095, (thumbstick_values.Y*32767L)/4095, 0, 0, 0, 0, 0, 0);  
    }else{
      update_thumbstick_tilt();
    }
    // Check for Thumbstick push button
    if(!thumbstick_values.SW){
      bleGamepad.press(BUTTON_16);
    } else {
      bleGamepad.release(BUTTON_16);
    }
  }
}