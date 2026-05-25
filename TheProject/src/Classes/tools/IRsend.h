#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#define IR_PIN 44
class IRSenderApp : public GlobalParentClass
{
public:
    IRSenderApp(MyOS *os) : GlobalParentClass(os), ir(IR_PIN) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;
    void OnExit() override;

    bool SendCommand(const String &commandName, const String &filePathh);

private:
    IRsend ir;
    std::vector<String> CommandFromFile;
    std::vector<String> getAllCommandNamesFromFile(const String &filePath);

    uint32_t getFrequencyFromFile(const String &name, const String &filePath);

    bool loadAndSendFromFile(const String &name, const String &filePath);
    int CameraY = 0;
    String DeviceID;
    int InCorrentCommand = 0;
    const int distancebetweenMenuLines = 30;

    std::vector<int> letterCmdIndices; // אינדקסים של פקודות לא-מספריות
    char letterMap[64];                // אות לכל פקודה

    int idxUp = -1, idxDown = -1, idxLeft = -1, idxRight = -1, idxEnter = -1;
    int idxNum[10]; // עבור 0-9
    int idxPower = -1, idxMute = -1, idVolUp = -1, idVolDown=-1, idChannelUp = -1, idChannelDown=-1,tempUp=-1,tempDown=-1;
};
