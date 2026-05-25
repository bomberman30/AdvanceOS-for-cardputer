#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include "Function.h"



// #define MAX_PIXEL_FOR_PAINTER 32400 // 240x135
const int MAX_PIXEL_FOR_PAINTER = 32400;
// const int MAX_PIXEL_FOR_PAINTER =57600; //240x240
enum ShapeMode
{
    NONE,
    LINE_MODE,
    TRIANGLE_MODE,
    CIRCLE_MODE,
    RECT_MODE,
    BUCKET_MODE,
    ELLIPSE_MODE,
    EYEDROPPER_MODE,
    OUTLINE_MODE,
    WORD_MODE,
    ZOOM_IN,
    ZOOM_OUT,
    BRUSH_P,
    BRUSH_M,
    SAVE,
    LOAD
};
class Painter : public GlobalParentClass
{
public:
    enum UIPanelState
    {
        UI_DRAWING = 0,
        UI_PANEL_SELECT = 1,
        UI_CLASSIC_MENU = 2
    };

    enum ActivePanel
    {
        PANEL_TOOLS = 0,
        PANEL_PALETTE = 1
    };
struct ImagePreset {
    const char* name;
    int size_x;
    int size_y;
    int zoom;
};

const std::vector<ImagePreset> imagePresets = {
    {"8x8", 8,   8,   12},
    {"16x16", 16,  16,  8},
    {"32x32", 32,  32,  5},
    {"100x100", 100, 100, 2},
    {"240x135", 240, 135, 1},
};
int selectedPreset = -1; // -1 = custom

    // משתנים חדשים במחלקה:
    UIPanelState uiState = UI_DRAWING;
    ActivePanel activePanel = PANEL_TOOLS;
    int panelCursor = 0; // אינדקס בכלי / בצבע

    // הגדרות פאנלים
    static const int TOOL_PANEL_W = 40;
    static const int PALETTE_PANEL_W = 38;
    // החלף את: bool menuOpen = false;
    // ב:

    bool PaintFromFile = false;
    Painter(MyOS *os) : GlobalParentClass(os) {}

    int cursorY;
    int cursorX;
    int Zoomlevel = 5;
    bool DrawGrid = true;

    uint16_t currentColor = BLACK;

    uint16_t PixelsArray[MAX_PIXEL_FOR_PAINTER]; // variable thet contain the corrent picture pixels for the cardputer RGB565

    void Begin() override;

    void Loop() override;
    void Draw() override;
std::vector<String> FirstmenuItems = {
    "Create New Image", 
    "Pic Width",        // case 1
    "Pic Hight",        // case 2
    "Preset Size",      // case 3 - חדש!
    "Open Image Folder",
    "Exit"
};
    void DrawFirstMenu();
    void DrawMenu();
// in the cpp file the size
/* int PicSizeX = 32;
int PicSizeY = 32; */

private:
    static const int TOOL_COUNT = 16;
bool updateSideBar=true;
    bool inFirstMenu = true;

    int brushSize = 0;
    bool shapeFilled = false;

    ShapeMode shapeMode = NONE;
    ShapeMode oldShapeMode = NONE;
    void LoadBMP_To_PixelsArray_var(String BMP_PATH);

    void LoadPNG_To_PixelsArray(String PNG_PATH);
    void DrawSidePanels();
    void HandleSidePanelInput();
    void DrawToolIcon(ShapeMode mode, int cx, int cy, uint16_t color);
    void DrawCanvasSizeMenu();
    void HandleCanvasSizeMenuInput();
    void SetPixelsArrayWHITE_Blank_canvas();
    void Draw_PixelsArray_Var_toScreen();
    void DrawCursor();
    void DrawBrushToBuffer();
    void DrawTriangleToBuffer(int ax, int ay, int bx, int by, int cx, int cy);
    void DrawLineToBuffer(int x0, int y0, int x1, int y1);
    void DrawCircleToBuffer(int cx, int cy, int r);
    void PlotCirclePoints(int cx, int cy, int x, int y);
    void DrawThickPixel(int cx, int cy);
    void DrawEllipseToBuffer(int cx, int cy, int rx, int ry);
    void FillTriangleHelper(int x1, int y1, int x2, int y2, int x3, int y3);
    void FillFlatTriangle(int v1x, int v1y, int v2x, int v2y, int v3x, int v3y);

    // void FloodFill(int startX, int startY);
    void DrawSquareToBuffer(int ax, int ay, int bx, int by);
    void FillBucket();
    // text
    void DrawTextToBuffer(String text, int x, int y, uint8_t size);
    // bool getPixelFromFont(char c, int x, int y);

    bool saveBMP(String fileName, uint16_t *pixelsArray, int width, int height);
bool ImageTooBigToMemory=false;
    void ResetShape();
    void SetShapeMode(ShapeMode mode);
    void HandleShapeLogic();
    // int brushMode = 0; // 0=pixel 1=square 2=circle

    bool drawing = false;

    bool firstPointSet = false;
    bool secondPointSet = false;
    int x1, y1;
    int x2, y2;

    void HandleMenuInput();
    void SavePic(bool Exit, bool forceNewFile = false);
    bool IsPixelNotWhite(int x, int y);
    // bool IsMarked(int index);
    void DrawAutoOutline();
    void DrawOutlineForColor();
    void DrawCursorHighlight();
    bool WorldToScreen(int imgX, int imgY, int &screenX, int &screenY);
    void LoadGIF_To_PixelsArray(String path);
    bool menuOpen = true;
    int menuSelection = 0;
    const int menuItemsCount = 12;

    String getShapeName(ShapeMode mode);
    int scrollOffset = 0;
    const int visibleItems = 6;

    int messegeDebounce = 0;
    void ShowQuickMessege(String Messege, int TickTime = 8000, int Xoffset = 0);
    void ShowQickColorOnScreen(uint16_t color);

    void ReloadPicture();
    int GetMoveTickDelay();
    void ChangeCanvasSize(bool up,bool down,bool right,bool left, int amount);
    int canvasSizeMenuSelection = 0;
int canvasSizeAmount        = 5;
bool inCanvasSizeMenu       = false;
};
