#include "StatusLedManager.h"

bool StatusLedManager::isBlinking = false;
TaskHandle_t StatusLedManager::blinkTaskHandle = nullptr;
uint8_t StatusLedManager::LED_PIN=-1;


StatusLedManager::StatusLedManager(uint8_t pin)
{
    LED_PIN = pin; 
} 

void StatusLedManager::begin()
{
    isBlinking = false;
    pinMode(LED_PIN, OUTPUT);
}

void StatusLedManager::blinkAsync(EPOWER status)
{
    // if exists then cancel
    if (blinkTaskHandle != nullptr)
    {
        vTaskDelete(blinkTaskHandle);
        blinkTaskHandle = nullptr;
        isBlinking = false;
    }
    Serial.println("blinkAsync");
    EPOWER *arg = new EPOWER(status);
    xTaskCreate(&StatusLedManager::blinkTask, "blink_task", 2048, arg, 5, &blinkTaskHandle);
}

void StatusLedManager::blinkTask(void *pvParameter)
{
    if (isBlinking)
    {
        vTaskDelete(NULL);
    }

    isBlinking = true;
    EPOWER status = *(EPOWER *)pvParameter;
    delete (EPOWER *)pvParameter;
    Serial.println("");
    Serial.print("blinkTask pin=");
    Serial.print(LED_PIN);
    Serial.println("");
    Serial.println("");
    switch (status)
    {
    case EPOWER::NORMAL:
        for (int i = 0; i < 3; ++i)
        {
            digitalWrite(LED_PIN, HIGH);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            digitalWrite(LED_PIN, LOW);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        digitalWrite(LED_PIN, HIGH);
        break;

    case EPOWER::LOW_BATTERY:
        for (int i = 0; i < 20; ++i)
        {
            digitalWrite(LED_PIN, HIGH);
            vTaskDelay(50 / portTICK_PERIOD_MS);
            digitalWrite(LED_PIN, LOW);
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
        break;

    case EPOWER::CHARGING:
        for (int i = 0; i < 10; ++i)
        {
            digitalWrite(LED_PIN, HIGH);
            vTaskDelay(50 / portTICK_PERIOD_MS);
            digitalWrite(LED_PIN, LOW);
            vTaskDelay(1950 / portTICK_PERIOD_MS);
        }
        break;
    }
    isBlinking = false;
    blinkTaskHandle = nullptr;
    vTaskDelete(NULL);
}
