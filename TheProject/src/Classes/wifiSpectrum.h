#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <WiFi.h>
#include <elapsedMillis.h>
class wifiSpectrum : public GlobalParentClass
{
public:
    wifiSpectrum(MyOS *os) : GlobalParentClass(os) {}
elapsedMillis DrawTime;
    void Begin() override;
    void Loop() override;
    void Draw() override;
        void OnExit() override;

private:
    int channelHeights[14]; 
    unsigned long lastScanTime = 0;
    const unsigned long scanInterval = 5000; 
    void performScan();
    char ssidNames[15][33];
};