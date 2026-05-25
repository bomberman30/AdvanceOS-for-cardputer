#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class HEX_Editor : public GlobalParentClass
{
public:


    HEX_Editor(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;
    void loadFile(const char* path);
void handleInput();

    std::vector<uint8_t> data;
    size_t cursor = 0;     // offset 
private:
};
