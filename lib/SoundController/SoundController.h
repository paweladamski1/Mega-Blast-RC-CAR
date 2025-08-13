#ifndef SOUND_CONTROLLER_H
#define SOUND_CONTROLLER_H

#include <Arduino.h>



class SoundController {
public:
    SoundController(int bclkPin, int lrclkPin, int dinPin);

    void begin();

    void startEngine();
    void stopEngine();
    void setEngineRpm(uint16_t rpm);

    void startBlinker();
    void stopBlinker();

    void playHorn();

private:
    int _bclkPin, _lrclkPin, _dinPin;
    TaskHandle_t _engineTaskHandle;
    TaskHandle_t _blinkerTaskHandle;
    TaskHandle_t _hornTaskHandle;
    volatile bool _engineOn;
    volatile bool _blinkerOn;
    volatile uint16_t _engineRpm;

    static void _engineTask(void* param);
    static void _blinkerTask(void* param);
    static void _hornTask(void* param);

    void _playSound(const uint8_t* data, size_t length);
    void _playEngineSound();
    void _stopTask(TaskHandle_t* handle);
};

#endif // SOUND_CONTROLLER_H
