#include "./EspProjectRemoteControl.h"

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "../../MyOS.h"
#include "../MainMenuV2.h"
#include "EspProjectRemoteControl.h"
#include <ArduinoJson.h>
#include "../FileBrowser.h"

#define ESPNOW_WIFI_CHANNEL
// uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static const uint8_t BROADCAST_ADDR[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// String StringIfSucces;

void OnSent(const uint8_t *mac, esp_now_send_status_t status) // cheak of messege send
{
    String msg;
    msg += "Send Status: ";

    msg += status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL";
    Serial.print("Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
    /*     M5Cardputer.Display.fillRect(0, 0, 240, 135, DARKGREY);
        M5Cardputer.Display.setCursor(4, 4);
        M5Cardputer.Display.unloadFont();
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(WHITE);

        M5Cardputer.Display.println(msg); */
    //  StringIfSucces = msg;
}

void EspProjectRemoteControl::Begin()
{
    SD.mkdir("/AdvanceOS");
    SD.mkdir("/AdvanceOS/EspProject_Remote_Control_Commands");
    // CreateFileMenuItems={"File Name","DeviceID","Add Command"};

    WiFi.mode(WIFI_STA);

    WiFi.disconnect();

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW init failed!");
        return;
    }

    esp_now_register_send_cb(OnSent);

    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, BROADCAST_ADDR, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    // String msg = "Hello From Sender!";
    // esp_now_send(BROADCAST_ADDR, (uint8_t *)msg.c_str(), msg.length());

    /*
       send_message("TurnOnLED");
   delay(1000);
           send_message("ResetSystem");
   delay(1000);

               send_message("PlaySound"); */
}

void EspProjectRemoteControl::Loop()
{

    // GlobalParentClass::Loop();
    if (mainOS->NewKey.ifKeyJustPress('`')) // esc
    {

        if (showCreateFileMenu)
        {
            showCreateFileMenu = false;
        }
        else
        {
             mainOS->ChangeMenu(new MainMenuV2(mainOS));

       /*      mainOS->SetFastBootVar(1);
            ESP.restart(); */
        }
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        if (!showCreateFileMenu) // main screen menu
        {
            if (MenuItem[inMenu] == "create Command File")
            {
                // M5Cardputer.Display.fillScreen(RED);
                showCreateFileMenu = true;
                isFileValid = FileValid();
            }
            else if (MenuItem[inMenu] == "Open RC files Directory")

            {
                mainOS->currentPath = "/AdvanceOS/EspProject_Remote_Control_Commands";
                mainOS->ChangeMenu(new FileBrowser(mainOS));
            }
            else if (MenuItem[inMenu] == "Send Current Command")
            {
                if (CurrentStringToSend.isEmpty())
                {
                    mainOS->ShowOnScreenMessege("You Can't Send Empty Command", 2000);
                    return;
                }
                else
                {
                    send_message(String("all:" + CurrentStringToSend));
                    mainOS->ShowOnScreenMessege("Messege: \"" + CurrentStringToSend + "\" Send!", 1000);
                    CurrentStringToSend = "";
                }
            }
        }
        else if (showCreateFileMenu) // create file menu
        {
            if (CreateFileMenuItems[inMenuCreateFile] == "Add Command")

            {
                CommandsInFile.push_back("");
            }

            else if (CreateFileMenuItems[inMenuCreateFile] == "Save File")
            {
                //  M5Cardputer.Display.fillScreen(RED);
                showCreateFileMenu = true;
                if (isFileValid)
                {

                    SaveCurrentFile();
                    isFileValid = FileValid();
                }
                else
                {
                    mainOS->ShowOnScreenMessege("File Name Not Valid", 1000);
                }
            }
        }
    }
    if (!showCreateFileMenu) // main screen menu loop
    {

        if (mainOS->NewKey.ifKeyJustPress(';')) // up
        {
            inMenu--;
            if (inMenu <= 0)
            {
                inMenu = 0;
            }
        }
        if (mainOS->NewKey.ifKeyJustPress('.'))
        {
            inMenu++;
            if (inMenu >= MenuItem.size())
            {
                inMenu = MenuItem.size() - 1;
            }
        }
        if (M5Cardputer.Keyboard.isChange())
        {
            auto &kb = M5Cardputer.Keyboard;
            if (kb.isPressed())
            {
                Keyboard_Class::KeysState status = kb.keysState();
                for (char c : status.word)
                {
                    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) // only nombers and letters
                    {
                        CurrentStringToSend += c;
                    }
                }

                if (status.del)
                {
                    if (CurrentStringToSend.length() > 0)
                        CurrentStringToSend.remove(CurrentStringToSend.length() - 1);
                }
            }
        }
    }
    else if (showCreateFileMenu) // if in create file menu loop
    {
        // typing in var
        if (M5Cardputer.Keyboard.isChange())

        {
            auto &kb = M5Cardputer.Keyboard;

            if (kb.isPressed())
            {
                Keyboard_Class::KeysState status = kb.keysState();

                for (char c : status.word)
                {
                    if ((c >= 'A' && c <= 'Z') ||
                        (c >= 'a' && c <= 'z') ||
                        (c >= '0' && c <= '9') ||
                        (c == ' '))
                    {
                        switch (inMenuCreateFile)
                        {
                        case 0: // change file name
                            fileNameToCreate += c;
                            isFileValid = FileValid();
                            break;
                        case 1: // change device id
                            DeviceID += c;
                            break;
                        default:
                            break;
                        }
                        if (inMenuCreateFile > CreateFileMenuItems.size() - 1) // if equal to 3 to cammands
                        {
                            CommandsInFile[inMenuCreateFile - CreateFileMenuItems.size()] += c;
                        }
                    }
                    // password += c;
                }

                if (status.del)
                {
                    switch (inMenuCreateFile)
                    {
                    case 0: // change file name
                        if (fileNameToCreate.length() > 0)
                            fileNameToCreate.remove(fileNameToCreate.length() - 1);
                        isFileValid = FileValid();

                        break;
                    case 1: // change device id
                        if (DeviceID.length() > 0)
                            DeviceID.remove(DeviceID.length() - 1);
                        break;
                    default:
                        break;
                    }
                    if (inMenuCreateFile > CreateFileMenuItems.size() - 1)
                    {
                        if (CommandsInFile[inMenuCreateFile - CreateFileMenuItems.size()].length() > 0)
                            CommandsInFile[inMenuCreateFile - CreateFileMenuItems.size()].remove(CommandsInFile[inMenuCreateFile - CreateFileMenuItems.size()].length() - 1);
                    }
                }
            }
        }

        // arrows keys
        int totalItems = CreateFileMenuItems.size() + CommandsInFile.size();

        if (mainOS->NewKey.ifKeyJustPress(';')) // up
        {
            inMenuCreateFile--;
        }
        if (mainOS->NewKey.ifKeyJustPress('.')) // down
        {
            inMenuCreateFile++;
        }

        if (inMenuCreateFile < 0)
            inMenuCreateFile = totalItems - 1;

        if (inMenuCreateFile >= totalItems)
            inMenuCreateFile = 0;
    }
    CameraY = inMenuCreateFile * distancebetweenMenuLines;
    Draw();
}

#define BG_COLOR TFT_BLACK
#define HEADER_BG TFT_DARKGREY
#define SELECT_BG TFT_BLUE
#define TEXT_COLOR TFT_WHITE
#define SUB_TEXT TFT_LIGHTGREY
void EspProjectRemoteControl::Draw()
{
    mainOS->sprite.createSprite(240, 135 - TopOffset);
    mainOS->sprite.fillSprite(BG_COLOR);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextWrap(false);
    if (!showCreateFileMenu)
    {

        /* ---------- HEADER ---------- */
        mainOS->sprite.fillRect(0, 0, 240, 16, HEADER_BG);
        mainOS->sprite.setTextColor(TEXT_COLOR);
        mainOS->sprite.setCursor(6, 4);
        mainOS->sprite.print("Remote Control");

        /* ---------- STATUS ---------- */
        mainOS->sprite.setTextColor(SUB_TEXT);
        mainOS->sprite.setCursor(6, 20);
        mainOS->sprite.print("Cmd: ");
        mainOS->sprite.print(CurrentStringToSend);

        /* ---------- MENU ---------- */
        int startY = 40;
        int lineH = 14;

        for (int i = 0; i < MenuItem.size(); i++)
        {
            int y = startY + i * lineH;

            if (inMenu == i)
            {
                mainOS->sprite.fillRect(4, y - 2, 232, lineH, SELECT_BG);
                mainOS->sprite.setTextColor(TEXT_COLOR);
            }
            else
            {
                mainOS->sprite.setTextColor(SUB_TEXT);
            }

            mainOS->sprite.setCursor(10, y);
            mainOS->sprite.print(MenuItem[i]);
        }

        /* ---------- FOOTER ---------- */
        mainOS->sprite.drawFastHLine(0, 132 - TopOffset, 240, HEADER_BG);
        mainOS->sprite.setCursor(6, 122 - TopOffset);
        mainOS->sprite.setTextColor(SUB_TEXT);
        mainOS->sprite.print("OK=Select  Back=Exit");
    }
    else if (showCreateFileMenu)
    {
        mainOS->sprite.fillSprite(BG_COLOR);

        /* ---------- STATUS ---------- */
        mainOS->sprite.setCursor(6, 20 - CameraY);
        if (isFileValid)
        {
            mainOS->sprite.setTextColor(TFT_GREEN);
            mainOS->sprite.print("File name available");
        }
        else
        {
            mainOS->sprite.setTextColor(TFT_RED);
            mainOS->sprite.print("File already exists");
        }

        /* ---------- MENU ---------- */
        int startY = 40 - CameraY;
        int lineH = 14;

        int totalItems = CreateFileMenuItems.size() + CommandsInFile.size();

        for (int i = 0; i < totalItems; i++)
        {
            int y = startY + i * lineH;

            // selection bar
            if (inMenuCreateFile == i)
            {
                mainOS->sprite.fillRect(4, y - 2, 232, lineH, SELECT_BG);
                mainOS->sprite.setTextColor(TEXT_COLOR);
            }
            else
            {
                mainOS->sprite.setTextColor(SUB_TEXT);
            }

            mainOS->sprite.setCursor(10, y);

            /* ---------- STATIC MENU ITEMS ---------- */
            if (i < CreateFileMenuItems.size())
            {
                mainOS->sprite.print(CreateFileMenuItems[i]);

                // input fields
                int valueX = 120;
                if (i == 1 || i == 0) // only draw round white rect for file name and devide id
                {
                    mainOS->sprite.fillRect(valueX - 2, y - 1, 115, 10, TFT_WHITE);
                }
                mainOS->sprite.setTextColor(TFT_BLACK);
                mainOS->sprite.setCursor(valueX, y);

                if (i == 0) // File name
                    mainOS->sprite.print(fileNameToCreate);
                else if (i == 1) // Device ID
                    mainOS->sprite.print(DeviceID);
            }
            /* ---------- COMMAND LIST ---------- */
            else
            {
                int j = i - CreateFileMenuItems.size();
                mainOS->sprite.print(">");
                mainOS->sprite.print(CommandsInFile[j]);
            }
        }
        /* ---------- HEADER ---------- */
        mainOS->sprite.fillRect(0, 0, 240, 16, HEADER_BG);
        mainOS->sprite.setTextColor(TEXT_COLOR);
        mainOS->sprite.setCursor(6, 4);
        mainOS->sprite.print("Create Command File");
    }
    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

void EspProjectRemoteControl::OnExit()
{
    WiFi.mode(WIFI_OFF);
    mainOS->WifiConnected=false;
}


bool EspProjectRemoteControl::FileValid()
{
    bool validName;
    String Name;
    Name = "/AdvanceOS/EspProject_Remote_Control_Commands/";
    Name += fileNameToCreate;
    Name += ".esprc";

    if (!SD.exists(Name))
    {
        validName = true;
    }
    else
    {
        validName = false;
    }
    return validName;
}
void EspProjectRemoteControl::SaveCurrentFile()
{
    bool AnyCommadEmpty = false;
    if (CommandsInFile.size() == 0)
    {
        mainOS->ShowOnScreenMessege("You not Create Any Commands", 1000);
        return;
    }
    else // if have enter commands to array cheack if all have text inside
    {
        for (size_t i = 0; i < CommandsInFile.size(); i++)
        {
            if (CommandsInFile[i].isEmpty())
            {
                AnyCommadEmpty = true;
            }
        }
    }
    if (AnyCommadEmpty)
    {
        mainOS->ShowOnScreenMessege("You Can't Have Empty Commands", 1000);
        return;
    }

    isFileValid = FileValid();
    CommandsInFile.clear();
    fileNameToCreate = "espProjectCommand";
    JsonDocument doc;
    doc["DeviceID"] = DeviceID;
    JsonArray CommandArray = doc["Commands"].to<JsonArray>();

    // JsonArray CommandArray = doc.createNestedArray("Commands");
    for (size_t i = 0; i < CommandsInFile.size(); i++)
    {
        CommandArray.add(CommandsInFile[i]);
    }

    String Name;
    Name = "/AdvanceOS/EspProject_Remote_Control_Commands/";
    Name += fileNameToCreate;
    Name += ".esprc";

    File file = SD.open(Name, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file");
        return;
    }

    serializeJson(doc, file);
    file.close();
    mainOS->ShowOnScreenMessege("File Saved In: " + Name, 2000);
    showCreateFileMenu = false;
}

void EspProjectRemoteControl::send_message(const String &msg)
{
    esp_now_send(BROADCAST_ADDR, (uint8_t *)msg.c_str(), msg.length());
}
