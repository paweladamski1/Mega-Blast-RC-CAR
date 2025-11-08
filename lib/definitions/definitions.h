#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include "driver/i2s.h"

// Pin definitions

/* MOTOR DRIVER */
#define PWMA 32
#define AIN2 33
#define AIN1 25
#define STBY 26
#define BIN1 27
#define BIN2 14
#define PWMB 12

/* LEDS */
#define LED_STATUS 21

/* LIGHTS */
#define LED_LEFT_INDICATOR 18
#define LED_BRAKE 5
#define LED_MAIN_REAR 17
#define LED_REVERSE 16
#define LED_AUX 4
#define LED_RIGHT_INDICATOR 0

/* SENSORS  */
#define CHARGER_DETECT_PIN 34

/* SOUND */
#define I2S_DIN 23
#define I2S_BCLK 2
#define I2S_LRCLK 19

/* SD CARD */
#define SD_MISO 35
#define SD_SCK 13
#define SD_MOSI 22
#define SD_CS 15

// Serial communication
#define BAUD_RATE 9600 // Serial baud rate

// File handling
#define NUM_BYTES_TO_READ_FROM_FILE 1024 // Number of bytes to read from file at once

// Gamepad button definitions
#define DPAD_ARROW_UP 0x0001
#define DPAD_ARROW_DOWN 0x0002
#define DPAD_ARROW_RIGHT 0x0004
#define DPAD_ARROW_LEFT 0x0008
#define PS4_BTN 0x01
#define PS4_L1 0x0010
#define PS4_R1 0x0020

// structure
struct SControlData
{
    int momentum; // 0 to 255
    int steering; // -100 to 100
    bool brake;
    int gear;
};

struct SPadData
{
    int throttle = 0;   // 0 - 1023
    int brakeForce = 0; // 0 - 1023
    int steering = 0;   // -100 - 100
    bool hasData = false;
    bool emergencyLights = false;
    bool auxLights = false;
    bool indicatorLeft = false;
    bool indicatorRight = false;
    bool horn = false;
    bool playMusic = false;
    bool nextMusic = false;
    bool prevMusic = false;
    bool volumeUp = false;
    bool volumeDown = false;

    bool systemBtn = false;

    unsigned long lastPacket = 0;

    inline bool isBrake() const
    {
        return brakeForce > 3;
    }
};

struct Index5
{
    uint8_t value = 0;
    void increment() { value = (value + 1) % 5; }
    void decrement() { value = (value + 4) % 5; }
};

static const i2s_config_t i2s_config =
    {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 22050, // Note, all files must be this
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
        .dma_buf_count = 8,                       // 8 buffers
        .dma_buf_len = 256,                       // 256 bytes per buffer, so 2K of buffer space
        .use_apll = 0,
        .tx_desc_auto_clear = true,
        .fixed_mclk = -1};

template <typename T>
bool inRange(T value, T minVal, T maxVal) {
    return (value >= minVal && value <= maxVal);
}
        
#endif // DEFINITIONS_H