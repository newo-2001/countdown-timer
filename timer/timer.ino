#include <TM1637Display.h>
#include <ESP8266WiFi.h>
#include <user_interface.h>
#include "TimerConfig.h"
#include "Time.h"

TM1637Display display = TM1637Display(PIN_CLOCK, PIN_DATA);

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    display.setBrightness(1);
    display.clear();
}

inline uint32_t getSeconds(uint32_t unixTime) {
    return unixTime % 60;
}

inline uint32_t getMinutes(uint32_t unixTime) {
    return unixTime / 60 % 60;
}

inline uint32_t getHours(uint32_t unixTime) {
    return unixTime / 3600;
}

void displayTime(int32_t time)
{
    if (time <= 0)
    {
        display.setSegments(FINISH_MESSAGE);
    }
    else
    {
        uint32_t hours = getHours(time);
        if (hours > 999)
        {
            display.showNumberDec(hours);
        }
        else if (hours > 99)
        {
            display.showNumberDec(hours, false, 3, 1);
        }
        else
        {
            uint32_t clockTime = hours * 100 + getMinutes(time);
            display.showNumberDecEx(clockTime, 0b11100000, true);
        }
    }
}

void loop()
{
    uint32_t time = currentTime();
    int32_t dt = EVENT_TIME - time;

    displayTime(dt);
    
    uint32_t hours = getHours(dt);
    if (hours > 99 || dt < 0)
    {
        ESP.deepSleep(3600ULL * 1000 * 1000);
    }
    else
    {
        light_sleep(60000);
    }
}