#include <Arduino.h>
#include <Bluepad32.h>
#include "MotorController.h"
#include "BluePad32Controller.h"

#include "PowerManager.h"
#include "LightLedController.h"
#include "AudioClipController.h"
#include "definitions.h"

bool IsShown = false;

PowerManager powerManager(CHARGER_DETECT_PIN, LED_STATUS);
MotorController motor(AIN1, AIN2, PWMA, BIN1, BIN2, PWMB, STBY);

AudioClipController sound(I2S_BCLK, I2S_LRCLK, I2S_DIN, SD_SCK, SD_MISO, SD_MOSI, SD_CS);
LightLedController lights(LED_LEFT_INDICATOR, LED_RIGHT_INDICATOR, LED_BRAKE, LED_MAIN_REAR, LED_REVERSE, LED_AUX, sound);
BluePad32Controller pad(lights, sound);
 

/*
 * SETUP
 */
int sampleRate;
void setup()
{
  Serial.begin(9600);
  sampleRate = 24000;

  delay(1000);
  Serial.println("");
  Serial.println("ESP Setup.....");

  sound.begin();

  powerManager.begin();
  analogReadResolution(12);
  lights.begin();
  motor.begin();
  pad.begin();

  delay(100);
  lights.setIndicator(true, true);
}

/*
 * LOOP
 */

const int timeout = 80000;
void loop()
{
  test_sound();
  if (PowerManager::isCharging())
  {
    Serial.println("Charging....");
    delay(1000);
    return;
  }
  pad.loop(motor);
  delay(50);
}

void test_sound()
{
  if (Serial.available())
  {
    // Display test mode information
    Serial.println("=== TEST MODE - Sound Control ===");
    Serial.println("Commands:");
    Serial.println("h - Play horn");
    Serial.println("e - Start engine");
    Serial.println("s - Stop engine");
    Serial.println("b - Start blinker");
    Serial.println("x - Stop blinker");
    Serial.println("r - Set RPM --100");
    Serial.println("q - Set RPM ++100");
    Serial.println("=================================");
    char c = Serial.read();
    Serial.println(c);

    switch (c)
    {
    case 'h':
      sound.playHorn("main");
      break;
    case 'e':
      sound.playStartEngine("main");
      break;
    case 's':
      sound.stopEngine("main");
      break;
    case 'b':
      sound.playStartBlinker("main");
      break;
    case 'x':
      sound.stopBlinker("main");
      break;
    case 'r':
      sampleRate = sampleRate - 100;
      Serial.println("new rate = ");
      Serial.print(sampleRate);
      sound.setEngineRpm("main", sampleRate);
      break;
    case 'q':
      sampleRate = sampleRate + 500;
      Serial.println("new rate = ");
      Serial.print(sampleRate);
      sound.setEngineRpm("main", sampleRate);
      break;
    }
  }
}
