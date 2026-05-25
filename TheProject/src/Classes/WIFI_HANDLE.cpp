#include "Wifi_handle.h"
#include "./MyOS.h"
#include "./Classes/MainMenuV2.h"
#include "WiFi.h"

void Wifi_handle::Begin()
{
    WiFi.mode(WIFI_STA);
    // scanning = true;

    // startScan();
}

void Wifi_handle::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('t')) //
    {
    WiFi.mode(WIFI_STA);

    }

        if (mainOS->NewKey.ifKeyJustPress('y')) //
    {
    WiFi.mode(WIFI_OFF);

    }
    if (mainOS->NewKey.ifKeyJustPress('`')) // esc
    {
        delay(400);
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
    }

    if (!mainOS->screenOff)
    {

        Draw();
    }
    auto &kb = M5Cardputer.Keyboard;

    // entering password
    if (enteringPassword)
    {
        if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
        {
            mainOS->ShowOnScreenMessege("Try To Connect", 10);

            enteringPassword = false;
            connecting = true;
            connectToSelected();
        }

        if (M5Cardputer.Keyboard.isChange())

        {
            if (kb.isPressed())
            {
                Keyboard_Class::KeysState status = kb.keysState();

                for (char c : status.word)
                {
                    if (c != 0 && c != '\0')
                        password += c;
                }

                
                if (status.del)
                {
                    if (password.length() > 0)
                        password.remove(password.length() - 1);
                }
            }
        }
    }

    if (scanning)
        return;

    if (mainOS->NewKey.ifKeyJustPress(';')) // up
    {
        selected--;
        if (selected < 0)
            selected = scanCount - 1;
    }

    if (mainOS->NewKey.ifKeyJustPress('.')) // down
    {
        selected++;
        if (selected >= scanCount)
            selected = 0;
    }

    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        if (!ScanAlredy && !mainOS->WifiConnected)
        {
            mainOS->ShowOnScreenMessege("SCAN WIFI", 10);

            scanning = true;
            startScan();
        }
        else if (mainOS->WifiConnected)

        {
            WiFi.disconnect(true); // force reset
            WiFi.mode(WIFI_OFF);   // close the wifi
            mainOS->WifiConnected = false;
        }
        else
        {
                        password = "";
            enteringPassword = true;
        }
    }
}

void Wifi_handle::Draw()
{
        int DistanceBetweenLines = 8;
        cameraY = selected * DistanceBetweenLines;
    // M5Cardputer.Display.clear();
    auto &d = mainOS->sprite;
    d.createSprite(240, 135 - TopOffset);
    d.setTextColor(WHITE, BLACK);
    d.unloadFont();
    d.setTextSize(1);
    if(!enteringPassword&&ScanAlredy)
    {d.setCursor(10, 2 - cameraY);}// if see lots of wifi scroll with camera
    else
    {
        d.setCursor(10, 2);
    }
    if (mainOS->WifiConnected)
    {
        d.print("You connected to a wifi press ENTER to Disconnect");
    }
    if (!ScanAlredy)
    {
        d.print("Press ENTER For Scan ");
    }
    if (enteringPassword)
    {
        d.print("Password for: ");
        d.println(WiFi.SSID(selected));
        d.println();
        d.print("Enter: ");
        d.print(password);
    }

    if (connecting)
    {
        d.println("Connecting to:");
        d.println(WiFi.SSID(selected));
    }

    if (scanning)
    {
        d.println("Scanning WiFi...");
    }

    if (!enteringPassword)
    {
        d.println("WiFi Networks:");
        d.println("----------------");
        for (int i = 0; i < scanCount; i++)
        {
            if (i == selected)
                d.print("> ");
            else
                d.print("  ");

            d.print(WiFi.SSID(i));
            d.print("  (");
            d.print(WiFi.RSSI(i));
            d.println("dBm)");
        }
    }
    d.pushSprite(0, 0);
    d.deleteSprite();
}

void Wifi_handle::startScan()
{
    scanCount = WiFi.scanNetworks();
    scanning = false;
    ScanAlredy = true;
    selected = 0;
}

void Wifi_handle::connectToSelected()
{
    WiFi.begin(WiFi.SSID(selected).c_str(), password.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 8000)
    {
        delay(100);
    }

    connecting = false;

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Connected successfully!");
        mainOS->ShowOnScreenMessege("Connected!", 1000);
        mainOS->WifiConnected = true;
    }
    else
    {
        Serial.println("Connection failed! Wrong password?");
        mainOS->ShowOnScreenMessege("Failed! Wrong password?", 1500);
    }
}


void Wifi_handle::OnExit()
{
    if(!mainOS->WifiConnected)
            {WiFi.mode(WIFI_OFF);}
    mainOS->WifiConnected=false;

}