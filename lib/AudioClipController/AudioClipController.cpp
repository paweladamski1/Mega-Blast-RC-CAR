#include "AudioClipController.h"
#include "AudioClip.h"

#include <algorithm>

AudioClipController::AudioClipController(int bclkPin, int lrclkPin, int dinPin,
                                         int _sd_sckPin, int _sd_misoPin, int _sd_mosiPin, int _sd_csPin)
    : _bclkPin(bclkPin), _lrclkPin(lrclkPin), _dinPin(dinPin),
      _engineOn_Req(false), _blinkerOn_Req(false), _hornOn(false), _engineRpm_Req(0),
      _sd_sckPin(_sd_sckPin), _sd_misoPin(_sd_misoPin), _sd_mosiPin(_sd_mosiPin), _sd_csPin(_sd_csPin),
      _MusicOn_Req(false), _MusicNext_Req(false), _engineIdle_Req(false)
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

    _musicItem[0] = new AudioClip(this, "/music1.wav", 0.9f, false, 100.0f);
    _musicItem[1] = new AudioClip(this, "/music2.wav", 0.9f, false, 100.0f);
    _musicItem[2] = new AudioClip(this, "/music3.wav", 0.9f, false, 100.0f);
    _musicItem[3] = new AudioClip(this, "/music4.wav", 0.9f, false, 100.0f);
    _musicItem[4] = new AudioClip(this, "/music5.wav", 0.9f, false, 100.0f);

    // when music end then play next
    for (int i = 0; i < 5; i++)
    {
        _musicItem[i]->onEnd = [](AudioClip *sender, AudioClipController *controller)
        {
            controller->playNextMusic();
        };
    }
    _blinkerItem = new AudioClip(this, "/blinker_runing.wav", 0.8f, true);
    _blinkerStartItem = new AudioClip(this, "/blinker_start.wav", 0.8f, false);
    _blinkerStopItem = new AudioClip(this, "/blinker_end.wav", 0.8f, false);

    _backingUpBeepItem = new AudioClip(this, "/backing_up_beep.wav", 0.5f, true);

    for (int i = 0; i < 5; i++)
        _hornItem[i] = new AudioClip(this, horn_filename[i], 0.6f, false);

    _fordMustangV8_StartItem = new AudioClip(this, "/1965FordMustangV8start.wav", 0.3f, false, 95.0f);
    _fordMustangV8_IdleItem = new AudioClip(this, "/1965FordMustangV8idle.wav", 0.4f, true, 100.0f);
    _fordMustangV8_EndItem = new AudioClip(this, "/1965FordMustangV8end.wav", 0.3f, false);
    _fordMustangV8_Accel_1_Item = new AudioClip(this, "/1965FordMustangV8accel1.wav", 0.9f);
    _fordMustangV8_Accel_2_Item = new AudioClip(this, "/1965FordMustangV8accel2.wav", 0.9f);

    _gearChangeItem = new AudioClip(this, "/gear_change.wav", 0.5f, false);
    _gearChangeFailItem = new AudioClip(this, "/gear_change_err.wav", 0.5f, false);

    _connectionLostItem = new AudioClip(this, "/onPadDisconnect.wav", 1.0f, false);
    _chargingItem = new AudioClip(this, "/onCharging.wav", 1.0f, false);

    _fordMustangV8_StartItem->onEnd = [](AudioClip *sender, AudioClipController *controller)
    {
        controller->_engineIdle_Req = true;
    };
    xTaskCreatePinnedToCore(_soundControllerTask, "_soundControllerTask", 4096, this, 1, NULL, 1);
    xTaskCreatePinnedToCore(_loopTask, "_loopTask", 4096, this, 1, NULL, 1);

    Serial.println("");
    Serial.println("AudioClipController::begin complete ");
}

void AudioClipController::_soundControllerTask(void *param)
{
    Serial.println("_soundControllerTask running.");
    auto *parent = static_cast<AudioClipController *>(param);
    parent->_soundControllerTask();
}

void AudioClipController::_soundControllerTask()
{
    static int hornChoice = random(0, 5);
    bool isAddedQueueFlag = false;

    bool engineOn_State = _engineOn_Req;
    bool blinkerOn_State = _blinkerOn_Req;
    bool backingUpOn_State = _backingUpOn_Req;
    bool musicOn_State = _MusicOn_Req;
    unsigned long connLostCanStartDaley = 10000; // 10 seconds delay after engine start
    unsigned long startTime = millis();
    unsigned long now = millis();
    bool connLostPlayed = false;

    int engineRpm_State = _engineRpm_Req;
    while (true)
    {

        if (_gearChange_Req)
        {
            _playAudioClipAndWaitForEnd(_gearChangeItem);
            _gearChange_Req = false;
        }

        if (_gearChangeFail_Req)
        {
            _playAudioClipAndWaitForEnd(_gearChangeFailItem);
            _gearChangeFail_Req = false;
        }

        if (_connectionLost_Req && engineOn_State && connLostPlayed)
            _playAudioClipAndWaitForEnd(_connectionLostItem);

        _connectionLost_Req = false;

        if (_isCharging_Req)
        {
            _playAudioClipAndWaitForEnd(_chargingItem);
            _isCharging_Req = false;
        }

        if (_hornOn)
        {
            _hornItem[hornChoice]->stop();
            hornChoice = random(0, 5);
            _hornItem[hornChoice]->play();
            _hornOn = false;
        }

        if (_engineIdle_Req)
        {
            _engineIdle_Req = false;
            _fordMustangV8_IdleItem->play();
        }

        if (_blinkerOn_Req && !blinkerOn_State)
        {
            _serialPrint("_controller", "START BLINKER");
            blinkerOn_State = true;
            _blinkerStartItem->play();
            _blinkerItem->play();
        }

        if (_engineOn_Req && !engineOn_State)
        {
            startTime = millis();
            engineOn_State = true;
            if (!_fordMustangV8_StartItem->isPlaying())
            {
                _serialPrint("_controller", "START ENGINE");
                // _fordMustangV8_StartItem->setPlaybackRange((uint32_t)0, (uint32_t)178176);
                _fordMustangV8_StartItem->play();
            }
        }

        if (_backingUpOn_Req && !backingUpOn_State)
        {

            _serialPrint("_controller", "Backing Up Beep On");
            backingUpOn_State = true;
            if (!_backingUpBeepItem->isPlaying())
                _backingUpBeepItem->play();
        }

        if (_MusicOn_Req && !musicOn_State)
        {
            _serialPrint("_controller", "Music on");
            musicOn_State = true;
            if (!_musicItem[_musicIdx.value]->isPlaying())
                _musicItem[_musicIdx.value]->play();
        }

        if (_MusicNext_Req)
        {
            _serialPrint("_controller", "NEXT Music");
            _MusicNext_Req = false;
            if (_musicItem[_musicIdx.value]->isPlaying())
                _musicItem[_musicIdx.value]->stop();

            _musicIdx.increment();
            _MusicOn_Req = true;
            musicOn_State = false;
        }

        if( _MusicPrev_Req)
        {
            _serialPrint("_controller", "PREV Music");
            _MusicPrev_Req = false;
            if (_musicItem[_musicIdx.value]->isPlaying())
                _musicItem[_musicIdx.value]->stop();

             _musicIdx.decrement(); 

            _MusicOn_Req = true;
            musicOn_State = false;
        }

        // turn of
        if (!_engineOn_Req && engineOn_State)
        {
            engineOn_State = _engineOn_Req;

            _fordMustangV8_EndItem->play();
            vTaskDelay(500 / portTICK_PERIOD_MS);
            _fordMustangV8_IdleItem->stop();
        }

        if (!_blinkerOn_Req && blinkerOn_State)
        {
            blinkerOn_State = _blinkerOn_Req;
            _blinkerStopItem ->play();
            _blinkerItem->stop();
        }

        if (!_backingUpOn_Req && backingUpOn_State)
        {
            backingUpOn_State = _backingUpOn_Req;
            _backingUpBeepItem->stop();
        }

        if (!_MusicOn_Req && musicOn_State)
        {
            musicOn_State = _MusicOn_Req;
            _musicItem[_musicIdx.value]->stop();
        }

        if (engineRpm_State != _engineRpm_Req)
        {
            if (_engineRpm_Req > engineRpm_State)
            {
                int d = _engineRpm_Req - engineRpm_State;
                if (d < 600)
                    _fordMustangV8_Accel_1_Item->play();
                else
                    _fordMustangV8_Accel_2_Item->play();
            }
            engineRpm_State = _engineRpm_Req;
        }

        if (engineOn_State && !connLostPlayed)
        {
            now = millis();
            if ((now - startTime) > connLostCanStartDaley)
                connLostPlayed = true;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void AudioClipController::playStartEngine()
{
    _engineOn_Req = true;
}

void AudioClipController::playStopEngine()
{
    _engineOn_Req = false;
}

void AudioClipController::setEngineRpm(uint16_t rpm)
{
    if (_engineRpm_Req != rpm)
    {
        _engineRpm_Req = rpm;
        Serial.print(" [r=");
        Serial.print(rpm);
        Serial.print(" ]");
    }
}

void AudioClipController::playStartBlinker()
{
    _blinkerOn_Req = true;
}

void AudioClipController::stopBlinker()
{
    _blinkerOn_Req = false;
}

void AudioClipController::playHorn()
{
    _hornOn = true;
}

void AudioClipController::playBackingUpBeep(bool isPlay)
{
    _backingUpOn_Req = isPlay;
}

void AudioClipController::playMusic()
{
    _MusicOn_Req = true;
    _fordMustangV8_IdleItem->stop(); // lower engine idle volume when music playing
}

void AudioClipController::playNextMusic()
{
    playMusic();
    _MusicNext_Req = true;
}

void AudioClipController::playPrevMusic()
{
    playMusic();
    _MusicPrev_Req = true;
}

void AudioClipController::setVolumeUp()
{
    _volume += 0.1f;
    if (_volume > 1.0f)
        _volume = 1.0f;
}

void AudioClipController::setVolumeDown()
{
    _volume -= 0.1f;
    if (_volume < 0.0f)
        _volume = 0.0f;
}

void AudioClipController::stopMusic()
{
    _MusicOn_Req = false;
    _fordMustangV8_IdleItem->play(); // restore engine idle volume when music stopped
}

bool AudioClipController::isMusicPlaying() const
{
    return _MusicOn_Req;
}

void AudioClipController::playGearChange()
{
    _gearChange_Req = true;
}

void AudioClipController::playGearChangeFail()
{
    _gearChangeFail_Req = true;
}

void AudioClipController::playConnectionLost()
{
    _connectionLost_Req = true;
}

void AudioClipController::playCharging()
{
    _isCharging_Req = true;
}

void AudioClipController::addClip(AudioClip *c)
{
    _clipListNewAdd.push_back(c);
    _cleanupClipList_Req = true;
}

void AudioClipController::removeClip()
{
    _cleanupClipList_Req = true;
}

bool AudioClipController::_hasActiveClips()
{
    for (AudioClip *item : _clipList)
    {
        if (item->isPlaying())
            if (item->isBufferNotEmpty())
                return true;
    }
    return false;
}

void AudioClipController::ClampSample(int32_t &mixedSample, int activeCount)
{
    if (mixedSample > 32767 || mixedSample < -32768)
        mixedSample /= activeCount;

    if (mixedSample > 32767)
        mixedSample = 32767;
    if (mixedSample < -32768)
        mixedSample = -32768;
}

bool AudioClipController::FillI2SBuffer(byte *Samples, uint16_t BytesInBuffer)
{
    // Writes bytes to buffer, returns true if all bytes sent else false, keeps track itself of how many left
    // to write, so just keep calling this routine until returns true to know they've all been written, then
    // you can re-fill the buffer

    size_t BytesWritten;           // Returned by the I2S write routine,
    static uint16_t BufferIdx = 0; // Current pos of buffer to output next
    uint8_t *DataPtr;              // Point to next data to send to I2S
    uint16_t BytesToSend;          // Number of bytes to send to I2S

    // To make the code eaier to understand I'm using to variables to some calculations, normally I'd write this calcs
    // directly into the line of code where they belong, but this make it easier to understand what's happening

    DataPtr = Samples + BufferIdx;                                            // Set address to next byte in buffer to send out
    BytesToSend = BytesInBuffer - BufferIdx;                                  // This is amount to send (total less what we've already sent)
    i2s_write(I2S_NUM_0, DataPtr, BytesToSend, &BytesWritten, portMAX_DELAY); // Send the bytes, wait 1 RTOS tick to complete
    BufferIdx += BytesWritten;                                                // increasue by number of bytes actually written

    if (BufferIdx >= BytesInBuffer)
    {
        // sent out all bytes in buffer, reset and return true to indicate this
        BufferIdx = 0;
        return true;
    }
    else
        return false; // Still more data to send to I2S so return false to indicate this
}

void AudioClipController::_serialPrint(const char *procName, const char *who)
{
#ifdef DEBUG
    Serial.println();
    Serial.print(" >>> AudioClipController:");
    Serial.print(procName);
    Serial.print(" called from: ");
    Serial.print(who);
    Serial.println();
#endif
}

void AudioClipController::_playAudioClipAndWaitForEnd(AudioClip *audio)
{
    audio->play();

    while (audio->isPlaying())
        vTaskDelay(10);
}

void AudioClipController::_loopTask(void *param)
{
    auto *parent = static_cast<AudioClipController *>(param);
    Serial.println("_loopTask runing.");
    // vTaskDelay(5000);
    while (true)
    {
        parent->_loopTask();
        vTaskDelay(1);
    }
    Serial.println();
    Serial.println("_________________________________________________________loopTask STOP!!!.");
}

void AudioClipController::_loopTask()
{
    _cleanupClips();
    if (_clipList.empty())
        return;

    static bool ReadingFilePhase = true;
    static bool IsRead = true;

    static uint16_t BytesReadFromFile;

    if (ReadingFilePhase) // Read next chunk of data in from files
    {
        // Read data into the wavs own buffers
        IsRead = false;
        for (AudioClip *item : _clipList)
        {
            if (item->read())
                IsRead = true;
        }

        if (!IsRead)
            return;

        BytesReadFromFile = _mix(_samples); // Mix the samples together and store in the samples buffer
        ReadingFilePhase = false;           // Switch to sending the buffer to the I2S
    }
    else
        ReadingFilePhase = FillI2SBuffer(_samples, BytesReadFromFile); // We keep calling this routine until it returns true, at which point
}

uint16_t AudioClipController::_mix(byte *samples)
{
    // Mix all playing wavs together, returns the max bytes that are in the buffer, usually this would be the full buffer but
    // in rare cases wavs may be close to the end of the file and thus not fill the entire buffer
    int32_t mixedSample;       // The mixed sample
    uint16_t i;                // index into main samples buffer
    uint16_t MaxBytesInBuffer; // Max bytes of data in buffer, most of time buffer will be full
                               //  const float Volume = 0.3;
    i = 0;

    for (AudioClip *item : _clipList)
        item->resetIdx();

    int activeCount = 0;

    while (_hasActiveClips())
    {
        mixedSample = 0;
        activeCount = 0;

        // sum all samples
        for (AudioClip *audio : _clipList)
        {
            if (audio->isReadyToMix())
            {
                mixedSample += audio->getNextSample();
                activeCount++;
            }
        }

        ClampSample(mixedSample, activeCount);

        if (i + 1 < NUM_BYTES_TO_READ_FROM_FILE)
            *((int16_t *)(samples + i)) = (int16_t)mixedSample;
        else
            break;

        i += 2;
    }

    MaxBytesInBuffer = 0;
    for (AudioClip *audio : _clipList)
        if (audio->getBytesInBuffer() > MaxBytesInBuffer)
            MaxBytesInBuffer = audio->getBytesInBuffer();

    if (MaxBytesInBuffer + 2 > NUM_BYTES_TO_READ_FROM_FILE)
        MaxBytesInBuffer = NUM_BYTES_TO_READ_FROM_FILE - 2;

    //  We now alter the data according to the volume control
    for (i = 0; i < MaxBytesInBuffer; i += 2)
        *((int16_t *)(samples + i)) = (*((int16_t *)(samples + i))) * _volume;

    return MaxBytesInBuffer;
}

void AudioClipController::_cleanupClips()
{
    if (_cleanupClipList_Req)
    {
        _cleanupClipList_Req = false;
        _clipList.erase(
            std::remove_if(_clipList.begin(), _clipList.end(),
                           [](AudioClip *clip)
                           {
                               return !clip->isPlaying();
                           }),
            _clipList.end());
    }

    if (!_clipListNewAdd.empty())
    {
        _clipList.insert(_clipList.end(), _clipListNewAdd.begin(), _clipListNewAdd.end());
        _clipListNewAdd.clear();
    }
}