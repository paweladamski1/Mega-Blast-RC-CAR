#include "PowerManager.h"

PowerManager::PowerManager(uint8_t chargerDetectPin, StatusLedManager& ledManager)
    : pin(chargerDetectPin), ledMgr(ledManager), currentState(EPOWER::NORMAL), charging(false)
{}

void PowerManager::begin()
{
    pinMode(pin, INPUT);
}

void PowerManager::loop()
{
    int analogValue = analogRead(pin);
    bool nowCharging = analogValue > 2048;

    if (nowCharging != charging)
    {
        charging = nowCharging;
        currentState = charging ? EPOWER::CHARGING : EPOWER::NORMAL;
        ledMgr.blinkAsync(currentState);
    }
}

bool PowerManager::isCharging() const
{
    return charging;
}
