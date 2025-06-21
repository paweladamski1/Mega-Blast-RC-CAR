#include <Arduino.h>
#include "BluetoothSerial.h"
#include <PS4Controller.h>
#include "MotorController.h"
#include "PS4ControllerInput.h"

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
#define LED_LIGHT  16

BluetoothSerial SerialBT;
bool IsShown = false;

// delcarations
void BlinkStatusLedAsync();
void blinkTask(void *pvParameter);



MotorController motor(AIN1, AIN2, PWMA,  BIN1, BIN2, PWMB, STBY); 
PS4ControllerInput controllerPS4(LED_LIGHT);

/*
 * SETUP
 */
void setup()
{
  //sets pins
  pinMode(LED_STATUS, OUTPUT);

  Serial.begin(9600);
  motor.begin();
  controllerPS4.begin(); 

  BlinkStatusLedAsync();
  delay(100);
  if (!SerialBT.begin("f8:b3:b7:58:f1:fa"))
  {
    Serial.println("BluetoothSerial initialization failed");
    return;
  }

  Serial.println("Bluetooth started.");
  Serial.print("ESP32 MAC address (BT): ");
  Serial.println(SerialBT.getBtAddressString());
}

/*
 * LOOP
 */
void loop()
{  
  controllerPS4.loop(motor);
  delay(20);
}

/*
 * LED Status Indicator Functions
 *
 * 1. Initialization Success:
 *    - LED blinks 3 times, then stays on to indicate successful startup
 *
 * 2. Low Battery Warning:
 *    - LED fast-blinks to indicate the battery needs charging
 *
 * 3. Battery Charging:
 *    - LED is normally off
 *    - Short blink every 2 seconds to indicate charging is in progress
 */
volatile bool isBlinking = false;

void BlinkStatusLedAsync()
{
  if (!isBlinking)
  {
    xTaskCreate(&blinkTask, "blink_task", 2048, NULL, 5, NULL);
  }
}

void blinkTask(void *pvParameter)
{
  if (isBlinking)
  {
    vTaskDelete(NULL); 
  }
  isBlinking = true;

  for (int i = 0; i < 5; ++i)
  {
    digitalWrite(LED_STATUS, HIGH);
    delay(100);
    digitalWrite(LED_STATUS, LOW);
    delay(100);
  }
  digitalWrite(LED_STATUS, HIGH);

  isBlinking = false;
  vTaskDelete(NULL); 
}



