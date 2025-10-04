#ifndef BLUEPAD32CONTROLLER_H
#define BLUEPAD32CONTROLLER_H

#include <Arduino.h>
#include <Bluepad32.h>
#include "MotorController.h"
#include "LightLedController.h"
#include "AudioClipController.h"
#include "definitions.h"


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
};

#endif