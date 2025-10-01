#include <Arduino.h>
#include <unity.h>
#include "AudioClipController.h"
#include "definitions.h" 


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
        Serial.println("");
        Serial.println(t);
        if (t == 15)
            sound.stopEngine(who);

        if (t == 25)
            sound.playStartEngine("test_audio");

        if (t == 27)
            sound.setEngineRpm(who, 30);
        if (t == 29)
            sound.setEngineRpm(who, 21);
        if (t == 35)
            sound.setEngineRpm(who, 0);
        if (t == 37)
            sound.setEngineRpm(who, 30);
        if (t == 45)
            sound.setEngineRpm(who, 10);
        if (t == 50)
            sound.setEngineRpm(who, 0);

        if (t == 58)
            sound.stopEngine(who);

        // events
        /*  if(t==15)  sound.playHorn(who);
          if(t == 25) sound.playStartBlinker(who);
          if(t == 30) sound.stopBlinker(who);
          if(t > 15) sound.setEngineRpm(who, t);
          if(t==30) sound.playMusic();
          if(t==40) sound.playNextMusic();
          if(t==45) sound.stopMusic();*/
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
