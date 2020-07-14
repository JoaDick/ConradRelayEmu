#pragma once

#include <Wire.h>
#include <Adafruit_SSD1306.h>

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#endif

#ifndef OLED_RESET
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#endif


class RelayBoxDisplay
{
    Adafruit_SSD1306 display{SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET};
    bool isReady = false;

public:
    void begin(uint8_t relayState)
    {
        // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
        if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        {
            isReady = true;
        }
        setRelayState(relayState);
    }

    void setRelayState(uint8_t relayState)
    {
        if(!isReady)
        {
            return;
        }

        display.clearDisplay();
        display.setCursor(0, 0); // Start at top-left corner
        display.setTextSize(2);  // Draw 2X-scale text

        for (uint8_t i = 0; i < 4; ++i)
        {
            display.setTextColor(WHITE);
            display.print(F("R "));
            display.print(i);
            display.print(F(" = "));

            const uint8_t mask = 0x01 << i;
            if (relayState & mask)
            {
                display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
                display.println(F(" 1 "));
            }
            else
            {
                display.println(F(" 0 "));
            }
        }

        display.display();
    }
};
