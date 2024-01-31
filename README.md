# Countdown Timer
Arduino code for the ESP8266 to count down to a specific date on a TM1637 LED display.
The chip is aware of the current time by regularly querying it over the internet using NTP.
The chip attempts to save as much power as possible by sleeping when applicable.
This project was originally made to count down to the release dates of the Persona 5 Royal PC port and Persona 3 Reload.

# Configuring
## Software
Copy `TimerConfig.example` to `TimerConfig.h` and enter the required information, namely:
- WiFi network name and password.
- Time of the event that is counted down (seconds in unix timestamp.)
- The segment configuration to display when the countdown has finished.
- The GPIO pins for the clock and data signals of the diplay.

## Hardware
- GPIO16 has to be connected to RESET to wake the chip from deep sleep.
- CLK and DATA should be connected to the clock and data signal on the display respectively.
- The display needs to be connected to 5/3.3v and GND.
