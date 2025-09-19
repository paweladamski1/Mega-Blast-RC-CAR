#include "BluePad32Controller.h"

bool BluePad32Controller::FirstConnectFlag = false;
bool BluePad32Controller::FirstDisconnectFlag = false;
GamepadPtr BluePad32Controller::gamepad = nullptr;
EDirection BluePad32Controller::Direction = EDirection::FORWARD;

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
        //todo
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
        sound.startBlinker("when disconnect");
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
        sound.startEngine(" Controller connected ");
        vTaskDelay(3000);

        gamepad->setColorLED(255, 255, 255);
        gamepad->playDualRumble(10, 500, 128, 255);
        motor.startEngine();
    }

    static bool emergencyLights = false,
                auxLights = false,
                prevYState = false,
                prevBState = false
                ;

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
    else
        lights.setIndicator(isArrowLeft(), isArrowRight());

    if (isArrowUp())
        Serial.print(" Arr. Up ");
    if (isArrowDown())
        Serial.print(" Arr. Down ");

    if (gamepad->a())
    {
        Serial.print(" Cross ");
        sound.playHorn("BluePad32Controller::loop");
    }

    

    if (gamepad->x())
        Serial.print(" Square ");

    if (isR1())
        Serial.print(" r1 ");

    if (isL1())
        Serial.print(" l1 ");

    updateControlData();
    motor.drive(ControlData.momentum, ControlData.steering, ControlData.direction);
    prevYState = gamepad->y();
    prevBState = gamepad->b();
}

void BluePad32Controller::updateControlData()
{
    if (!isConnected())
    {
        // reset
        ControlData.momentum = 0;
        ControlData.steering = 0;
        ControlData.brake = false;
        onIdleAction();
    }

    static int _s_loop_cnt = 0;
    _s_loop_cnt++;

    int throttle = gamepad->throttle(); // (0 - 1023)
    int brakeForce = gamepad->brake();  // (0 - 1023)

    bool isBrake = gamepad->brake() > 2;
    int steering = map(gamepad->axisX(), -511, 512, -100, 100);

    if (isR1() && ControlData.momentum == 0)
    {
        toggleDirection();
    }

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
    if (ControlData.momentum == 0)
    {
        // Serial.print(" momentium:  ");
        // Serial.print(ControlData.momentum);
        // Serial.print("  ");

        onIdleAction();
    }
    else
    {
        // Serial.print(" momentium:  ");
        // Serial.print(ControlData.momentum);
        // Serial.print("  ");
        // onCoastingAction(Direction);
    }
    lights.setBrakeLight(isBrake);
    ControlData.momentum = constrain(ControlData.momentum, 0, 255);
    ControlData.steering = steering;
    ControlData.brake = isBrake;
    ControlData.direction = Direction;
}

void BluePad32Controller::onAcceleratingAction(int throttle)
{
    ControlData.momentum += map(throttle, 0, 1023, 0, 50);
    Serial.print(" Accelerating ");
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

void BluePad32Controller::onIdleAction()
{
    // Serial.print(" onIdleAction ");
    gamepad->setColorLED(255, 255, 255);
}

void BluePad32Controller::onCoastingAction(EDirection dir)
{
    Serial.print(" onCoastingAction ");
    Serial.print(dir);
    switch (dir)
    {
    case FORWARD:
        gamepad->setColorLED(0, 255, 0);
        break;
    case REVERSE:
        gamepad->setColorLED(255, 0, 0);
        break;
    }
}

void BluePad32Controller::onChangeDirectionAction(EDirection dir)
{
}

void BluePad32Controller::toggleDirection()
{
    Direction = (Direction == EDirection::FORWARD) ? EDirection::REVERSE : EDirection::FORWARD;
    Serial.print(" switch Direction:");
    onChangeDirectionAction(Direction);
    Serial.print(Direction);
    delay(500);
}

bool BluePad32Controller::isArrowRight()
{
    return gamepad->dpad() & DPAD_ARROW_RIGHT;
}

bool BluePad32Controller::isArrowLeft()
{
    return gamepad->dpad() & DPAD_ARROW_LEFT;
}

bool BluePad32Controller::isArrowUp()
{
    return gamepad->dpad() & DPAD_ARROW_UP;
}

bool BluePad32Controller::isArrowDown()
{
    return gamepad->dpad() & DPAD_ARROW_DOWN;
}

bool BluePad32Controller::isL1()
{
    return gamepad->l1();
}

bool BluePad32Controller::isR1()
{
    return gamepad->r1();
}
