#ifndef PS4CONTROLLERINPUT_H
#define PS4CONTROLLERINPUT_H

#include <Arduino.h>
#include <PS4Controller.h>
#include "MotorController.h"

struct ControlData
{
    int momentum; // 0 to 255
    int steering; // -100 to 100
    bool brake;
    bool IsReverse;
};

enum EDirection
{
    FORWARD,
    REVERSE
};

class PS4ControllerInput
{
public:
    PS4ControllerInput(int led_light);
    void begin();
    void loop(MotorController &motor);
    void setLight(bool on);
    ControlData getControlData();

private:
    void controllerAcceleratingAction(int throttle);
    void controllerBreakingAction(int power);
    void controllerIdleAction();
    void controllerCoastingAction();
    void controllerSetDirectionAction(EDirection dir);

    ControlData currentData;
    static bool IsConnected;
    int _ledLight;
};

#endif