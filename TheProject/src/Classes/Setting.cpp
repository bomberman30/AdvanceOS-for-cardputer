#include "Setting.h"
#include "./MyOS.h"
// #include "Pic/logo.cpp"
#include "Pic/logo.cpp"
#include "MainMenuV2.h"
#include "MenuItemManeger.h"
void Setting::Begin()
{
    SlectedString = &menuItems;
}

void Setting::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
        return;
    }
    if (mainOS->screenOff)
    {
        return;
    }
    // if (M5Cardputer.Keyboard.isChange())
    //{

    if (mainOS->NewKey.ifKeyJustPress('.'))

    // if (M5Cardputer.Keyboard.isKeyPressed('.'))
    {
        MenuIdFocus++;
        if (MenuIdFocus > SlectedString->size() - 1)
            MenuIdFocus = 0;
        targetCameraY = MenuIdFocus * DistanceBetweenTXT;
    }
    if (mainOS->NewKey.ifKeyJustPress(';'))

    // if (M5Cardputer.Keyboard.isKeyPressed(';'))
    {
        MenuIdFocus--;
        if (MenuIdFocus < 0)
            MenuIdFocus = SlectedString->size() - 1;
        targetCameraY = MenuIdFocus * DistanceBetweenTXT;
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))

    //  if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))
    {
        if ((*SlectedString)[MenuIdFocus] == "Brightness")
        {
            SlectedString = &Brightness;
            MenuIdFocus = 0;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Screen Dim Timer")
        {
            SlectedString = &DimTimer;
        }
        else if ((*SlectedString)[MenuIdFocus] == "UI Sound")
        {
            SlectedString = &GUIsound;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Fonts")
        {
            SlectedString = &Fonts;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Set CPU Frequency (for Test)")
        {
            SlectedString = &FREQ;
        }

        else if ((*SlectedString)[MenuIdFocus] == "Brightness100%")
        {
            mainOS->setBrightness(127);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Brightness90%")
        {
            mainOS->setBrightness(90);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Brightness70%")
        {
            mainOS->setBrightness(70);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Brightness50%")
        {
            mainOS->setBrightness(50);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Brightness40%")
        {
            mainOS->setBrightness(40);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Brightness10%")
        {
            mainOS->setBrightness(10);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Show/Unshow Memory Status")
        {
            mainOS->showMemoryOnScreen = !mainOS->showMemoryOnScreen;
            mainOS->saveSettings();
        }

        else if ((*SlectedString)[MenuIdFocus] == "Turn Off In 100 Second")
        {
            mainOS->ScreenDimInTimeSecond = 100;
            mainOS->saveSettings();
            SlectedString = &menuItems;
            MenuIdFocus = 0;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Turn Off In 50 Second")
        {
            mainOS->ScreenDimInTimeSecond = 50;
            mainOS->saveSettings();
            SlectedString = &menuItems;
            MenuIdFocus = 0;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Turn Off In 30 Second")
        {
            mainOS->ScreenDimInTimeSecond = 30;
            mainOS->saveSettings();
            SlectedString = &menuItems;
            MenuIdFocus = 0;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Turn Off In 20 Second")
        {
            mainOS->ScreenDimInTimeSecond = 20;
            mainOS->saveSettings();
            SlectedString = &menuItems;
            MenuIdFocus = 0;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Turn Off In 10 Second")
        {
            mainOS->ScreenDimInTimeSecond = 10;
            mainOS->saveSettings();
            SlectedString = &menuItems;
            MenuIdFocus = 0;
        }
        else if ((*SlectedString)[MenuIdFocus] == "SOUND ON")
        {
            mainOS->UI_SOUND = true;
            mainOS->saveSettings();
            SlectedString = &menuItems;
            MenuIdFocus = 0;
        }
        else if ((*SlectedString)[MenuIdFocus] == "SOUND OFF")
        {
            mainOS->UI_SOUND = false;
            mainOS->saveSettings();
            SlectedString = &menuItems;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Default Font")
        {
            mainOS->FontSelected = 0;
            mainOS->saveSettings();
            SlectedString = &menuItems;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Second Font")
        {
            mainOS->FontSelected = 1;
            mainOS->saveSettings();
            SlectedString = &menuItems;
        }

        else if ((*SlectedString)[MenuIdFocus] == "240 MHz")
        {

            setCpuFrequencyMhz(240);
            SlectedString = &menuItems;
        }
        else if ((*SlectedString)[MenuIdFocus] == "160 MHz")
        {
            setCpuFrequencyMhz(160);

            SlectedString = &menuItems;
        }
        else if ((*SlectedString)[MenuIdFocus] == "80 MHz")
        {
            setCpuFrequencyMhz(80);

            SlectedString = &menuItems;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Menu Items")
        {
            mainOS->ChangeMenu(new MenuItemManeger(mainOS));
            return;
        }

        else if ((*SlectedString)[MenuIdFocus] == "Reset Password Vault")
        {
            mainOS->ShowOnScreenMessege("To Reset Vault Delete the file \"AdvanceOS/Vault/data.bin\" IMPORTENT!! it will also delete the password that you Store in the vault", 4000);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Show Wallpaper In Main Menu")
        {
            mainOS->ShowWallpaperInMainMenu = !mainOS->ShowWallpaperInMainMenu;
            if (mainOS->ShowWallpaperInMainMenu)
            {
                mainOS->ShowOnScreenMessege("Show wallpaper", 1000);
            }
            else
            {
                mainOS->ShowOnScreenMessege("wallpaper Disable", 1000);
            }
            mainOS->saveSettings();
        }

        else if ((*SlectedString)[MenuIdFocus] == "Reset To Default Theme")
        {
            mainOS->ResetToDefaultTheme();
        }
        else if ((*SlectedString)[MenuIdFocus] == "Turn Off Timer (Deep Sleep)")
        {

            SlectedString = &TrunOffTimer;
        }
        else if ((*SlectedString)[MenuIdFocus] == "10min")
        {
            mainOS->TimeToTurnOff = 600;
            mainOS->saveSettings();

            SlectedString = &menuItems;
        }
        else if ((*SlectedString)[MenuIdFocus] == "30min")
        {
            mainOS->TimeToTurnOff = 1800;
            mainOS->saveSettings();
            SlectedString = &menuItems;
        }
        else if ((*SlectedString)[MenuIdFocus] == "1hr")
        {
            mainOS->TimeToTurnOff = 3600;
                                  //  mainOS->TimeToTurnOff = 10;//test 10 second

            mainOS->saveSettings();
            SlectedString = &menuItems;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Never Turn Off")
        {
            mainOS->TimeToTurnOff = -1;
                     //   mainOS->TimeToTurnOff = 20;//test 20 second

            mainOS->saveSettings();
            SlectedString = &menuItems;
        }
        else if ((*SlectedString)[MenuIdFocus] == "Credit")
        {
            M5Cardputer.Display.setSwapBytes(true);
            M5Cardputer.Display.pushImage(0, 0, 240, 135, AdvanceOS_LOGO);

            M5Cardputer.Display.setTextColor(BLACK);

            M5Cardputer.Display.setCursor(2, 90);
            M5Cardputer.Display.setTextSize(1);
            M5Cardputer.Display.setTextColor(WHITE);
            M5Cardputer.Display.print("By Bomberman30");
            M5Cardputer.Display.setCursor(2, 110);
            M5Cardputer.Display.print("github.com/bomberman30");
            M5Cardputer.Display.setSwapBytes(false);
            M5Cardputer.Display.setTextColor(BLACK);

            M5Cardputer.Display.setSwapBytes(false);
            delay(1000);
            bool Presed = false;
            while (!Presed)
            {
                M5Cardputer.update();
                if (M5Cardputer.Keyboard.isChange())
                {
                    delay(1000);
                    Presed = true;
                }
            }
        }

        MenuIdFocus = 0;
        targetCameraY = MenuIdFocus * DistanceBetweenTXT;
    }
    // }

    float finalCameraYPos = MenuIdFocus * DistanceBetweenTXT;
    cameraY = cameraY + (targetCameraY - cameraY) * 0.12f;
    if (!mainOS->screenOff)

    {
        Draw();
    }
}

void Setting::Draw()
{
    int y = 10 - cameraY;

    sprite.createSprite(240, 135 - TopOffset);
    sprite.fillScreen(TFT_BLACK);
    sprite.setTextWrap(false);

    for (int i = 0; i < SlectedString->size(); i++)
    {
        bool selected = (MenuIdFocus == i);

        int itemY = y + i * DistanceBetweenTXT;

        if (selected)
        {
            // Beckground
            sprite.fillRoundRect(10, itemY - 2, 220, DistanceBetweenTXT, 6, DARKGREY);

            // arrow
            sprite.setTextColor(TFT_WHITE);
            sprite.setCursor(14, itemY);
            sprite.print(">");
        }

        sprite.setTextColor(TFT_BLACK);
        sprite.setCursor(30 + 1, itemY + 1);
        sprite.print((*SlectedString)[i]);

        sprite.setTextColor(selected ? TFT_WHITE : TFT_YELLOW);
        sprite.setCursor(30, itemY);
        sprite.print((*SlectedString)[i]);
    }
    sprite.drawRoundRect(2, 2, 236, 131 - TopOffset, 6, TFT_DARKGREY);
    sprite.pushSprite(0, 0);
    sprite.deleteSprite();
}