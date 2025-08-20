#include "SampleItem.h"

SampleItem::SampleItem(String id,
                       AudioOutputMixer *mixer,
                       const uint8_t *sampleData,
                       size_t sampleLen,
                       float gain,
                       bool loop)
    : _data(sampleData),
      _len(sampleLen),
      _loop(loop),
      _stub(nullptr),
      _file(nullptr),
      _wav(nullptr),
      _id(id),
      _isStop(false),
      _rateHz(24000),
      _changeRateOn(false)
{
    // Create mixer input (owned by mixer, do not delete)
    _stub = mixer ? mixer->NewInput() : nullptr;
    if (_stub)
    {
        _stub->SetGain(gain);
        
    }

    // Create decoder; source will be created on play()
    _wav = new AudioGeneratorWAV();
    _rateHz=24000;
}

SampleItem::~SampleItem()
{
    stop();
    if (_wav)
    {
        delete _wav;
        _wav = nullptr;
    }
    if (_file)
    {
        delete _file;
        _file = nullptr;
    }
    // _stub is managed by mixer
}

void SampleItem::_rewindSource()
{
    if (_file)
    {
        delete _file;
        _file = nullptr;
    }
    // Recreate the PROGMEM source to start from the beginning (seek not used)
    _file = new AudioFileSourcePROGMEM(_data, _len);
    serialPrint(" - SampleItem::_rewindSource() ");
}

void SampleItem::play(bool isReplay)
{
    if (_isStop && isReplay)
        return;
    _isStop=false;    
    _sync = false;
    serialPrint("Play");
    if (!_stub)
    {
        Serial.println("_stub is null !");
        return;
    }
    // If already playing, do nothing (or call stop() if you prefer restart)
    if (_wav && _wav->isRunning())
    {
         Serial.println("isRunning!");
        return;
    }

    _rewindSource();
    if (!_wav)
        _wav = new AudioGeneratorWAV();
    _wav->begin(_file, _stub);
    if(_changeRateOn)
        _stub->SetRate(_rateHz);    

    _sync = true;
}

void SampleItem::stop()
{
    _isStop=true;
    if (isPlaying())
    {
        _wav->stop();
        serialPrint("Stop");
    }
}

bool SampleItem::isPlaying() const
{
    return (_wav && _wav->isRunning());
}

void SampleItem::loop()
{
    if (isPlaying())
    {
        if (_sync)
        {
            if (!_wav->loop())
            {
                // Finished
                if (_loop)
                {
                    _wav->stop();
                    play(true);
                }
                else
                {
                    _wav->stop();
                }
            }
        }
        else
        {
            serialPrint("na sync!");
        }
    }
}

void SampleItem::serialPrint(const char *data)
{
    return;
    Serial.println();
    Serial.print(_id);
    Serial.print(" ");
    Serial.print(data);
    Serial.print(" ");
}

void SampleItem::setGain(float gain)
{
    if (_stub)
        _stub->SetGain(gain);
}

void SampleItem::setRate(int hz)
{
    _changeRateOn=true;
    _rateHz=hz;
}

void SampleItem::setLoop(bool loop)
{
    _loop = loop;
}
