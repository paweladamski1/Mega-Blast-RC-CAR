#ifndef LIGHT_LED_CONTROLLER_H
#define LIGHT_LED_CONTROLLER_H

#include <Arduino.h>
#include "AudioClipController.h"

class LightLedController {
public:
    LightLedController(
        uint8_t pinLeftIndicator,
        uint8_t pinRightIndicator,
        uint8_t pinBrake,
        uint8_t pinRear,
        uint8_t pinReverse,
        uint8_t pinAux,
        AudioClipController &sound
    );

    void begin();

    void setBrakeLight(bool on);
    void setMainRearLight(bool on);
    void setReverseLight(bool on);
    void setAuxLight(bool on);
    void setIndicator(bool leftOn, bool rightOn); 
private:
    bool _leftIndicatorOn;
    bool _rightIndicatorOn;
    AudioClipController &sound;
    TaskHandle_t _taskHandle = NULL;

    uint8_t PIN_LEFT_INDICATOR;
    uint8_t PIN_RIGHT_INDICATOR;
    uint8_t PIN_BRAKE;
    uint8_t PIN_MAIN_REAR;
    uint8_t PIN_REVERSE;
    uint8_t PIN_AUX;

    const TickType_t _blinkInterval = 500 / portTICK_PERIOD_MS;
    static void _indicatorTask(void *param);    
    void _indicatorTask(); 
};

#endif
