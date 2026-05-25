#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include "ResistorRes.h"
class ResistorCalc : public GlobalParentClass
{
public:
    ResistorCalc(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;

    const int pwmFreq = 5000;
    const int pwmResolution = 8;
    const int pwmLedChannelTFT = 0;

    int tt[4] = {1, 2, 5, 10};
    int cT[4] = {TFT_BROWN, TFT_RED, TFT_GOLD, TFT_SILVER};
    int colors[12] = {TFT_BLACK, TFT_BROWN, TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_BLUE, TFT_VIOLET, TFT_LIGHTGREY, TFT_WHITE, TFT_GOLD, TFT_SILVER};
    double multis[12] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 100000000, 0.1, 0.01};
    String mm[12] = {"x1", "x10", "x100", "x1K", "x10K", "x100K", "x1M", "x10M", "x100M", "x1G", "x0.1", "x0.01"};
    int band = 0;
    int color = 0;

    int first = 1;
    int secon = 2;
    int multiplier = 1;
    int tolerance = 2;
    int deb1 = 0;
    int deb2 = 0;

    int x = 56;
    int y = 24;
    int w = 126;
    int h = 20;

    int bPos[4] = {86, 98, 110, 138};
    // void Task_AudioC(void *pvParameters);
private:
};
