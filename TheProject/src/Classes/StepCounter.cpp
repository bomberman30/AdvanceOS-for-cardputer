#include "StepCounter.h"
#include "./MyOS.h"

#include <elapsedMillis.h>
#include "./Function.h"
#include "./Classes/MainMenuV2.h"
void StepCounter::Begin()
{
    // mainOS->StepCounterWork=true;
}

void StepCounter::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
      //  delay(500);
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        mainOS->StepCounterWork = !mainOS->StepCounterWork;
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_BACKSPACE))
    {
        mainOS->StepCounterWork = false;
        mainOS->steps = 0;
    }

    // GlobalParentClass::Loop();
    Draw();
}
void StepCounter::Draw()
{
    mainOS->sprite.createSprite(240, 135 - TopOffset);

    uint16_t bgColor = mainOS->StepCounterWork ? rgb565(200, 255, 200) : rgb565(255, 200, 200);
    uint16_t titleColor = mainOS->StepCounterWork ? rgb565(0, 150, 0) : rgb565(150, 0, 0);

    mainOS->sprite.fillScreen(bgColor);

    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextWrap(false);

    mainOS->sprite.setTextSize(2);
    mainOS->sprite.setTextColor(titleColor);
    mainOS->sprite.setCursor(10, 5);
    mainOS->sprite.print(mainOS->StepCounterWork ? "Step Counter: ON" : "Step Counter: OFF");

    mainOS->sprite.setTextSize(3);
    mainOS->sprite.setTextColor(TFT_BLACK);
    mainOS->sprite.setCursor(10, 40);

    String stepsStr = "Steps: ";
    stepsStr += String(mainOS->steps);
    mainOS->sprite.print(stepsStr);

    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextColor(TFT_DARKGREY);

    mainOS->sprite.setCursor(10, 80);
    mainOS->sprite.print("ENTER: Start / Pause");

    mainOS->sprite.setCursor(10, 90);
    mainOS->sprite.print("DEL: Reset steps");

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}