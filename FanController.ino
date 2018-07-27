// Define these to exclude the alarm search functions and new operator
#define REQUIRESALARMS false
#define REQUIRESNEW false

#include <OneWire.h>
#include <DallasTemperature.h>

const int PIN_ONEWIRE = 3;
const int PIN_CONTROL = 2;
const int PIN_BLUE_LED = 1;
const int PIN_GREEN_LED = 4;
const int PIN_BUTTON = 0;

const int TEMP_DIFF_DEG_C = 2;

// 0: 28 FF 72 A0 24 17 3 65
// 1: 28 FF 69 A1 24 17 3 E6

int thermometerInside = 0;
int thermometerOutside = 1;
int fanState = 0;

OneWire oneWire(PIN_ONEWIRE);
DallasTemperature thermometers(&oneWire);

const int MODE_AUTO = 0;
const int MODE_MANUAL_ON = 1;
const int MODE_MANUAL_OFF = 2;

// State and mode variables
int currentMode = MODE_AUTO;
int blueLEDState = 0;
int greenLEDState = 0;
int buttonState = 1;

const unsigned long ledDuration = 500000;
unsigned long ledTime = micros();

unsigned long debounceTime = 0;
const unsigned long debounceDuration = 2000;

void setup() {
  // put your setup code here, to run once:
  // Serial.begin(115200);
  pinMode(PIN_BLUE_LED, OUTPUT);
  pinMode(PIN_GREEN_LED, OUTPUT);
  pinMode(PIN_CONTROL, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);

  digitalWrite(PIN_BLUE_LED, 1);
  digitalWrite(PIN_GREEN_LED, 0);
  digitalWrite(PIN_CONTROL, 0);
  
  thermometers.begin();
  // Serial.println("Setup finished");
  uint8_t count = thermometers.getDeviceCount();
  // Serial.print(count);
  // Serial.println(" thermometers");
  DeviceAddress deviceAddr;
  for (int i = 0; i < count; i++) {
    if (thermometers.getAddress(deviceAddr, i)) {
      // Serial.print("Address ");
      // Serial.print(i);
      // Serial.print(": ");
      for (int j = 0; j < 8; j++) {
         // Serial.print(deviceAddr[j], HEX);
         // Serial.print(" ");
      }
      if (deviceAddr[7] == 0x65) {
        thermometerInside = i;
        // Serial.println("Found thermometer inside!");
      } else if (deviceAddr[7] == 0xE6) {
        thermometerOutside = i;
        // Serial.println("Found thermometer outside!");
      }
      
      // Serial.println();
    } else {
      // Serial.print("getAddress failed for ");
      // Serial.println(i);
    }
  }
  
  // Serial.print("Thermometer inside: ");
  // Serial.println(thermometerInside);
  // Serial.print("Thermometer outside: ");
  // Serial.println(thermometerOutside); 
}

void fanOn() {
  fanState = 1;
  greenLEDState = 1;
  digitalWrite(PIN_GREEN_LED, 1);
  digitalWrite(PIN_CONTROL, 1);
}

void fanOff() {
  fanState = 0;
  greenLEDState = 0;  
  digitalWrite(PIN_GREEN_LED, 0);
  digitalWrite(PIN_CONTROL, 0);
}

void incrementMode() {
  switch(currentMode) {
    case MODE_AUTO:
    currentMode = MODE_MANUAL_ON;
    digitalWrite(PIN_BLUE_LED, 1);
    fanOn();
    break;

    case MODE_MANUAL_ON:
    currentMode = MODE_MANUAL_OFF;
    fanOff();
    digitalWrite(PIN_BLUE_LED, 1);
    break;

    case MODE_MANUAL_OFF:
    currentMode = MODE_AUTO;
    fanOff();
    break;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (currentMode == MODE_AUTO) {
    // Serial.print("Requesting temperatures...");  
    thermometers.requestTemperatures();
    // Serial.println("Done!");
    int tempInside = thermometers.getTempCByIndex(thermometerInside);
    int tempOutside = thermometers.getTempCByIndex(thermometerOutside);
    if (tempInside - tempOutside >= TEMP_DIFF_DEG_C && fanState == 0) {
      // Serial.println("Turn on the fan!");
      fanOn();
    }
    if (tempInside - tempOutside <= 0 && fanState == 1) {
      // Serial.println("Turn off the fan!");
      fanOff();
    }
    // Serial.print("Outside: ");
    // Serial.print(tempOutside);
    // Serial.print("  Inside: ");
    // Serial.print(tempInside);
    // Serial.print(" Fan state: ");
    // Serial.println(fanState);
  }

  unsigned long now = micros();

  if (debounceTime == 0 || now - debounceTime > debounceDuration) {
    debounceTime = 0; 
    // Check the button state
    int currentState = digitalRead(PIN_BUTTON);
    // Serial.print("Current state: ");
    // Serial.println(currentState);
    if (buttonState == 0 && currentState == 1) {
      buttonState = 1;
      // Serial.println("Button release!");
      debounceTime = now;
    } else if (buttonState == 1 && currentState == 0) {
      buttonState = 0;
      debounceTime = now;
      incrementMode();
      // Serial.println("Button press!");
    }  
  }

  // Blink the LED if we're in auto mode
  if (currentMode == MODE_AUTO) {
    if (now - ledTime > ledDuration) {
      blueLEDState = (blueLEDState == 0 ? 1 : 0);
      digitalWrite(PIN_BLUE_LED, blueLEDState);
      ledTime = now;        
    }
  }
}
