#pragma once
#include "Objects.h"


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

class Tssd1306 : public TControl,public Adafruit_SSD1306
{
private:


public:


    Tssd1306() : TControl(),Adafruit_SSD1306(128, 32, &Wire, -1)
    {
        begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
        clearDisplay();
        setTextColor(SSD1306_WHITE);
    };   


    ~Tssd1306();
};

Tssd1306::~Tssd1306()
{

}
