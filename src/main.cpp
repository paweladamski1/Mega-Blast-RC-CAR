#include <Arduino.h>
#include <Bluepad32.h>
#include "MotorController.h"
#include "PS4ControllerInput.h"
#include "StatusLedManager.h"
#include "PowerManager.h"

// ch2 A
#define AIN1 25
#define AIN2 26
#define PWMA 27

// ch1 B
#define BIN1 33
#define BIN2 32
#define PWMB 14

// Standby pin
#define STBY 13

#define LED_STATUS 2
#define LED_LIGHT 16
#define CHARGER_DETECT_PIN 34


bool IsShown = false;

StatusLedManager ledStatusManager(LED_STATUS);
PowerManager powerManager(CHARGER_DETECT_PIN, ledStatusManager);
MotorController motor(AIN1, AIN2, PWMA, BIN1, BIN2, PWMB, STBY);
PS4ControllerInput controllerPS4(LED_LIGHT);

/*
 * SETUP
 */
void setup()
{
  // sets pins
  ledStatusManager.begin();
  powerManager.begin();

  ledStatusManager.blinkAsync(EPOWER::NORMAL);

  pinMode(CHARGER_DETECT_PIN, INPUT);
  analogReadResolution(12);

  Serial.begin(9600);
  motor.begin();
  controllerPS4.begin();

  delay(100);

  Serial.println("Bluetooth started.");
  Serial.print("ESP32 MAC address (BT): ");
}

/*
 * LOOP
 */
void loop()
{
  powerManager.loop();
  if (powerManager.isCharging())
  {
    delay(1000);
    return;
  }

  controllerPS4.loop(motor);
  delay(100);
}

EPOWER currentPowerState = EPOWER::NORMAL;

bool isCharging()
{
  int analogValue = analogRead(CHARGER_DETECT_PIN);
  return analogValue > 2048; // np. 5V z ładowarki → wartość ADC > ~2.6V
}