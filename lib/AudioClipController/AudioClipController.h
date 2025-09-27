#pragma once
#ifndef AUDIO_CLIP_CONTROLLER_H
#define AUDIO_CLIP_CONTROLLER_H

#include <Arduino.h>
#include "SD.h"
#include "driver/i2s.h"
#include <iostream>
#include <list>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class AudioClip;

struct Index5
{
    uint8_t value = 0;
    void increment() { value = (value + 1) % 5; }
};

static const i2s_config_t i2s_config =
    {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 22050, // Note, all files must be this
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
        .dma_buf_count = 8,                       // 8 buffers
        .dma_buf_len = 256,                       // 256 bytes per buffer, so 2K of buffer space
        .use_apll = 0,
        .tx_desc_auto_clear = true,
        .fixed_mclk = -1};

class AudioClipController
{
    friend class AudioClip; // AudioClip can access private stuff

public:
    AudioClipController(int bclkPin, int lrclkPin, int dinPin,
                        int _sd_sckPin, int _sd_misoPin, int _sd_mosiPin, int _sd_csPin);

    void begin();

    void playStartEngine(const char *who);
    void stopEngine(const char *who);
    void setEngineRpm(const char *who, uint16_t rpm);

    void playStartBlinker(const char *who);
    void stopBlinker(const char *who);

    void playHorn(const char *who);

    void playBackingUpBeep(bool isPlay);

    void playMusic();    
    void playNextMusic();
    void stopMusic();
    void playGearChange();
    void playGearChangeFail();




   


private:
    void addClip(AudioClip *c);
    void removeClip(AudioClip *c);

    const String horn_filename[2] = {"/horn1.wav", "/horn2.wav"};

    int _bclkPin, _lrclkPin, _dinPin;
    int _sd_sckPin, _sd_misoPin, _sd_mosiPin, _sd_csPin;

    volatile bool _engineOn_Req;
    volatile bool _engineRuning_Req;
    volatile bool _blinkerOn_Req;
    volatile bool _backingUpOn_Req;
    volatile bool _hornOn;
    volatile bool _MusicOn_Req;
    volatile bool _MusicNext_Req;

    volatile bool _gearChange_Req;
    volatile bool _gearChangeFail_Req;

    volatile uint16_t _engineRpm_Req;

    std::list<AudioClip *> _clipList;
    SemaphoreHandle_t clipMutex;

    Index5 _musicIdx;
    AudioClip *musicItem[5];

    AudioClip *engineStartItem;
    AudioClip *engineRuningItem;
    AudioClip *engineStopItem;

    AudioClip *blinkerItem;
    AudioClip *backingUpBeepItem;
    AudioClip *gearChangeItem;
    AudioClip *gearChangeFailItem;
    AudioClip *hornItem;

    // AudioClip *blinkerStartItem;//todo
    // AudioClip *blinkerEndItem;//todo

    
    

    static void _soundControllerTask(void *param);
    void _soundControllerTask();
    void _playAudioClipAndWaitForEnd(AudioClip * audio);

    static void _loopTask(void *param);
    void _loopTask();
    
    


    void serialPrint(const char *procName, const char *who);

    static uint16_t Mix(byte *samples, std::list<AudioClip *> &items);
    static bool hasActiveClips(const std::list<AudioClip *> &items);

    static void clampSample(int32_t &mixedSample, int activeCount);

    static bool FillI2SBuffer(byte *Samples, uint16_t BytesInBuffer);
};

#endif // AUDIO_CLIP_CONTROLLER_H
