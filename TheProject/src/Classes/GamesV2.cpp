#include "GamesV2.h"
#include "MyOS.h"
#include "MainMenuV2.h"
#include <Update.h>
#include "esp_ota_ops.h"
#include <Preferences.h>
#include "partition_maneger.h"
#include "Function.h"
void GamesV2::Begin()
{

    if (mainOS->isInBruceLauncherNormalLayout())
    {

        if (mainOS->AskSomthing("We detect that you install AdvanceOS\n on Launcher. to make the games\n to work we need to change\n partition Layout.\n Change Now???\n (only Press Y if you are install Via Launcher)"))
        {
            mainOS->ChangePartitionLayoutToAdvanceOSinLauncher = true;
            mainOS->ChangeMenu(new partition_maneger(mainOS));
        }
        else

        {
            mainOS->ShowOnScreenMessege("can't play the games return to main menu", 2000);
            mainOS->ChangeMenu(new MainMenuV2(mainOS));
        }
    }
    SD.mkdir("/AdvanceOS/GamesSave"); // folder for the theme
    SD.mkdir("/AdvanceOS/Screenshots"); // folder for the theme

    if (mainOS->FastLoadGameFromFileExplorer)
    {
        mainOS->FastLoadGameFromFileExplorer = false;
        GameSelectedFromMenu(true);
    }
    //  GetGamesList();
    GetGameListFromSD();
    showTopBar = false;
}

void GamesV2::Loop()
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

    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('.', 700, 50))
    {
        ItemSelectID += 1;
        if (ItemSelectID > GamePathList.size() - 1)
        {
            ItemSelectID = 0;
            // ItemSelectID = GamePathList.size() - 1;
        }
        refrashGamePic = true;
    }

    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(';', 700, 50))
    {
        ItemSelectID -= 1;
        if (ItemSelectID < 0)
        {
            // ItemSelectID = 0;
            ItemSelectID = GamePathList.size() - 1;
        }
        refrashGamePic = true;
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        GameSelectedFromMenu();
    }
    Draw();
}

void GamesV2::Draw()
{
    mainOS->sprite.createSprite(SCREEN_W - 135, SCREEN_H);
    mainOS->sprite.fillSprite(BLACK);
    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextWrap(false);

    mainOS->sprite.drawFastHLine(0, 20, SCREEN_W - 135, DARKGREY);

    /* ===== LIST ===== */
    mainOS->sprite.setTextSize(1);

    int visibleLines = (SCREEN_H - 48) / DestanceBetweenLines;
    int maxCameraY = max(0, (int)GamePathList.size() - visibleLines) * DestanceBetweenLines;

    if (ItemSelectID * DestanceBetweenLines < -CameraY)
    {
        CameraY = -ItemSelectID * DestanceBetweenLines;
    }
    else if (ItemSelectID * DestanceBetweenLines > -CameraY + (visibleLines - 1) * DestanceBetweenLines)
    {
        CameraY = -(ItemSelectID - visibleLines + 1) * DestanceBetweenLines;
    }

    if (CameraY > 0)
        CameraY = 0;
    if (CameraY < -maxCameraY)
        CameraY = -maxCameraY;

    int y = 48 + CameraY;
    if (GamePathList.size() == 0)
    {
        mainOS->sprite.fillSprite(BLACK);
        mainOS->sprite.setCursor(40, 50);
        mainOS->sprite.print("You Need To Put Games\n In \"Games\" Folder");
    }
    for (size_t i = 0; i < GamePathList.size(); i++)
    {
        bool selected = (ItemSelectID == i);

        if (selected)
        {
            mainOS->sprite.fillRoundRect(4, y - 2, SCREEN_W - 135 - 8, 14, 3, DARKGREY);
            mainOS->sprite.setTextColor(WHITE);
            mainOS->sprite.setCursor(8, y);
            mainOS->sprite.print("> ");
        }
        else
        {
            mainOS->sprite.setTextColor(YELLOW);
            mainOS->sprite.setCursor(14, y);
        }

        mainOS->sprite.print(mainOS->getFileNameFromPath(GamePathList[i]));
        y += DestanceBetweenLines;
    }
    /* ===== HEADER ===== */
    mainOS->sprite.fillRect(0, 0, 240, 20, BLACK);
    mainOS->sprite.setTextSize(2);
    mainOS->sprite.setTextColor(CYAN);
    mainOS->sprite.setCursor(10, 4);
    mainOS->sprite.print("GAMES");

    /*  mainOS->sprite.setCursor(10, 22);
     mainOS->sprite.print("GAME LIST"); */

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
    if (refrashGamePic)
    {
        refrashGamePic = false;
        GetGamePic();
    }
}

void GamesV2::GetGamePic()
{
    if (GamePathList.size() == 0)
    {
        M5Cardputer.Display.setCursor(135, 70);
        M5Cardputer.Display.unloadFont();
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(WHITE);
        M5Cardputer.Display.print("No Games Found");
        return;
    }
    M5Cardputer.Display.fillRect(SCREEN_W - 135, 0, 135, 135, BLACK);
    GameHavePic = false;
    String GamePath = GamePathList[ItemSelectID];

    int lastDotIndex = GamePath.lastIndexOf('.');

    if (lastDotIndex != -1)
    {
        String PicPath = GamePath.substring(0, lastDotIndex) + ".png";
        if (!SD.exists(PicPath.c_str()))
        {
            M5Cardputer.Display.setCursor(135, 70);
            M5Cardputer.Display.unloadFont();
            M5Cardputer.Display.setTextSize(1);
            M5Cardputer.Display.setTextColor(WHITE);
            M5Cardputer.Display.print("No Game Picture");
        }
        else
        {
            GameHavePic = true;
            GamePicPath = PicPath;

            IMG_x_POS = SCREEN_W - 135,
            IMG_y_POS = 0;
            DrawPNG(GamePicPath.c_str());
        }
    }
}

void GamesV2::GameSelectedFromMenu(bool FromFileExplorer)
{
    String GameToLoadPath;
    if (FromFileExplorer)
    {
        GameToLoadPath = mainOS->FileSelectedInFS;
    }
    else // select from game menu
    {
        GameToLoadPath = GamePathList[ItemSelectID];
    }

    String Ext = mainOS->GetExtensionLower(GameToLoadPath.c_str());
    // mainOS->ShowOnScreenMessege(String("Exte:  "+Ext));
    if (mainOS->isInBruceLauncherNormalLayout())
    {
        // need to fix protations
        mainOS->ShowOnScreenMessege("AdvanceOS is Installed Via Launcher , You Need To Change partition Layout!!", 2000);
        return;
    }
    /*
        if (mainOS->LastAppInstalledPath == GameToLoadPath || GameToLoadPath == "Play Last Game You Played") // if this app alredy install and it's on the app1 partition
        {
            mainOS->ShowOnScreenMessege(String("quick Loading Game... " + GameToLoadPath + " last app installed: " + mainOS->LastAppInstalledPath), 1000);

            SetAppStats();
            // mainOS->ShowOnScreenMessege("Alredy installed", 1000);
            const esp_partition_t *running = esp_ota_get_running_partition();

            const esp_partition_t *prev_partition = esp_ota_get_next_update_partition(running);
            esp_ota_set_boot_partition(prev_partition);
            // esp_ota_check_rollback_is_possible();
            // esp_ota_mark_app_invalid_rollback_and_reboot();
            if (FromFileExplorer)
            {
                mainOS->SetFastBootVar(4);
            }
            else
            {
                mainOS->SetFastBootVar(7);
            }
            ESP.restart();
        }
        else // if not same bin installed in app1 or 2
        { */
    if (Ext == "ard") // if bin is arduboy file
    {
        File bin = SD.open(GameToLoadPath);
        if (!bin)
            return;

        size_t updateSize = bin.size();
        const esp_partition_t *Current = esp_ota_get_running_partition();

        const esp_partition_t *update_partition = esp_ota_get_next_update_partition(Current);
        esp_ota_set_boot_partition(update_partition);

        mainOS->ShowOnScreenMessege(String("Loading Arduboy Game... " + GameToLoadPath), 10);

        if (!Update.begin(updateSize, U_FLASH, update_partition->address))
        {
            bin.close();
            return;
        }

        Update.writeStream(bin);
        bin.close();

        if (!Update.end(false))
        {
            return;
        }

        esp_err_t err = esp_ota_set_boot_partition(update_partition);

        if (err != ESP_OK)
        {
            return;
        }

        mainOS->LastAppInstalledPath = GameToLoadPath;
        // mainOS->saveSettings();
        SetAppStats();
        if (mainOS->FileOpenFromMainMenu)
        {
            mainOS->SetFastBootVar(1);
        }
        else
        {
            if (FromFileExplorer)
            {
                mainOS->SetFastBootVar(4);
            }
            else
            {
                mainOS->SetFastBootVar(7);
            }
        }

        ESP.restart();
    }
    else if (Ext == "nes" || Ext == "gb" || Ext == "gbc")
    {

        SetRomToLoadPath(GameToLoadPath); // save to prefrance to survive restart
                                          // GameSelectedFromMenu();

        if (mainOS->FilePathHasExtension(mainOS->LastAppInstalledPath.c_str(), "nes") || mainOS->FilePathHasExtension(mainOS->LastAppInstalledPath.c_str(), "gb") || mainOS->FilePathHasExtension(mainOS->LastAppInstalledPath.c_str(), "gbc")) // that mean other nes file was loaded so keep the emulator frameware
        {
            // mainOS->ShowOnScreenMessege("Alredy installed", 1000);
            const esp_partition_t *running = esp_ota_get_running_partition();
            //  mainOS->ShowOnScreenMessege(String("quick Loading Rom... " + GameToLoadPath), 1000);

            const esp_partition_t *prev_partition = esp_ota_get_next_update_partition(running);
            esp_ota_set_boot_partition(prev_partition);
            // esp_ota_check_rollback_is_possible();
            // esp_ota_mark_app_invalid_rollback_and_reboot();
            mainOS->LastAppInstalledPath = GameToLoadPath;

            SetAppStats();
            if (mainOS->FileOpenFromMainMenu)
            {
                mainOS->SetFastBootVar(1);
            }
            else
            {
                if (FromFileExplorer)
                {
                    mainOS->SetFastBootVar(4);
                }
                else
                {
                    mainOS->SetFastBootVar(7);
                }
            }

            ESP.restart();
        }
        else // if emulator not installed
        {
            File bin = SD.open("/AdvanceOS/EmulatorV3.4.extension");
            if (!bin)
            {
                mainOS->ShowOnScreenMessege("You Don't have Emulator Extension For AdvanceOS Please Download ExtensionV3.4 From The Github Page", 7000);
                return;
            }
            size_t updateSize = bin.size();
            const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
            mainOS->ShowOnScreenMessege(String("Loading Rom... " + GameToLoadPath), 10);

            // mainOS->ShowOnScreenMessege("Loading Emulator Extension...", 10);

            if (!Update.begin(updateSize, U_FLASH, update_partition->address))
            {
                bin.close();
                return;
            }

            Update.writeStream(bin);
            bin.close();

            if (!Update.end(false))
            {
                return;
            }

            esp_err_t err = esp_ota_set_boot_partition(update_partition);

            if (err != ESP_OK)
            {
                return;
            }
            // mainOS->LastAppInstalledPath = GameToLoadPath;
            if (mainOS->FileOpenFromMainMenu)
            {
                mainOS->SetFastBootVar(1);
            }
            else
            {
                if (FromFileExplorer)
                {
                    mainOS->SetFastBootVar(4);
                }
                else
                {
                    mainOS->SetFastBootVar(7);
                }
            }
            //  mainOS->saveSettings();
            mainOS->LastAppInstalledPath = GameToLoadPath;

            SetAppStats();

            ESP.restart();
        }
    }
}

void GamesV2::SetRomToLoadPath(String RomPath)
{

    prefsRomToLoad.begin("app", false);
    prefsRomToLoad.putString("RomToLoad", RomPath);
    prefsRomToLoad.end();
}

void GamesV2::GetGameListFromSD()
{
    SD.mkdir("/Games");

    File root = SD.open("/Games");

    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }

    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        root.close();
        return;
    }

    File file = root.openNextFile();
    int fileCount = 0;

    while (file && fileCount < 100)
    {
        if (!file.isDirectory())
        {
            const char *name = file.name();
            // String EXT = mainOS->GetExtensionLower(name);
            if (mainOS->FilePathHasExtension(name, ".nes") || mainOS->FilePathHasExtension(name, ".ard") || mainOS->FilePathHasExtension(name, ".gb") || mainOS->FilePathHasExtension(name, ".gbc"))

            //   if (EXT == ".ard" || EXT == ".gb" || EXT == ".gbc")
            {
                String fullPath = "/Games/";
                fullPath += name;
                GamePathList.push_back(fullPath);
                fileCount++;
            }
        }

        file.close();
        file = root.openNextFile();
    }

    root.close();
}
void GamesV2::SetAppStats()
{

    prefsRomToLoad.begin("app", false);
    prefsRomToLoad.putString("AppLoadedPath", mainOS->LastAppInstalledPath);
    prefsRomToLoad.putBool("TryToRunApp", true);
    prefsRomToLoad.putBool("RunAppSucsses", false);

    /*     bool TryToRunApp=false;
    bool RunAppSucsses=false; */
    prefsRomToLoad.end();
}
