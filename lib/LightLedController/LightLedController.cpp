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
        indicatorTask,
        "LeftIndicator",
        1024,
        this,
        1,
        &taskHandle,
        1);
}

void LightLedController::indicatorTask(void *param)
{
    LightLedController *self = static_cast<LightLedController *>(param);
    bool state = false;
    bool _leftIndicatorOn = self->_leftIndicatorOn;
    bool _rightIndicatorOn = self->_rightIndicatorOn;
    while (true)
    {
        if (self->_leftIndicatorOn)
        {
            _leftIndicatorOn = true;
            digitalWrite(self->PIN_LEFT_INDICATOR, true);
            self->sound.startBlinker("LightLedController - left indicator ON");
        }
        if (self->_rightIndicatorOn)
        {
            _rightIndicatorOn = true;
            digitalWrite(self->PIN_RIGHT_INDICATOR, true);
            self->sound.startBlinker("LightLedController - right indicator ON");
        }

        vTaskDelay(self->blinkInterval);

        if (_leftIndicatorOn)
            digitalWrite(self->PIN_LEFT_INDICATOR, false);
        if (_rightIndicatorOn)
            digitalWrite(self->PIN_RIGHT_INDICATOR, false);

        vTaskDelay(self->blinkInterval);

        if ((!self->_leftIndicatorOn && _leftIndicatorOn) || (!self->_rightIndicatorOn && _rightIndicatorOn))
        {
            if (!self->_leftIndicatorOn && !self->_rightIndicatorOn)
                self->sound.stopBlinker("LightLedController - right or left indicator OFF");
            _leftIndicatorOn = self->_leftIndicatorOn;
            _rightIndicatorOn = self->_rightIndicatorOn;
        }
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
