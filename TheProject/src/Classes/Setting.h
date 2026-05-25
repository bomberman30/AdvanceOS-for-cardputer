#pragma once
#include "./GlobalParentClass.h"
#include <M5Cardputer.h>
class Setting : public GlobalParentClass
{
private:
    /* data */
public:
    M5Canvas sprite;
    std::vector<String> *SlectedString;

    Setting(MyOS *os) : GlobalParentClass(os), sprite(&M5Cardputer.Display) {}

    ~Setting() {}

    // std::vector<String> menuItems= {"Brightness", "RESET FONT (if select from file manager)","test1","test2"};
    std::vector<String> menuItems = {"Brightness", "Screen Dim Timer","Menu Items" ,"UI Sound", "Fonts","Show/Unshow Memory Status","Turn Off Timer (Deep Sleep)", "Set CPU Frequency (for Test)","Reset To Default Theme", "Reset Password Vault" /* ,"Show Wallpaper In Main Menu" */,"Credit"};

    std::vector<String> Brightness = {"Brightness100%", "Brightness90%", "Brightness70%", "Brightness50%", "Brightness40%", "Brightness10%"};
    std::vector<String> DimTimer = {"Turn Off In 100 Second", "Turn Off In 50 Second", "Turn Off In 30 Second", "Turn Off In 20 Second", "Turn Off In 10 Second"};
    std::vector<String> GUIsound = {"SOUND ON", "SOUND OFF"};
    std::vector<String> Fonts = {"Default Font", "Second Font"};
    std::vector<String> FREQ = {"240 MHz", "160 MHz", "80 MHz"};
    std::vector<String> TrunOffTimer = {"10min", "30min", "1hr","Never Turn Off"};

    // String menuItems[4] = {"Brightness", "RESET FONT (if select from file manager)"};
    // int menuCount = 2;
    int MenuIdFocus = 0;
    float cameraY = 0;
    float targetCameraY = 0;
    int DistanceBetweenTXT = 13;

    void Begin() override;

    void Loop() override;
    void Draw() override;
};
