#include "MotorController.h"

MotorController::MotorController(uint8_t ain1, uint8_t ain2, uint8_t pwma,
                    uint8_t bin1, uint8_t bin2, uint8_t pwmb,
                    uint8_t stby)
    : _ain1(ain1), _ain2(ain2), _pwma(pwma),
      _bin1(bin1), _bin2(bin2), _pwmb(pwmb),
      _stby(stby)
{
    
}

void MotorController::begin()
{
    _isRunningState = false; // Initialize the running state      

    pinMode(_ain1, OUTPUT);
    pinMode(_ain2, OUTPUT);
    pinMode(_pwma, OUTPUT);

    pinMode(_bin1, OUTPUT);
    pinMode(_bin2, OUTPUT);
    pinMode(_pwmb, OUTPUT);

    pinMode(_stby, OUTPUT); 
    #ifdef DEBUG
    Serial.printf(" MotorController::begin AIN1=%d, AIN2=%d, PWMA=%d, BIN1=%d, BIN2=%d, PWMB=%d, STBY=%d\n",
              _ain1, _ain2, _pwma, _bin1, _bin2, _pwmb, _stby);
    #endif
}

void MotorController::drive(SControlData &controlData)
{
    if (!_isRunningState)       
        return;

    int momentum = controlData.momentum; // 0 to 255
    int steering = controlData.steering; // -100 to 100
    int gear = controlData.gear;         // -1, 0, 1, 2

    momentum = constrain(momentum, 0, 255);
    steering = constrain(steering, -100, 100);
    if (steering > -5 && steering < 5)
        steering = 0;

    int _lSteering = (steering < 0) ? abs(steering) : 0;
    int _rSteering = (steering > 0) ? steering : 0;

    _lSteering = map(_lSteering, 0, 100, 0, 255);
    _rSteering = map(_rSteering, 0, 100, 0, 255);

    int leftSpeed = constrain((momentum - _lSteering) + abs(_rSteering), 0, 255);
    int rightSpeed = constrain((momentum - _rSteering) + abs(_lSteering), 0, 255);

   /*if (momentum == 0)
    {
        if (leftSpeed == 0 and rightSpeed > 0)
            leftSpeed = -rightSpeed;

        if (rightSpeed == 0 and leftSpeed > 0)
            rightSpeed = -leftSpeed;
    }*/
#ifdef DEBUG
    Serial.printf("AIN1=%d, AIN2=%d, PWMA=%d, BIN1=%d, BIN2=%d, PWMB=%d, STBY=%d\n",
              _ain1, _ain2, _pwma, _bin1, _bin2, _pwmb, _stby);
#endif
    _setMotor(_ain1, _ain2, _pwma, (gear >= 0) ? leftSpeed : -leftSpeed);
    _setMotor(_bin1, _bin2, _pwmb, (gear >= 0) ? rightSpeed : -rightSpeed);
}

void MotorController::stopEngine()
{
    Serial.println("MotorController::stopEngine");
    
    digitalWrite(_stby, LOW);
    _setMotor(_ain1, _ain2, _pwma, 0);
    _setMotor(_bin1, _bin2, _pwmb, 0);
    _isRunningState = false;
}

void MotorController::startEngine()
{
    Serial.println("MotorController::startEngine");
    _isRunningState = true;
    digitalWrite(_stby, HIGH);
}

void MotorController::_setMotor(uint8_t in1, uint8_t in2, uint8_t pwmPin, int speed)
{  
    const int MAX_SAFE_PWM = 255; // 100%
    bool forward = true;
    forward = speed >= 0;

    uint16_t in1Data = (forward) ? HIGH : LOW;
    uint16_t in2Data = (!forward) ? HIGH : LOW;
    digitalWrite(in1, in1Data);
    digitalWrite(in2, in2Data);
    speed = constrain(speed, -MAX_SAFE_PWM, MAX_SAFE_PWM);
    analogWrite(pwmPin, abs(speed));
}
