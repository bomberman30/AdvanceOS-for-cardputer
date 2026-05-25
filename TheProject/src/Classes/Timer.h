#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class Timer : public GlobalParentClass
{
public:
    // M5Canvas sprite;
    //  M5Canvas SpeakerSprite;
    // M5Canvas Note1Sprite;
    // M5Canvas Note2Sprite;

    Timer(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;

void HandleKeyboard();
void ParseTime();


    int setSeconds = 0;    

String inputBuffer = ""; 


private:
};
