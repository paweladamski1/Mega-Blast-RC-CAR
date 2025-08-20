#ifndef SOUND_CONTROLLER_H
#define SOUND_CONTROLLER_H

#include <Arduino.h>
#include "SampleItem.h"

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "AudioOutputMixer.h"



class SoundController
{
public:
    SoundController(int bclkPin, int lrclkPin, int dinPin);

    void begin();

    void startEngine();
    void stopEngine();
    void setEngineRpm(uint16_t rpm);

    void startBlinker();
    void stopBlinker();

    void playHorn();

    void loop();

private:

    int _bclkPin, _lrclkPin, _dinPin;

    volatile bool _engineOn;
    volatile bool _blinkerOn;
    volatile bool _hornOn;
    
    volatile uint16_t _engineRpm;

    SampleItem* engineStartItem;
    SampleItem* hornItem;
    SampleItem* blinkerItem;
    SampleItem* engineRunItem;

    AudioOutputI2S* outI2S;
    AudioOutputMixer* mixer;

    static void _soundControllerTask(void *param);
    static void _loopTask(void *param);

};

#endif // SOUND_CONTROLLER_H
