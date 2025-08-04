#ifndef PS4CONTROLLERINPUT_H
#define PS4CONTROLLERINPUT_H

#include <Arduino.h>
#include <Bluepad32.h>
#include "MotorController.h"

struct SControlData
{
    int momentum; // 0 to 255
    int steering; // -100 to 100
    bool brake;
    EDirection direction;
};



#define DPAD_ARROW_UP        0x0001
#define DPAD_ARROW_DOWN      0x0002
#define DPAD_ARROW_RIGHT     0x0004
#define DPAD_ARROW_LEFT      0x0008
#define PS4_BTN              0x01
#define PS4_L1               0x0010
#define PS4_R1               0x0020

class PS4ControllerInput
{
public:
    PS4ControllerInput(int led_light);
    void begin();
    void loop(MotorController &motor);
    void setLight(bool on);
    

private:
    static void onConnected(GamepadPtr gp);
    static void onDisconnected(GamepadPtr gp);

    void updateControlData();

    
    void onAcceleratingAction(int throttle);    
    void onBrakingAction(int brakeForce);
    void onIdleAction();
    void onCoastingAction(EDirection dir);
    void onChangeDirectionAction(EDirection dir);
    void toggleDirection();


    bool isArrowRight();
    bool isArrowLeft();
    bool isArrowUp();
    bool isArrowDown();

    bool isL1();
    bool isR1();

    bool isConnected();
    static GamepadPtr gamepad;
    SControlData ControlData;
    static bool FirstConnectFlag;
    static bool FirstDisconnectFlag;
    int _ledLight;
    static EDirection Direction;
};

#endif