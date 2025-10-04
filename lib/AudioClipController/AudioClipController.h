#pragma once
#ifndef AUDIO_CLIP_CONTROLLER_H
#define AUDIO_CLIP_CONTROLLER_H

#include <Arduino.h>
#include "SD.h"
#include "driver/i2s.h"
#include <iostream>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "definitions.h"



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

    void playStartEngine();
    void playStopEngine();
    void setEngineRpm( uint16_t rpm);

    void playStartBlinker();
    void stopBlinker();

    void playHorn();

    void playBackingUpBeep(bool isPlay);

    void playMusic();
    void playNextMusic();
    void stopMusic();
    void playGearChange();
    void playGearChangeFail();

    

private:
    void addClip(AudioClip *c);
    void removeClip();
    
    Index5 _hornIdx;
    const String horn_filename[5] = {"/horn1.wav", "/horn2.wav" ,"/horn3.wav", "/horn4.wav", "/horn5.wav"};

    int _bclkPin, _lrclkPin, _dinPin;
    int _sd_sckPin, _sd_misoPin, _sd_mosiPin, _sd_csPin;

    volatile bool _engineOn_Req;
    volatile bool _engineIdle_Req;
    volatile bool _blinkerOn_Req;
    volatile bool _backingUpOn_Req;
    volatile bool _hornOn;
    volatile bool _MusicOn_Req;
    volatile bool _MusicNext_Req;
    volatile bool _cleanupClipList_Req  = false;


    volatile bool _gearChange_Req;
    volatile bool _gearChangeFail_Req;
    volatile uint16_t _engineRpm_Req;
    float _volume = 0.5f; // master volume 0.0 to 1.0

    byte _samples[NUM_BYTES_TO_READ_FROM_FILE];

    std::vector<AudioClip*> _clipList; 
    std::vector<AudioClip*> _clipListNewAdd; 

    
    Index5 _musicIdx;
    AudioClip *musicItem[5];
    AudioClip *fordMustangV8_StartItem;
    AudioClip *fordMustangV8_IdleItem;    
    AudioClip *fordMustangV8_Accel_1_Item;
    AudioClip *fordMustangV8_Accel_2_Item;
    AudioClip *fordMustangV8_EndItem;
    AudioClip *blinkerItem;
    AudioClip *backingUpBeepItem;
    AudioClip *gearChangeItem;
    AudioClip *gearChangeFailItem;

    AudioClip *hornItem[5]; 

    static void _soundControllerTask(void *param);
    void _soundControllerTask();
    void _playAudioClipAndWaitForEnd(AudioClip *audio);

    static void _loopTask(void *param);
    void _loopTask();

    void _serialPrint(const char *procName, const char *who);

    uint16_t Mix(byte *samples);
    bool hasActiveClips();

    static void clampSample(int32_t &mixedSample, int activeCount);

    static bool FillI2SBuffer(byte *Samples, uint16_t BytesInBuffer);
    void _cleanupClips();
};

#endif // AUDIO_CLIP_CONTROLLER_H
