#include "MotorController.h"

MotorController::MotorController(int ain1, int ain2, int pwma,
                                 int bin1, int bin2, int pwmb,
                                 int stby)
    : _ain1(ain1), _ain2(ain2), _pwma(pwma),
      _bin1(bin1), _bin2(bin2), _pwmb(pwmb),
      _stby(stby) {}

void MotorController::begin()
{
    pinMode(_ain1, OUTPUT);
    pinMode(_ain2, OUTPUT);
    pinMode(_pwma, OUTPUT);

    pinMode(_bin1, OUTPUT);
    pinMode(_bin2, OUTPUT);
    pinMode(_pwmb, OUTPUT);

    pinMode(_stby, OUTPUT);
    
    digitalWrite(_stby, HIGH);
    isStopFlag = true;
}

void MotorController::drive(int throttle, int steering, EDirection direction)
{
    isStopFlag = (throttle == 0) ? true : false;

    throttle = constrain(throttle, -255, 255);
    steering = constrain(steering, -100, 100);
    if (steering > -5 && steering < 5)
        steering = 0;

    int _lSteering = (steering < 0) ? abs(steering) : 0;
    int _rSteering = (steering > 0) ? steering : 0;

    _lSteering = map(_lSteering, 0, 100, 0, 255);
    _rSteering = map(_rSteering, 0, 100, 0, 255);

    int leftSpeed = constrain((throttle - _lSteering) + abs(_rSteering), 0, 255);
    int rightSpeed = constrain((throttle - _rSteering) + abs(_lSteering), 0, 255);

    setMotor(_ain1, _ain2, _pwma, (direction == EDirection::FORWARD) ? leftSpeed : -leftSpeed);
    setMotor(_bin1, _bin2, _pwmb, (direction == EDirection::FORWARD) ? rightSpeed : -rightSpeed);
}

void MotorController::stop()
{
    if (isStopFlag)
        return;
    isStopFlag = true;
    setMotor(_ain1, _ain2, _pwma, 0);
    setMotor(_bin1, _bin2, _pwmb, 0);

}

void MotorController::startEngine()
{
    digitalWrite(_stby, HIGH);    
}

void MotorController::setMotor(int in1, int in2, int pwmPin, int speed)
{
    /*Serial.print(" ");
    if (in1 == _ain1)
        Serial.print("A");
    else if (in1 == _bin1)
        Serial.print("B");*/

    const int MAX_SAFE_PWM = 255; // 100%
    bool forward = true;
    forward = speed >= 0;

    uint16_t in1Data = (forward) ? HIGH : LOW;
    uint16_t in2Data = (!forward) ? HIGH : LOW;
    digitalWrite(in1, in1Data);
    digitalWrite(in2, in2Data);
    speed = constrain(speed, -MAX_SAFE_PWM, MAX_SAFE_PWM);

   /*Serial.print("      in1:");
    Serial.print(in1Data);

    Serial.print(" in2:");
    Serial.print(in2Data);

    Serial.print(" speed:");
    Serial.print(speed);
    if (in1 == _bin1)    
        Serial.println("");*/
        
    analogWrite(pwmPin, abs(speed));
}
