#include "GPIOcontrol.h"
#include "./MyOS.h"
#include "./Function.h"
#include "./Classes/MainMenuV2.h"
GPIOcontrol::GPIOcontrol(MyOS *os) : GlobalParentClass(os)
{

    pins[0] = {1, 0, 0, -1};
    pins[1] = {2, 0, 0, -1};
    pins[2] = {3, 0, 0, -1};
    pins[3] = {4, 0, 0, -1};
    pins[4] = {6, 0, 0, -1};
    pins[5] = {40, 0, 0, -1};
    pins[6] = {14, 0, 0, -1};
    pins[7] = {39, 0, 0, -1};
    pins[8] = {5, 0, 0, -1};
    pins[9] = {13, 0, 0, -1};
    pins[10] = {15, 0, 0, -1};
    // static const int PIN_COUNT = 11;change in header
}

void GPIOcontrol::Begin()
{
    for (int i = 0; i < PIN_COUNT; ++i)
    {
        pins[i].pwmChannel = -1;
        applyMode(i);
    }
}

void GPIOcontrol::applyMode(int i)
{
    int pin = pins[i].pin;

    if (pins[i].pwmChannel >= 0)
    {
        ledcDetachPin(pin);
        pins[i].pwmChannel = -1;
    }

    switch (pins[i].mode)
    {
    case 0: // none
            // pinMode(pin, INPUT);
        break;
    case 1: // INPUT
        pinMode(pin, INPUT);
        break;
    case 2: // INPUT_PULLUP
        pinMode(pin, INPUT_PULLUP);
        break;
    case 3: // OUTPUT
        pinMode(pin, OUTPUT);
        digitalWrite(pin, pins[i].value ? HIGH : LOW);
        break;
    case 4: // PWM
    {
        int channel = i % 16;
        const int freq = 5000;
        const int resolution = 8; // 0-255
        ledcSetup(channel, freq, resolution);
        ledcAttachPin(pin, channel);
        pins[i].pwmChannel = channel;
        ledcWrite(channel, pins[i].value);
        break;
    }
    case 5:
        pinMode(pin, INPUT);
        break;
    default:
        pinMode(pin, INPUT);
    }
}

void GPIOcontrol::handleKeyboard()
{

        if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
       // delay(500);
    }
    if (mainOS->NewKey.ifKeyJustPress(';')) // UP
        if (ShowMenu)
        {
            inMenuID--;
            if (inMenuID < 0)
            {
                inMenuID = MenuItems.size() - 1;
            }
        }
        else
        {
            selected = (selected - 1 + PIN_COUNT) % PIN_COUNT;
        }

    if (mainOS->NewKey.ifKeyJustPress('.')) // DOWN
        if (ShowMenu)
        {
            inMenuID++;
            if (inMenuID >= MenuItems.size())
            {
                inMenuID = 0;
            }
        }
        else
        {
            selected = (selected + 1) % PIN_COUNT;
        }
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        if (ShowMenu)
        {
            ApplyFromMenu();
        }
        else
        {
            ShowMenu = true;
        }
    }
    if (mainOS->NewKey.ifKeyJustPress(',')) // LEFT
    {
        if (ShowMenu)
        {
            return;
        }
        else
        {
            if (pins[selected].mode == 4)
            {
                pins[selected].value = min(255, pins[selected].value - 10);
                if (pins[selected].pwmChannel >= 0)
                    ledcWrite(pins[selected].pwmChannel, pins[selected].value);
            }
            else if (pins[selected].mode == 3) // mode OUTPUT
            {
                //   if (mainOS->NewKey.ifKeyJustPress('=') || mainOS->NewKey.ifKeyJustPress('-'))
                // if (pins[selected].mode == 2) // OUTPUT

                pins[selected].value = !pins[selected].value;
                digitalWrite(pins[selected].pin, pins[selected].value ? HIGH : LOW);
            }
        }
        // pins[selected].value = 0;
        // pins[selected].mode = (pins[selected].mode - 1 + 6) % 6; //????
        //  applyMode(selected);
    }

    if (mainOS->NewKey.ifKeyJustPress('/')) // RIGHT
    {
        if (ShowMenu)
        {
            return;
        }
        else // decrease value
        {
            if (pins[selected].mode == 4)
            {
                pins[selected].value = min(255, pins[selected].value + 10);
                if (pins[selected].pwmChannel >= 0)
                    ledcWrite(pins[selected].pwmChannel, pins[selected].value);
            }

            else if (pins[selected].mode == 3) // mode OUTPUT
            {
                //   if (mainOS->NewKey.ifKeyJustPress('=') || mainOS->NewKey.ifKeyJustPress('-'))
                // if (pins[selected].mode == 2) // OUTPUT

                pins[selected].value = !pins[selected].value;
                digitalWrite(pins[selected].pin, pins[selected].value ? HIGH : LOW);
            }
        }
        // pins[selected].value = 0;

        //  pins[selected].mode = (pins[selected].mode + 1) % 6; //????
        //  applyMode(selected);
    }


}

void GPIOcontrol::Loop()
{
    //GlobalParentClass::Loop();
    handleKeyboard();

    for (int i = 0; i < PIN_COUNT; ++i)
    {
        if (pins[i].mode == 5)
        {
            pins[i].value = analogRead(pins[i].pin);
        }
        if (pins[i].mode == 1) // digital read
        {
            pins[i].value = digitalRead(pins[i].pin);
        }
    }
    CameraY = lerpFloat(CameraY, selected * rowH - 40, 0.1); //
                                                        // CameraY=;// 40 offset
    if (!mainOS->screenOff)
    {
        Draw();
    }
}
int getTextColorByRow(int row)
{
    if (row == 3 || row == 7) // green
    {
        return TFT_BLACK;
    }
    else
    {
        {
            return WHITE;
        }
    }
}

int getColorByRow(int row)
{
    switch (row)
    {
    case 0:
        return TFT_NAVY;
        break;
    case 1:
        return TFT_RED;
        break;
    case 2:
        return TFT_BLACK;
        break;
    case 3:
        return TFT_GREEN;
        break;
    case 4:
        return TFT_PURPLE;
        break;
    case 5:
        return TFT_BROWN;
        break;
    case 6:
        return TFT_DARKGREEN;
        break;
    case 7:
        return TFT_YELLOW;
        break;
    case 8:
        return TFT_ORANGE;
        break;
    case 9:
        return TFT_GOLD;
        break;
    case 10:
        return TFT_MAROON;
        break;

    default:
        return TFT_NAVY;

        break;
    }
}
void GPIOcontrol::Draw()
{
    auto &sp = mainOS->sprite;
    sp.createSprite(240, 135 - TopOffset);
    sp.fillScreen(TFT_BLACK);
    sp.unloadFont();
    sp.setTextSize(1);

    sp.setTextColor(TFT_CYAN);
    sp.setCursor(5, -20 - CameraY);
    sp.println("GPIO Control - Cardputer ADV");
    sp.println("ENTER to Change Mode,\nRight Left change value On PMW or output");
    sp.println("Use max 3.3v for input,\nUse resistor for output!!");

    int y = 20; 
    for (int i = 0; i < PIN_COUNT; i++)
    {
        bool sel = (i == selected);

        uint16_t bg = sel ? TFT_DARKGREY : getColorByRow(i);
        uint16_t border = sel ? TFT_YELLOW : TFT_WHITE;

        sp.fillRect(0, y - CameraY, 240, rowH, bg);

        sp.drawRect(0, y - CameraY, 240, rowH, border);

        sp.setTextColor(sel ? TFT_YELLOW : getTextColorByRow(i));

        String modeStr;
        switch (pins[i].mode)
        {
        case 0:
            modeStr = "NONE";
            break; 
        case 1:
            modeStr = "DIGITAL IN MAX 3.3V";
            break; 
        case 2:
            modeStr = "IN_PULLUP MAX 3.3V";
            break;
        case 3:
            modeStr = "OUTPUT";
            break; 
        case 4:
            modeStr = "PWM";
            break; 
        case 5:
            modeStr = "ANALOG IN MAX 3.3V";
            break; 
        default:
            modeStr = "UNKNOWN";
            break; 
        }

        // draw pin nomber
        sp.setCursor(4, y - CameraY + 2);
        sp.printf("PIN %02d", pins[i].pin);
        // w mode type
        sp.setCursor(45, y - CameraY + 2);
        sp.printf("| %6s", modeStr.c_str());
        // draw value
        sp.setCursor(170, y - CameraY + 2);

        if (pins[i].mode == 4 && pins[i].pwmChannel >= 0) // if pin mode pmw draw
        {
            sp.printf("| val %3d  ch %d", pins[i].value, pins[i].pwmChannel);
        }
        else if (pins[i].mode == 0) // pin mode NONE dont draw thing
        {
        }
        else if (pins[i].mode == 3) // pin mode output
        {
            String t = "| ";
            if (pins[i].value == 1)
            {
                t += "HIGH 3.3V";
            }
            else
            {
                t += "LOW 0V";
            }
            sp.print(t);
        }
        else // else analog or digital  input or input pullup
        {
            sp.printf("| val %4d", pins[i].value);
        }
        y += rowH;
    }
    if (ShowMenu)
    {
        RenderMenu();
    }

    sp.pushSprite(0, 0);
    sp.deleteSprite();
}
void GPIOcontrol::ApplyFromMenu()
{
    pins[selected].value = 0;
    //  std::vector<String> MenuItems = {"INPUT", "INPUT_PULLUP", "ANALOG_READ", "DIGITAL_OUT", "PMW"};
    if (MenuItems[inMenuID] == "INPUT")
    {
        pins[selected].mode = 1;
        //  applyMode(selected);
    }
    else if (MenuItems[inMenuID] == "INPUT_PULLUP")
    {
        pins[selected].mode = 2;
        //    applyMode(selected);
    }
    else if (MenuItems[inMenuID] == "ANALOG_READ")
    {
        pins[selected].mode = 5;
        // applyMode(selected);
    }
    else if (MenuItems[inMenuID] == "DIGITAL_OUT")
    {
        pins[selected].mode = 3;
        //  applyMode(selected);
    }
    else if (MenuItems[inMenuID] == "PMW")
    {
        pins[selected].mode = 4;
    }
    else if (MenuItems[inMenuID] == "Close Menu")
    {
        // pins[selected].mode = 4;
        ShowMenu = false;

        return;
    }
    ShowMenu = false;
    applyMode(selected);

    // pins[selected].value = 0;
    // pins[selected].mode = (pins[selected].mode - 1 + 6) % 6; //????
    //  applyMode(selected);
}
void GPIOcontrol::RenderMenu()
{
    int Pos = 0;
    const int DistanceBetweenMenu = 10;
    auto &sp = mainOS->sprite;
    sp.fillRect(10, 10, 150, 100, BLACK);
    sp.drawRect(10, 10, 150, 100, YELLOW);
    for (int i = 0; i < MenuItems.size(); i++)

    {
        if (i == inMenuID)
        {
            sp.setTextColor(RED);
        }
        else
        {
            sp.setTextColor(YELLOW);
        }
        sp.setCursor(12, 12 + Pos);
        sp.print(MenuItems[i]);
        Pos += DistanceBetweenMenu;
    }
}