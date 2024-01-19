#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Time.h"
#include "TimerConfig.h"

extern os_timer_t* timer_list;

WiFiUDP udp;

uint32_t last_ntp_request_time = 0;
uint32_t last_ntp_response = 0;
uint32_t current_time = 0;

void connectToWifi()
{
    Serial.print("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    char buff[128];
    sprintf(buff, "\nConnected to %s\nIP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    Serial.print(buff);
}

void sendNtpRequest(uint8_t buffer[], size_t length)
{
    IPAddress timeServerIp;
    if (!WiFi.hostByName(TIME_SERVER, timeServerIp)) {
        Serial.println("Time server DNS lookup failed. Rebooting.");
        Serial.flush();
        ESP.reset();
    }

    Serial.println("Sending NTP request...");

    constexpr uint16_t NTP_PORT = 123;
    udp.begin(NTP_PORT);

    memset(buffer, 0, length);
    buffer[0] = 0b11100011; // LI, Version, Mode
    
    udp.beginPacket(TIME_SERVER, NTP_PORT);
    udp.write(buffer, length);
    udp.endPacket();
}

uint32_t parseNtpPacket(uint8_t buffer[], size_t length) {
    if (!udp.parsePacket()) return 0;

    udp.read(buffer, length);

    uint32_t ntpTime = buffer[40] << 24 | buffer[41] << 16 | buffer[42] << 8 | buffer[43];

    // Convert to unix time (s) by subtracting 70 years
    return ntpTime - 2208988800UL;
}

void synchronizeClock()
{
    connectToWifi();

    constexpr uint32_t NTP_TIMEOUT = 5000;
    constexpr size_t NTP_PACKET_SIZE = 48;

    uint32_t sendTime = 0;
    uint32_t ntpResponse = 0;
    uint8_t ntpBuffer[NTP_PACKET_SIZE];

    while (!ntpResponse)
    {
        // If we got no response within 5 seconds send a new request
        if (!sendTime || millis() - sendTime > NTP_TIMEOUT)
        {
            sendNtpRequest(ntpBuffer, NTP_PACKET_SIZE);
            sendTime = millis();
        }

        delay(1000);
        ntpResponse = parseNtpPacket(ntpBuffer, NTP_PACKET_SIZE);
    }

    // Turn WiFi off the save power
    udp.stop();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

    Serial.println("Received NTP response");
    last_ntp_request_time = current_time;
    last_ntp_response = ntpResponse;
}

uint32_t currentTime()
{
    // Synchronize our clock every hour using NTP
    constexpr uint32_t NTP_TTL = 3600000;
    uint32_t time_since_response = current_time - last_ntp_request_time;

    if (!last_ntp_request_time || time_since_response > NTP_TTL)
    {
        synchronizeClock();
        time_since_response = 0;
    }

    return last_ntp_response + time_since_response / 1000;
}

void wakeup_cb()
{
    Serial.println("Waking from light sleep");
    Serial.flush();
}

void light_sleep(uint32_t millis)
{
    WiFi.mode(WIFI_OFF);
    timer_list = nullptr;
    wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_set_wakeup_cb(wakeup_cb);
    wifi_fpm_do_sleep(millis * 1000);
    delay(millis + 1);
    wifi_fpm_do_wakeup();
    wifi_fpm_close();

    current_time += millis;
}