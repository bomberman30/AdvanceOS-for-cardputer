#include "ESP_RemoteControlFile.h"
#include <ArduinoJson.h>
#include "../../MyOS.h"
#include "../FileBrowser.h"
#include <WiFi.h>
#include <esp_now.h>
#include "../../Function.h"
static const uint8_t BROADCAST_ADDR[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void OnSentFile(const uint8_t *mac, esp_now_send_status_t status) // cheak of messege send
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

void ESP_RemoteControlFile::Begin()
{
    InCorrentCommand = 0;
    WiFi.mode(WIFI_STA);

    WiFi.disconnect();

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW init failed!");
        return;
    }

    esp_now_register_send_cb(OnSentFile);

    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, BROADCAST_ADDR, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    File file = SD.open(mainOS->FileSelectedInFS, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open settings file");
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err)
    {
        Serial.print("JSON error: ");
        Serial.println(err.c_str());
        return;
    }

    CommandFromFile.clear();

    JsonArray arr = doc["Commands"].as<JsonArray>();
    if (arr.isNull())
    {
        Serial.println("Commands array not found");
        return;
    }

    for (JsonVariant v : arr)
    {
        CommandFromFile.push_back(v.as<String>());
    }
    DeviceID = doc["DeviceID"] | "all";
}

void ESP_RemoteControlFile::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`')) // esc
    {

        mainOS->ChangeMenu(new FileBrowser(mainOS));
    }
    if (mainOS->NewKey.ifKeyJustPress(';')) // up
    {
        InCorrentCommand--;
    }
    if (mainOS->NewKey.ifKeyJustPress('.')) // down
    {
        InCorrentCommand++;
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER)) // down
    {
        String msg = DeviceID + ":" + CommandFromFile[InCorrentCommand];
        send_message(msg.c_str());
    }
    if (InCorrentCommand < 0)
        InCorrentCommand = CommandFromFile.size() - 1;

    if (InCorrentCommand >= CommandFromFile.size())
        InCorrentCommand = 0;
    CameraY = lerpFloat(CameraY, InCorrentCommand * distancebetweenMenuLines, 0.1);

    Draw();
}

void ESP_RemoteControlFile::Draw()
{

    mainOS->sprite.createSprite(240, 135 - TopOffset); 
    mainOS->sprite.fillSprite(ORANGE);

    mainOS->sprite.setTextWrap(false);

    mainOS->sprite.unloadFont();

    mainOS->sprite.setTextSize(2);
    int btnX = 5;
    int btnW = 230;
    int btnH = 20;

    for (int i = 0; i < CommandFromFile.size(); i++)
    {
        // int y = i*distancebetweenMenuLines+60-CameraY;// * (cardH + distancebetweenMenuLines);

        int y = i * distancebetweenMenuLines + 60 - CameraY;

        bool selected = (InCorrentCommand == i);

        uint16_t bgColor = selected ? BLUE : DARKGREY;
        uint16_t textColor = selected ? WHITE : BLACK;

        mainOS->sprite.fillRoundRect(btnX, y, btnW, btnH, 6, bgColor);
        mainOS->sprite.drawRoundRect(btnX, y, btnW, btnH, 6, WHITE);

        mainOS->sprite.setTextColor(textColor);
        // mainOS->sprite.setTextDatum(ML_DATUM);
        mainOS->sprite.drawString(
            CommandFromFile[i],
            btnX + 8,
            y + 3);

        y += btnH + 6; 
    }
   
    mainOS->sprite.setTextSize(1);

    mainOS->sprite.setTextColor(BLACK, MAROON);

    mainOS->sprite.setCursor(2, 2);
    mainOS->sprite.print("RemoteControl Device: " + DeviceID);
    // mainOS->sprite.setCursor(5, 10);
    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

void ESP_RemoteControlFile::OnExit()
{
            WiFi.mode(WIFI_OFF);
                mainOS->WifiConnected=false;


}

void ESP_RemoteControlFile::send_message(const String &msg)
{
    esp_now_send(BROADCAST_ADDR, (uint8_t *)msg.c_str(), msg.length());
    mainOS->ShowOnScreenMessege("Messege: \"" + msg + "\" Send!", 1000);
}
