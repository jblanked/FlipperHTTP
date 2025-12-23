#pragma once
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
#include <TFT_eSPI.h>
#include <Arduino.h>
#undef TFT_BLACK
#define TFT_BLACK 0x0000
#undef TFT_WHITE
#define TFT_WHITE 0xFFFF
#endif

class LCD
{
public:
    LCD()
    {
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
        this->displayTFT = new TFT_eSPI();
        this->displayTFT->init();
        delay(10); // Small delay after initialization
        this->displayTFT->setRotation(0);
        this->displayTFT->fillScreen(TFT_BLACK);

        this->canvasTFT = new TFT_eSprite(this->displayTFT);
        this->canvasTFT->setColorDepth(8);
        if (this->canvasTFT->createSprite(320, 320))
        {
            this->canvasTFT->setTextFont(2);
            this->canvasTFT->setTextSize(1);
            this->canvasTFT->setTextColor(TFT_WHITE);
            this->canvasTFT->fillSprite(TFT_BLACK);
        }
#endif
    }

    ~LCD()
    {
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
        if (this->canvasTFT)
        {
            this->canvasTFT->deleteSprite();
            delete this->canvasTFT;
        }
        if (this->displayTFT)
        {
            delete this->displayTFT;
        }
#endif
    }

    void clear(uint16_t color = 0x0000)
    {
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
        if (this->canvasTFT)
        {
            this->canvasTFT->fillRect(0, 0, 320, 320, color);
        }
#endif
    }

    void print(uint16_t x, uint16_t y, const char *text)
    {
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
        if (this->canvasTFT)
        {
            this->canvasTFT->setCursor(x, y);
            this->canvasTFT->print(text);
        }
#endif
    }

    void swap()
    {
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
        if (this->canvasTFT)
        {
            this->canvasTFT->pushSprite(0, 0);
        }
#endif
    }

    void text(uint16_t x, uint16_t y, const char *text)
    {
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
        if (this->canvasTFT && this->displayTFT)
        {
            this->clear(TFT_BLACK);
            this->print(x, y, text);
            this->swap();
        }
#endif
    }

private:
#if defined(BOARD_PICOCALC_W) || defined(BOARD_PICOCALC_2W)
    TFT_eSPI *displayTFT = nullptr;   //  TFT SPI library
    TFT_eSprite *canvasTFT = nullptr; // Off-screen buffer ("sprite")
#endif
};