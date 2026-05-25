#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class Wifi_handle : public GlobalParentClass
{
public:
    Wifi_handle(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop() override;
    void Draw() override;
            void OnExit() override;

//bool WIFI_connected=false;
private:
bool ScanAlredy=false;
    int scanCount = 0;
    int selected = 0;
    bool scanning = false;
    bool connecting = false;
    String password = "";

    bool enteringPassword = false;
int cameraY=0;
    void startScan();
    void connectToSelected();
};
