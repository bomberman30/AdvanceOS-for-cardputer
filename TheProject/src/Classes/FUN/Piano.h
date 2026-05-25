#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

struct PianoKey {
    char keyChar;
    int noteFreq;
    bool isPressed;
};

class Piano : public GlobalParentClass
{
public:
    Piano(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop() override;
    void Draw() override;

private:
    // 19 keys mapping from 'q' through 'l'
    PianoKey keys[19] = {
        {'q', 261, false}, {'w', 277, false}, {'e', 293, false}, {'r', 311, false},
        {'t', 329, false}, {'y', 349, false}, {'u', 369, false}, {'i', 392, false},
        {'o', 415, false}, {'p', 440, false}, {'a', 466, false}, {'s', 493, false},
        {'d', 523, false}, {'f', 554, false}, {'g', 587, false}, {'h', 622, false},
        {'j', 659, false}, {'k', 698, false}, {'l', 740, false}
    };

    const int keyWidth = 11; // Slimmer to fit 19 keys
    const int startX = 5;
    const int keyHeight = 100;
};