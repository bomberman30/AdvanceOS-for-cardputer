#include "ResistorCalc.h"
#include "../../MyOS.h"
#include "./Classes/MainMenuV2.h"

void ResistorCalc::Begin()
{

    showTopBar = false;
    M5Canvas &spr = mainOS->sprite;

    spr.createSprite(240, 135); 
    spr.setSwapBytes(true);
    // spr.setRotation(1);
    spr.unloadFont();
    spr.setTextSize(1);
    spr.fillSprite(TFT_BLACK);
    spr.drawString("Resistor CALC", x, 0, 2);
    spr.pushImage(x, y, 120, 26, resistor);

    spr.fillRect(x + 30, y, 6, 26, colors[first]);
    spr.fillRect(x + 42, y + 3, 6, 20, colors[secon]);
    spr.fillRect(x + 54, y + 3, 6, 20, colors[multiplier]);
    spr.fillRect(x + 82, y, 6, 26, cT[tolerance]);

    spr.fillCircle(bPos[band] + 2, 58, 2, TFT_WHITE);

    for (int i = 0; i < 12; i++)
    {

        if (i < 10)
        {
            spr.fillRect(0, i * 13, 11, 13, colors[i]);
            spr.fillRect(22, i * 13, 11, 13, colors[i]);

            if (i == 0)
                spr.setTextColor(TFT_WHITE, colors[i]);
            else
                spr.setTextColor(TFT_BLACK, colors[i]);

            spr.drawString(String(i), 3, (i * 13) + 2);
            spr.drawString(String(i), 25, (i * 13) + 2);
            spr.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        if (i == 0)
            spr.drawCircle(235, (i * 11) + 5, 4, TFT_WHITE);
        else
            spr.fillCircle(235, (i * 11) + 5, 4, colors[i]);

        spr.drawString(mm[i], 194, (i * 11) + 1);
    }

    /*
    spr.drawString("<",14,(first*13)+2);
    spr.drawString("<",36,(secon*13)+2);
     */

    spr.fillCircle(16, (first * 13) + 5, 2, TFT_WHITE);
    spr.fillCircle(38, (secon * 13) + 5, 2, TFT_WHITE);
    spr.fillCircle(188, (multiplier * 11) + 5, 2, TFT_WHITE);

    for (int i = 0; i < 4; i++)
    {
        spr.fillRect(44 + (i * 34), 117, 34, 11, cT[i]);
        spr.setTextColor(TFT_BLACK, cT[i]);
        spr.drawString(String(tt[i]) + "%", 54 + (i * 34), 119);
    }
    spr.fillCircle(61 + (tolerance * 34), 133, 2, TFT_WHITE);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.pushSprite(0, 0);
    // spr.deleteSprite();
}

void ResistorCalc::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        //  delay(400);
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
        // delay(500);
    }
    // GlobalParentClass::Loop();
    M5Canvas &spr = mainOS->sprite;
    // spr.createSprite(240, 135); 

    if (M5Cardputer.Keyboard.isChange())
        if (M5Cardputer.Keyboard.isPressed())
        {
            if (M5Cardputer.Keyboard.isKeyPressed('/'))
            {
        mainOS->PlayCuteEvilTone();
                spr.fillCircle(bPos[band] + 2, 58, 2, TFT_BLACK);

                band++;
                if (band > 3)
                    band = 0;
                spr.fillCircle(bPos[band] + 2, 58, 2, TFT_WHITE);
            }

            if (M5Cardputer.Keyboard.isKeyPressed(','))
            {
        mainOS->PlayCuteEvilTone();
                spr.fillCircle(bPos[band] + 2, 58, 2, TFT_BLACK);

                band--;
                if (band < 0)
                    band = 3;
                spr.fillCircle(bPos[band] + 2, 58, 2, TFT_WHITE);
            }

            if (M5Cardputer.Keyboard.isKeyPressed('.'))
            {
        mainOS->PlayCuteEvilTone();
                if (band == 0)
                {
                    spr.fillCircle(16, (first * 13) + 5, 2, TFT_BLACK);
                    first++;
                    if (first > 9)
                        first = 0;
                    spr.fillRect(x + 30, y, 6, 26, colors[first]);
                    spr.fillCircle(16, (first * 13) + 5, 2, TFT_WHITE);
                }
                if (band == 1)
                {
                    spr.fillCircle(38, (secon * 13) + 5, 2, TFT_BLACK);
                    secon++;
                    if (secon > 9)
                        secon = 0;
                    spr.fillCircle(38, (secon * 13) + 5, 2, TFT_WHITE);
                    spr.fillRect(x + 42, y + 3, 6, 20, colors[secon]);
                }

                if (band == 2)
                {
                    spr.fillCircle(188, (multiplier * 11) + 5, 2, TFT_BLACK);
                    multiplier++;
                    if (multiplier > 11)
                        multiplier = 0;
                    spr.fillCircle(188, (multiplier * 11) + 5, 2, TFT_WHITE);
                    spr.fillRect(x + 54, y + 3, 6, 20, colors[multiplier]);
                }

                if (band == 3)
                {
                    spr.fillCircle(61 + (tolerance * 34), 133, 2, TFT_BLACK);
                    tolerance++;
                    if (tolerance > 3)
                        tolerance = 0;
                    spr.fillCircle(61 + (tolerance * 34), 133, 2, TFT_WHITE);
                    spr.fillRect(x + 82, y, 6, 26, cT[tolerance]);
                }

                double res = ((first * 10) + secon) * multis[multiplier];
                spr.fillRect(44, 70, 136, 46, 0x00A5);
                double maxx = res + (res * (tt[tolerance] / 100.00));
                double minn = res - (res * (tt[tolerance] / 100.00));
                spr.setTextColor(TFT_WHITE, 0x00A5);

                if (res >= 1000 && res < 1000000)
                {
                    spr.drawString(String(first) + String(secon) + "x10 =" + String(res / 1000) + " K", 48, 76, 2);
                    spr.drawString(String(multiplier), 85, 72);
                    spr.drawString("MAX:" + String(maxx), 48, 94);
                    spr.drawString("MIN:" + String(minn), 48, 106);
                }

                else if (res >= 1000000)
                {
                    spr.drawString(String(first) + String(secon) + "x10 =" + String(res / 100000) + " M", 48, 76, 2);
                    spr.drawString(String(multiplier), 85, 72);
                    spr.drawString("MAX:" + String(maxx), 48, 94);
                    spr.drawString("MIN:" + String(minn), 48, 106);
                }

                else
                {
                    spr.drawString(String(first) + String(secon) + "x10 =" + String(res) + " ohm", 48, 76, 2);
                    spr.drawString(String(multiplier), 85, 72);
                    spr.drawString("MAX:" + String(maxx), 48, 94);
                    spr.drawString("MIN:" + String(minn), 48, 106);
                }

                spr.setTextColor(TFT_WHITE, TFT_BLACK);
            }
        }
    spr.pushSprite(0, 0);
    //  spr.deleteSprite();
}

void ResistorCalc::Draw()
{
}
