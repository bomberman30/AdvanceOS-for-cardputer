#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class ColorPicker : public GlobalParentClass
{
public:
    ColorPicker(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop() override;
    void Draw() override;

private:
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    int selectedBar = 0; // 0=R 1=G 2=B 3=A

    void HandleInput();
    uint32_t GetColor() const;
    String GetHexString() const;
};