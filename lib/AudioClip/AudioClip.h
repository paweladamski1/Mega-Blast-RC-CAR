#pragma once
#ifndef AUDIO_CLIP_H
#define AUDIO_CLIP_H

#include <Arduino.h>
#include "SD.h"
#include "driver/i2s.h"
#include <iostream>
#include <list>

class AudioClipController;

// How many bytes to read from wav file at a time
#define NUM_BYTES_TO_READ_FROM_FILE 1024

/**
 * @file AudioClip.h
 * @brief AudioClip class for WAV file playback via I2S interface.
 *
 * This class facilitates the playback of WAV audio files stored on an SD card
 * through the I2S interface, enabling high-quality sound output on ESP32-based
 * microcontrollers. It supports functionalities such as reading WAV file headers,
 * initializing I2S communication, and streaming audio data to an external DAC.
 *
 * Inspired by the tutorial at:
 * http://www.xtronical.com/i2s-ep4
 *
 * @note Ensure the WAV files are in the correct format (e.g., 32-bit, mono, 44100Hz)
 *       and stored in a supported directory on the SD card.
 */

struct WavHeader_Struct
{
    //   RIFF Section
    char RIFFSectionID[4]; // Letters "RIFF"
    uint32_t Size;         // Size of entire file less 8
    char RiffFormat[4];    // Letters "WAVE"

    //   Format Section
    char FormatSectionID[4]; // letters "fmt"
    uint32_t FormatSize;     // Size of format section less 8
    uint16_t FormatID;       // 1=uncompressed PCM
    uint16_t NumChannels;    // 1=mono,2=stereo
    uint32_t SampleRate;     // 44100, 16000, 8000 etc.
    uint32_t ByteRate;       // =SampleRate * Channels * (BitsPerSample/8)
    uint16_t BlockAlign;     // =Channels * (BitsPerSample/8)
    uint16_t BitsPerSample;  // 8,16,24 or 32

    // Data Section
    char DataSectionID[4]; // The letters "data"
    uint32_t DataSize;     // Size of the data that follows
}; 

class AudioClip
{
    
public:
    AudioClip(AudioClipController *c, const String &fileName, float volume = 0.6f, bool repeat = false, float callEndWhenPercent=95.0f);
    ~AudioClip();

    void play();
    void stop();
    bool read(); 

    void increaseVolume();
    void decreaseVolume();
    void setVolume(float newVolume);

    // events
    // event fired when clip ends
    std::function<void(AudioClip* sender, AudioClipController* controller)> onEnd;

    // check states
    bool isPlaying() const;
    float getPlayingProgress() const;
    bool Repeat;                               // If true, when wav ends, it will auto start again

    // play process
    void resetIdx();
    uint16_t getIdx() const;    
    bool isBufferNotEmpty() const;
    bool isReadyToMix() const;
    int16_t getNextSample();
    uint16_t getBytesInBuffer() const;


private:
    
    /* WAV file struct data */
    File _wavFile;                              // Object for accessing the opened wavfile
    uint32_t _wavDataSize;                      // Size of wav file data
    bool _isPlaying = false;                    // Is file playing
    
    byte _samplesArr[NUM_BYTES_TO_READ_FROM_FILE]; // Buffer to store data red from file
    uint32_t _totalBytesRead = 0;               // Number of bytes read from file so far
    uint16_t _lastNumBytesRead;                 // Num bytes actually read from the wav file which will either be
                                               // NUM_BYTES_TO_READ_FROM_FILE or less than this if we are very
                                               // near the end of the file. i.e. we can't read beyond the file.
    bool _isCallOnEnd;
    float _volume, _progressPercent, _callEndWhenPercent;
    String _id, _fileName;
    uint32_t _soundSize;
    //uint16_t _Idx;
    AudioClipController *_controller;
    int _sampleIdx = 0;
    

    bool _load();

    
    bool _validWavData(WavHeader_Struct *Wav);
    void _dumpWAVHeader(WavHeader_Struct *Wav);

    
    
    void _callOnEnd_event();
    
    void serialPrint(const char *data);                   // for debug
    void serialPrint(const char *data, const uint32_t n); // for debug
    void serialPrint_fileHeader(const char *Data, uint8_t NumBytes);
};

#endif