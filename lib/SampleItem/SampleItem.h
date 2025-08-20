#pragma once
#include <Arduino.h>
#include <AudioOutputMixer.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorWAV.h>

class SampleItem
{
public:
    // mixer: already created with AudioOutputI2S
    // sampleData/sampleLen: pointer + length of WAV-in-PROGMEM buffer
    // gain: 0.0..1.0
    // loop: auto-restart when the sample finishes
    SampleItem(String id, 
               AudioOutputMixer *mixer,
               const uint8_t *sampleData,
               size_t sampleLen,
               float gain = 0.6f,
               bool loop = false);
    ~SampleItem();

    void play(bool isReplay=false); // start from the beginning (no seek used)
    void stop(); // stop if running
    void loop(); // call frequently in main loop
    void serialPrint(const char * data);
    bool isPlaying() const;

    void setGain(float gain);
    void setRate(int hz);
    void setLoop(bool loop);

private:
    const uint8_t *_data;
    size_t _len;
    bool _loop;
    String _id; 
    bool _sync;
    bool _isStop;
    int _rateHz;
    bool _changeRateOn;
    AudioOutputMixerStub *_stub;
    AudioFileSourcePROGMEM *_file;
    AudioGeneratorWAV *_wav;

    void _rewindSource(); // recreate _file to rewind to start (seek not used)
};
