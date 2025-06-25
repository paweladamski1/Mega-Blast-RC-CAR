#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "StatusLedManager.h"

class PowerManager {
public:
    PowerManager(uint8_t chargerDetectPin, StatusLedManager& ledManager);

    void begin();
    void loop();
    bool isCharging() const;

private:
    uint8_t pin;
    StatusLedManager& ledMgr;
    EPOWER currentState;
    bool charging;
};

#endif
