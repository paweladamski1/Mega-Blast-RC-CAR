#include "PS4ControllerInput.h"

bool PS4ControllerInput::FirstConnectFlag = false;
bool PS4ControllerInput::FirstDisconnectFlag = false;
GamepadPtr PS4ControllerInput::gamepad = nullptr;
EDirection PS4ControllerInput::Direction = EDirection::FORWARD;

PS4ControllerInput::PS4ControllerInput(int led_light) : _ledLight(led_light)
{
    ControlData = {0, 0, false};
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
            0,   // delayedStartMs
            100, // durationMs
            0, // weakMagnitude
            255  // strongMagnitude
        );
        Serial.println("motor.stop");
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

    updateControlData();
    motor.drive(ControlData.momentum, ControlData.steering, ControlData.direction);
}

void PS4ControllerInput::setLight(bool turn_on)
{
    digitalWrite(_ledLight, (turn_on) ? HIGH : LOW);
}

void PS4ControllerInput::updateControlData()
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
        Serial.print(" momentium:  ");
        Serial.print(ControlData.momentum);
        Serial.print("  ");

        onIdleAction();
    }
    else
    {
        Serial.print(" momentium:  ");
        Serial.print(ControlData.momentum);
        Serial.print("  ");
        onCoastingAction(Direction);
    }
    ControlData.momentum = constrain(ControlData.momentum, 0, 255);
    ControlData.steering = steering;
    ControlData.brake = isBrake;
    ControlData.direction = Direction;
}



void PS4ControllerInput::onAcceleratingAction(int throttle)
{
    ControlData.momentum += map(throttle, 0, 1023, 0, 50);
    Serial.print(" Accelerating ");
    gamepad->playDualRumble(0, 200, 0, 150);
}

void PS4ControllerInput::onBrakingAction(int brakeForce)
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



void PS4ControllerInput::onIdleAction()
{
    // Serial.print(" onIdleAction ");
    gamepad->setColorLED(255, 255, 255);
}

void PS4ControllerInput::onCoastingAction(EDirection dir)
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

void PS4ControllerInput::onChangeDirectionAction(EDirection dir)
{
}

void PS4ControllerInput::toggleDirection()
{
    Direction = (Direction == EDirection::FORWARD) ? EDirection::REVERSE : EDirection::FORWARD;
    Serial.print(" switch Direction:");
    onChangeDirectionAction(Direction);
    Serial.print(Direction);
    delay(500);
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
