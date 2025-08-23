#include "SampleItem.h"

SampleItem::SampleItem(String id,
                       AudioOutputMixer *mixer,
                       const char *filename,
                       float gain,
                       bool loop)
    : _filename(filename),
      _loop(loop),
      _mixerOut(nullptr),
      _file(nullptr), 
      _player(nullptr), 
      _id(id),
      _isStop(false),
      _changeRateOn(false),
      _rateHz(160000)
{
    

    Serial.println("");
    Serial.println("init SampleItem");
    Serial.print(filename);
    // Create mixer input (owned by mixer, do not delete)
    _mixerOut = mixer ? mixer->NewInput() : nullptr;
    if (_mixerOut)
        _mixerOut->SetGain(gain);
}

 

SampleItem::~SampleItem()
{
    stop();   
    if (_file)
    {
        delete _file;
        _file = nullptr;
    }
    // _mixerOut is managed by mixer
}

 
void SampleItem::play(bool isReplay)
{
    if (_isStop && isReplay)
        return;
    _isStop=false;    
    _sync = false;
    serialPrint("Play");
    
    if (!_mixerOut)
    {
        Serial.println("_mixerOut is null !");
        return;
    }
    
    if (_player && _player->isRunning())
    {
         Serial.println("isRunning!");
        return;
    }
    
    if (_file)
    {
        delete _file;
        _file = nullptr;
    }
    
    _file = new AudioFileSourceSD(_filename);    

    if (!_player)
        _player = new AudioGeneratorMP3();
    _player->begin(_file, _mixerOut);

    if(_changeRateOn)
        _mixerOut->SetRate(_rateHz);     

    _sync = true;
}

void SampleItem::stop()
{
    _isStop=true;
    _sync=false;
    if (isPlaying())
    {
        serialPrint("Stop 1");
        _player->stop();
        serialPrint("Stop 2");
    }
}

bool SampleItem::isPlaying() const
{
    return (_player && _player->isRunning());
}

void SampleItem::loop()
{
    if (isPlaying())
    {
        if (_sync)
        {
            if (!_player->loop())
            {
                // Finished
                if (_loop)
                {
                    _player->stop();
                    play(true);
                }
                else
                {
                    _player->stop();
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
    Serial.println();
    Serial.print(_id);
    Serial.print(" ");
    Serial.print(data);
    Serial.print(" ");
}

void SampleItem::setGain(float gain)
{
    if (_mixerOut)
        _mixerOut->SetGain(gain);
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
