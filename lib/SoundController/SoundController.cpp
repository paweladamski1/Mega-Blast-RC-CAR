#include "SoundController.h"
#include "driver/i2s.h"
#include "sound_engine.h"
#include "sound_horn.h"
#include "sound_blinker.h"

SoundController::SoundController(int bclkPin, int lrclkPin, int dinPin)
    : _bclkPin(bclkPin), _lrclkPin(lrclkPin), _dinPin(dinPin),
      _engineTaskHandle(nullptr), _blinkerTaskHandle(nullptr), _hornTaskHandle(nullptr),
      _engineOn(false), _blinkerOn(false), _engineRpm(0)
{}

void SoundController::begin() {
    // Uninstall any existing driver
    i2s_driver_uninstall(I2S_NUM_0);

    i2s_config_t i2s_cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pins = {
        .bck_io_num = _bclkPin,
        .ws_io_num = _lrclkPin,
        .data_out_num = _dinPin,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    esp_err_t e = i2s_driver_install(I2S_NUM_0, &i2s_cfg, 0, NULL);
    if (e != ESP_OK) {
        Serial.printf("i2s_driver_install failed: %s (%d)\n", esp_err_to_name(e), e);
        return;
    }

    e = i2s_set_pin(I2S_NUM_0, &pins);
    if (e != ESP_OK) {
        Serial.printf("i2s_set_pin failed: %s (%d)\n", esp_err_to_name(e), e);
        return;
    }

    i2s_set_clk(I2S_NUM_0, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

    Serial.println("I2S driver installed.");
}

void SoundController::startEngine() {
    if (!_engineOn) {
        _engineOn = true;
        xTaskCreatePinnedToCore(_engineTask, "engineTask", 4096, this, 1, &_engineTaskHandle, 1);
    }
}

void SoundController::stopEngine() {
    _engineOn = false;
    _stopTask(&_engineTaskHandle);
}

void SoundController::setEngineRpm(uint16_t rpm) {
    _engineRpm = rpm;
}

void SoundController::startBlinker() {
    if (!_blinkerOn) {
        _blinkerOn = true;
        xTaskCreatePinnedToCore(_blinkerTask, "blinkerTask", 4096, this, 1, &_blinkerTaskHandle, 1);
    }
}

void SoundController::stopBlinker() {
    _blinkerOn = false;
    _stopTask(&_blinkerTaskHandle);
}

void SoundController::playHorn() {
    if (_hornTaskHandle == nullptr) {
        xTaskCreatePinnedToCore(_hornTask, "hornTask", 4096, this, 1, &_hornTaskHandle, 1);
    }
}

void SoundController::_playSound(const uint8_t* data, size_t length) {
    size_t bytes_written;
    for (size_t i = 0; i < length; i++) {
        int16_t sample = ((int16_t)pgm_read_byte(&data[i]) - 128) << 8; // Convert 8-bit to 16-bit
        i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
}

void SoundController::_playEngineSound() {
    float speedFactor = 0.5f + ((float)_engineRpm / 1000.0f) * 1.5f;
    size_t len = engineSound_len;

    float index = 0.0f;
    while (_engineOn) {
        if (index >= len) index = 0;
        int16_t sample = ((int16_t)pgm_read_byte(&engineSound[(size_t)index]) - 128) << 8;
        size_t bytes_written;
        i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
        index += speedFactor;
    }
}

void SoundController::_stopTask(TaskHandle_t* handle) {
    if (*handle != nullptr) {
        vTaskDelete(*handle);
        *handle = nullptr;
    }
}

void SoundController::_engineTask(void* param) {
    auto* self = static_cast<SoundController*>(param);
    self->_playEngineSound();
    vTaskDelete(nullptr);
}

void SoundController::_blinkerTask(void* param) {
    auto* self = static_cast<SoundController*>(param);
    while (self->_blinkerOn) {
        self->_playSound(blinkerSound, blinkerSound_len);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
    vTaskDelete(nullptr);
}

void SoundController::_hornTask(void* param) {
    auto* self = static_cast<SoundController*>(param);
    self->_playSound(hornSound, hornSound_len);
    self->_hornTaskHandle = nullptr;
    vTaskDelete(nullptr);
}
