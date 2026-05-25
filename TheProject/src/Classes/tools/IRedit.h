#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>



#define IR_PIN 44 
  #define IR_RECIVE_PIN 1 

class IR_Editor : public GlobalParentClass
{
public:

    IR_Editor(MyOS *os): GlobalParentClass(os), ir(IR_PIN), irRecive(IR_RECIVE_PIN){}

    void Begin() override;

    void Loop() override;
    void Draw() override;
        std::vector<String> FirstmenuItems = {"Emit IR signal","Create New Signal", "Edit Signal Name", "Change This IR Signal", "Delete Signal", "Exit"};
int menuSelection=0;
bool inMenu=false;
    void DrawFirstMenu();
    void OnExit() override;

    bool SendCommand(const String& commandName,const String &filePathh);

private:

    IRsend ir;   
        IRrecv irRecive;   

std::vector<String> CommandFromFile;
std::vector<String> getAllCommandNamesFromFile(const String &filePath);

    bool loadAndSendFromFile(const String& name,const String &filePath);
    bool renameSignalInFile(const String &oldName, const String &newName, const String &filePath);
    bool deleteSignalFromFile(const String &name, const String &filePath);
    bool updateSignalDataInFile(const String &name, const String &newData, uint32_t frequency, const String &filePath);
    uint32_t getFrequencyFromFile(const String &name, const String &filePath);
    void StartNewFile();
    int CameraY = 0;
    String DeviceID;
    int InCorrentCommand = 0;
    const int distancebetweenMenuLines = 30;
};
