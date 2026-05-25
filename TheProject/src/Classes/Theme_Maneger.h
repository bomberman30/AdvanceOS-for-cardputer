#pragma once
#include "./GlobalParentClass.h"
#include <M5Cardputer.h>
class Theme_Maneger : public GlobalParentClass
{
private:
    /* data */
   // bool inEditThemeFileMode = false;

public:
    // M5Canvas sprite;
    std::vector<String> *SlectedString;
String FileSelected="";
    Theme_Maneger(MyOS *os) : GlobalParentClass(os) {}

    ~Theme_Maneger() {}

    // std::vector<String> menuItems= {"Brightness", "RESET FONT (if select from file manager)","test1","test2"};
    std::vector<String> menuItems = {
        /*      "Open Theme Folder",
             "Reset To Default",
             "Save Current As Theme", */

        "Set Bar Color 1",
        "Set Bar Color 2",
        "Set Bar Text Color",
        "Show Wallpaper?",
        "Set Background Color",
  /*       "Set Menu 1 MUSIC Color 1",
        "Set Menu 1 MUSIC Color 2",
        "Set Menu 2 FILES Color 1",
        "Set Menu 2 FILES Color 2",
        "Set Menu 3 BOT Color 1",
        "Set Menu 3 BOT Color 2",
        "Set Menu 4 NOTES Color 1",
        "Set Menu 4 NOTES Color 2",

        "Set Menu 5 EXTRA Color 1",
        "Set Menu 5 EXTRA Color 2",
        "Set Menu 6 GAMES Color 1",
        "Set Menu 6 GAMES Color 2",
        "Set Menu 7 SETTING Color 1",
        "Set Menu 7 SETTING Color 2",
        "Set Menu 8 PAINT Color 1",
        "Set Menu 8 PAINT Color 2",
        "Set Menu 9 COMPOSER Color 1",
        "Set Menu 9 COMPOSER Color 2", */
    };

    // std::vector<String> Brightness= {"Brightness100%","Brightness90%","Brightness70%", "Brightness50%","Brightness40%","Brightness10%"};
    //   std::vector<String> DimTimer= {"Turn Off In 100 Second","Turn Off In 50 Second","Turn Off In 30 Second","Turn Off In 20 Second","Turn Off In 10 Second"};

    // String menuItems[4] = {"Brightness", "RESET FONT (if select from file manager)"};
    // int menuCount = 2;
    int MenuIdFocus = 0;
    float cameraY = 0;
    float targetCameraY = 0;
    int DistanceBetweenTXT = 25;

    void Begin() override;

    void Loop() override;
    void Draw() override;
};
