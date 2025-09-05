#include "PowerManager.h"

bool PowerManager::_charging = false; 

PowerManager::PowerManager(int chargerDetectPin, int ledStatusPin)
    : _chargerDetectPin(chargerDetectPin), _ledStatusPin(ledStatusPin)
{
}

void PowerManager::begin()
{
    pinMode(_chargerDetectPin, INPUT);
    pinMode(_ledStatusPin, OUTPUT);

    xTaskCreatePinnedToCore(_task, "_task", 4096, this, 1, NULL, 1);
}

void PowerManager::_task(void *param)
{
    auto *parent = static_cast<PowerManager *>(param);

    const int detectPin = parent->_chargerDetectPin;
    const int _ledStatusPin = parent->_ledStatusPin;
    _ledBootSequence(_ledStatusPin);

    int analogValue;
    while (true)
    {
        _charging = _getIsCharging(detectPin);
        if (isCharging)
        {
            _ledChargingStatus(_ledStatusPin);

        }else
            vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void PowerManager::_ledChargingStatus(const int _ledStatusPin)
{
    digitalWrite(_ledStatusPin, LOW);
    vTaskDelay(1950 / portTICK_PERIOD_MS);

    digitalWrite(_ledStatusPin, HIGH);
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

void PowerManager::_ledBootSequence(const int _ledStatusPin)
{
    for (int i = 0; i < 3; ++i)
    {
        digitalWrite(_ledStatusPin, HIGH);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        digitalWrite(_ledStatusPin, LOW);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    digitalWrite(_ledStatusPin, HIGH);
}

bool PowerManager::_getIsCharging(int detectPin)
{
    int analogValue = analogRead(detectPin);
    return analogValue > 2048;
}

bool PowerManager::isCharging()
{
    return _charging;
}