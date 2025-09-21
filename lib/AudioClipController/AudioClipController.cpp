#include "AudioClipController.h"
#include "AudioClip.h"

#include <algorithm>

AudioClipController::AudioClipController(int bclkPin, int lrclkPin, int dinPin,
                                         int _sd_sckPin, int _sd_misoPin, int _sd_mosiPin, int _sd_csPin)
    : _bclkPin(bclkPin), _lrclkPin(lrclkPin), _dinPin(dinPin),
      _engineOn_Req(false), _blinkerOn_Req(false), _hornOn(false), _engineRpm(0),
      _sd_sckPin(_sd_sckPin), _sd_misoPin(_sd_misoPin), _sd_mosiPin(_sd_mosiPin), _sd_csPin(_sd_csPin)
{
    clipMutex = xSemaphoreCreateMutex();
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

    // TODO:
    // blinkerStartItem = new AudioClip(this, "/blinker_start.wav", 1.0f, true);
    // blinkerEndItem   = new AudioClip(this, "/blinker_end.wav", 1.0f, true);

    blinkerItem = new AudioClip(this, "/blinker_runing.wav", 1.0f, true);
    backingUpBeepItem = new AudioClip(this, "/backing_up_beep.wav", 1.0f, true);

    int choice = random(0, 2);
    hornItem = new AudioClip(this, horn_filename[choice], 1.0f, false);
    engineStartItem = new AudioClip(this, "/engine_start1.wav", 1.0f, false);
    engineRuningItem = new AudioClip(this, "/engine_runing1.wav", 0.4f, true);

    vTaskDelay(100);

    Serial.println("");
    xTaskCreatePinnedToCore(_soundControllerTask, "_soundControllerTask", 4096, this, 1, NULL, 1);
    xTaskCreatePinnedToCore(_loopTask, "_loopTask", 4096, this, 1, NULL, 1);

    Serial.println("");
    Serial.println("AudioClipController::begin complete ");
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
                parent->engineStartItem->onEnd = [](AudioClip *sender, AudioClipController *controller)
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
            // engineRuningItem->setSpeed(parent->_engineRpm);
            // engineRpm = parent->_engineRpm;
            // todo
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
    Serial.print(" ");
    Serial.print(_hornOn);
    _hornOn = true;
}

void AudioClipController::addClip(AudioClip *c)
{
    if (xSemaphoreTake(clipMutex, portMAX_DELAY))
    {
        _clipList.push_back(c);
        xSemaphoreGive(clipMutex);
    }
}

void AudioClipController::removeClip(AudioClip *c)
{
    if (xSemaphoreTake(clipMutex, portMAX_DELAY))
    {
        _clipList.remove(c);
        xSemaphoreGive(clipMutex);
    }
}

void AudioClipController::loop()
{
    if (!xSemaphoreTake(clipMutex, portMAX_DELAY))
    {
        vTaskDelay(5);
        return;
    }
    bool _noPlaying = true;

    for (AudioClip *item : _clipList)
    {
        if (item->isPlaying())
        {
            _noPlaying = false;
            break;
        }
    }
    if (_noPlaying)
    {
        vTaskDelay(1000);
        return;
    }

    static bool ReadingFile = true;
    static bool IsRead = true;

    static byte Samples[NUM_BYTES_TO_READ_FROM_FILE];

    static uint16_t BytesReadFromFile;
    if (ReadingFile) // Read next chunk of data in from files
    {
        // Read data into the wavs own buffers
        IsRead = false;
        for (AudioClip *item : _clipList)
        {
            if (item->read())
                IsRead = true;
        }
        if (!IsRead)
        {
            Serial.print(" AudioClip::loop -- no _isPlaying!");
            vTaskDelay(1000);
            return;
        }
        BytesReadFromFile = Mix(Samples, _clipList); // Mix the samples together and store in the samples buffer
        ReadingFile = false;                         // Switch to sending the buffer to the I2S
    }
    else
        ReadingFile = FillI2SBuffer(Samples, BytesReadFromFile); // We keep calling this routine until it returns true, at which point
    xSemaphoreGive(clipMutex);
}

uint16_t AudioClipController::Mix(byte *samples, std::list<AudioClip *> &items)
{
    // Mix all playing wavs together, returns the max bytes that are in the buffer, usually this would be the full buffer but
    // in rare cases wavs may be close to the end of the file and thus not fill the entire buffer
    int32_t mixedSample;       // The mixed sample
    uint16_t i;                // index into main samples buffer
    uint16_t MaxBytesInBuffer; // Max bytes of data in buffer, most of time buffer will be full
    const float Volume = 0.3;
    i = 0;

    for (AudioClip *item : items)
        item->resetIdx();

    int activeCount = 0;

    while (hasActiveClips(items))
    {
        mixedSample = 0;
        activeCount = 0;

        // sum all samples
        for (AudioClip *audio : items)
        {
            if (audio->isReadyToMix())
            {
                mixedSample += audio->getNextSample();
                activeCount++;
            }
        }

        clampSample(mixedSample, activeCount);

        if (i + 1 < NUM_BYTES_TO_READ_FROM_FILE)
            *((int16_t *)(samples + i)) = (int16_t)mixedSample;
        else
            break;

        i += 2;
    }

    MaxBytesInBuffer = 0;
    for (AudioClip *audio : items)
        if (audio->getBytesInBuffer() > MaxBytesInBuffer)
            MaxBytesInBuffer = audio->getBytesInBuffer();

    if (MaxBytesInBuffer + 2 > NUM_BYTES_TO_READ_FROM_FILE)
        MaxBytesInBuffer = NUM_BYTES_TO_READ_FROM_FILE - 2;

    //  We now alter the data according to the volume control
    for (i = 0; i < MaxBytesInBuffer; i += 2)
        *((int16_t *)(samples + i)) = (*((int16_t *)(samples + i))) * Volume;

    return MaxBytesInBuffer;
}

bool AudioClipController::hasActiveClips(const std::list<AudioClip *> &items)
{
    for (AudioClip *item : items)
    {
        if (item->isBufferNotEmpty())
            return true;
    }
    return false;
}

void AudioClipController::clampSample(int32_t &mixedSample, int activeCount)
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
