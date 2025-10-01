#ifndef DEFINITIONS_H
#define DEFINITIONS_H

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

#endif // DEFINITIONS_H