#include "Theme_Maneger.h"
#include "MyOS.h"
#include "FileBrowser.h"
#include "./Classes/MainMenuV2.h"
void Theme_Maneger::Begin()
{
    SlectedString = &menuItems;

    if (mainOS->inEditThemeFromFileExplorer /* ||mainOS->CurrentThemeSelectedPath != "" */) // them file selected from file explorer
    {
        mainOS->inEditThemeFromFileExplorer = false;
        FileSelected = mainOS->FileSelectedInFS;
        menuItems.push_back("Update Current Theme File");
    }
    else if (mainOS->CurrentThemeSelectedPath != "") // if not selected but theme file was apply in startup
    {
        FileSelected = mainOS->CurrentThemeSelectedPath;
        menuItems.push_back("Update Current Theme File");
    }

    menuItems.push_back("Open Theme Folder");
    menuItems.push_back("Reset To Default");
    menuItems.push_back("Save Current Theme As New File");
}

void Theme_Maneger::Loop()
{

    if (mainOS->screenOff)
    {
        return;
    }
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
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
        if ((*SlectedString)[MenuIdFocus] == "Save Current Theme As New File")
        {
            delay(500);
            if (mainOS->SaveCurrentTheme())
            {
                mainOS->ShowOnScreenMessege("theme saved in AdvanceOS/Theme folder");
            }
            else
            {
                mainOS->ShowOnScreenMessege("save failed");
            }
        }
        else if ((*SlectedString)[MenuIdFocus] == "Set Bar Color 1")
        {
            delay(500);

            mainOS->BAR_COLOR_1 = mainOS->AskForColor("Choose color 1 for top bar", mainOS->BAR_COLOR_1);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Set Bar Color 2")
        {
            delay(500);

            mainOS->BAR_COLOR_2 = mainOS->AskForColor("Choose color 2 for top bar", mainOS->BAR_COLOR_2);
        }

        else if ((*SlectedString)[MenuIdFocus] == "Set Bar Text Color")
        {
            delay(500);

            mainOS->BAR_TEXT_COLOR = mainOS->AskForColor("Choose color  bar text", mainOS->BAR_TEXT_COLOR);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Set Background Color")
        {
            delay(500);

            mainOS->BACKGROUND_COLOR = mainOS->AskForColor("Choose color  bar text", mainOS->BACKGROUND_COLOR);
        }
        else if ((*SlectedString)[MenuIdFocus] == "Show Wallpaper?")
        {
            delay(500);

            mainOS->ShowWallpaperInMainMenu = !mainOS->ShowWallpaperInMainMenu;
            if (mainOS->ShowWallpaperInMainMenu)
            {
                mainOS->ShowOnScreenMessege("Show wallpaper", 1000);
            }
            else
            {
                mainOS->ShowOnScreenMessege("wallpaper Disable", 1000);
            }
        }

        else if ((*SlectedString)[MenuIdFocus] == "Open Theme Folder")
        {
            delay(500);
            mainOS->currentPath = "/AdvanceOS/Theme";
            mainOS->ChangeMenu(new FileBrowser(mainOS));
        }
        else if ((*SlectedString)[MenuIdFocus] == "Reset To Default")
        {
            delay(500);
            mainOS->ResetToDefaultTheme();
        }

        else if ((*SlectedString)[MenuIdFocus] == "Update Current Theme File")
        {
            delay(500);
            mainOS->SaveCurrentTheme(FileSelected);
            mainOS->ChangeMenu(new MainMenuV2(mainOS));
        }
    }
    // }

    float finalCameraYPos = MenuIdFocus * DistanceBetweenTXT;
    cameraY = cameraY + (targetCameraY - cameraY) * 0.12f;
    if (!mainOS->screenOff && !AddWillBeDeleted)
    {
        Draw();
    }
}

void Theme_Maneger::Draw()
{
    const int HEADER_H = 18;
    const int MENU_X = 14;

    int y = HEADER_H + 6 - cameraY;

    mainOS->sprite.createSprite(240, 135 - TopOffset);
    mainOS->sprite.fillScreen(BLACK);
    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextWrap(false);

    /* ---------- MENU ITEMS ---------- */
    for (int i = 0; i < SlectedString->size(); i++)
    {
        if (MenuIdFocus == i)
        {
            mainOS->sprite.fillRect(0, y - 2, 240, DistanceBetweenTXT, TFT_DARKGREEN);
            mainOS->sprite.setTextColor(WHITE);

            mainOS->sprite.setCursor(4, y);
            mainOS->sprite.print(">");
        }
        else
        {
            mainOS->sprite.setTextColor(YELLOW);
        }

        mainOS->sprite.setCursor(MENU_X, y);
        mainOS->sprite.print((*SlectedString)[i]);

        y += DistanceBetweenTXT;
    }
    /* ---------- HEADER ---------- */
    mainOS->sprite.fillRect(0, 0, 240, HEADER_H, DARKGREY);
    mainOS->sprite.setTextColor(WHITE);
    mainOS->sprite.setCursor(6, 5);
    String headerText = "Theme MENU";
    
    if (!FileSelected.isEmpty())
    {
        headerText += "  Current Theme  ";
        headerText += mainOS->getFileNameFromPath(FileSelected);
    }
    mainOS->sprite.print(headerText);

    // seperate line
    mainOS->sprite.drawFastHLine(0, HEADER_H, 240, DARKGREY);
    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}
