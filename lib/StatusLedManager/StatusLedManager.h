#ifndef STATUSLEDMANAGER_H
#define STATUSLEDMANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

enum class EPOWER
{
    NORMAL,     // Init OK
    CHARGING,   // Slow blink every 2s
    LOW_BATTERY // Fast blinking
};

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

class StatusLedManager
{
public:
    StatusLedManager(uint8_t pin);
    void begin();
    void blinkAsync(EPOWER status);

private:
    uint8_t ledPin;
    static bool isBlinking;
    static TaskHandle_t blinkTaskHandle;

    static void blinkTask(void *pvParameter);
};

#endif
