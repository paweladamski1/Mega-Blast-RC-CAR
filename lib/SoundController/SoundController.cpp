#include "SoundController.h"

#include "sound_engine.h"
#include "sound_horn.h"
#include "sound_blinker.h"
#include <algorithm>

SoundController::SoundController(int bclkPin, int lrclkPin, int dinPin)
    : _bclkPin(bclkPin), _lrclkPin(lrclkPin), _dinPin(dinPin),
      _engineOn(false), _blinkerOn(false), _hornOn(false), _engineRpm(0)
{
}

void SoundController::begin()
{
    outI2S = new AudioOutputI2S();
    outI2S->SetPinout(_bclkPin, _lrclkPin, _dinPin);
    outI2S->begin();

    mixer = new AudioOutputMixer(16, outI2S);
    engineStartItem = new SampleItem("engineStart", mixer, sampleEngineStart, sampleEngineStart_len, 0.7f, false);
    engineRunItem = new SampleItem("endgineRun", mixer, sampleEngineRun, sampleEngineRun_len, 0.3f, true);
    hornItem = new SampleItem("horn", mixer, sampleHorn, sampleHorn_len, 1.0f, false);
    blinkerItem = new SampleItem("blinker", mixer, sampleBlinker, sampleBlinker_len, 1.0f, true);

    xTaskCreatePinnedToCore(_soundControllerTask, "soundLoopTask", 4096, this, 1, NULL, 1);
    xTaskCreatePinnedToCore(_loopTask, "soundLoopTask", 4096, this, 1, NULL, 1);
}

void SoundController::_soundControllerTask(void *param)
{
    Serial.println("_soundControllerTask runing.");
    bool isEnginStarted = false;
    bool isAddedQueueFlag = false;
    auto *parent = static_cast<SoundController *>(param);
    auto *engineStartItem = parent->engineStartItem;
    auto *engineRunItem = parent->engineRunItem;
    auto *hornItem = parent->hornItem;
    auto *blinkerItem = parent->blinkerItem;

    bool engineOn = parent->_engineOn;
    bool blinkerOn = parent->_blinkerOn;
    int engineRpm = parent->_engineRpm;
    bool engineRunPrev = false;
    while (true)
    {
        if (parent->_hornOn)
        {
            hornItem->play();
            vTaskDelay(1000);
            parent->_hornOn = false;
        }


        if (parent->_blinkerOn && !blinkerOn)
        {
            blinkerOn = true;
            engineRunPrev = engineOn;
            engineRunItem->stop();
            blinkerItem->play();
        }
        
        if (parent->_engineOn && !engineOn)
        {
            engineOn=true;
            if (!engineRunItem->isPlaying())
            {
                engineStartItem->play();
                while (engineStartItem->isPlaying())
                {
                    vTaskDelay(1);
                }
                engineRunItem->play();
            }
        }

        if (!parent->_engineOn && engineOn)
        {
            engineOn=false;
            engineRunPrev=false;
            engineRunItem->stop();
        }

        if (!parent->_blinkerOn && blinkerOn)
        {
            blinkerOn=false;
            if(engineRunPrev)
                engineRunItem->play();
            blinkerItem->stop();
        }

        if(engineRpm != parent->_engineRpm)
        {
            engineRunItem->setRate(parent->_engineRpm);
            engineRpm = parent->_engineRpm;
        }
        vTaskDelay(100);
    }
}

void SoundController::_loopTask(void *param)
{
    auto *parent = static_cast<SoundController *>(param);
    Serial.println("_loopTask runing.");
    while(true)
    {
        parent->loop();
    }
}


void SoundController::startEngine()
{
    _engineOn = true;
}

void SoundController::stopEngine()
{
    _engineOn = false;
}

void SoundController::setEngineRpm(uint16_t rpm)
{
    _engineRpm = rpm;
}

void SoundController::startBlinker()
{
    _blinkerOn = true;
}

void SoundController::stopBlinker()
{
    _blinkerOn = false;
}

void SoundController::playHorn()
{
    _hornOn = true;
}

void SoundController::loop()
{     
    engineStartItem->loop();
    engineRunItem->loop();    
    hornItem->loop();
    blinkerItem->loop();
}
