#include "BluePad32Controller.h"
#include "PowerManager.h"

GamepadPtr BluePad32Controller::gamepad = nullptr;

BluePad32Controller::BluePad32Controller(LightLedController &lights, AudioClipController &sound, MotorController &motor) : lights(lights), sound(sound), motor(motor)
{
    _controlData = {0, 0, false};
}

void BluePad32Controller::begin()
{
    String fv = BP32.firmwareVersion();
    Serial.print("Firmware version installed: ");
    Serial.println(fv);

    // To get the BD Address (MAC address) call:
    const uint8_t *addr = BP32.localBdAddress();
    Serial.print("BD Address: ");
    for (int i = 0; i < 6; i++)
    {
        Serial.print(addr[i], HEX);
        if (i < 5)
            Serial.print(":");
        else
            Serial.println();
    }

    Serial.println("Bluepad32 setup...");
    BP32.setup(&BluePad32Controller::BP32_onConnected, &BluePad32Controller::BP32_onDisconnected);
    Serial.println("Bluetooth started. Waiting for controller...");
    BP32.forgetBluetoothKeys();

    _padData.hasData = false;
    _padData.throttle = 0;
    _padData.brakeForce = 0;
    _padData.steering = 0;
}

void BluePad32Controller::BP32_onConnected(GamepadPtr gp)
{
    if (gamepad == nullptr)
    {
        Serial.println("BP32_onConnected - Controller connected");
        Serial.print(gp->getModelName());      
        gamepad = gp;
    }
}

void BluePad32Controller::BP32_onDisconnected(GamepadPtr gp)
{
    if (gamepad == nullptr)
        return;
    gamepad->disconnect();
    Serial.println("BP32_onDisconnected - Controller disconnected");
}

void BluePad32Controller::loop()
{
    BP32.update();

    _handleInput();
    _handleConnectPad();
    _handleConnectionTimeout();
    _handleCharging();    
    _handleActionButtons();
    _handleGearBox();
    _handleControl();
    _handleMotor();
}

void BluePad32Controller::_handleInput()
{
    if (!_isConnected())
        return;

    _padData.hasData = gamepad->hasData();
    _padData.throttle = gamepad->throttle();
    _padData.brakeForce = gamepad->brake();
    _padData.steering = map(gamepad->axisX(), -511, 512, -100, 100);

    if (_get_PadB_ToggleState())
        _padData.auxLights = !_padData.auxLights;

    if (_get_PadY_ToggleState())
        _padData.emergencyLights = !_padData.emergencyLights;

    if (_get_PadLeft_ToggleState())
    {
        _padData.indicatorLeft = !_padData.indicatorLeft;
        _padData.indicatorRight = false;
    }

    if (_get_PadRight_ToggleState())
    {
        _padData.indicatorRight = !_padData.indicatorRight;
        _padData.indicatorLeft = false;
    }

    _padData.horn = gamepad->a();

    _padData.playMusic = _get_PadX_ToggleState();

    _padData.systemBtn = gamepad->miscButtons() & PS4_BTN;

    if (_padData.hasData)
        _padData.lastPacket = millis();
}

void BluePad32Controller::_handleConnectPad()
{
    static bool IsConnectFlag = false;

    if (_isConnected() && !IsConnectFlag)
        onJustConnected();
        else if (!_isConnected() && IsConnectFlag)
            onJustDisconnected();

    IsConnectFlag = _isConnected();
}

void BluePad32Controller::_handleConnectionTimeout()
{
    if (!_isConnected())
        return;

    unsigned long now = millis();

    if (now - _padData.lastPacket > 1000)
    {
        _padData.throttle = 0;
        _padData.brakeForce = 1023;
        sound.playConnectionLost();
    }
}

void BluePad32Controller::_handleCharging()
{
    if (PowerManager::isCharging())
    {
        _padData.throttle = 0;
        _padData.brakeForce = 1023;
        sound.playCharging();
    }
}

void BluePad32Controller::_handleActionButtons()
{
    if (!_isConnected())
        return;

    if (_padData.systemBtn)
    {
        _onAction_UserStopEngine();
        return;
    }
    lights.setAuxLight(_padData.auxLights);

    if (_padData.emergencyLights)
        lights.setIndicator(true, true);
    else
        lights.setIndicator(_padData.indicatorLeft, _padData.indicatorRight);

    if (_padData.horn)
    {
        sound.playHorn();
        _padData.horn = false;
    }

    if (_padData.playMusic)
    {
        static bool xState = false;
        xState = !xState;

        Serial.print(" Square ");
        if (xState)
            sound.playMusic();
        else
            sound.stopMusic();
    }

    if (_get_PadL1_ToggleState())
        Serial.print(" l1 ");
}

void BluePad32Controller::_onAction_UserStopEngine()
{
    gamepad->setColorLED(0, 0, 0);
    gamepad->playDualRumble(
        0,   // delayedStartMs
        100, // durationMs
        0,   // weakMagnitude
        255  // strongMagnitude
    );

    sound.playStopEngine();
    sound.playStartBlinker();
    motor.stopEngine();
    delay(2000);
    gamepad->disconnect();
    delay(1000);
    esp_restart();
}

void BluePad32Controller::_handleGearBox()
{
    if (!_isConnected())
        return;

    bool prevGear = _gear;

    if (_get_PadUp_ToggleState())
    {
        Serial.print(" Arr. Up ");
        _gear++;
        if (_gear > 2)
        {
            _gear = 2;
            sound.playGearChangeFail();
        }
        else
            sound.playGearChange();
    }
    else if (_get_PadDown_ToggleState())
    {
        Serial.print(" Arr. Down ");
        _gear--;
        if (_gear < -1)
        {
            _gear = -1;
            sound.playGearChangeFail();
        }
        else
            sound.playGearChange();
    }

    if (prevGear != _gear)
    {
        lights.setReverseLight(_gear < 0);
        if (_gear > 0)
            gamepad->setColorLED(0, 255, 0);
        else if (_gear < 0)
            gamepad->setColorLED(255, 0, 0);
        else
            gamepad->setColorLED(255, 255, 255);
    }
}

void BluePad32Controller::_handleControl()
{
    if (!_isConnected())
    {
        // reset
        _controlData.momentum = 0;
        _controlData.steering = 0;
        _controlData.brake = true;
        return;
    }

    static int _s_loop_cnt = 0;
    _s_loop_cnt++;
    bool isBrake = _padData.isBrake();

    sound.setEngineRpm(_padData.throttle);

    if (_controlData.momentum >= _padData.throttle && !isBrake) // free-rolling simulation
    {
        if (_s_loop_cnt % 2 == 0 && _controlData.momentum > 0)
            _controlData.momentum--;
    }
    else if (isBrake)
        onBrakingAction(_padData.brakeForce);
    else if (_padData.throttle > _controlData.momentum)
        onAcceleratingAction(_padData.throttle);

    lights.setBrakeLight(isBrake);

    switch (_gear)
    {
    case -1:
        _controlData.momentum = constrain(_controlData.momentum, 0, 125);
        break;
    case 0:
        _controlData.momentum = constrain(_controlData.momentum, 0, 0);
        break;
    case 1:
        _controlData.momentum = constrain(_controlData.momentum, 0, 125);
        break;
    case 2:
        _controlData.momentum = constrain(_controlData.momentum, 0, 255);
        break;
    }

    _controlData.steering = _padData.steering;
    _controlData.brake = isBrake;
    _controlData.gear = _gear;
}

void BluePad32Controller::_handleMotor()
{
    motor.drive(_controlData);
}

void BluePad32Controller::onJustConnected()
{
    if (!_isConnected())
        return;

    Serial.println("Controller connected");
    lights.setIndicator(false, false);
    sound.playStartEngine();
    vTaskDelay(3000);
    lights.setMainRearLight(true);
    gamepad->setColorLED(255, 255, 255);
    gamepad->playDualRumble(10, 500, 128, 255);
    motor.startEngine();
}

void BluePad32Controller::onJustDisconnected()
{
    Serial.println("Controller disconnected");
    lights.setIndicator(false, false);
    lights.setMainRearLight(false);
    sound.playStopEngine();
    _controlData.momentum = 0;
    _padData.throttle = 0;
    _padData.brakeForce = 1023;
}

void BluePad32Controller::onAcceleratingAction(int throttle)
{
    _controlData.momentum += map(throttle, 0, 1023, 0, 50);
    Serial.print(" Accelerating ");
    Serial.print(_controlData.momentum);

    gamepad->playDualRumble(0, 200, 0, 150);
}

void BluePad32Controller::onBrakingAction(int brakeForce)
{
    Serial.print(" braking ");
    Serial.print(brakeForce);
    Serial.print(" ");
    if (_controlData.momentum > 0)
    {
        _controlData.momentum -= map(brakeForce, 0, 1023, 0, 50);
        if (_controlData.momentum < 0)
            _controlData.momentum = 0;
    }

    if (brakeForce > 1000)
        _controlData.momentum = 0;
}

bool BluePad32Controller::_get_PadRight_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->dpad() & DPAD_ARROW_RIGHT;

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_get_PadLeft_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->dpad() & DPAD_ARROW_LEFT;

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_get_PadUp_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->dpad() & DPAD_ARROW_UP;

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_get_PadDown_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->dpad() & DPAD_ARROW_DOWN;

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

/**
 * @brief Toggles a state based on the L1 button press.
 *
 * This function detects the rising edge of the L1 button (transition
 * from released to pressed). Each press inverts the internal toggle
 * state, which is then returned. Holding the button does not cause
 * repeated toggles; the state changes only once per press.
 *
 * @return true if the toggle is currently active, false otherwise.
 */
bool BluePad32Controller::_get_PadL1_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->l1();

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

/**
 * @brief Toggles a state based on the R1 button press.
 *
 * This function detects the rising edge of the R1 button (transition
 * from released to pressed). Each press inverts the internal toggle
 * state, which is then returned. Holding the button does not cause
 * repeated toggles; the state changes only once per press.
 *
 * @return true if the toggle is currently active, false otherwise.
 */
bool BluePad32Controller::_get_PadR1_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->r1();

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_get_PadA_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->a();

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_get_PadB_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->b();

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_get_PadX_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->x();

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_get_PadY_ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->y();

    if (!oldState && currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_isConnected() const
{
    return gamepad != nullptr && gamepad->isConnected();
}