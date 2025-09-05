#include "SoundController.h"
#include <algorithm>

SoundController::SoundController(int bclkPin, int lrclkPin, int dinPin,
                                 int _sd_sckPin, int _sd_misoPin, int _sd_mosiPin, int _sd_csPin)
    : _bclkPin(bclkPin), _lrclkPin(lrclkPin), _dinPin(dinPin),
      _engineOn(false), _blinkerOn(false), _hornOn(false), _engineRpm(0),
      _sd_sckPin(_sd_sckPin), _sd_misoPin(_sd_misoPin), _sd_mosiPin(_sd_mosiPin), _sd_csPin(_sd_csPin)
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

    i2s_pin_config_t pin_config =
        {
            .bck_io_num = _bclkPin,          // The bit clock connectiom, goes to pin 27 of ESP32
            .ws_io_num = _lrclkPin,          // Word select, also known as word select or left right clock
            .data_out_num = _dinPin,         // Data out from the ESP32, connect to DIN on 38357A
            .data_in_num = I2S_PIN_NO_CHANGE // we are not interested in I2S data into the ESP32
        };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);

    Serial.println("");
    musicItem = new AudioClip("/music1.wav", 0.2f, false);
   // blinkerStartItem = new AudioClip("/blinker_start.wav", 1.0f, true);
    blinkerRunItem   = new AudioClip("/blinker_runing.wav", 1.0f, true);
   // blinkerEndItem   = new AudioClip("/blinker_end.wav", 1.0f, true);
    
    
    int choice=random(0, 2);
    const String horn_filename[2]= {"/horn1.wav", "/horn2.wav"};
    hornItem =  new AudioClip(horn_filename[choice], 1.0f, false); 
                            
    
    engineStartItem = new AudioClip("/engine_start1.wav", 0.2f, false);
    engineRuningItem = new AudioClip("/engine_runing1.wav", 0.2f, true);
    //engineStopItem = new AudioClip("/engine_stop1.wav", 0.2f, true);

    

    _clipList.push_back(musicItem);
    _clipList.push_back(engineRuningItem);
    _clipList.push_back(engineStartItem);    
    //_clipList.push_back(engineStopItem);
    
   // _clipList.push_back(blinkerStartItem);
    _clipList.push_back(blinkerRunItem);
   // _clipList.push_back(blinkerEndItem);
    _clipList.push_back(hornItem);

    vTaskDelay(100);

    Serial.println("");
    xTaskCreatePinnedToCore(_soundControllerTask, "_soundControllerTask", 4096, this, 1, NULL, 1);
    xTaskCreatePinnedToCore(_loopTask, "_soundControllerTask", 4096, this, 1, NULL, 1);

    Serial.println("");
    Serial.println("SoundController::begin complette ");
}

void SoundController::_soundControllerTask(void *param)
{
    Serial.println("_soundControllerTask runing.");
    bool isEnginStarted = false;
    bool isAddedQueueFlag = false;
    auto *parent = static_cast<SoundController *>(param);

    bool engineOn = parent->_engineOn;
    bool blinkerOn = parent->_blinkerOn;
    int engineRpm = parent->_engineRpm;
    bool engineRunPrev = false;
    bool engineStarted = false;
    Serial.println("_soundControllerTask play.");
    parent->musicItem->play();

    int test_i = 10;
    int test_y = 20;
    while (true)
    {
        test_i--;
        test_y--;

        if (test_i == 0)
        {
            Serial.println("");
            Serial.println("");
            Serial.println("_soundControllerTask blinker on!");
            parent->blinkerRunItem->play();
            test_i=100;
        }

        if (test_y == 0)
        {
            Serial.println("");
            Serial.println("");
            Serial.println("_soundControllerTask horn_train on!");
            parent->hornItem->play();
            test_y=80;
        }

        if (!parent->musicItem->isPlaying())
        {
            Serial.println("");
            Serial.println("");
            Serial.println("_soundControllerTask wait 10s");
            vTaskDelay(10000);
            parent->musicItem->play();
        }

        /*if (parent->_hornOn)
         {
             parent->stopAll();
             playNext = NULL;
             if (parent->_blinkerOn)
                 playNext = blinkerItem;
             else if (parent->_engineOn)
                 playNext = engineRuningItem;

             hornItem->play(playNext);

             while (hornItem->isPlaying())
                 vTaskDelay(10);
             parent->_hornOn = false;
         }

         if (parent->_blinkerOn && !blinkerOn)
         {
             parent->stopAll();
             Serial.println("SoundController::_soundControllerTask - START BLINKER");
             blinkerOn = true;
             engineRunPrev = engineOn;
             blinkerItem->play();
         }

         if (parent->_engineOn && !engineOn)
         {
             Serial.println();
             Serial.println("SoundController::_soundControllerTask - START ENGINE");
             engineOn = true;
             if (!engineRuningItem->isPlaying())
             {
                 parent->stopAll();
                 parent->_blinkerOn = false;
                 vTaskDelay(50);
                 if (!engineStarted)
                 {
                     engineStarted = true;
                     engineRuningItem->loadData();
                     engineStartItem->play(engineRuningItem);
                 }
             }
         }

         if (!parent->_engineOn && engineOn)
         {
             engineOn = false;
             engineRunPrev = false;
             engineStarted = false;
             engineRuningItem->stop();
         }

         if (!parent->_blinkerOn && blinkerOn)
         {
             blinkerOn = false;
             blinkerItem->stop();
             vTaskDelay(50);
             if (engineRunPrev)
                 engineRuningItem->play();
         }

         if (engineRpm != parent->_engineRpm)
         {
             engineRuningItem->setRate(parent->_engineRpm);
             engineRpm = parent->_engineRpm;
         }*/
        vTaskDelay(1000);
    }
}

void SoundController::_loopTask(void *param)
{
    auto *parent = static_cast<SoundController *>(param);
    Serial.println("_loopTask runing.");
    vTaskDelay(5000);
    while (true)
    {
        parent->loop();
        vTaskDelay(1);
    }
}

void SoundController::startEngine(const char *who)
{
    if (!_engineOn)
        serialPrint("startEngine", who);
    _engineOn = true;
}

void SoundController::stopEngine(const char *who)
{
    if (_engineOn)
        serialPrint("stopEngine", who);
    _engineOn = false;
}

void SoundController::setEngineRpm(const char *who, uint16_t rpm)
{
    if (_engineRpm != rpm)
        serialPrint("setEngineRpm", who);

    _engineRpm = rpm;
}

void SoundController::startBlinker(const char *who)
{
    if (!_blinkerOn)
        serialPrint("startBlinker", who);

    _blinkerOn = true;
}

void SoundController::stopBlinker(const char *who)
{
    if (_blinkerOn)
        serialPrint("stopBlinker", who);

    _blinkerOn = false;
}

void SoundController::serialPrint(const char *procName, const char *who)
{
    Serial.println();
    Serial.print(" >>> SoundController:");
    Serial.print(procName);
    Serial.print(" called from: ");
    Serial.print(who);
    Serial.println();
}

void SoundController::playHorn(const char *who)
{
    Serial.println("SoundController::playHorn()");
    Serial.print(who);
    _hornOn = true;
}

void SoundController::loop()
{
    /*engineStartItem->loop();
    engineRuningItem->loop();
    hornItem->loop();
    blinkerItem->loop();*/
    AudioClip::loop(_clipList);
}

void SoundController::stopAll()
{
    Serial.println("SoundController::stopAll()");
    /*engineRuningItem->stop();
    engineStartItem->stop();
    hornItem->stop();
    blinkerItem->stop();*/
    musicItem->stop();
    vTaskDelay(100);
}
