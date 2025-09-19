#include "AudioClipController.h"
#include <algorithm>

AudioClipController::AudioClipController(int bclkPin, int lrclkPin, int dinPin,
                                         int _sd_sckPin, int _sd_misoPin, int _sd_mosiPin, int _sd_csPin)
    : _bclkPin(bclkPin), _lrclkPin(lrclkPin), _dinPin(dinPin),
      _engineOn_Req(false), _blinkerOn_Req(false), _hornOn(false), _engineRpm(0),
      _sd_sckPin(_sd_sckPin), _sd_misoPin(_sd_misoPin), _sd_mosiPin(_sd_mosiPin), _sd_csPin(_sd_csPin)
{
}

void AudioClipController::begin()
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
    musicItem = new AudioClip(this, "/music1.wav", 0.5f, false);
    // blinkerStartItem = new AudioClip(this, "/blinker_start.wav", 1.0f, true);
    blinkerItem = new AudioClip(this, "/blinker_runing.wav", 1.0f, true);
    // blinkerEndItem   = new AudioClip(this, "/blinker_end.wav", 1.0f, true);

    int choice = random(0, 2);
    const String horn_filename[2] = {"/horn1.wav", "/horn2.wav"};
    hornItem = new AudioClip(this, horn_filename[choice], 1.0f, false);

    engineStartItem = new AudioClip(this, "/engine_start1.wav", 1.0f, false);
    engineRuningItem = new AudioClip(this, "/engine_runing1.wav", 0.4f, true);
    // engineStopItem = new AudioClip(this, "/engine_stop1.wav", 0.2f, true);

    _clipList.push_back(musicItem);
    _clipList.push_back(engineRuningItem);
    _clipList.push_back(engineStartItem);
    //_clipList.push_back(engineStopItem);

    // _clipList.push_back(blinkerStartItem);
    _clipList.push_back(blinkerItem);
    // _clipList.push_back(blinkerEndItem);
    _clipList.push_back(hornItem);

    vTaskDelay(100);

    Serial.println("");
    xTaskCreatePinnedToCore(_soundControllerTask, "_soundControllerTask", 4096, this, 1, NULL, 1);
    xTaskCreatePinnedToCore(_loopTask, "_loopTask", 4096, this, 1, NULL, 1);

    Serial.println("");
    Serial.println("AudioClipController::begin complette ");
}

void AudioClipController::_soundControllerTask(void *param)
{
    Serial.println("_soundControllerTask runing.");
    bool isEnginStarted = false;
    bool isAddedQueueFlag = false;
    auto *parent = static_cast<AudioClipController *>(param);

    bool engineOn_State = parent->_engineOn_Req;
    bool blinkerOn_State = parent->_blinkerOn_Req;
    int engineRpm = parent->_engineRpm;

    AudioClip *hornItem = parent->hornItem;
    AudioClip *blinkerItem = parent->blinkerItem;
    AudioClip *engineRuningItem = parent->engineRuningItem;
    AudioClip *engineStartItem = parent->engineStartItem;

    
    Serial.println("_soundControllerTask start control.");

    while (true)
    {
        if (parent->_hornOn)
        {
            hornItem->play();

            while (hornItem->isPlaying())
                vTaskDelay(10);
            parent->_hornOn = false;
        }

        if (parent->_blinkerOn_Req && !blinkerOn_State)
        {
            Serial.println("AudioClipController::_soundControllerTask - START BLINKER");
            blinkerOn_State = true;
            blinkerItem->play();
        }

        if (parent->_engineOn_Req && !engineOn_State)
        {
            Serial.println();
            Serial.println("AudioClipController::_soundControllerTask - START ENGINE");
            engineOn_State = true;
            if (!engineRuningItem->isPlaying())
            {
                parent->engineStartItem->play();
                parent->engineStartItem->onEnd = [](AudioClip*sender, AudioClipController *controller)
                {
                    Serial.println("");                      
                    controller->engineRuningItem->play();
                };
            }
        }

        if (!parent->_engineOn_Req && engineOn_State)
        {
            engineOn_State = parent->_engineOn_Req;
            engineRuningItem->stop();
        }

        if (!parent->_blinkerOn_Req && blinkerOn_State)
        {
            blinkerOn_State = parent->_blinkerOn_Req;
            blinkerItem->stop();
        }

        if (engineRpm != parent->_engineRpm)
        {
            //engineRuningItem->setSpeed(parent->_engineRpm);
            //engineRpm = parent->_engineRpm;
            //todo
        }
        vTaskDelay(500);
    }
}

void AudioClipController::_loopTask(void *param)
{
    auto *parent = static_cast<AudioClipController *>(param);
    Serial.println("_loopTask runing.");
    vTaskDelay(5000);
    while (true)
    {
        parent->loop();
        vTaskDelay(1);
    }
}

void AudioClipController::startEngine(const char *who)
{
    if (!_engineOn_Req)
        serialPrint("startEngine", who);
    _engineOn_Req = true;
}

void AudioClipController::stopEngine(const char *who)
{
    if (_engineOn_Req)
        serialPrint("stopEngine", who);
    _engineOn_Req = false;
}

void AudioClipController::setEngineRpm(const char *who, uint16_t rpm)
{
    if (_engineRpm != rpm)
        serialPrint("setEngineRpm", who);

    _engineRpm = rpm;
}

void AudioClipController::startBlinker(const char *who)
{
    if (!_blinkerOn_Req)
        serialPrint("startBlinker", who);

    _blinkerOn_Req = true;
}

void AudioClipController::stopBlinker(const char *who)
{
    if (_blinkerOn_Req)
        serialPrint("stopBlinker", who);

    _blinkerOn_Req = false;
}

void AudioClipController::serialPrint(const char *procName, const char *who)
{
    Serial.println();
    Serial.print(" >>> AudioClipController:");
    Serial.print(procName);
    Serial.print(" called from: ");
    Serial.print(who);
    Serial.println();
}

void AudioClipController::playHorn(const char *who)
{
    Serial.println("AudioClipController::playHorn()");
    Serial.print(who);
    _hornOn = true;
}

void AudioClipController::loop()
{
    AudioClip::loop(_clipList);
}
