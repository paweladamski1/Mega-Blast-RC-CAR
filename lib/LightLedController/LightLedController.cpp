#include "LightLedController.h"

LightLedController::LightLedController(
    uint8_t pinLeftIndicator,
    uint8_t pinRightIndicator,
    uint8_t pinBrake,
    uint8_t pinRear,
    uint8_t pinReverse,
    uint8_t pinAux
) :
    PIN_LEFT_INDICATOR(pinLeftIndicator),
    PIN_RIGHT_INDICATOR(pinRightIndicator),
    PIN_BRAKE(pinBrake),
    PIN_MAIN_REAR(pinRear),
    PIN_REVERSE(pinReverse),
    PIN_AUX(pinAux)
{}

void LightLedController::begin() {
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
}

void LightLedController::setBrakeLight(bool on) {
    digitalWrite(PIN_BRAKE, on ? HIGH : LOW);
}

void LightLedController::setMainRearLight(bool on) {
    digitalWrite(PIN_MAIN_REAR, on ? HIGH : LOW);
}

void LightLedController::setReverseLight(bool on) {
    digitalWrite(PIN_REVERSE, on ? HIGH : LOW);
}

void LightLedController::setAuxLight(bool on) {
    digitalWrite(PIN_AUX, on ? HIGH : LOW);
}

void LightLedController::startLeftIndicator() {
    if (leftTaskHandle == NULL) {
        xTaskCreatePinnedToCore(
            leftIndicatorTask,
            "LeftIndicator",
            1024,
            this,
            1,
            &leftTaskHandle,
            1
        );
    }
}

void LightLedController::stopLeftIndicator() {
    if (leftTaskHandle != NULL) {
        vTaskDelete(leftTaskHandle);
        leftTaskHandle = NULL;
        digitalWrite(PIN_LEFT_INDICATOR, LOW);
    }
}

void LightLedController::startRightIndicator() {
    if (rightTaskHandle == NULL) {
        xTaskCreatePinnedToCore(
            rightIndicatorTask,
            "RightIndicator",
            1024,
            this,
            1,
            &rightTaskHandle,
            1
        );
    }
}

void LightLedController::stopRightIndicator() {
    if (rightTaskHandle != NULL) {
        vTaskDelete(rightTaskHandle);
        rightTaskHandle = NULL;
        digitalWrite(PIN_RIGHT_INDICATOR, LOW);
    }
}

void LightLedController::leftIndicatorTask(void *param) {
    LightLedController* self = static_cast<LightLedController*>(param);
    bool state = false;
    while (true) {
        state = !state;
        digitalWrite(self->PIN_LEFT_INDICATOR, state ? HIGH : LOW);
        vTaskDelay(self->blinkInterval);
    }
}

void LightLedController::rightIndicatorTask(void *param) {
    LightLedController* self = static_cast<LightLedController*>(param);
    bool state = false;
    while (true) {
        state = !state;
        digitalWrite(self->PIN_RIGHT_INDICATOR, state ? HIGH : LOW);
        vTaskDelay(self->blinkInterval);
    }
}
