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
BluePad32Controller pad(lights, sound, motor);
 

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
  pad.loop();
  delay(50);
}

