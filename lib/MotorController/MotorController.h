#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <Arduino.h>
#include "definitions.h"
 
class MotorController {
public:
    MotorController(uint8_t ain1, uint8_t ain2, uint8_t pwma,
                    uint8_t bin1, uint8_t bin2, uint8_t pwmb,
                    uint8_t stby);

    void begin();
    void drive(SControlData & controlData);
    void stopEngine();
    void startEngine();

private:
    static void _setMotor(uint8_t in1, uint8_t in2, uint8_t pwmPin, int speed);

    uint8_t _ain1, _ain2, _pwma;
    uint8_t _bin1, _bin2, _pwmb;
    uint8_t _stby;
    bool _isRunningState;
};

#endif
