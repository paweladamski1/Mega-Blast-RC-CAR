#include <Arduino.h>
#include <Bluepad32.h>
#include "MotorController.h"
#include "BluePad32Controller.h"
#include "StatusLedManager.h"
#include "PowerManager.h"
#include "LightLedController.h"
#include "SoundController.h"
 

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

/* LIGHTS */
#define LED_LEFT_INDICATOR 18
#define LED_BRAKE 5
#define LED_MAIN_REAR 17
#define LED_REVERSE 16
#define LED_AUX 4
#define LED_RIGHT_INDICATOR 0

/* SENSORS  */
#define CHARGER_DETECT_PIN 34

/* SOUND */
#define I2S_BCLK 22
#define I2S_LRCLK 19
#define I2S_DIN 23

bool IsShown = false;


StatusLedManager ledStatusManager(LED_STATUS);
PowerManager powerManager(CHARGER_DETECT_PIN, ledStatusManager);
MotorController motor(AIN1, AIN2, PWMA, BIN1, BIN2, PWMB, STBY);
BluePad32Controller pad(LED_MAIN_REAR);
LightLedController lights(LED_LEFT_INDICATOR, LED_RIGHT_INDICATOR, LED_BRAKE, LED_MAIN_REAR, LED_REVERSE, LED_AUX);
SoundController sound(I2S_BCLK, I2S_LRCLK, I2S_DIN);

/*
 * SETUP
 */
void setup()
{
  Serial.begin(9600);

  Serial.println("ESP Setup.....");
  lights.begin();
 

  sound.begin();
//  sound.startEngine();
  //sound.setEngineRpm(500);
  //sound.startBlinker();
 

  ledStatusManager.begin();
  powerManager.begin();

  ledStatusManager.blinkAsync(EPOWER::NORMAL);

  pinMode(CHARGER_DETECT_PIN, INPUT);
  analogReadResolution(12);

  motor.begin();
  pad.begin();

  delay(100);

}

/*
 * LOOP
 */
void loop()
{
   if (Serial.available()) {
        char c = Serial.read();
        switch (c) {
            case 'h': sound.playHorn(); break;
            case 'e': sound.startEngine(); break;
            case 's': sound.stopEngine(); break;
            case 'b': sound.startBlinker(); break;
            case 'x': sound.stopBlinker(); break;
            case 'r': sound.setEngineRpm(1000); break;
            case 'q': sound.setEngineRpm(250); break;
        }
    }
    
  powerManager.loop();
  if (powerManager.isCharging())
  {
    delay(1000);
    return;
  }

  pad.loop(motor);
  delay(100);
}

EPOWER currentPowerState = EPOWER::NORMAL;

bool isCharging()
{
  int analogValue = analogRead(CHARGER_DETECT_PIN);
  return analogValue > 2048; // np. 5V z ładowarki → wartość ADC > ~2.6V
}