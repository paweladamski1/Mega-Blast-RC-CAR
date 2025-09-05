#ifndef SOUND_CONTROLLER_H
#define SOUND_CONTROLLER_H

#include <Arduino.h>
#include "SD.h"
#include "driver/i2s.h"
#include "AudioClip.h"

static const i2s_config_t i2s_config =
    {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100, // Note, all files must be this
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
        .dma_buf_count = 8,                       // 8 buffers
        .dma_buf_len = 256,                       // 256 bytes per buffer, so 2K of buffer space
        .use_apll = 0,
        .tx_desc_auto_clear = true,
        .fixed_mclk = -1};

class SoundController
{
public:
    SoundController(int bclkPin, int lrclkPin, int dinPin,
                    int _sd_sckPin, int _sd_misoPin, int _sd_mosiPin, int _sd_csPin);

    void begin();

    void startEngine(const char *who);
    void stopEngine(const char *who);
    void setEngineRpm(const char *who, uint16_t rpm);

    void startBlinker(const char *who);
    void stopBlinker(const char *who);

    void playHorn(const char *who);

    void loop();

    void stopAll();

private:
    int _bclkPin, _lrclkPin, _dinPin;
    int _sd_sckPin, _sd_misoPin, _sd_mosiPin, _sd_csPin;

    volatile bool _engineOn;
    volatile bool _blinkerOn;
    volatile bool _hornOn;

    volatile uint16_t _engineRpm;
    AudioClip *musicItem;
    std::list<AudioClip *> _clipList;
    AudioClip *engineStartItem;
    AudioClip *engineRuningItem;
    //AudioClip *engineStopItem;
    AudioClip *blinkerRunItem;
    //AudioClip *blinkerEndItem; 
    //AudioClip *blinkerStartItem;
    AudioClip *hornItem;


    static void _soundControllerTask(void *param);
    static void _loopTask(void *param);
    void serialPrint(const char *procName, const char *who);
};

#endif // SOUND_CONTROLLER_H
