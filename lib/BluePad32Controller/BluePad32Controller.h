#ifndef BLUEPAD32CONTROLLER_H
#define BLUEPAD32CONTROLLER_H

#include <Arduino.h>
#include <Bluepad32.h>
#include "MotorController.h"
#include "LightLedController.h"
#include "AudioClipController.h"

struct SControlData
{
    int momentum; // 0 to 255
    int steering; // -100 to 100
    bool brake;
    int gear;
};

#define DPAD_ARROW_UP 0x0001
#define DPAD_ARROW_DOWN 0x0002
#define DPAD_ARROW_RIGHT 0x0004
#define DPAD_ARROW_LEFT 0x0008
#define PS4_BTN 0x01
#define PS4_L1 0x0010
#define PS4_R1 0x0020

class BluePad32Controller
{
public:
    BluePad32Controller(LightLedController &lights, AudioClipController &sound);
    void begin();
    void loop(MotorController &motor);

    

private:
    LightLedController &lights;
    AudioClipController &sound;
    int _gear=0;

    static void onConnected(GamepadPtr gp);
    static void onDisconnected(GamepadPtr gp);



    void updateControlData();
    void onAcceleratingAction(int throttle);
    void onBrakingAction(int brakeForce); 


    void _gearBox();

    bool _getArrowRightToggleState();
    bool _getArrowLeftToggleState();
    bool _getArrowUpToggleState();
    bool _getArrowDownToggleState();

    bool _getL1ToggleState();
    bool _getR1ToggleState();
    bool _getXToggleState();

    bool isConnected();
    static GamepadPtr gamepad;
    
    SControlData ControlData;


    static bool FirstConnectFlag;
    static bool FirstDisconnectFlag;
    bool _isConnectedState;
};

#endif