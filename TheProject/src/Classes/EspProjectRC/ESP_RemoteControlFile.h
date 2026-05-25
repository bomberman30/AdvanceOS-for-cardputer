#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class ESP_RemoteControlFile : public GlobalParentClass
{
public:
    // M5Canvas sprite;
    //  M5Canvas SpeakerSprite;
    // M5Canvas Note1Sprite;
    // M5Canvas Note2Sprite;

    ESP_RemoteControlFile(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;
        void OnExit() override;

    // void Task_AudioC(void *pvParameters);
private:
void send_message(const String &msg);
int CameraY=0;
std::vector<String> CommandFromFile;
String DeviceID;
int InCorrentCommand=0;
const int distancebetweenMenuLines=25;
};
