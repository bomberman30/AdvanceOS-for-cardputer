#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class Sample : public GlobalParentClass
{
public:


    Sample(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;
    void OnExit() override;
private:
};


/* M5Cardputer.Keyboard.isKeyPressed('/');//right
M5Cardputer.Keyboard.isKeyPressed(',');//left
M5Cardputer.Keyboard.isKeyPressed(';');//up
M5Cardputer.Keyboard.isKeyPressed('.');//down
M5Cardputer.Keyboard.isKeyPressed('p');//P button

M5Cardputer.Display.fillRoundRect()// fil round rect

M5Cardputer screen size is 240x135
example for Draw

    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H);
    mainOS->sprite.pushSprite(0, 0);
    // draw login here
    mainOS->sprite.deleteSprite();

*/
