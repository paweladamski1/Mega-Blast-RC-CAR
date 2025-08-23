#include "SoundController.h"
#include <algorithm>

SoundController::SoundController(int bclkPin, int lrclkPin, int dinPin,
                                 int _sd_sckPin, int _sd_misoPin, int _sd_mosiPin, int _sd_csPin)
    : _bclkPin(bclkPin), _lrclkPin(lrclkPin), _dinPin(dinPin),
      _engineOn(false), _blinkerOn(false), _hornOn(false), _engineRpm(0)
{
}

void SoundController::begin()
{
    SPI.begin(_sd_sckPin, _sd_misoPin, _sd_mosiPin, _sd_csPin);
    if (!SD.begin(_sd_csPin, SPI))
    {
        Serial.println("SD Card mount failed!");
        while (1)
            delay(100);
    }
    Serial.println("SD Card OK");

    outI2S = new AudioOutputI2S();
    outI2S->SetPinout(_bclkPin, _lrclkPin, _dinPin);
    outI2S->begin();

    mixer = new AudioOutputMixer(16, outI2S);
    engineStartItem = new SampleItem("engineStart", mixer, "/engine_start1.mp3", 1.0f, false);
    engineRuningItem = new SampleItem("endgineRun", mixer, "/engine_runing1.mp3", 0.5f, true);
    hornItem = new SampleItem("horn", mixer, "/horn1.mp3", 1.0f, false);
    blinkerItem = new SampleItem("blinker", mixer, "/blinker.mp3", 1.0f, true);

    xTaskCreatePinnedToCore(_soundControllerTask, "soundLoopTask", 4096, this, 1, NULL, 1);
    xTaskCreatePinnedToCore(_loopTask, "soundLoopTask", 4096, this, 1, NULL, 1);
     Serial.println("");
    Serial.println("SoundController::begin complette ");
}

void SoundController::_soundControllerTask(void *param)
{
    Serial.println("_soundControllerTask runing.");
    bool isEnginStarted = false;
    bool isAddedQueueFlag = false;
    auto *parent = static_cast<SoundController *>(param);
    auto *engineStartItem = parent->engineStartItem;
    auto *engineRuningItem = parent->engineRuningItem;
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
            blinkerItem->play();
        }

        if (parent->_engineOn && !engineOn)
        {
            engineOn = true;
            if (!engineRuningItem->isPlaying())
            {
                blinkerItem->stop();
                parent->_blinkerOn=false;
                vTaskDelay(1000);
                engineStartItem->play();
                while (engineStartItem->isPlaying())
                {
                    vTaskDelay(1);
                }
                engineRuningItem->play();
            }
        }

        if (!parent->_engineOn && engineOn)
        {
            engineOn = false;
            engineRunPrev = false;
            engineRuningItem->stop();
        }

        if (!parent->_blinkerOn && blinkerOn)
        {
            blinkerOn = false;
            if (engineRunPrev)
                engineRuningItem->play();
            blinkerItem->stop();
        }

        if (engineRpm != parent->_engineRpm)
        {
            engineRuningItem->setRate(parent->_engineRpm);
            engineRpm = parent->_engineRpm;
        }
        vTaskDelay(100);
    }
}

void SoundController::_loopTask(void *param)
{
    auto *parent = static_cast<SoundController *>(param);
    if(!parent)
    {
        Serial.println("Parent null!!!");    
        while(1);
    }
    Serial.println("_loopTask runing.");
    while (true)
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
    engineRuningItem->loop();   
    hornItem->loop();   
    blinkerItem->loop();   
}
