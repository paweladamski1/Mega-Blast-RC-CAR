#ifndef SOUND_CONTROLLER_H
#define SOUND_CONTROLLER_H

#include <Arduino.h>
#include "SampleItem.h"
#include "AudioOutputI2S.h"
#include "AudioOutputMixer.h"

class SoundController
{
public:
    SoundController(int bclkPin, int lrclkPin, int dinPin,
                    int _sd_sckPin, int _sd_misoPin, int _sd_mosiPin, int _sd_csPin);

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
    int _sd_sckPin, _sd_misoPin, _sd_mosiPin, _sd_csPin;

    volatile bool _engineOn;
    volatile bool _blinkerOn;
    volatile bool _hornOn;

    volatile uint16_t _engineRpm;

    SampleItem *engineStartItem;
    SampleItem *hornItem;
    SampleItem *blinkerItem;
    SampleItem *engineRuningItem;

    AudioOutputI2S *outI2S;
    AudioOutputMixer *mixer;

    static void _soundControllerTask(void *param);
    static void _loopTask(void *param);
};

#endif // SOUND_CONTROLLER_H
