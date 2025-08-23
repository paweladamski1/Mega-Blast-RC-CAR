#pragma once
#include <Arduino.h>
#include <AudioOutputMixer.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

class SampleItem
{
public:
    // mixer: already created with AudioOutputI2S
    // name of filename from SD Card
    // gain: 0.0..1.0
    // loop: auto-restart when the sample finishes
    SampleItem(String id, 
               AudioOutputMixer *mixer,
               const char *filename,
               float gain = 0.6f,
               bool loop = false);
    ~SampleItem();

    void play(bool isReplay=false); // start from the beginning (no seek used)
    void stop(); // stop if running
    void loop(); // call frequently in main loop
    
    bool isPlaying() const;

    void setGain(float gain);
    void setRate(int hz);
    void setLoop(bool loop);
    
    void serialPrint(const char * data); // for debug
private:
    const char *_filename;
    bool _loop;
    String _id; 
    bool _sync;
    bool _isStop; 
    bool _changeRateOn;
    int _rateHz;
    AudioOutputMixerStub *_mixerOut;
    AudioFileSourceSD *_file;
    AudioGeneratorMP3 *_player; 
};
