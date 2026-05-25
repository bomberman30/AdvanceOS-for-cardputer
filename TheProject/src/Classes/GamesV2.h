#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <Preferences.h>
class GamesV2 : public GlobalParentClass
{
public:
Preferences prefsRomToLoad;
Preferences AppStats;
void SetAppStats();
    GamesV2(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;
    bool GameHavePic=false;
    String GamePicPath;
    void GetGamePic();
std::vector<String> GamePathList;
private:
       int ItemSelectID = 0;
    const int DestanceBetweenLines = 22;
    int CameraY = 0;
void GameSelectedFromMenu(bool FromFileExplorer=false);
void SetRomToLoadPath(String RomPath);
void GetGameListFromSD();
bool refrashGamePic=true;
};
