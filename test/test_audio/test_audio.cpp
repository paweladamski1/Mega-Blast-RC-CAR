#include <Arduino.h>
#include <unity.h>
#include "AudioClipController.h"

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
AudioClipController sound(I2S_BCLK, I2S_LRCLK, I2S_DIN, SD_SCK, SD_MISO, SD_MOSI, SD_CS);

void test_controller_play(void)
{
    const char *who = "test_controller_play";
    sound.begin();
    delay(1000);
    sound.playStartEngine("test_audio");
    Serial.println("------------------------------------ Starting manual playback...");
    int t = 0;
    int t_out = 60;
    while (t < t_out)
    {
        t++;
        delay(1000);
        Serial.print("+");

        // events
        if(t==15)  sound.playHorn(who);
        if(t == 25) sound.playStartBlinker(who);
        if(t == 30) sound.stopBlinker(who);
        if(t > 15) sound.setEngineRpm(who, t);
        if(t==30) sound.playMusic();
        if(t==40) sound.playNextMusic();
        if(t==45) sound.stopMusic();

    }
    sound.stopMusic();
    sound.stopEngine("test_audio");
    TEST_ASSERT_TRUE(true);
}

void setup()
{
    Serial.begin(9600);
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_controller_play);

    UNITY_END();
}

void loop()
{
}
