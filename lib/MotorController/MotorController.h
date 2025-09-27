#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <Arduino.h>
 
class MotorController {
public:
    MotorController(int ain1, int ain2, int pwma,
                    int bin1, int bin2, int pwmb,
                    int stby);

    void begin();
    void drive(int throttle, int steering, int gear); // throttle: -255 to 255, steering: -100 to 100
    void stop();
    void startEngine();

private:
    void setMotor(int in1, int in2, int pwmPin, int speed);

    int _ain1, _ain2, _pwma;
    int _bin1, _bin2, _pwmb;
    int _stby;
    bool isStopFlag;
};

#endif
