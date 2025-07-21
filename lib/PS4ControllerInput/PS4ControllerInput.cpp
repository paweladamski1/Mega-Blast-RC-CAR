#include "PS4ControllerInput.h"

bool PS4ControllerInput::FirstConnectFlag = false;
bool PS4ControllerInput::FirstDisconnectFlag = false;
GamepadPtr PS4ControllerInput::gamepad = nullptr;
EDirection PS4ControllerInput::Direction = EDirection::FORWARD;

PS4ControllerInput::PS4ControllerInput(int led_light) : _ledLight(led_light)
{
    currentData = {0, 0, false};
    FirstConnectFlag = false;
}

void PS4ControllerInput::begin()
{
    pinMode(_ledLight, OUTPUT);

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
    BP32.setup(&PS4ControllerInput::onConnected, &PS4ControllerInput::onDisconnected);
    Serial.println("Bluetooth started. Waiting for controller...");
    BP32.forgetBluetoothKeys();
}

void PS4ControllerInput::onConnected(GamepadPtr gp)
{
    if (gamepad == nullptr)
    {
        Serial.println("Controller connected");
        Serial.print(gp->getModelName());
        gamepad = gp;
        FirstConnectFlag = false;
    }
}

void PS4ControllerInput::onDisconnected(GamepadPtr gp)
{
    gamepad = nullptr;
    FirstConnectFlag = false;
}

bool PS4ControllerInput::isConnected()
{
    return gamepad != nullptr && gamepad->isConnected();
}

void PS4ControllerInput::loop(MotorController &motor)
{
    BP32.update();

    if (!isConnected())
    {
        setLight(false);
        motor.stop();
        return;
    }

    if (gamepad->miscButtons() & PS4_BTN)
    {
        gamepad->setColorLED(0, 0, 0);
        gamepad->playDualRumble(
            0, // delayedStartMs
            500, // durationMs
            128, // weakMagnitude
            255  // strongMagnitude
        );
        Serial.println("restart esp32");
        gamepad->disconnect();
        delay(5000);
        
        esp_restart();
        return;
    }

    if (!FirstConnectFlag)
    {
        FirstConnectFlag = true;
        setLight(true);
        gamepad->setColorLED(255, 255, 255);
        gamepad->playDualRumble(10, 500, 128, 255);
        motor.startEngine();
    }

    if (isArrowRight())
        Serial.print(" Arr. Right ");
    if (isArrowLeft())
        Serial.print(" Arr. Left ");
    if (isArrowUp())
        Serial.print(" Arr. Up ");
    if (isArrowDown())
        Serial.print(" Arr. Down ");

    if (gamepad->a())
        Serial.print(" Cross ");

    if (gamepad->b())
        Serial.print(" Circle ");

    if (gamepad->x())
        Serial.print(" Square ");

    if (gamepad->y())
        Serial.print(" Triangle ");

    if (isR1())
        Serial.print(" r1 ");

    if (isL1())
        Serial.print(" l1 ");

    ControlData data = getControlData();
    motor.drive(data.momentum, data.steering, data.direction);
}

void PS4ControllerInput::setLight(bool turn_on)
{
    digitalWrite(_ledLight, (turn_on) ? HIGH : LOW);
}

ControlData PS4ControllerInput::getControlData()
{
    if (!isConnected())
    {
        // reset
        currentData.momentum = 0;
        currentData.steering = 0;
        currentData.brake = false;
        controllerIdleAction();
        return currentData;
    }

    static int _s_loop_cnt = 0;
    _s_loop_cnt++;

    int throttle = gamepad->throttle(); // (0 - 1023)
    int brakeForce = gamepad->brake();  // (0 - 1023)

    bool brake = gamepad->brake() > 0;
    int steering = map(gamepad->axisX(), -511, 512, -100, 100);

    if (isR1() && currentData.momentum == 0)
    {
        toggleDirection();
    }

    if (currentData.momentum >= throttle && !brake) // free-rolling simulation
    {
        if (_s_loop_cnt % 2 == 0 && currentData.momentum > 0)
            currentData.momentum--;
    }
    else if (currentData.momentum > throttle && brake) // braking
    {
        currentData.momentum -= brakeForce;
    }
    else if (throttle > currentData.momentum)
    {
        controllerAcceleratingAction(throttle);
        currentData.momentum += 10;
    }
    if (currentData.momentum == 0)
    {
        Serial.print(" momentium:  ");
        Serial.print(currentData.momentum);
        Serial.print("  ");

        controllerIdleAction();
    }
    else
    {
        Serial.print(" momentium:  ");
        Serial.print(currentData.momentum);
        Serial.print("  ");
        controllerCoastingAction(Direction);
    }
    currentData.momentum = constrain(currentData.momentum, 0, 255);
    currentData.steering = steering;
    currentData.brake = brake;
    currentData.direction = Direction;

    return currentData;
}

void PS4ControllerInput::controllerAcceleratingAction(int throttle)
{
    gamepad->playDualRumble(0, 100, 128, 255);
}

void PS4ControllerInput::controllerBreakingAction(int power)
{
    gamepad->playDualRumble(0, 100, 128, 0);
}

void PS4ControllerInput::controllerIdleAction()
{
    Serial.print(" controllerIdleAction ");
    gamepad->setColorLED(255, 255, 255);
}

void PS4ControllerInput::controllerCoastingAction(EDirection dir)
{
    Serial.print(" controllerCoastingAction ");
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

void PS4ControllerInput::controllerOnChangeDirectionAction(EDirection dir)
{
}

void PS4ControllerInput::toggleDirection()
{
    Direction = (Direction == EDirection::FORWARD) ? EDirection::REVERSE : EDirection::FORWARD;
    Serial.print(" switch Direction:");
    controllerOnChangeDirectionAction(Direction);
    Serial.print(Direction);
    delay(2000);
}

bool PS4ControllerInput::isArrowRight()
{
    return gamepad->dpad() & DPAD_ARROW_RIGHT;
}

bool PS4ControllerInput::isArrowLeft()
{
    return gamepad->dpad() & DPAD_ARROW_LEFT;
}

bool PS4ControllerInput::isArrowUp()
{
    return gamepad->dpad() & DPAD_ARROW_UP;
}

bool PS4ControllerInput::isArrowDown()
{
    return gamepad->dpad() & DPAD_ARROW_DOWN;
}

bool PS4ControllerInput::isL1()
{
    return gamepad->l1();
}

bool PS4ControllerInput::isR1()
{
    return gamepad->r1();
}
