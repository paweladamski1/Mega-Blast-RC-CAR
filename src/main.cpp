#include <Arduino.h>
#include <Bluepad32.h>
#include "MotorController.h"
#include "BluePad32Controller.h"
#include "StatusLedManager.h"
#include "PowerManager.h"

/* MOTOR DRIVER */
#define PWMA 32
#define AIN2 33
#define AIN1 25
#define STBY 26
#define BIN1 27
#define BIN2 14
#define PWMB 12


/* LEDS */
#define LED_STATUS 21

/* lights */
#define LED_LEFT_INDICATOR  18
#define LED_BRAKE           5
#define LED_MAIN_REAR       17
#define LED_REVERSE         16
#define LED_AUX             4
#define LED_RIGHT_INDICATOR 0

/* SENSORS  */
#define CHARGER_DETECT_PIN 34

bool IsShown = false;

StatusLedManager ledStatusManager(LED_STATUS);
PowerManager powerManager(CHARGER_DETECT_PIN, ledStatusManager);
MotorController motor(AIN1, AIN2, PWMA, BIN1, BIN2, PWMB, STBY);
BluePad32Controller controllerPS4(LED_MAIN_REAR);

/*
 * SETUP
 */
void setup()
{
  Serial.begin(9600);

  Serial.println("ESP Setup.....");

  ledStatusManager.begin();
  powerManager.begin();

  ledStatusManager.blinkAsync(EPOWER::NORMAL);

  pinMode(CHARGER_DETECT_PIN, INPUT);
  analogReadResolution(12);

  motor.begin();
  controllerPS4.begin();

  delay(100);
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