#include "BluePad32Controller.h"

bool BluePad32Controller::FirstConnectFlag = false;
bool BluePad32Controller::FirstDisconnectFlag = false;
GamepadPtr BluePad32Controller::gamepad = nullptr;

BluePad32Controller::BluePad32Controller(LightLedController &lights, AudioClipController &sound) : lights(lights), sound(sound)
{
    ControlData = {0, 0, false};
    FirstConnectFlag = false;
    _isConnectedState = true;
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
    BP32.setup(&BluePad32Controller::onConnected, &BluePad32Controller::onDisconnected);
    Serial.println("Bluetooth started. Waiting for controller...");
    BP32.forgetBluetoothKeys();
}

void BluePad32Controller::onConnected(GamepadPtr gp)
{
    if (gamepad == nullptr)
    {
        Serial.println("Controller connected");
        Serial.print(gp->getModelName());
        gamepad = gp;
        FirstConnectFlag = false;
    }
}

void BluePad32Controller::onDisconnected(GamepadPtr gp)
{
    gamepad = nullptr;
    FirstConnectFlag = false;
}

bool BluePad32Controller::isConnected()
{
    return gamepad != nullptr && gamepad->isConnected();
}

void BluePad32Controller::loop(MotorController &motor)
{
    BP32.update();

    if (!isConnected() && _isConnectedState)
    {
        // todo
        lights.setMainRearLight(false);
        sound.stopEngine("is not connect");
        motor.stop();
        return;
    }
    _isConnectedState = true;
    if (gamepad->miscButtons() & PS4_BTN)
    {
        gamepad->setColorLED(0, 0, 0);
        gamepad->playDualRumble(
            0,   // delayedStartMs
            100, // durationMs
            0,   // weakMagnitude
            255  // strongMagnitude
        );
        Serial.println("motor.stop");
        sound.stopEngine("when disconnect");
        sound.playStartBlinker("when disconnect");
        motor.stop();
        delay(2000);
        Serial.println("gamepad->disconnect");
        gamepad->disconnect();
        delay(1000);
        Serial.println("restart esp32");
        esp_restart();
        return;
    }

    if (!FirstConnectFlag)
    {
        FirstConnectFlag = true;
        lights.setMainRearLight(true);
        lights.setIndicator(false, false);
        sound.playStartEngine(" Controller connected ");
        vTaskDelay(3000);

        gamepad->setColorLED(255, 255, 255);
        gamepad->playDualRumble(10, 500, 128, 255);
        motor.startEngine();
    }

    static bool emergencyLights = false,
                auxLights = false,
                prevYState = false,
                prevBState = false,
                indicatorLeft = false,
                indicatorRight = false;

    if (!gamepad->y() && prevYState)
    {
        emergencyLights = !emergencyLights;
        Serial.print(" Triangle ");
    }

    if (!gamepad->b() && prevBState)
    {
        auxLights = !auxLights;
        Serial.print(" Circle ");
    }

    lights.setAuxLight(auxLights);

    if (emergencyLights)
        lights.setIndicator(true, true);
    {
        if (_getArrowLeftToggleState())
            indicatorLeft = !indicatorLeft;

        if (_getArrowRightToggleState())
            indicatorRight = !indicatorRight;

        lights.setIndicator(indicatorLeft, indicatorRight);
    }

    _gearBox();

    if (gamepad->a())
    {
        Serial.print(" Cross ");
        sound.playHorn("BluePad32Controller::loop");
    }

    if (_getXToggleState())
    {
        static bool xState = false;
        xState = !xState;

        Serial.print(" Square ");
        if (xState)
            sound.playMusic();
        else
            sound.stopMusic();
    }

    if (_getL1ToggleState())
        Serial.print(" l1 ");

    updateControlData();
    motor.drive(ControlData.momentum, ControlData.steering, ControlData.gear);
    prevYState = gamepad->y();
    prevBState = gamepad->b();
}

void BluePad32Controller::_gearBox()
{
    bool prevGear = _gear;
    if (_getArrowUpToggleState())
    {
        Serial.print(" Arr. Up ");
        _gear++;
        if (_gear > 2)
        {
            // TODO music fx
            _gear = 2;
            sound.playGearChangeFail();
        }
        else
            sound.playGearChange();
    }
    else if (_getArrowDownToggleState())
    {
        Serial.print(" Arr. Down ");
        _gear--;
        if (_gear < -1)
        {
            // TODO: music fx
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

void BluePad32Controller::updateControlData()
{
    if (!isConnected())
    {
        // reset
        ControlData.momentum = 0;
        ControlData.steering = 0;
        ControlData.brake = false;
    }

    static int _s_loop_cnt = 0;
    _s_loop_cnt++;

    int throttle = gamepad->throttle(); // (0 - 1023)
    int brakeForce = gamepad->brake();  // (0 - 1023)

    bool isBrake = gamepad->brake() > 2;
    int steering = map(gamepad->axisX(), -511, 512, -100, 100);

    /*if (_getR1ToggleState() && ControlData.momentum == 0)
     {
         Serial.print(" R1 ");
         toggleDirection();
     }*/

     sound.setEngineRpm("BluePad32Controller::updateControlData" , throttle);

    if (ControlData.momentum >= throttle && !isBrake) // free-rolling simulation
    {
        if (_s_loop_cnt % 2 == 0 && ControlData.momentum > 0)
            ControlData.momentum--;
    }
    else if (isBrake) // braking
    {
        onBrakingAction(brakeForce);
    }
    else if (throttle > ControlData.momentum)
    {
        onAcceleratingAction(throttle);
    }

    lights.setBrakeLight(isBrake);

    switch (_gear)
    {
    case -1:
        ControlData.momentum = constrain(ControlData.momentum, 0, 35);
        break;
    case 0:
        ControlData.momentum = constrain(ControlData.momentum, 0, 0);
        break;
    case 1:
        ControlData.momentum = constrain(ControlData.momentum, 0, 50);
        break;
    case 2:
        ControlData.momentum = constrain(ControlData.momentum, 0, 255);
        break;
    }

    ControlData.steering = steering;
    ControlData.brake = isBrake;
    ControlData.gear = _gear;
}

void BluePad32Controller::onAcceleratingAction(int throttle)
{
    ControlData.momentum += map(throttle, 0, 1023, 0, 50);
    Serial.print(" Accelerating ");
    Serial.print(ControlData.momentum);

    gamepad->playDualRumble(0, 200, 0, 150);
}

void BluePad32Controller::onBrakingAction(int brakeForce)
{
    Serial.print(" braking ");
    Serial.print(brakeForce);
    Serial.print(" ");
    if (ControlData.momentum > 0)
    {
        ControlData.momentum -= map(brakeForce, 0, 1023, 0, 50);
        if (ControlData.momentum < 0)
            ControlData.momentum = 0;
    }

    if (brakeForce > 1000)
        ControlData.momentum = 0;
}

bool BluePad32Controller::_getArrowRightToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->dpad() & DPAD_ARROW_RIGHT;

    if (oldState && !currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_getArrowLeftToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->dpad() & DPAD_ARROW_LEFT;

    if (oldState && !currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_getArrowUpToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->dpad() & DPAD_ARROW_UP;

    if (oldState && !currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_getArrowDownToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->dpad() & DPAD_ARROW_DOWN;

    if (oldState && !currentState)
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
bool BluePad32Controller::_getL1ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->l1();

    if (oldState && !currentState)
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
bool BluePad32Controller::_getR1ToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->r1();

    if (oldState && !currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}

bool BluePad32Controller::_getXToggleState()
{
    static bool oldState = false;
    static bool ret = false;

    bool currentState = gamepad->x();

    if (oldState && !currentState)
        ret = true;
    else
        ret = false;

    oldState = currentState;
    return ret;
}
