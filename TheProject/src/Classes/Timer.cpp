#include "Timer.h"
// #include <ESP32Time.h>
#include "./MyOS.h"
#include "./Function.h"
#include "./Classes/MainMenuV2.h"
void Timer::Begin()
{
    setSeconds = 0;
    inputBuffer = "";
}

void Timer::Loop()
{

\
    HandleKeyboard();
    if (!mainOS->screenOff)
    {
        Draw();
    }
}

void Timer::Draw()
{
    mainOS->sprite.createSprite(240, 135 - TopOffset);

    mainOS->sprite.fillScreen(rgb565(240, 240, 255));
    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextWrap(false);

    mainOS->sprite.setTextColor(TFT_NAVY);
    mainOS->sprite.setTextSize(2);
    mainOS->sprite.setCursor(10, 5);
    mainOS->sprite.print("Timer");

    mainOS->sprite.setTextColor(TFT_DARKGREY);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(10, 35);
    mainOS->sprite.print("Set time (mm:ss) and press ENTER:");

    mainOS->sprite.setTextSize(2);
    mainOS->sprite.setCursor(10, 55);
    mainOS->sprite.setTextColor(TFT_BLACK);
    mainOS->sprite.print(inputBuffer);

    mainOS->sprite.setTextSize(3);
    mainOS->sprite.setCursor(10, 85);

    int m = mainOS->remainingSeconds / 60;
    int s = mainOS->remainingSeconds % 60;

    char buffer[10];
    sprintf(buffer, "%02d:%02d", m, s);

    mainOS->sprite.setTextColor(mainOS->timerRunning ? TFT_DARKGREEN : TFT_RED);
    mainOS->sprite.print(buffer);

    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(10, 120);
    mainOS->sprite.setTextColor(TFT_DARKGREY);
    mainOS->sprite.print("DEL = delete");

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

void Timer::HandleKeyboard()
{

    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
    }

    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        if (mainOS->timerRunning)
            mainOS->timerRunning = false;
        else
        {
            ParseTime();
            inputBuffer = "";
        }
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_BACKSPACE))
    {
        if (inputBuffer.length() > 0)
            inputBuffer.clear();
        return;
    }
    if (M5Cardputer.Keyboard.isChange())
    {
        if (M5Cardputer.Keyboard.isPressed())
        {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            for (auto c : status.word)
            {
                if (c >= '0' && c <= '9')
                {
                    if (inputBuffer.length() < 5)
                    {
                        inputBuffer += c;

                        if (inputBuffer.length() == 3)
                        {
                            int value = inputBuffer.toInt();
                            int minutes = value / 100;
                            int seconds = value % 100;

                            char buf[6];
                            sprintf(buf, "%d:%02d", minutes, seconds);
                            inputBuffer = buf;
                        }

                        else if (inputBuffer.length() == 4 || inputBuffer.length() == 5)
                        {
                            String digits = "";
                            for (char ch : inputBuffer)
                                if (ch >= '0' && ch <= '9')
                                    digits += ch;

                            int value = digits.toInt();
                            int minutes = value / 100;
                            int seconds = value % 100;

                            char buf[6];
                            sprintf(buf, "%d:%02d", minutes, seconds);
                            inputBuffer = buf;
                        }
                    }
                }
            }
        }
    }
}
// convert from 1111 to actual 11:11
void Timer::ParseTime()
{
    // mainOS->ShowOnScreenMessege("in pars",1000);
    if (inputBuffer.length() < 1) //
        return;
    // mainOS->ShowOnScreenMessege("calculate second",1000);

    int minutes = 0, seconds = 0; 

    int colon = inputBuffer.indexOf(':'); 
    if (colon >= 0)                       
    {
        minutes = inputBuffer.substring(0, colon).toInt();  
        seconds = inputBuffer.substring(colon + 1).toInt(); 
    }
    else
    {
        seconds = inputBuffer.toInt();
    }

    setSeconds = minutes * 60 + seconds;
    mainOS->TimerStart(setSeconds);
    mainOS->lastTick = millis(); 
}