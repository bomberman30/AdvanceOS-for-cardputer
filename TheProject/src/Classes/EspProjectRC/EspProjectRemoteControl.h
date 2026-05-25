#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class EspProjectRemoteControl : public GlobalParentClass
{
public:
const std::vector<String> MenuItem={"create Command File","Open RC files Directory","Send Current Command",};
int inMenu=0;
// create file items
bool showCreateFileMenu=false;
std::vector<String> CreateFileMenuItems={"File Name","DeviceID","Add Command","Save File"};
int inMenuCreateFile=0;
String DeviceID="all";
std::vector<String> CommandsInFile;// must be empty
//std::vector<String> DescriptionToCommandsInFile;
String fileNameToCreate= "espProjectCommand";
// end 
    EspProjectRemoteControl(MyOS *os) : GlobalParentClass(os) {}
void send_message(const String &msg);
    void Begin() override;

    void Loop() override;
    void Draw() override;
        void OnExit() override;

int CameraY=0;
bool FileValid();
void SaveCurrentFile();
private:
String CurrentStringToSend="";
bool isFileValid;
const int distancebetweenMenuLines = 10;
};
