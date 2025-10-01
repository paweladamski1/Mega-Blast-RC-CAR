#include "LightLedController.h"

LightLedController::LightLedController(
    uint8_t pinLeftIndicator,
    uint8_t pinRightIndicator,
    uint8_t pinBrake,
    uint8_t pinRear,
    uint8_t pinReverse,
    uint8_t pinAux,
    AudioClipController &sound) : PIN_LEFT_INDICATOR(pinLeftIndicator),
                                  PIN_RIGHT_INDICATOR(pinRightIndicator),
                                  PIN_BRAKE(pinBrake),
                                  PIN_MAIN_REAR(pinRear),
                                  PIN_REVERSE(pinReverse),
                                  PIN_AUX(pinAux),
                                  _leftIndicatorOn(false),
                                  _rightIndicatorOn(false),
                                  sound(sound)

{
}

void LightLedController::begin()
{
    pinMode(PIN_LEFT_INDICATOR, OUTPUT);
    pinMode(PIN_RIGHT_INDICATOR, OUTPUT);
    pinMode(PIN_BRAKE, OUTPUT);
    pinMode(PIN_MAIN_REAR, OUTPUT);
    pinMode(PIN_REVERSE, OUTPUT);
    pinMode(PIN_AUX, OUTPUT);

    digitalWrite(PIN_LEFT_INDICATOR, LOW);
    digitalWrite(PIN_RIGHT_INDICATOR, LOW);
    digitalWrite(PIN_BRAKE, LOW);
    digitalWrite(PIN_MAIN_REAR, LOW);
    digitalWrite(PIN_REVERSE, LOW);
    digitalWrite(PIN_AUX, LOW);

    xTaskCreatePinnedToCore(
        _indicatorTask,
        "indicatorTask",
        1024,
        this,
        1,
        &_taskHandle,
        1);
}

void LightLedController::_indicatorTask(void *param)
{
    LightLedController *self = static_cast<LightLedController *>(param);
    bool state = false;

    while (true)
    {
        self->_indicatorTask();
    }
}

void LightLedController::_indicatorTask()
{
    static bool leftIndicatorOn_State = _leftIndicatorOn;
    static bool rightIndicatorOn_State = _rightIndicatorOn;
    if (_leftIndicatorOn)
    {
        leftIndicatorOn_State = true;
        digitalWrite(PIN_LEFT_INDICATOR, true);
        sound.playStartBlinker("LightLedController - left indicator ON");
    }
    if (_rightIndicatorOn)
    {
        rightIndicatorOn_State = true;
        digitalWrite(PIN_RIGHT_INDICATOR, true);
        sound.playStartBlinker("LightLedController - right indicator ON");
    }

    vTaskDelay(_blinkInterval);

    if (leftIndicatorOn_State)
        digitalWrite(PIN_LEFT_INDICATOR, false);
    if (rightIndicatorOn_State)
        digitalWrite(PIN_RIGHT_INDICATOR, false);

    vTaskDelay(_blinkInterval);

    // check if state changed during the delay
    if ((!_leftIndicatorOn && leftIndicatorOn_State) || (!_rightIndicatorOn && rightIndicatorOn_State))
    {
        if (!_leftIndicatorOn && !_rightIndicatorOn)
            sound.stopBlinker("LightLedController - right or left indicator OFF");

        leftIndicatorOn_State = _leftIndicatorOn;
        rightIndicatorOn_State = _rightIndicatorOn;
    }
}

void LightLedController::setBrakeLight(bool on)
{
    digitalWrite(PIN_BRAKE, on ? HIGH : LOW);
}

void LightLedController::setMainRearLight(bool on)
{
    digitalWrite(PIN_MAIN_REAR, on ? HIGH : LOW);
}

void LightLedController::setReverseLight(bool on)
{
    digitalWrite(PIN_REVERSE, on ? HIGH : LOW);
    sound.playBackingUpBeep(on);
}

void LightLedController::setAuxLight(bool on)
{
    digitalWrite(PIN_AUX, on ? HIGH : LOW);
}

void LightLedController::setIndicator(bool leftOn, bool rightOn)
{
    _leftIndicatorOn = leftOn;
    _rightIndicatorOn = rightOn;
}
