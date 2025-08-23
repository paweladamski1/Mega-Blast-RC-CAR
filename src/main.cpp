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

/* SD CARD */
#define SD_MISO 35
#define SD_SCK 13
#define SD_MOSI 2
#define SD_CS 15

bool IsShown = false;

StatusLedManager ledStatusManager(LED_STATUS);
PowerManager powerManager(CHARGER_DETECT_PIN, ledStatusManager);
MotorController motor(AIN1, AIN2, PWMA, BIN1, BIN2, PWMB, STBY);

SoundController sound(I2S_BCLK, I2S_LRCLK, I2S_DIN, SD_SCK, SD_MISO, SD_MOSI, SD_CS);
LightLedController lights(LED_LEFT_INDICATOR, LED_RIGHT_INDICATOR, LED_BRAKE, LED_MAIN_REAR, LED_REVERSE, LED_AUX, sound);
BluePad32Controller pad(lights, sound);

/*
 * SETUP
 */
int sampleRate;
void setup()
{
  sampleRate = 24000;
  Serial.begin(9600);
  delay(1000);
  Serial.println("");
  Serial.println("ESP Setup.....");

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SPI))
  {
    Serial.println("SD Card mount failed!");
    while (1)
      delay(100);
  }
  Serial.println("SD Card OK"); 
  
  ledStatusManager.begin();
  powerManager.begin();

  ledStatusManager.blinkAsync(EPOWER::NORMAL);

  pinMode(CHARGER_DETECT_PIN, INPUT);
  analogReadResolution(12);

  sound.begin();
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
  powerManager.loop();
  if (powerManager.isCharging())
  {
    delay(1000);
    return;
  }
  pad.loop(motor);
  delay(100);
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
      sound.playHorn();
      break;
    case 'e':
      sound.startEngine();
      break;
    case 's':
      sound.stopEngine();
      break;
    case 'b':
      sound.startBlinker();
      break;
    case 'x':
      sound.stopBlinker();
      break;
    case 'r':
      sampleRate = sampleRate - 100;
      Serial.println("new rate = ");
      Serial.print(sampleRate);
      sound.setEngineRpm(sampleRate);
      break;
    case 'q':
      sampleRate = sampleRate + 500;
      Serial.println("new rate = ");
      Serial.print(sampleRate);
      sound.setEngineRpm(sampleRate);
      break;
    }
  }
}

EPOWER currentPowerState = EPOWER::NORMAL;

bool isCharging()
{
  int analogValue = analogRead(CHARGER_DETECT_PIN);
  return analogValue > 2048; // np. 5V z ładowarki → wartość ADC > ~2.6V
}