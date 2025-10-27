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
    BluePad32Controller(LightLedController &lights, AudioClipController &sound, MotorController &motor);
    void begin();
    void loop();



private:
    LightLedController &lights;
    AudioClipController &sound;
    MotorController &motor;
    int _gear=0;

    static void BP32_onConnected(GamepadPtr gp);
    static void BP32_onDisconnected(GamepadPtr gp);

    void onAcceleratingAction(int throttle);
    void onBrakingAction(int brakeForce); 
 

    bool _get_PadRight_ToggleState();
    bool _get_PadLeft_ToggleState();
    bool _get_PadUp_ToggleState();
    bool _get_PadDown_ToggleState();

    bool _get_PadL1_ToggleState();
    bool _get_PadR1_ToggleState(); 

    bool _get_PadA_ToggleState();
    bool _get_PadB_ToggleState();
    bool _get_PadX_ToggleState();
    bool _get_PadY_ToggleState();

    
    static GamepadPtr gamepad;
    
 
    
    
    SControlData _controlData;  
    SPadData _padData;

    void _handleInput();
    void _handleConnectionTimeout();
    void _handleCharging();
    void _handleConnectPad();
    void _handleActionButtons();
    void _onAction_UserStopEngine();   

    void _handleGearBox();
    void _handleControl();
    void _handleMotor();

    void onJustDisconnected();
    void onJustConnected();

    bool _isConnected() const;
};

#endif