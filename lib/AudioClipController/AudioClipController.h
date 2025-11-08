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
    void playPrevMusic();
    void setVolumeUp();
    void setVolumeDown();
    void stopMusic();
    bool isMusicPlaying() const;
    void playGearChange();
    void playGearChangeFail();

    void playConnectionLost();
    void playCharging();
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
    volatile bool _MusicPrev_Req;
    volatile bool _cleanupClipList_Req  = false;
    volatile bool _connectionLost_Req  = false;
    volatile bool _isCharging_Req  = false;


    volatile bool _gearChange_Req;
    volatile bool _gearChangeFail_Req;
    volatile uint16_t _engineRpm_Req;
    float _volume = 0.5f; // master volume 0.0 to 1.0

    byte _samples[NUM_BYTES_TO_READ_FROM_FILE];

    std::vector<AudioClip*> _clipList; 
    std::vector<AudioClip*> _clipListNewAdd; 

    
    Index5 _musicIdx;
    AudioClip *_musicItem[5];
    AudioClip *_fordMustangV8_StartItem;
    AudioClip *_fordMustangV8_IdleItem;    
    AudioClip *_fordMustangV8_Accel_1_Item;
    AudioClip *_fordMustangV8_Accel_2_Item;
    AudioClip *_fordMustangV8_EndItem;
    AudioClip *_blinkerItem;
    AudioClip *_blinkerStartItem;
    AudioClip *_blinkerStopItem;
    AudioClip *_backingUpBeepItem;
    AudioClip *_gearChangeItem;
    AudioClip *_gearChangeFailItem;
    AudioClip *_connectionLostItem;
    AudioClip *_chargingItem;

    AudioClip *_hornItem[5]; 

    static void _soundControllerTask(void *param);
    void _soundControllerTask();
    void _playAudioClipAndWaitForEnd(AudioClip *audio);

    static void _loopTask(void *param);
    void _loopTask();

    void _serialPrint(const char *procName, const char *who);

    uint16_t _mix(byte *samples);
    bool _hasActiveClips();

    static void ClampSample(int32_t &mixedSample, int activeCount);
    static bool FillI2SBuffer(byte *Samples, uint16_t BytesInBuffer);
    void _cleanupClips();
};

#endif // AUDIO_CLIP_CONTROLLER_H
