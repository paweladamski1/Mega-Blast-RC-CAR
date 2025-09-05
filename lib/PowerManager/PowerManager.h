#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>


class PowerManager {
public:
    PowerManager(int chargerDetectPin, int ledStatusPin );

    void begin();    
    static bool isCharging();

private:
    int _chargerDetectPin;
    int _ledStatusPin;
    static bool _charging;

    static void _task(void *param);
    static void _ledChargingStatus(const int _ledStatusPin);
    static void _ledBootSequence(const int _ledStatusPin);
    static bool _getIsCharging(int detectPin);
};

#endif
