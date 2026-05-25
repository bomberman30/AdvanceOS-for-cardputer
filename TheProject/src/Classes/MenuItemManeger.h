#pragma once
#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class MenuItemManeger : public GlobalParentClass
{
public:
    MenuItemManeger(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop()  override;
    void Draw()  override;
    void OnExit()  override;

private:
    int  selectedIndex = 0;   // איזה שורה מסומנת
std::vector<int> drawOrder;

    bool prevUp    = false;
    bool prevDown  = false;
    bool prevLeft  = false;
    bool prevRight = false;
    bool prevEnter = false;

    bool isVisible(int appIndex);
    void toggleVisible(int appIndex);
    void moveUp(int appIndex);
    void moveDown(int appIndex);
    int  findPosition(int appIndex);  // מיקום ב-mainScreenIndices, או -1
};