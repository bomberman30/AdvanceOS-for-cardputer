#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <vector>
class GPIOcontrol : public GlobalParentClass
{
public:
    GPIOcontrol(MyOS *os);

    void Begin() override;
    void Loop() override;
    void Draw() override;

private:
    struct PinInfo {
        int pin;
        int mode;       // 0=input,1=input_pullup,2=output,3=pwm,4=analog
        int value;      // state / pwm value / analog read
        int pwmChannel; // -1 if not used
    };

    static const int PIN_COUNT = 11;
    PinInfo pins[PIN_COUNT];

    int selected = 0;
    const int rowH = 12;     
    int CameraY=0;


    bool ShowMenu=false;
    int inMenuID=0;
    void ApplyFromMenu();
    std::vector<String> MenuItems={"Close Menu","INPUT","INPUT_PULLUP","ANALOG_READ","DIGITAL_OUT","PMW"};
void RenderMenu();

    void applyMode(int index);
    void handleKeyboard();
};
