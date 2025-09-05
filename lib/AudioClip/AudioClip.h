#pragma once
#include <Arduino.h>
#include "SD.h"
#include "driver/i2s.h"
#include <iostream>
#include <list>


// How many bytes to read from wav file at a time
#define NUM_BYTES_TO_READ_FROM_FILE 1024



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

// The data for one particular wav file
struct Wav_Struct
{
    File WavFile;                              // Object for accessing the opened wavfile
    uint32_t DataSize;                         // Size of wav file data
    bool Playing = false;                      // Is file playing
    bool Repeat;                               // If true, when wav ends, it will auto start again
    byte Samples[NUM_BYTES_TO_READ_FROM_FILE]; // Buffer to store data red from file
    uint32_t TotalBytesRead = 0;               // Number of bytes read from file so far
    uint16_t LastNumBytesRead;                 // Num bytes actually read from the wav file which will either be
                                               // NUM_BYTES_TO_READ_FROM_FILE or less than this if we are very
                                               // near the end of the file. i.e. we can't read beyond the file.
};



class AudioClip
{
public:
    AudioClip(String FileName,
               float volume = 0.6f,
               bool repeat = false);
    ~AudioClip();

    void play();
    void stop();

    static void loop(std::list<AudioClip *> &items);

    void (*onStart)() = nullptr;
    void (*onEnd)() = nullptr;

    bool isPlaying() const;

private:
    Wav_Struct _wav;

    float _volume;

    String _id;

    uint32_t _soundSize;
    uint16_t _wavIdx;
    bool _loadWavFileHeader(String FileName);
    bool _validWavData(WavHeader_Struct* Wav);
    void _dumpWAVHeader(WavHeader_Struct* Wav);



    bool _read();
    static uint16_t Mix(byte *samples, std::list<AudioClip *> &items);
    static bool FillI2SBuffer(byte *Samples, uint16_t BytesInBuffer);

    void serialPrint(const char *data);                   // for debug
    void serialPrint(const char *data, const uint32_t n); // for debug
    void serialPrint_fileHeader(const char* Data,uint8_t NumBytes);
};
