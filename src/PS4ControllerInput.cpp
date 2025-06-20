#include "PS4ControllerInput.h"

bool PS4ControllerInput::IsConnected = false;

PS4ControllerInput::PS4ControllerInput(int led_light) : _ledLight(led_light)
{
    currentData = {0, 0, false};
    IsConnected = false;
}

void PS4ControllerInput::begin()
{
    pinMode(_ledLight, OUTPUT);
    PS4.begin();
    PS4.setLed(0, 0, 255);
}

void PS4ControllerInput::loop(MotorController &motor)
{
    if (!PS4.isConnected())
        return;

    if (PS4.PSButton())
    {
        Serial.println("PS button was pressed - esp_restart!");
        setLight(false);
        esp_restart();
    }

    if (!IsConnected)
    {
        setLight(true);
        PS4.setFlashRate(10, 10);
        PS4.setRumble(50, 0);
        PS4.sendToController();
        delay(200);
        PS4.setRumble(0, 0);
        PS4.sendToController();
        IsConnected = true;
        motor.startEngine();
        Serial.println("Connected to PS4 Controller!");
    }

    if (PS4.Right())
        Serial.println("Strzałka w prawo");
    if (PS4.Left())
        Serial.println("Strzałka w lewo");
    if (PS4.Up())
        Serial.println("Strzałka w górę");
    if (PS4.Down())
        Serial.println("Strzałka w dół");

    if (PS4.Square())
        Serial.println("Kwadrat");

    if (PS4.Cross())
        Serial.println("Krzyżyk");

    if (PS4.Circle())
        Serial.println("Kółko");

    if (PS4.Triangle())
        Serial.println("Trójkąt");

    ControlData data = getControlData();
    motor.drive(data.momentum, data.steering, data.IsReverse);
    delay(70);
}

void PS4ControllerInput::setLight(bool turn_on)
{
    digitalWrite(_ledLight, (turn_on) ? HIGH : LOW);
}

ControlData PS4ControllerInput::getControlData()
{
    if (!PS4.isConnected())
    {
        // reset
        currentData.momentum = 0;
        currentData.steering = 0;
        currentData.brake = false;
        return currentData;
    }
    static bool isReverse = false;
    static int _s_loop_cnt = 0;
    _s_loop_cnt++;

    int throttle = PS4.R2Value();
    int brakeForce = PS4.L2Value();

    bool brake = PS4.L2Value() > 0;
    int steering = map(PS4.LStickX(), -128, 127, -100, 100);
    if (PS4.R1() && currentData.momentum == 0)
    {
        isReverse = !isReverse;
        Serial.println("switch direction:");
        controllerSetDirectionAction((!isReverse)? EDirection::FORWARD : EDirection::REVERSE );
        Serial.print(isReverse);
        delay(500);
    }

    if (currentData.momentum >= throttle && !brake) // free-rolling simulation
    {
        if (_s_loop_cnt % 5 == 0)
            currentData.momentum--;
    }
    else if (currentData.momentum > throttle && brake) // braking
    {
        if (_s_loop_cnt % 5 == 0)
            currentData.momentum -= brakeForce;
    }
    else if (throttle > currentData.momentum)
    {
        controllerAcceleratingAction(throttle);
        currentData.momentum += 10;
    }
    if (currentData.momentum == 0)
    {
        controllerIdleAction();
    }
    currentData.momentum = constrain(currentData.momentum, 0, 255);

    // currentData.momentum=throttle;
    // currentData.throttle=constrain(currentData.throttle, 0, 255);
    currentData.steering = steering;
    currentData.brake = brake;
    currentData.IsReverse = isReverse;

    return currentData;
}

void PS4ControllerInput::controllerAcceleratingAction(int throttle)
{
    PS4.setRumble(0, throttle);
    PS4.sendToController();
}

void PS4ControllerInput::controllerBreakingAction(int power)
{
    PS4.setRumble(0, power);
    PS4.sendToController();
}

void PS4ControllerInput::controllerIdleAction()
{
    PS4.setRumble(0, 0);
    PS4.sendToController();
}

void PS4ControllerInput::controllerCoastingAction()
{
    PS4.setRumble(0, 0);
    PS4.sendToController();
}

void PS4ControllerInput::controllerSetDirectionAction(EDirection dir)
{
    switch(dir)
    {
       case FORWARD:
            PS4.setLed(255,0,0);
            break;
        case REVERSE:
            PS4.setLed(255,255,255);
            break;
    }
    PS4.sendToController();

}
