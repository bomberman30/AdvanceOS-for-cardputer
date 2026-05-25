#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <vector>
#include "MyOS.h"
#define ICON_SIZE        40
#define SUB_ICON_SIZE    40
#define ICON_GAP_HORIZON 35

#define GRID_COLS    3
#define GRID_PAD_X   20
#define GRID_PAD_Y   10
#define GRID_GAP_X   15
#define GRID_GAP_Y   18

#define ICON_STRIDE  (ICON_SIZE + 20)
#define ANIM_SPEED   0.18f

class MainMenuV2 : public GlobalParentClass
{
public:
    MainMenuV2(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop()  override;
    void DrawOnceIfNoAnimation();
    void Draw() override;

    void DrawAppINFO();
bool noAnimation = false;

private:
bool DrawFirstFrame=true;
 String CurrentThemePath;
 String WallpaperPath;
    void saveState();
int selectedFileIndex = -1; // -1 = לא בחרנו קובץ
bool inFileRow = false;
void drawFileRow();
bool walpaperFromSD_exist_And_Show=false;
float camY = 0;
float targetCamY = 0;

    // ── כל האפליקציות במקום אחד ──────────────────────────────

    // ── מה מוצג על המסך הראשי (אינדקסים לתוך allApps) ────────
   // std::vector<int> mainScreenIndices;

    // ── סאב-מנואים ───────────────────────────────────────────
    std::vector<SubMenu> subMenus;

    // ── מצב נוכחי ────────────────────────────────────────────
    std::vector<int>* currentIndices = nullptr;
    bool  inSubMenu   = false;
    int   parentIndex = 0;          // אינדקס ב-mainScreenIndices

    float camX       = 0;
    float targetCamX = 0;

    bool prevLeft  = false;
    bool prevRight = false;
    bool prevUp    = false;
    bool prevDown  = false;
    bool prevEnter = false;
    bool prevBack  = false;

    // ── עוזרים ───────────────────────────────────────────────
    MenuItem& getItem(int vectorIndex)       { return mainOS->allApps[vectorIndex]; }
    MenuItem& currentItem(int pos)           { return mainOS->allApps[(*currentIndices)[pos]]; }
    int       currentCount()                 { return (*currentIndices).size(); }

    void updateCamera();
    void moveSubMenuTo(int col);
    void openSubMenu(int subMenuId);
    void closeSubMenu();
bool Show_APP_INFO=false;
    void drawMainGrid();
    void drawSubMenuHorizontal();
    void drawIcon(MenuItem& item, int sx, int sy, int cellW, int cellH, bool selected);
    void drawBreadcrumb();
    void DrawINFO_Window();
};