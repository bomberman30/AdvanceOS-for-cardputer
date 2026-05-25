#include "Painter.h"
#include "MyOS.h"
#include "MainMenuV2.h"
#include "FileBrowser.h"
#include "Pic/PainterPicture.c"
#include "Function.h"
// #include "Function.h"
// File gifFilePainter;
uint16_t *pixelArrayPointer = nullptr;
#define BACK_COLOR TFT_LIGHTGREY
int PicSizeX = 32;
int PicSizeY = 32;

struct ToolEntry
{
    const char *label;
    ShapeMode mode;
};
static const ToolEntry TOOLS[] = {
    {"Brush (key B)", NONE},
    {"Line Mode (key L)", LINE_MODE},
    {"Circle Mode (key C)", CIRCLE_MODE},
    {"Rect Mode (key R)", RECT_MODE},
    {"Ellipse Mode (key E)", ELLIPSE_MODE},
    {"Triangle Mode (key T)", TRIANGLE_MODE},
    {"Bucket Mode (key G)", BUCKET_MODE},
    {"Outline Mode (key O)", OUTLINE_MODE},
    {"Eyedropper Mode (key I)", EYEDROPPER_MODE},
    {"Text Mode (key W)", WORD_MODE},
    {"Zoom In (key 3)", ZOOM_IN},
    {"Zoom Out (key 2)", ZOOM_OUT},
    {"BRUSH + (key +)", BRUSH_P},
    {"BRUSH - (key -)", BRUSH_M},
    {"Save - (key S)", SAVE},
    {"Load - (key A)", LOAD},

};

/* static const uint16_t PALETTE[] = {
    TFT_RED,
    TFT_BLUE,
    TFT_GREEN,
    TFT_YELLOW,
    TFT_ORANGE,
    TFT_PURPLE,
    TFT_CYAN,
    TFT_MAGENTA,
    TFT_WHITE,
    TFT_BLACK,
    TFT_MAROON,
    TFT_NAVY,
    TFT_OLIVE,
    TFT_DARKGREEN,
    0xFD20,
    0xF81F,
};
static const int PALETTE_COUNT = 16;
 */
static const uint16_t PALETTE[] = {
    // ── שחור / לבן / אפורים ──
    TFT_BLACK,
    TFT_WHITE,
    0x4208,
    0x9CF3, // אפור כהה, אפור בהיר

    // ── אדומים ──
    TFT_MAROON,
    TFT_RED,
    0xFCB0, // ורוד עמוק, ורוד בהיר

    // ── כתומים / צהובים ──
    0xFD20,     // כתום, כתום-צהוב
    TFT_YELLOW, // צהוב, צהוב פסטל

    // ── ירוקים ──
    TFT_DARKGREEN,
    0x07E0,
    0xB7E0, // ירוק טהור, ירוק בהיר

    // ── ציאן / טורקיז ──
    TFT_DARKCYAN,
    0x07FF,
    0xAFFF, // ציאן עמוק, ציאן בהיר

    // ── כחולים ──
    TFT_NAVY,
    TFT_BLUE,
    0x3D9F, // כחול בינוני, כחול-אפור

    // ── סגולים / מגנטה ──
    0x8010, // סגול, סגול-כחול
    0xF81F, // מגנטה, ורוד-ניאון

    // ── חומים / עור ──
    TFT_OLIVE, // זית, חום
    0xFDA0,    // חום-כתום, בז'
};

static const int PALETTE_COUNT = sizeof(PALETTE) / sizeof(PALETTE[0]); // = 32

void Painter::Begin()
{
    showTopBar = false;

    if (mainOS->EditFromFile)
    {
        menuOpen = false;
        inFirstMenu = false;
        mainOS->EditFromFile = false;
        String Ext = mainOS->GetExtensionLower(mainOS->FileSelectedInFS.c_str());
        if (Ext == "bmp")
        {
            PaintFromFile = true;

            LoadBMP_To_PixelsArray_var(mainOS->FileSelectedInFS);
        }
        else if (Ext == "png")
        {
            pixelArrayPointer = PixelsArray;

            LoadPNG_To_PixelsArray(mainOS->FileSelectedInFS);
        }

        else if (Ext == "gif")
        {
            pixelArrayPointer = PixelsArray;

            LoadGIF_To_PixelsArray(mainOS->FileSelectedInFS);
        }
        Draw();

        cursorX = PicSizeX / 2;
        cursorY = PicSizeY / 2;
        Draw();
        return;
    }
    cursorX = PicSizeX / 2;
    cursorY = PicSizeY / 2;
    SetPixelsArrayWHITE_Blank_canvas();
    Draw();
}

void Painter::Loop()
{

    if (messegeDebounce > 0)
    {
        messegeDebounce -= 1;
        if (messegeDebounce <= 0)
        {
            updateSideBar = true;
            Draw();
        }
    }
    if (inCanvasSizeMenu)
    {
        HandleCanvasSizeMenuInput();
        menuOpen = false;

        return;
    }

    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        updateSideBar = true;
        if (inFirstMenu)
        {
            mainOS->ChangeMenu(new MainMenuV2(mainOS));
            return;
        }
        if (!inFirstMenu)
        {
            if (firstPointSet)
            {
                ResetShape();
                Draw();
                return;
            }

            switch (uiState)
            {
            case UI_DRAWING:
                menuOpen = false;
                if (inCanvasSizeMenu)
                {
                    uiState = UI_DRAWING;
                }
                uiState = UI_PANEL_SELECT;
                activePanel = PANEL_TOOLS;
                panelCursor = 0;
                // מצא את הכלי הנוכחי כנקודת התחלה
                for (int i = 0; i < TOOL_COUNT; i++)
                    if (TOOLS[i].mode == shapeMode)
                    {
                        panelCursor = i;
                        break;
                    }
                Draw();
                return;

            case UI_PANEL_SELECT:
                uiState = UI_CLASSIC_MENU;
                if (inCanvasSizeMenu)
                {
                    uiState = UI_DRAWING;
                }
                menuOpen = true;
                Draw();
                return;

            case UI_CLASSIC_MENU:
                uiState = UI_DRAWING;

                menuOpen = false;
                Draw();
                return;
            }
        }
    }
    if (uiState == UI_PANEL_SELECT)
    {

        // ── ניווט ימין ──────────────────────────────────────────────
        if (mainOS->NewKey.ifKeyJustPress('/'))
        {
            updateSideBar = true;
            int col = panelCursor % 2;

            if (activePanel == PANEL_TOOLS)
            {
                if (col == 0)
                {
                    // עמודה שמאלית → עמודה ימנית (אותה שורה)
                    if (panelCursor + 1 < TOOL_COUNT)
                        panelCursor += 1;
                }
                else
                {
                    // עמודה ימנית → מעבר לפלטה, עמודה שמאלית
                    activePanel = PANEL_PALETTE;
                    // מצא עמודה שמאלית (זוגית) באותה גובה יחסי
                    int row = panelCursor / 2;
                    int newCursor = row * 2;
                    if (newCursor >= PALETTE_COUNT)
                        newCursor = PALETTE_COUNT - 2;
                    if (newCursor < 0)
                        newCursor = 0;
                    panelCursor = newCursor;
                }
            }
            else // PANEL_PALETTE
            {
                if (col == 0)
                {
                    // עמודה שמאלית → עמודה ימנית
                    if (panelCursor + 1 < PALETTE_COUNT)
                        panelCursor += 1;
                }
                // עמודה ימנית → אין לאן לעבור ימינה (קצה מסך)
            }
            Draw();
            if (activePanel == PANEL_TOOLS)

            {
                ShowQuickMessege(String(" ") + String(TOOLS[panelCursor].label), 8000, 30);
            }

            return;
        }

        // ── ניווט שמאל ──────────────────────────────────────────────
        if (mainOS->NewKey.ifKeyJustPress(','))
        {
            updateSideBar = true;
            int col = panelCursor % 2;

            if (activePanel == PANEL_PALETTE)
            {
                if (col == 1)
                {
                    // עמודה ימנית → עמודה שמאלית
                    panelCursor -= 1;
                }
                else
                {
                    // עמודה שמאלית → מעבר לכלים, עמודה ימנית
                    activePanel = PANEL_TOOLS;
                    int row = panelCursor / 2;
                    int newCursor = row * 2 + 1;
                    if (newCursor >= TOOL_COUNT)
                        newCursor = TOOL_COUNT - 1;
                    panelCursor = newCursor;
                }
            }
            else // PANEL_TOOLS
            {
                if (col == 1)
                {
                    // עמודה ימנית → עמודה שמאלית
                    panelCursor -= 1;
                    ShowQuickMessege(String(" ") + String(TOOLS[panelCursor].label), 8000, 30);
                }
                // עמודה שמאלית → קצה שמאלי, אין מעבר
            }
            Draw();
            if (activePanel == PANEL_TOOLS)

            {
                ShowQuickMessege(String(" ") + String(TOOLS[panelCursor].label), 8000, 30);
            }

            return;
        }

        // מעלה/מטה — ניווט בפאנל
        if (mainOS->NewKey.ifKeyJustPress(';')) // up
        {
            updateSideBar = true;

            int maxItems = (activePanel == PANEL_TOOLS) ? TOOL_COUNT : PALETTE_COUNT;
            panelCursor = (panelCursor - 2 + maxItems) % maxItems;
            Draw();
            if (activePanel == PANEL_TOOLS)
            {
                ShowQuickMessege(String(" ") + String(TOOLS[panelCursor].label), 8000, 30);
            }

            return;
        }
        if (mainOS->NewKey.ifKeyJustPress('.')) // down
        {
            updateSideBar = true;

            int maxItems = (activePanel == PANEL_TOOLS) ? TOOL_COUNT : PALETTE_COUNT;
            panelCursor = (panelCursor + 2) % maxItems;
            Draw();
            if (activePanel == PANEL_TOOLS)

            {
                ShowQuickMessege(String(" ") + String(TOOLS[panelCursor].label), 8000, 30);
            }

            return;
        }

        // Enter — בחר
        if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
        {
            if (activePanel == PANEL_TOOLS)
            {
                if (TOOLS[panelCursor].mode == ZOOM_IN) // foutue zoom mode
                {
                    Zoomlevel++;
                    Draw();
                    ShowQuickMessege(String("Zoom Level: " + String(Zoomlevel)));
                }
                else if (TOOLS[panelCursor].mode == ZOOM_OUT) // foutue zoom mode
                {
                    Zoomlevel--;
                    if (Zoomlevel < 1)
                        Zoomlevel = 1;
                    Draw();
                    ShowQuickMessege(String("Zoom Level: " + String(Zoomlevel)));
                }
                else if (TOOLS[panelCursor].mode == BRUSH_M) // foutue zoom mode
                {
                    brushSize -= 1;
                    if (brushSize < 0)
                    {
                        brushSize = 0;
                    }
                    Draw();
                    ShowQuickMessege(String("Brush Size: " + String(brushSize + 1)));
                }
                else if (TOOLS[panelCursor].mode == BRUSH_P) // foutue zoom mode
                {
                    brushSize += 1;

                    Draw();
                    ShowQuickMessege(String("Brush Size: " + String(brushSize + 1)));
                }
                else if (TOOLS[panelCursor].mode == SAVE) // foutue zoom mode
                {
                    SavePic(false);
                    Draw();
                    ShowQuickMessege("Saved");
                }
                else if (TOOLS[panelCursor].mode == LOAD) // foutue zoom mode
                {
                    ReloadPicture();
                    // Draw();
                }
                else
                {
                    SetShapeMode(TOOLS[panelCursor].mode);
                    uiState = UI_DRAWING;
                    Draw();
                    ShowQuickMessege(String("Choose: ") + String(TOOLS[panelCursor].label));
                }
            }
            else
            {
                currentColor = PALETTE[panelCursor];

                uiState = UI_DRAWING;
                Draw();
                ShowQuickMessege("Color Selected!");
                ShowQickColorOnScreen(currentColor);
            }

            return;
        }

        return; // בזמן PANEL_SELECT לא מאפשרים ציור
    }
    if (mainOS->NewKey.ifKeyJustPress('2'))
    {
        Zoomlevel--;
        if (Zoomlevel < 1)
            Zoomlevel = 1;
        Draw();
        ShowQuickMessege(String("Zoom Level: " + String(Zoomlevel)));
    }
    if (mainOS->NewKey.ifKeyJustPress('3'))
    {
        Zoomlevel++;
        Draw();
        ShowQuickMessege(String("Zoom Level: " + String(Zoomlevel)));
    }
    if (menuOpen)
    {
        HandleMenuInput();
        //  menuSelection = (menuSelection + menuItemsCount) % menuItemsCount;

        if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER)) // menu select
        {
            if (inFirstMenu)
            {
                if (FirstmenuItems[menuSelection] == "Exit")
                {
                    mainOS->ChangeMenu(new MainMenuV2(mainOS));
                    return;
                }
                else if (FirstmenuItems[menuSelection] == "Create New Image")
                {
                    inFirstMenu = false;
                    menuOpen = false;
                    if ((PicSizeX * PicSizeY) > MAX_PIXEL_FOR_PAINTER)
                    {
                        mainOS->ShowOnScreenMessege("Picture Too Big For The Cardputer RAM Choose Lower Size! Exiting..");
                        mainOS->ChangeMenu(new MainMenuV2(mainOS));
                        return;
                    }
                    cursorX = PicSizeX / 2;
                    cursorY = PicSizeY / 2;
                    SetPixelsArrayWHITE_Blank_canvas();

                    Draw();
                }
                else if (FirstmenuItems[menuSelection] == "Pic Hight")
                {
                    PicSizeY = mainOS->AskFromUserForString("Set Picture Hight", true, false, true).toInt();
                    Zoomlevel = 1;
                    DrawFirstMenu();
                }
                else if (FirstmenuItems[menuSelection] == "Pic Width")
                {
                    PicSizeX = mainOS->AskFromUserForString("Set Picture Width", true, false, true).toInt();
                    Zoomlevel = 1;

                    DrawFirstMenu();
                }
                else if (FirstmenuItems[menuSelection] == "Open Image Folder")
                {
                    mainOS->currentPath = "/AdvanceOS/Paint";
                    mainOS->ChangeMenu(new FileBrowser(mainOS));
                }
                else if (FirstmenuItems[menuSelection] == "Preset Size")
                {
                    // מחזור בין ה-presets
                    selectedPreset = (selectedPreset + 1) % imagePresets.size();
                    PicSizeX = imagePresets[selectedPreset].size_x;
                    PicSizeY = imagePresets[selectedPreset].size_y;
                    Zoomlevel = imagePresets[selectedPreset].zoom;
                    DrawFirstMenu();
                }
                return;
            }
            switch (menuSelection)
            {
            case 0:
                brushSize = (brushSize + 1) % 10;
                break;
            case 1:
                shapeMode = (ShapeMode)((shapeMode + 1) % 8);
                break;
            case 2:
                shapeFilled = !shapeFilled;
                break;

            case 3:
                menuOpen = false;
                DrawAutoOutline();
                break;
            case 4:
                menuOpen = false;
                currentColor = mainOS->AskForColor("choose color", currentColor);
                Draw();
                ShowQuickMessege("Color Selected!");
                ShowQickColorOnScreen(currentColor);
                if (!menuOpen)
                    return;
                break;

            case 5:

                // String text_to_draw = mainOS->AskFromUserForString("Enter Text:", false);
                menuOpen = false;
                DrawTextToBuffer(mainOS->AskFromUserForString("Enter Text:", false), cursorX, cursorY, brushSize + 1);
                Draw();
                if (!menuOpen)
                    return;
                break;
            case 6: // change canvas size
                inCanvasSizeMenu = true;
                menuOpen = false;
                Draw();
                break;

            case 7: // only save
                menuOpen = false;

                SavePic(false);
                break;
            case 8: // only save
                menuOpen = false;

                SavePic(false, true);
                break;

            case 9: // save and quit
                menuOpen = false;

                SavePic(true);

                break;
            case 10:
                menuOpen = false;

                ReloadPicture();
                return;
            case 11:
                mainOS->ChangeMenu(new MainMenuV2(mainOS));
                return;
            }
            Draw();
            // DrawMenu();
        }
        return;
    }
    if (mainOS->NewKey.ifKeyJustPress('o')) // 'o' = Auto Outline
    {
        SetShapeMode(OUTLINE_MODE);

        ShowQuickMessege("Outline Around Color Mode");
    }

    if (mainOS->NewKey.ifKeyJustPress('7')) // canvas size menu

    {
        inCanvasSizeMenu = true;
        menuOpen = false;
        Draw();
        // DrawCanvasSizeMenu();
    }
    if (mainOS->NewKey.ifKeyJustPress('p'))
    {
        currentColor = mainOS->AskForColor("Choose color", currentColor);
        updateSideBar = true;
        Draw();
        ShowQuickMessege("Color Selected!");
        ShowQickColorOnScreen(currentColor);
    }
    if (mainOS->NewKey.ifKeyJustPress('='))
    {
        brushSize += 1;

        ShowQuickMessege(String("Brush Size: " + String(brushSize + 1)));
    }
    if (mainOS->NewKey.ifKeyJustPress('-'))
    {
        brushSize -= 1;
        if (brushSize < 0)
        {
            brushSize = 0;
        }

        ShowQuickMessege(String("Brush Size: " + String(brushSize + 1)));
    }

    if (mainOS->NewKey.ifKeyJustPress('b'))
    {
        SetShapeMode(NONE);

        ShowQuickMessege("Draw mode BRUSH");
    }
    if (mainOS->NewKey.ifKeyJustPress('l'))
    {
        SetShapeMode(LINE_MODE);

        ShowQuickMessege("Draw mode LINE");
    }
    if (mainOS->NewKey.ifKeyJustPress('t'))
    {

        ShowQuickMessege("Draw mode TRIANGLE");
        SetShapeMode(TRIANGLE_MODE);
    }
    if (mainOS->NewKey.ifKeyJustPress('i'))
    {
        SetShapeMode(EYEDROPPER_MODE);

        ShowQuickMessege("EYEDROPPER Choose color from PIC");
    }
    if (mainOS->NewKey.ifKeyJustPress('w'))
    {
        SetShapeMode(WORD_MODE);

        ShowQuickMessege("Press Enter To Put A Word");
    }
    if (mainOS->NewKey.ifKeyJustPress('c'))
    {
        SetShapeMode(CIRCLE_MODE);

        ShowQuickMessege("Draw mode CIRCLE");
    }
    if (mainOS->NewKey.ifKeyJustPress('g'))
    {
        SetShapeMode(BUCKET_MODE);
        Draw();
        ShowQuickMessege("Draw mode FILL BUCKET");
    }
    if (mainOS->NewKey.ifKeyJustPress('r'))
    {
        SetShapeMode(RECT_MODE);

        ShowQuickMessege("Draw mode Rectangle");
    }
    if (mainOS->NewKey.ifKeyJustPress('e'))
    {
        SetShapeMode(ELLIPSE_MODE);

        ShowQuickMessege("Draw mode ELLIPSE");
    }
    if (mainOS->NewKey.ifKeyJustPress('f'))
    {
        shapeFilled = !shapeFilled;

        ShowQuickMessege(shapeFilled ? "Fill: ON" : "Fill: OFF");
    }
    if (mainOS->NewKey.ifKeyJustPress('s'))
    {
        SavePic(false);
    }
    if (mainOS->NewKey.ifKeyJustPress('a'))
    {
        ReloadPicture();
    }
    if (mainOS->NewKey.ifKeyJustPress('m'))
    {

        ShowQuickMessege(getHeapInfoKB());
    }

    bool moved = false;
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {

        if (shapeMode == NONE)
        {
            DrawBrushToBuffer();
            Draw();
        }
        else if (shapeMode == WORD_MODE)
        {
            String text_to_draw = mainOS->AskFromUserForString("Enter Text:", false);
            updateSideBar = true;

            if (text_to_draw != "")
            {
                DrawTextToBuffer(text_to_draw, cursorX, cursorY, brushSize + 1);
                Draw();
            }
        }
        else
        {
            HandleShapeLogic();
        }
    }

    int oldX = cursorX;
    int oldY = cursorY;

    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(';', 300, GetMoveTickDelay()))
    {
        if (messegeDebounce > 0)
        {
            messegeDebounce = 0;
            updateSideBar = true;
        }
        cursorY--;
        moved = true;
    }
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('.', 300, GetMoveTickDelay()))
    {
        if (messegeDebounce > 0)
        {
            messegeDebounce = 0;
            updateSideBar = true;
        }
        cursorY++;
        moved = true;
    }
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(',', 300, GetMoveTickDelay()))
    {
        if (messegeDebounce > 0)
        {
            messegeDebounce = 0;
            updateSideBar = true;
        }
        cursorX--;
        moved = true;
    }
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('/', 300, GetMoveTickDelay()))
    {
        if (messegeDebounce > 0)
        {
            messegeDebounce = 0;
            updateSideBar = true;
        }
        cursorX++;
        moved = true;
    }
    cursorX = constrain(cursorX, 0, PicSizeX - 1);
    cursorY = constrain(cursorY, 0, PicSizeY - 1);

    if (drawing)
    {
        int index = cursorY * PicSizeX + cursorX;
        PixelsArray[index] = currentColor;
        DrawBrushToBuffer();
    }

    if (moved || drawing)
    {
        Draw();
    }
}

void Painter::Draw()
{

    if (inFirstMenu)
    {
        DrawFirstMenu();
        return;
    }
    Draw_PixelsArray_Var_toScreen();
    DrawCursor();
    // DrawCursorHighlight();
    if (firstPointSet)
    {
        if (shapeMode == LINE_MODE)
        {

            // M5.Lcd.drawLine(x1, y1, cursorX, cursorY, TFT_RED);
            int sx1, sy1, sx2, sy2;

            WorldToScreen(x1, y1, sx1, sy1);
            WorldToScreen(cursorX, cursorY, sx2, sy2);

            M5.Lcd.drawLine(sx1, sy1, sx2, sy2, TFT_RED);
        }
        else if (shapeMode == TRIANGLE_MODE)
        {
            /*             if (!secondPointSet)
                        {
                            M5.Lcd.drawLine(x1, y1, cursorX, cursorY, TFT_RED);
                        }
                        else
                        {
                            M5.Lcd.drawLine(x1, y1, x2, y2, TFT_RED);
                            M5.Lcd.drawLine(x2, y2, cursorX, cursorY, TFT_RED);
                            M5.Lcd.drawLine(cursorX, cursorY, x1, y1, TFT_RED);
                        } */
            int sx1, sy1, sx2, sy2, sx3, sy3;

            // נקודה ראשונה
            WorldToScreen(x1, y1, sx1, sy1);

            // נקודה שנייה
            if (secondPointSet)
                WorldToScreen(x2, y2, sx2, sy2);
            else
                WorldToScreen(cursorX, cursorY, sx2, sy2);

            // נקודה שלישית (תמיד הקורסר)
            WorldToScreen(cursorX, cursorY, sx3, sy3);

            // ציור
            M5.Lcd.drawLine(sx1, sy1, sx2, sy2, TFT_RED);
            M5.Lcd.drawLine(sx2, sy2, sx3, sy3, TFT_RED);
            M5.Lcd.drawLine(sx3, sy3, sx1, sy1, TFT_RED);
        }
        else if (shapeMode == CIRCLE_MODE)
        {
            /*           int dx = cursorX - x1;
                      int dy = cursorY - y1;
                      int radius = sqrt(dx * dx + dy * dy);

                      M5.Lcd.drawCircle(x1, y1, radius, TFT_RED); */
            int sx, sy;
            WorldToScreen(x1, y1, sx, sy);

            int dx = cursorX - x1;
            int dy = cursorY - y1;
            int radius = sqrt(dx * dx + dy * dy);

            // בזום צריך להכפיל
            if (Zoomlevel > 1)
                radius *= Zoomlevel;

            M5.Lcd.drawCircle(sx, sy, radius, TFT_RED);
        }
        else if (shapeMode == RECT_MODE)
        {
            /*             int xMin = min(x1, cursorX);
                        int xMax = max(x1, cursorX);
                        int yMin = min(y1, cursorY);
                        int yMax = max(y1, cursorY);

                        M5.Lcd.drawRect(xMin, yMin, xMax - xMin, yMax - yMin, TFT_RED); */
            int sx1, sy1, sx2, sy2;

            WorldToScreen(x1, y1, sx1, sy1);
            WorldToScreen(cursorX, cursorY, sx2, sy2);

            int xMin = min(sx1, sx2);
            int yMin = min(sy1, sy2);
            int w = abs(sx2 - sx1);
            int h = abs(sy2 - sy1);

            M5.Lcd.drawRect(xMin, yMin, w, h, TFT_RED);
        }
        else if (shapeMode == ELLIPSE_MODE && firstPointSet)
        {
            /*             int rx = abs(cursorX - x1);
                        int ry = abs(cursorY - y1);

                        M5.Lcd.drawEllipse(x1, y1, rx, ry, TFT_RED); */
            int sx, sy;
            WorldToScreen(x1, y1, sx, sy);

            // רדיוסים לפי העולם
            int rx = abs(cursorX - x1);
            int ry = abs(cursorY - y1);

            // בזום צריך להגדיל
            if (Zoomlevel > 1)
            {
                rx *= Zoomlevel;
                ry *= Zoomlevel;
            }

            // ציור
            M5.Lcd.drawEllipse(sx, sy, rx, ry, TFT_RED);
        }
    }
    if (!menuOpen)
    {
        if (uiState == UI_CLASSIC_MENU)
        {
            uiState = UI_DRAWING;
        }
    }
    if (uiState != UI_CLASSIC_MENU)
    {
        if (updateSideBar)
        {
            DrawSidePanels();
        }
    }
    else
    {
        {
            DrawMenu();
        }
    }

    if (inCanvasSizeMenu)

    {
        DrawCanvasSizeMenu();
    }
}

void Painter::DrawCursorHighlight()
{
    int screenW = 240;
    int screenH = 135;

    int centerX = screenW / 2;
    int centerY = screenH / 2;

    // אם מחוץ לתמונה → לא מציירים
    if (cursorX < 0 || cursorY < 0 || cursorX >= PicSizeX || cursorY >= PicSizeY)
        return;

    if (Zoomlevel > 1)
    {
        // 🔥 בזום - ריבוע בגודל הפיקסל
        int size = Zoomlevel;

        int x = centerX - size / 2;
        int y = centerY - size / 2;

        // מסגרת
        //   M5.Lcd.drawRect(x, y, size, size, TFT_WHITE);
        uint16_t color = PixelsArray[cursorY * PicSizeX + cursorX];
        uint16_t inverted = ~color;

        M5.Lcd.drawRect(x, y, size, size, inverted);
    }
    else
    {
        // 🔥 מצב רגיל - צלב קטן
        M5.Lcd.drawFastHLine(centerX - 2, centerY, 5, TFT_WHITE);
        M5.Lcd.drawFastVLine(centerX, centerY - 2, 5, TFT_WHITE);
    }
}

void Painter::Draw_PixelsArray_Var_toScreen()
{
    M5.Lcd.setSwapBytes(true);
    M5.Lcd.startWrite();

    int displayW = 240;
    int displayH = 135;

    int areaX = TOOL_PANEL_W;                              // תחילת אזור האמצע
    int areaW = displayW - TOOL_PANEL_W - PALETTE_PANEL_W; // רוחב אזור האמצע
    int areaX2 = areaX + areaW;                            // סוף אזור האמצע

    if (Zoomlevel == 1)
    {
        int camX = cursorX - displayW / 2;
        int camY = cursorY - displayH / 2;

        int drawX = -camX;
        int drawY = -camY;

        int imgX1 = max(drawX, areaX);
        int imgY1 = max(drawY, 0);
        int imgX2 = min(drawX + PicSizeX, areaX2);
        int imgY2 = min(drawY + PicSizeY, displayH);

        // מילוי שחור רק באזור האמצע
        if (drawX > areaX)
            M5.Lcd.fillRect(areaX, 0, drawX - areaX, displayH, BACK_COLOR);
        if (drawX + PicSizeX < areaX2)
            M5.Lcd.fillRect(drawX + PicSizeX, 0, areaX2 - (drawX + PicSizeX), displayH, BACK_COLOR);
        if (drawY > 0)
            M5.Lcd.fillRect(areaX, 0, areaW, drawY, BACK_COLOR);
        if (drawY + PicSizeY < displayH)
            M5.Lcd.fillRect(areaX, drawY + PicSizeY, areaW, displayH - (drawY + PicSizeY), BACK_COLOR);

        // ציור התמונה - pushImage מצייר רק את מה שבתוך המסך, אבל לא חוצה לפאנלים
        // לכן נצייר שורה-שורה רק את הפיקסלים שבאזור האמצע
        int srcX = max(0, areaX - drawX);
        int srcY = max(0, 0 - drawY);
        int srcX2 = min(PicSizeX, areaX2 - drawX);
        int srcY2 = min(PicSizeY, displayH - drawY);

        if (srcX < srcX2 && srcY < srcY2)
        {
            // pushImage עם offset - נצייר רק את החלק הנראה באמצע
            // נשתמש ב-pushImage עם src offset
            for (int row = srcY; row < srcY2; row++)
            {
                int screenY = drawY + row;
                int screenX = drawX + srcX;
                M5.Lcd.pushImage(screenX, screenY, srcX2 - srcX, 1,
                                 &PixelsArray[row * PicSizeX + srcX]);
            }
        }
    }
    else // Zoomlevel > 1
    {
        int centerX = displayW / 2;
        int centerY = displayH / 2;

        int drawX = centerX - (cursorX * Zoomlevel) - (Zoomlevel / 2);
        int drawY = centerY - (cursorY * Zoomlevel) - (Zoomlevel / 2);

        int imgX1 = drawX;
        int imgY1 = drawY;
        int imgX2 = drawX + (PicSizeX * Zoomlevel);
        int imgY2 = drawY + (PicSizeY * Zoomlevel);

        // מילוי שחור רק באזור האמצע
        if (imgX1 > areaX)
            M5.Lcd.fillRect(areaX, 0, imgX1 - areaX, displayH, BACK_COLOR);
        if (imgX2 < areaX2)
            M5.Lcd.fillRect(imgX2, 0, areaX2 - imgX2, displayH, BACK_COLOR);
        if (imgY1 > 0)
            M5.Lcd.fillRect(areaX, 0, areaW, imgY1, BACK_COLOR);
        if (imgY2 < displayH)
            M5.Lcd.fillRect(areaX, imgY2, areaW, displayH - imgY2, BACK_COLOR);

        // Culling - רק פיקסלים שנמצאים באזור האמצע
        int startX = max(0, (areaX - drawX) / Zoomlevel);
        int endX = min(PicSizeX, (areaX2 - drawX) / Zoomlevel + 1);
        int startY = max(0, (0 - drawY) / Zoomlevel);
        int endY = min(PicSizeY, (displayH - drawY) / Zoomlevel + 1);

        for (int y = startY; y < endY; y++)
        {
            for (int x = startX; x < endX; x++)
            {
                int screenX = drawX + (x * Zoomlevel);
                int screenY = drawY + (y * Zoomlevel);

                // בדיקה ידנית שלא חורגים מאזור האמצע
                if (screenX < areaX || screenX + Zoomlevel > areaX2)
                    continue;

                uint16_t color = PixelsArray[y * PicSizeX + x];
                M5.Lcd.fillRect(screenX, screenY, Zoomlevel, Zoomlevel, color);
            }
        }

        // גריד
        if (DrawGrid && Zoomlevel > 4)
        {
            for (int x = startX; x <= endX; x++)
            {
                int gx = drawX + (x * Zoomlevel);
                if (gx >= areaX && gx <= areaX2)
                    M5.Lcd.drawFastVLine(gx, max(0, imgY1), min(displayH, imgY2) - max(0, imgY1), TFT_DARKGREY);
            }
            for (int y = startY; y <= endY; y++)
            {
                int gy = drawY + (y * Zoomlevel);
                if (gy >= 0 && gy <= displayH)
                    M5.Lcd.drawFastHLine(max(areaX, imgX1), gy, min(areaX2, imgX2) - max(areaX, imgX1), TFT_DARKGREY);
            }
        }
    }

    M5.Lcd.endWrite();
    M5.Lcd.setSwapBytes(false);
}
/* void Painter::DrawCursor()
{
    int size = 2;

    for (int i = -size; i <= size; i++)
    {
        M5.Lcd.drawPixel(120 + i, 67 + i, TFT_RED);

        M5.Lcd.drawPixel(120 + i, 67 - i, TFT_RED);
    }
} */
void Painter::DrawCursor()
{
    int size = 2;

    // צל שחור (ימין+למטה)
    for (int i = -size; i <= size; i++)
    {
        M5.Lcd.drawPixel(120 + i + 1, 67 + i + 1, TFT_BLACK);
        M5.Lcd.drawPixel(120 + i + 1, 67 - i + 1, TFT_BLACK);
    }

    // X אדום קטן יותר
    for (int i = -size; i <= size; i++)
    {
        M5.Lcd.drawPixel(120 + i, 67 + i, TFT_RED);
        M5.Lcd.drawPixel(120 + i, 67 - i, TFT_RED);
    }
}
/* void Painter::DrawFirstMenu()
{
    // MAX_PIXEL_FOR_PAINTER זה מקסימום פיקסלים אפשריים
    // רקע צהוב עם קצוות עגולים
    int mx = 10, my = 10;
    int mw = 200, mh = 120;
    int radius = 8;

    M5.Lcd.fillRoundRect(mx, my, mw, mh,
                         radius, YELLOW);
    M5.Lcd.drawRoundRect(mx, my, mw, mh,
                         radius, BLACK);

    // כותרת רקע
    M5.Lcd.fillRoundRect(mx + 1, my + 1, mw - 2, 14,
                         radius, 0xFD20); // כתום כהה
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(mx + 6, my + 4);
    M5.Lcd.print("Paint Menu");

    int rowH = 13;
    int startY = my + 18;

    for (size_t i = 0; i < FirstmenuItems.size(); i++)
    {
        int rowY = startY + i * rowH;

        if (menuSelection == i)
        {
            // שורה נבחרת - כתום עם קצוות עגולים
            M5.Lcd.fillRoundRect(mx + 3, rowY - 1,
                                 mw - 6, rowH, 4,
                                 0xFD20); // ORANGE
            M5.Lcd.setTextColor(WHITE);
        }
        else
        {
            M5.Lcd.setTextColor(BLACK);
        }

        M5.Lcd.setCursor(mx + 8, rowY + 2);
        M5.Lcd.print(FirstmenuItems[i]);

        if (i == 1) // Pic W
        {
            M5.Lcd.setCursor(mx + 120, rowY + 2);
            M5.Lcd.print(String(PicSizeX));
        }
        if (i == 2) // Pic H
        {
            M5.Lcd.setCursor(mx + 120, rowY + 2);
            M5.Lcd.print(String(PicSizeY));
        }
    }
}
 */

void Painter::DrawFirstMenu()
{
    int mx = 10, my = 10;
    int mw = 200, mh = 140; // הגדלנו את הגובה ב-20 לפרוגרס בר
    int radius = 8;
    M5.Lcd.fillRoundRect(mx, my, mw, mh,
                         radius, YELLOW);
    M5.Lcd.drawRoundRect(mx, my, mw, mh,
                         radius, BLACK);

    // כותרת רקע
    M5.Lcd.fillRoundRect(mx + 1, my + 1, mw - 2, 14,
                         radius, 0xFD20);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(mx + 6, my + 4);
    // M5.Lcd.unloadFont();
    M5.Display.setFont(nullptr);

    M5.Lcd.print("Paint Menu");

    int rowH = 13;
    int startY = my + 18;

    for (size_t i = 0; i < FirstmenuItems.size(); i++)
    {
        int rowY = startY + i * rowH;

        if (menuSelection == i)
        {
            M5.Lcd.fillRoundRect(mx + 3, rowY - 1,
                                 mw - 6, rowH, 4,
                                 0xFD20);
            M5.Lcd.setTextColor(WHITE);
        }
        else
        {
            M5.Lcd.setTextColor(BLACK);
        }

        M5.Lcd.setCursor(mx + 8, rowY + 2);
        M5.Lcd.print(FirstmenuItems[i]);

        if (i == 1)
        {
            M5.Lcd.setCursor(mx + 120, rowY + 2);
            M5.Lcd.print(String(PicSizeX));
        }
        if (i == 2)
        {
            M5.Lcd.setCursor(mx + 120, rowY + 2);
            M5.Lcd.print(String(PicSizeY));
        }
        /*         if (i == 3) // Preset Size
                {
                    M5.Lcd.setCursor(mx + 100, rowY + 2);
                    M5.Lcd.print(imagePresets[selectedPreset].name);
                } */
        if (i == 3) // Preset row
        {
            M5.Lcd.setCursor(mx + 80, rowY + 2);
            if (selectedPreset >= 0)
                M5.Lcd.print(imagePresets[selectedPreset].name);
            else
                M5.Lcd.print("Custom");
        }
    }

    // ===== PROGRESS BAR =====
    int barX = mx + 4;
    int barY = my + mh - 35;
    int barW = mw - 8;
    int barH = 10;
    int barRadius = 3;
    int currentPixelCount = PicSizeX * PicSizeY;
    // חישוב אחוז שימוש
    float usage = (float)currentPixelCount / (float)MAX_PIXEL_FOR_PAINTER;
    if (usage > 1.0f)
        usage = 1.0f;
    bool overLimit = (currentPixelCount > MAX_PIXEL_FOR_PAINTER);

    // רקע אפור לבר
    M5.Lcd.fillRoundRect(barX, barY, barW, barH,
                         barRadius, TFT_DARKGREY);

    // מילוי הבר בצבע מתאים
    int fillW = (int)(barW * usage);
    if (fillW > 0)
    {
        uint16_t barColor = overLimit ? TFT_RED : TFT_GREEN;
        M5.Lcd.fillRoundRect(barX, barY, fillW, barH,
                             barRadius, barColor);
    }

    // מסגרת שחורה
    M5.Lcd.drawRoundRect(barX, barY, barW, barH,
                         barRadius, BLACK);

    // טקסט אחוזים מתחת לבר
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(1);

    int percent = (int)(usage * 100);
    String pctText = String(percent) + "%";

    M5.Lcd.setCursor(barX, barY + barH + 2);
    M5.Lcd.print("Max Pixel Usage: " + pctText);
}

void Painter::DrawMenu()
{
    if (inFirstMenu)
    {
        return;
    }
    int menuW = 180;
    int menuH = 135;
    int menuX = 20;
    int menuY = 2;

    M5.Lcd.fillRect(menuX, menuY, menuW, menuH, TFT_DARKGREY);
    M5.Lcd.drawRect(menuX, menuY, menuW, menuH, TFT_WHITE);
    M5.Lcd.drawRect(menuX + 2, menuY + 2, menuW - 4, menuH - 4, TFT_BLACK);

    // header
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.setTextSize(1);
    // M5.Display.unloadFont();
    M5.Display.setFont(nullptr);
    M5.Lcd.setCursor(menuX + 10, menuY + 8);
    M5.Lcd.print("--- PAINTER MENU ---");

    String options[] = {
        "Brush Size(+/-): " + String(brushSize + 1),
        "Shape: " + getShapeName(shapeMode),
        "Fill Mode: " + String(shapeFilled ? "[ON]" : "[OFF]"),
        "Outline All",
        "Current Color(P): ",
        "Enter Word (W)",
        "Change Canvas Size",
        "Save  Image (S)",
        "Save As New Image",
        "Save And Quit Image",
        "Reload Picture From File (A)",
        "Exit"};

    for (int i = 0; i < visibleItems; i++)
    {
        int itemIndex = scrollOffset + i;
        if (itemIndex >= menuItemsCount)
            break;

        int itemY = menuY + 22 + (i * 14);

        if (itemIndex == menuSelection)
        {
            M5.Lcd.fillRect(menuX + 5, itemY - 2, menuW - 10, 12, TFT_BLUE);
            M5.Lcd.setTextColor(TFT_WHITE);
        }
        else
        {
            M5.Lcd.setTextColor(TFT_LIGHTGREY);
        }

        if (itemIndex == 4)
        {
            int swatchX = menuX + 110;
            M5.Lcd.fillRect(swatchX, itemY - 1, 15, 9, currentColor);
            M5.Lcd.drawRect(swatchX - 1, itemY - 2, 17, 11, TFT_WHITE);
        }

        M5.Lcd.setCursor(menuX + 10, itemY);
        M5.Lcd.print(options[itemIndex]);
    }

    M5.Lcd.setTextColor(TFT_CYAN);
    M5.Lcd.setCursor(menuX + 10, menuY + menuH - 12);
    M5.Lcd.print("Arrows to move");
}

String Painter::getShapeName(ShapeMode mode)
{
    switch (mode)
    {
    case NONE:
        return "Free Hand (B)";
    case LINE_MODE:
        return "Line (L)";
    case TRIANGLE_MODE:
        return "Triangle (T)";
    case CIRCLE_MODE:
        return "Circle (C)";
    case RECT_MODE:
        return "Rectangle (R)";
    case ELLIPSE_MODE:
        return "Ellipse (E)";
    case BUCKET_MODE:
        return "Bucket (G)";
    case EYEDROPPER_MODE:
        return "Eyedropper (I)";
    case OUTLINE_MODE:
        return "Outline Around Color (I)";
    default:
        return "Unknown";
    }
}

void Painter::ShowQuickMessege(String Messege, int TickTime, int Xoffset)
{
    messegeDebounce = TickTime;
    M5Cardputer.Display.fillRect(20 + Xoffset, 3, 200, 25, YELLOW);
    M5Cardputer.Display.drawRect(20 + Xoffset, 3, 200, 25, BLACK);
    M5Cardputer.Display.setCursor(25 + Xoffset, 7);
    M5Cardputer.Display.unloadFont();
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(BLACK);
    M5Cardputer.Display.print(Messege);
}

void Painter::ShowQickColorOnScreen(uint16_t color)
{
    messegeDebounce = 8000;
    M5Cardputer.Display.fillRect(180, 3, 50, 50, color);
    M5Cardputer.Display.drawRect(180, 3, 50, 50, BLACK);
}

void Painter::ReloadPicture()
{
    if (!PaintFromFile)
    {
        updateSideBar = true;
        Draw();

        ShowQuickMessege("The paint Not Save In Any File");
    }
    else
    {
        menuOpen = false;

        LoadBMP_To_PixelsArray_var(mainOS->FileSelectedInFS);
        //  cursorX = PicSizeX / 2;
        // cursorY = PicSizeY / 2;
        updateSideBar = true;
        Draw();
        ShowQuickMessege("Reload The Image");
    }
}

int Painter::GetMoveTickDelay()
{
    if (Zoomlevel > 1)
    {
        return Zoomlevel * 10;
    }
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_FN))
    {
        return 80;
    }
    return 12;
}

void Painter::ChangeCanvasSize(bool up, bool down, bool right, bool left, int amount)
{
    int newSizeX = PicSizeX + (right || left ? amount : 0);
    int newSizeY = PicSizeY + (up || down ? amount : 0);

    if (newSizeX * newSizeY > MAX_PIXEL_FOR_PAINTER || newSizeX <= 0 || newSizeY <= 0)
    {
        ImageTooBigToMemory=true;
        return;
    }

    uint16_t *newPixels = new uint16_t[newSizeX * newSizeY];
    for (int i = 0; i < newSizeX * newSizeY; i++)
        newPixels[i] = WHITE;

    // expand: offset חיובי = התמונה זזה פנימה
    // crop: offset 0 = חותכים מהצד המבוקש
    int offsetX = 0;
    int offsetY = 0;

    if (left)
        offsetX = amount; // expand שמאל: תמונה זזה ימינה / crop שמאל: שלילי = חיתוך עמודות ראשונות
    if (up)
        offsetY = amount; // expand למעלה: תמונה זזה למטה / crop למעלה: שלילי = חיתוך שורות ראשונות

    for (int row = 0; row < PicSizeY; row++)
    {
        int newRow = row + offsetY;
        if (newRow < 0 || newRow >= newSizeY)
            continue;

        for (int col = 0; col < PicSizeX; col++)
        {
            int newCol = col + offsetX;
            if (newCol < 0 || newCol >= newSizeX)
                continue;

            newPixels[newRow * newSizeX + newCol] = PixelsArray[row * PicSizeX + col];
        }
    }

    memcpy(PixelsArray, newPixels, newSizeX * newSizeY * sizeof(uint16_t));
    delete[] newPixels;

    PicSizeX = newSizeX;
    PicSizeY = newSizeY;
}
/*
void Painter::ChangeCanvasSize(bool up, bool down, bool right, bool left, int amount)
{

    // first cheack if the pixel size not higher from MAX_PIXEL_FOR_PAINTER
    // if more DrawMessege("cant change canvas that much it extent the cardputer RAM");,return;

    if (up || down)
    {
        PicSizeY += amount;
    }
    else if (right || left)
    {
        PicSizeX += amount;
    }

    // uint16_t PixelsArray[MAX_PIXEL_FOR_PAINTER]; // variable thet contain the corrent picture pixels for the cardputer RGB565
    // need to calculate by the upscale
    if (up)
    {
        // move all the picture down pixel by "amount" and fill the pixel that we add to value WHITE
    }
    else if (down)
    {
        // only fill the pixel we added to WHITE
    }
    else if (right)
    {
        // only fill the pixel we added to WHITE
    }
    else if (left)
    {
        // move all the picture right pixel by "amount" and fill the pixel that we add from left to value WHITE
    }
    //need to calculate the negetive like amount =-20 crop the pixels
} */

void Painter::LoadBMP_To_PixelsArray_var(String BMP_PATH)
{

    File bmpFile = SD.open(BMP_PATH.c_str(), FILE_READ);
    if (!bmpFile)
    {
        Serial.println("Error: Could not open file");
        return;
    }

    uint8_t header[54];
    bmpFile.read(header, 54);

    int32_t width = *(int32_t *)&header[18];
    int32_t height = *(int32_t *)&header[22];

    PicSizeX = abs(width);
    PicSizeY = abs(height);
    if (PicSizeX * PicSizeY > MAX_PIXEL_FOR_PAINTER)
    {
        Serial.println("Error: Image too large for the Cardputer Memory!");
        bmpFile.close();
        return;
    }

    bool flip = true;
    if (height < 0)
    {
        height = -height;
        flip = false;
    }

    int rowSize = (width * 3 + 3) & ~3;

    uint8_t lineBuffer[width * 3];

    for (int y = 0; y < PicSizeY; y++)
    {
        int pos = flip ? (height - 1 - y) : y;

        bmpFile.seek(54 + (pos * rowSize));
        bmpFile.read(lineBuffer, width * 3);

        for (int x = 0; x < PicSizeX; x++)
        {
            uint8_t b = lineBuffer[x * 3];
            uint8_t g = lineBuffer[x * 3 + 1];
            uint8_t r = lineBuffer[x * 3 + 2];

            uint16_t pixelColor = ((r & 0xF8) << 8) |
                                  ((g & 0xFC) << 3) |
                                  ((b & 0xF8) >> 3);

            PixelsArray[y * PicSizeX + x] = pixelColor;
        }
    }

    bmpFile.close();
}

int pngToPixelsArrayCallback(PNGDRAW *pDraw)
{
    // מערך זמני לקבלת השורה בפורמט RGB565 (כמו שהצייר שלך מצפה)
    uint16_t lineBuffer[MAX_IMAGE_WIDTH];

    // שליפת השורה מה-PNG והמרה ל-RGB565
    // שים לב: השתמשתי ב-PNG_RGB565_LITTLE_ENDIAN כי רוב המיקרו-בקרים (ESP32)
    // עובדים ככה בזיכרון, אם הצבעים יוצאים הפוכים, שנה ל-BIG_ENDIAN
    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);

    // העתקת השורה למערך הפיקסלים הגלובלי של הצייר
    for (int x = 0; x < pDraw->iWidth; x++)
    {
        // חישוב האינדקס במערך החד-מימדי
        pixelArrayPointer[pDraw->y * pDraw->iWidth + x] = lineBuffer[x];
        // pixelArrayPointer[pDraw->y * PicSizeX + x] = lineBuffer[x];
    }

    return 1;
}

void Painter::SetPixelsArrayWHITE_Blank_canvas()
{
    int totalPixels = PicSizeX * PicSizeY;

    for (int i = 0; i < totalPixels; i++)
    {
        PixelsArray[i] = 0xFFFF;
    }
}

void Painter::DrawBrushToBuffer()
{
    for (int dy = -brushSize; dy <= brushSize; dy++)
    {
        for (int dx = -brushSize; dx <= brushSize; dx++)
        {
            if (dx * dx + dy * dy > brushSize * brushSize)
                continue;

            int px = cursorX + dx;
            int py = cursorY + dy;

            if (px < 0 || px >= PicSizeX ||
                py < 0 || py >= PicSizeY)
                continue;

            PixelsArray[py * PicSizeX + px] = currentColor;
        }
    }
}

void Painter::DrawLineToBuffer(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        // PixelsArray[y0 * PicSizeX + x0] = currentColor;
        DrawThickPixel(x0, y0);
        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void Painter::DrawTriangleToBuffer(int x1, int y1, int x2, int y2, int x3, int y3)
{
    if (!shapeFilled)
    {
        DrawLineToBuffer(x1, y1, x2, y2);
        DrawLineToBuffer(x2, y2, x3, y3);
        DrawLineToBuffer(x3, y3, x1, y1);
    }
    else
    {

        FillTriangleHelper(x1, y1, x2, y2, x3, y3);
    }
}

void Painter::ResetShape()
{
    firstPointSet = false;
    secondPointSet = false;
}

void Painter::SetShapeMode(ShapeMode mode)
{
    updateSideBar = true;

    if (shapeMode == (ShapeMode)EYEDROPPER_MODE)
    {
        if (mode == (ShapeMode)EYEDROPPER_MODE)
        {
            return;
        }
    } // if eyedropper alredy choosen and the user choose again it will failed for after the eydroper pick color it will return ti old shape mode
    oldShapeMode = shapeMode;
    shapeMode = mode;

    firstPointSet = false;
    secondPointSet = false;
}
void Painter::HandleShapeLogic()
{
    if (shapeMode == EYEDROPPER_MODE)
    {
        int index = cursorY * PicSizeX + cursorX;

        if (index >= 0 && index < (PicSizeX * PicSizeY))
        {
            currentColor = PixelsArray[index];

            ShowQuickMessege("Color Picked!");
            ShowQickColorOnScreen(currentColor);
            // todo color rect
            shapeMode = oldShapeMode;
        }
    }
    else if (shapeMode == BUCKET_MODE)
    {
        FillBucket();
        // FloodFill(cursorX, cursorY);
        Draw();
        ShowQuickMessege("Color Filled");
    }
    else if (shapeMode == OUTLINE_MODE)
    {
        menuOpen = false;
        DrawOutlineForColor();
        Draw();
    }
    // from now on two or three choose points
    else if (!firstPointSet)
    {
        x1 = cursorX;
        y1 = cursorY;
        firstPointSet = true;

        Draw(); // preview
        ShowQuickMessege("First Point Selected");
    }
    else if (shapeMode == LINE_MODE)
    {
        DrawLineToBuffer(x1, y1, cursorX, cursorY);
        ResetShape();
        Draw();
    }
    else if (shapeMode == TRIANGLE_MODE)
    {
        if (!secondPointSet)
        {
            x2 = cursorX;
            y2 = cursorY;
            secondPointSet = true;

            Draw();
            ShowQuickMessege("Second Point Selected");
        }
        else
        {
            DrawTriangleToBuffer(x1, y1, x2, y2, cursorX, cursorY);
            ResetShape();
            Draw();
        }
    }
    else if (shapeMode == CIRCLE_MODE)
    {
        int dx = cursorX - x1;
        int dy = cursorY - y1;
        int radius = sqrt(dx * dx + dy * dy);

        DrawCircleToBuffer(x1, y1, radius);
        ResetShape();
        Draw();
    }
    else if (shapeMode == ELLIPSE_MODE)
    {
        if (!firstPointSet)
        {
            x1 = cursorX;
            y1 = cursorY;
            firstPointSet = true;
            Draw();
            ShowQuickMessege("First Point Selected");
        }
        else
        {
            int rx = abs(cursorX - x1);
            int ry = abs(cursorY - y1);

            DrawEllipseToBuffer(x1, y1, rx, ry);
            ResetShape();
            Draw();
        }
    }

    else if (shapeMode == RECT_MODE)
    {

        DrawSquareToBuffer(x1, y1, cursorX, cursorY);
        ResetShape();
        Draw();
    }
}

void Painter::DrawCircleToBuffer(int cx, int cy, int r)
{
    if (r <= 0)
    {
        DrawThickPixel(cx, cy);
        return;
    }

    if (!shapeFilled)
    {
        int x = r, y = 0;
        int err = 1 - x;

        while (x >= y)
        {
            DrawThickPixel(cx + x, cy + y);
            DrawThickPixel(cx + y, cy + x);
            DrawThickPixel(cx - y, cy + x);
            DrawThickPixel(cx - x, cy + y);
            DrawThickPixel(cx - x, cy - y);
            DrawThickPixel(cx - y, cy - x);
            DrawThickPixel(cx + y, cy - x);
            DrawThickPixel(cx + x, cy - y);

            y++;
            if (err < 0)
            {
                err += 2 * y + 1;
            }
            else
            {
                x--;
                err += 2 * (y - x) + 1;
            }
        }
    }
    else
    {

        int originalBrush = brushSize;
        brushSize = 0;

        for (int y = -r; y <= r; y++)
        {
            int x = (int)sqrt(r * r - y * y);
            DrawLineToBuffer(cx - x, cy + y, cx + x, cy + y);
        }

        brushSize = originalBrush;
    }
}

void Painter::PlotCirclePoints(int cx, int cy, int x, int y)
{
    auto plot = [&](int px, int py)
    {
        if (px >= 0 && px < PicSizeX && py >= 0 && py < PicSizeY)
            PixelsArray[py * PicSizeX + px] = currentColor;
    };

    plot(cx + x, cy + y);
    plot(cx - x, cy + y);
    plot(cx + x, cy - y);
    plot(cx - x, cy - y);
    plot(cx + y, cy + x);
    plot(cx - y, cy + x);
    plot(cx + y, cy - x);
    plot(cx - y, cy - x);
}

void Painter::DrawThickPixel(int cx, int cy)
{
    for (int y = -brushSize; y <= brushSize; y++)
    {
        for (int x = -brushSize; x <= brushSize; x++)
        {
            if (x * x + y * y <= brushSize * brushSize)
            {
                int px = cx + x;
                int py = cy + y;

                if (px >= 0 && px < PicSizeX &&
                    py >= 0 && py < PicSizeY)
                {
                    PixelsArray[py * PicSizeX + px] = currentColor;
                }
            }
        }
    }
}

/* void Painter::FloodFill(int startX, int startY)
{
    uint16_t targetColor = PixelsArray[startY * PicSizeX + startX];

    if (targetColor == currentColor)
        return;

    std::vector<std::pair<int, int>> stack;
    stack.push_back({startX, startY});

    while (!stack.empty())
    {
        auto [x, y] = stack.back();
        stack.pop_back();

        int index = y * PicSizeX + x;

        if (PixelsArray[index] != targetColor)
            continue;

        PixelsArray[index] = currentColor;

        if (x > 0)
            stack.push_back({x - 1, y});
        if (x < PicSizeX - 1)
            stack.push_back({x + 1, y});
        if (y > 0)
            stack.push_back({x, y - 1});
        if (y < PicSizeY - 1)
            stack.push_back({x, y + 1});
    }
} */

void Painter::DrawSquareToBuffer(int ax, int ay, int bx, int by)
{
    int xMin = min(ax, bx);
    int xMax = max(ax, bx);
    int yMin = min(ay, by);
    int yMax = max(ay, by);

    if (!shapeFilled)
    {
        DrawLineToBuffer(xMin, yMin, xMax, yMin);
        DrawLineToBuffer(xMin, yMax, xMax, yMax);
        DrawLineToBuffer(xMin, yMin, xMin, yMax);
        DrawLineToBuffer(xMax, yMin, xMax, yMax);
    }
    else
    {
        for (int y = yMin; y <= yMax; y++)
        {
            DrawLineToBuffer(xMin, y, xMax, y);
        }
    }
}

#include <queue>
#include <algorithm>

void Painter::FillBucket()
{
    uint16_t targetColor = PixelsArray[cursorY * PicSizeX + cursorX];

    if (currentColor == targetColor)
        return;

    std::queue<int> q;
    int startX = cursorX;
    int startY = cursorY;

    if (startX < 0 || startX >= PicSizeX || startY < 0 || startY >= PicSizeY)
    {
        return;
    }

    int startIndex = startY * PicSizeX + startX;

    q.push(startIndex);
    PixelsArray[startIndex] = currentColor;

    while (!q.empty())
    {
        int currentIndex = q.front();
        q.pop();

        int x = currentIndex % PicSizeX;
        int y = currentIndex / PicSizeX;

        const int dx[] = {0, 0, 1, -1};
        const int dy[] = {-1, 1, 0, 0};

        for (int i = 0; i < 4; ++i)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx >= 0 && nx < PicSizeX && ny >= 0 && ny < PicSizeY)
            {
                int nIndex = ny * PicSizeX + nx;

                if (PixelsArray[nIndex] == targetColor)
                {
                    PixelsArray[nIndex] = currentColor;
                    q.push(nIndex);
                }
            }
        }
    }
}

bool Painter::saveBMP(String fileName, uint16_t *pixelsArray, int width, int height)
{

    // 1. Check if the SD card is initialized
    if (!SD.begin())
    {
        os.ShowOnScreenMessege("no sd");
        return false;
    }
    if (!PaintFromFile)
    {
        if (SD.exists(fileName.c_str()))
        {
            if (mainOS->AskSomthing("File alredy Exist Writing over the existing file?"))
            {
            }
            else
            {
                return false;
            }
        }
    }
    // Open file for writing (overwrite existing)
    File file = SD.open(fileName, FILE_WRITE);
    if (!file)
    {
        os.ShowOnScreenMessege("eror open file");

        return false;
    }

    // 2. Calculate Row Padding
    // BMP requires each row to be a multiple of 4 bytes (32-bit alignment).
    // Each pixel in RGB888 is 3 bytes.
    int rowBytes = width * 3;
    int padding = (4 - (rowBytes % 4)) % 4;

    // 3. Calculate Total File Size
    // Header (54 bytes) + Data ((RowSize + Padding) * Height)
    uint32_t dataSize = (rowBytes + padding) * height;
    uint32_t fileSize = 54 + dataSize;

    // 4. Prepare BMP Header
    unsigned char header[54] = {
        'B', 'M',    // Signature
        0, 0, 0, 0,  // File Size (filled below)
        0, 0, 0, 0,  // Reserved
        54, 0, 0, 0, // Offset to Pixel Array
        40, 0, 0, 0, // Info Header Size
        0, 0, 0, 0,  // Width (filled below)
        0, 0, 0, 0,  // Height (filled below)
        1, 0,        // Planes
        24, 0,       // Bits per Pixel (RGB888)
        0, 0, 0, 0,  // Compression
        0, 0, 0, 0,  // Image Size (can be 0 for uncompressed)
        0, 0, 0, 0,  // Horizontal Resolution
        0, 0, 0, 0,  // Vertical Resolution
        0, 0, 0, 0,  // Colors Used
        0, 0, 0, 0   // Important Colors
    };

    // Fill Header Fields (Little Endian)
    header[2] = fileSize & 0xFF;
    header[3] = (fileSize >> 8) & 0xFF;
    header[4] = (fileSize >> 16) & 0xFF;
    header[5] = (fileSize >> 24) & 0xFF;

    header[18] = width & 0xFF;
    header[19] = (width >> 8) & 0xFF;
    header[20] = (width >> 16) & 0xFF;
    header[21] = (width >> 24) & 0xFF;

    // Note: Height is stored as positive for "Bottom-Up" BMP format.
    // We will write the data from bottom to top.
    header[22] = height & 0xFF;
    header[23] = (height >> 8) & 0xFF;
    header[24] = (height >> 16) & 0xFF;
    header[25] = (height >> 24) & 0xFF;

    // Write Header
    file.write(header, 54);

    // Buffer for one row of RGB888 data + padding
    // Max width is usually small enough to fit in stack, but if >1920 it might be tight.
    // Assuming typical screen sizes, this is safe. If needed, allocate on heap.
    int bufferSize = (width * 3) + padding;
    unsigned char *rowBuffer = new unsigned char[bufferSize];

    if (!rowBuffer)
    {
        file.close();
        os.ShowOnScreenMessege("33");

        return false;
    }

    // 5. Convert RGB565 to RGB888 and Write Rows (Bottom-Up)
    for (int y = height - 1; y >= 0; y--)
    {
        int rowOffset = y * width;

        // Clear padding area just in case
        memset(rowBuffer, 0, bufferSize);

        for (int x = 0; x < width; x++)
        {
            uint16_t pixel565 = pixelsArray[rowOffset + x];

            // Extract RGB from 565 format
            // R: bits 15-11 (shift right 11) -> *8 to get 8-bit
            int r = ((pixel565 >> 11) & 0x1F);
            // G: bits 10-5 (mask 0x3F, shift 5) -> *4 to get 8-bit
            int g = ((pixel565 >> 5) & 0x3F);
            // B: bits 4-0 (mask 0x1F) -> *8 to get 8-bit
            int b = (pixel565 & 0x1F);

            // Expand to 8-bit precision (simple scaling by multiplying)
            r = (r << 3) | (r >> 2); // 5 bits -> 8 bits
            g = (g << 2) | (g >> 4); // 6 bits -> 8 bits
            b = (b << 3) | (b >> 2); // 5 bits -> 8 bits

            // BMP stores as BGR, so we write Blue first, then Green, then Red
            rowBuffer[x * 3] = b;
            rowBuffer[x * 3 + 1] = g;
            rowBuffer[x * 3 + 2] = r;
        }

        // Write the row (including padding) to file
        if (!file.write(rowBuffer, bufferSize))
        {
            delete[] rowBuffer;
            file.close();

            return false;
        }
    }

    delete[] rowBuffer;
    file.close();
    return true;
}

void Painter::DrawEllipseToBuffer(int x0, int y0, int rx, int ry)
{
    if (rx <= 0 || ry <= 0)
    {
        DrawThickPixel(x0, y0);
        return;
    }
    if (!shapeFilled)
    {
        long x = -rx, y = 0; /* II. quadrant */
        long e2 = ry, dx = (1 + 2 * x) * e2 * e2;
        long dy = x * x, err = dx + dy;

        do
        {
            DrawThickPixel(x0 - x, y0 + y); /* I. Quadrant */
            DrawThickPixel(x0 + x, y0 + y); /* II. Quadrant */
            DrawThickPixel(x0 + x, y0 - y); /* III. Quadrant */
            DrawThickPixel(x0 - x, y0 - y); /* IV. Quadrant */

            e2 = 2 * err;
            if (e2 >= dx)
            {
                x++;
                err += dx += 2 * (long)ry * ry;
            }
            if (e2 <= dy)
            {
                y++;
                err += dy += 2 * (long)rx * rx;
            }
        } while (x <= 0);

        while (y++ < ry)
        {
            DrawThickPixel(x0, y0 + y);
            DrawThickPixel(x0, y0 - y);
        }
    }
    else
    {
        for (int y = -ry; y <= ry; y++)
        {
            int x = sqrt(1.0 - (double)(y * y) / (ry * ry)) * rx;
            DrawLineToBuffer(x0 - x, y0 + y, x0 + x, y0 + y);
        }
    }
}
void Painter::FillTriangleHelper(int x1, int y1, int x2, int y2, int x3, int y3)
{
    if (y1 > y2)
    {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }
    if (y1 > y3)
    {
        std::swap(y1, y3);
        std::swap(x1, x3);
    }
    if (y2 > y3)
    {
        std::swap(y2, y3);
        std::swap(x2, x3);
    }

    if (y1 == y3)
        return;

    int originalBrush = brushSize;
    brushSize = 0;

    int x4 = x1 + (int)((float)(y2 - y1) / (y3 - y1) * (x3 - x1));

    FillFlatTriangle(x1, y1, x2, y2, x4, y2);

    FillFlatTriangle(x3, y3, x2, y2, x4, y2);

    brushSize = originalBrush;
}
void Painter::FillFlatTriangle(int v1x, int v1y, int v2x, int v2y, int v3x, int v3y)
{
    float invslope1 = (float)(v2x - v1x) / (v2y - v1y);
    float invslope2 = (float)(v3x - v1x) / (v3y - v1y);

    float curx1 = v1x;
    float curx2 = v1x;

    if (v1y < v2y)
    {
        for (int scanlineY = v1y; scanlineY <= v2y; scanlineY++)
        {
            DrawLineToBuffer((int)curx1, scanlineY, (int)curx2, scanlineY);
            curx1 += invslope1;
            curx2 += invslope2;
        }
    }
    else
    {
        for (int scanlineY = v1y; scanlineY > v2y; scanlineY--)
        {
            DrawLineToBuffer((int)curx1, scanlineY, (int)curx2, scanlineY);
            curx1 -= invslope1;
            curx2 -= invslope2;
        }
    }
}

void Painter::DrawTextToBuffer(String text, int x, int y, uint8_t size)
{
    if (text.length() == 0)
        return;

    LGFX_Sprite tempSprite(&M5.Lcd);

    tempSprite.setTextSize(size);
    int textW = tempSprite.textWidth(text) + 2;
    int textH = 8 * size;

    if (!tempSprite.createSprite(textW, textH))
        return;

    tempSprite.fillSprite(0x0000);
    tempSprite.setTextColor(0xFFFF);
    tempSprite.setCursor(0, 0);
    tempSprite.print(text);

    for (int iY = 0; iY < textH; iY++)
    {
        for (int iX = 0; iX < textW; iX++)
        {

            uint16_t pixel = tempSprite.readPixel(iX, iY);

            if (pixel != 0x0000)
            {
                int targetX = x + iX;
                int targetY = y + iY;

                if (targetX >= 0 && targetX < PicSizeX && targetY >= 0 && targetY < PicSizeY)
                {
                    PixelsArray[targetY * PicSizeX + targetX] = currentColor;
                }
            }
        }
    }

    tempSprite.deleteSprite();
}

void Painter::HandleMenuInput()
{
    bool changed = false;

    // DOWN
    if (mainOS->NewKey.ifKeyJustPress('.'))
    {
        menuSelection++;
        if (inFirstMenu)
        {
            if (menuSelection >= FirstmenuItems.size())
                menuSelection = 0; // wrap to start
        }
        else
        {
            if (menuSelection >= menuItemsCount)
                menuSelection = 0; // wrap to start
        }
        changed = true;
    }

    // UP
    if (mainOS->NewKey.ifKeyJustPress(';'))
    {
        menuSelection--;
        if (inFirstMenu)
        {
            if (menuSelection < 0)
                menuSelection = FirstmenuItems.size() - 1; // wrap
        }
        else
        {
            if (menuSelection < 0)
                menuSelection = menuItemsCount - 1; // wrap
        }
        changed = true;
    }

    if (mainOS->NewKey.ifKeyJustPress('/')) // right
    {
        /*        if (inFirstMenu && menuSelection == 3)
               {
                   selectedPreset = (selectedPreset + 1) % imagePresets.size();
                   PicSizeX = imagePresets[selectedPreset].size_x;
                   PicSizeY = imagePresets[selectedPreset].size_y;
                   Zoomlevel = imagePresets[selectedPreset].zoom;
                   changed = true;
                   return;
               } */
        switch (menuSelection)
        {
        case 0:
            brushSize = (brushSize + 1) % 10;
            changed = true;

            break;

        case 1:
            shapeMode = (ShapeMode)((shapeMode + 1) % 8);
            changed = true;

            break;
        }
    }
    // LEFT
    if (mainOS->NewKey.ifKeyJustPress(','))
    {

        switch (menuSelection)
        {
        case 0:
            brushSize = (brushSize - 1) % 10;
            if (brushSize < 0)
            {
                brushSize = 0;
            }
            changed = true;

            break;
        case 1:
            shapeMode = (ShapeMode)((shapeMode - 1) % 8);
            changed = true;
            if ((ShapeMode)(shapeMode) < 0)
            {
                shapeMode = (ShapeMode)0;
            }
            break;
        }
    }

    if (!changed)
        return;

    // update scroll
    if (menuSelection < scrollOffset)
        scrollOffset = menuSelection;

    if (menuSelection >= scrollOffset + visibleItems)
        scrollOffset = menuSelection - visibleItems + 1;
    Draw();
    DrawMenu();
}

void Painter::SavePic(bool Exit, bool forceNewFile)
{
    menuOpen = false;
    if (PaintFromFile && forceNewFile == false)
    {
        bool success = saveBMP(mainOS->FileSelectedInFS.c_str(), PixelsArray, PicSizeX, PicSizeY);
        if (success)
        {
            updateSideBar = true;
            Draw();
            ShowQuickMessege(String(mainOS->FileSelectedInFS + " Saved"));
            if (Exit)
            {
                mainOS->ChangeMenu(new MainMenuV2(mainOS));
                return;
            }
        }
    }
    else
    {
        SD.mkdir("/AdvanceOS/Paint"); // folder for the theme
        String Name = "/AdvanceOS/Paint/";
        String input = mainOS->AskFromUserForString("Enter File Name");
        Name += input;
        Name += ".bmp";
        bool success = saveBMP(Name.c_str(), PixelsArray, PicSizeX, PicSizeY);
        if (success)
        {
            updateSideBar = true;
            Draw();
            ShowQuickMessege("save Compleate in AdvanceOS/Paint");
            //  mainOS->ChangeMenu(new MainMenu(mainOS));
            // return;
            PaintFromFile = true;
            mainOS->FileSelectedInFS = Name;
            if (Exit)
            {
                mainOS->ChangeMenu(new MainMenuV2(mainOS));
                return;
            }
        }
    }
}

///////////////////////////outline sector

// פונקציה עזר לביצועים: בודקת אם יש צבע לא לבן (רקע)
bool Painter::IsPixelNotWhite(int x, int y)
{
    if (x < 0 || x >= PicSizeX || y < 0 || y >= PicSizeY)
        return false;
    int index = y * PicSizeX + x;
    return PixelsArray[index] != 0xFFFF;
}

void Painter::DrawAutoOutline()
{
    uint16_t whiteColor = 0xFFFF; // הגדרה של לבן ב-RGB565

    // יצירת עותק זמני כדי לא "לזהות" את ה-Outline החדש תוך כדי ריצה
    uint16_t *tempArray = new uint16_t[PicSizeX * PicSizeY];
    if (!tempArray)
    {
        Serial.println("Error: Could not allocate memory for temp array");
        return; // הגנה מפני קריסה במקרה של חוסר בזיכרון
    }
    memcpy(tempArray, PixelsArray, PicSizeX * PicSizeY * sizeof(uint16_t));

    // אנו עוברים על כל פיקסל בתמונה
    for (int y = 0; y < PicSizeY; y++)
    {
        for (int x = 0; x < PicSizeX; x++)
        {
            int index = y * PicSizeX + x;

            // בדיקה אם הפיקסל הנוכחי בעותק המקורי הוא לבן
            if (tempArray[index] == whiteColor)
            {
                bool hasColoredNeighbor = false;

                // בדיקת שכנים (8 כיוונים - כולל אלכסונים, לקבלת outline מלא יותר)
                int dx[] = {0, 0, 1, -1, 1, 1, -1, -1};
                int dy[] = {1, -1, 0, 0, 1, -1, 1, -1};

                for (int i = 0; i < 8; i++)
                {
                    int nx = x + dx[i];
                    int ny = y + dy[i];

                    if (nx >= 0 && nx < PicSizeX && ny >= 0 && ny < PicSizeY)
                    {
                        uint16_t neighborColor = tempArray[ny * PicSizeX + nx];
                        if (neighborColor != whiteColor)
                        {
                            hasColoredNeighbor = true;
                            break; // מצאנו שכן צבעוני, אין צורך להמשיך לבדוק
                        }
                    }
                }

                // אם הפיקסל לבן ויש לו שכן צבעוני, נצייר "פיקסל עבה"
                if (hasColoredNeighbor)
                {
                    // שימוש בפונקציה הקיימת שמציירת עיגול בגודל brushSize
                    DrawThickPixel(x, y);
                }
            }
        }
    }

    // שחרור הזיכרון הזמני
    delete[] tempArray;
}

void Painter::DrawOutlineForColor()
{
    // 1. דגימת הצבע שנמצא כרגע תחת הסמן
    int startIndex = cursorY * PicSizeX + cursorX;
    if (startIndex < 0 || startIndex >= (PicSizeX * PicSizeY))
        return;

    uint16_t colorToOutline = PixelsArray[startIndex];

    // 2. יצירת עותק זמני של התמונה כדי למנוע "זליגה" של ה-Outline תוך כדי הציור
    uint16_t *tempArray = new uint16_t[PicSizeX * PicSizeY];
    if (!tempArray)
    {
        Serial.println("Memory Error!");
        return;
    }
    memcpy(tempArray, PixelsArray, PicSizeX * PicSizeY * sizeof(uint16_t));

    // 3. מעבר על כל התמונה וחיפוש הגבולות של הצבע שדגמנו
    for (int y = 0; y < PicSizeY; y++)
    {
        for (int x = 0; x < PicSizeX; x++)
        {
            int index = y * PicSizeX + x;

            // אנחנו מחפשים פיקסל שהוא *לא* צבע המטרה (כאן נצייר את המסגרת)
            if (tempArray[index] != colorToOutline)
            {
                bool isBorderingTargetColor = false;

                // בדיקת 8 שכנים
                int dx[] = {0, 0, 1, -1, 1, 1, -1, -1};
                int dy[] = {1, -1, 0, 0, 1, -1, 1, -1};

                for (int i = 0; i < 8; i++)
                {
                    int nx = x + dx[i];
                    int ny = y + dy[i];

                    if (nx >= 0 && nx < PicSizeX && ny >= 0 && ny < PicSizeY)
                    {
                        if (tempArray[ny * PicSizeX + nx] == colorToOutline)
                        {
                            isBorderingTargetColor = true;
                            break;
                        }
                    }
                }

                // אם מצאנו פיקסל "חיצוני" שנוגע בצבע המטרה - נצייר Outline בעובי המברשת
                if (isBorderingTargetColor)
                {
                    DrawThickPixel(x, y);
                }
            }
        }
    }

    delete[] tempArray;
}

bool Painter::WorldToScreen(int imgX, int imgY, int &screenX, int &screenY)
{
    int displayW = 240;
    int displayH = 135;

    if (Zoomlevel == 1)
    {
        int camX = cursorX - displayW / 2;
        int camY = cursorY - displayH / 2;

        screenX = imgX - camX;
        screenY = imgY - camY;
    }
    else
    {
        int viewW = displayW / Zoomlevel;
        int viewH = displayH / Zoomlevel;

        int CameraX = cursorX - viewW / 2;
        int CameraY = cursorY - viewH / 2;

        screenX = (imgX - CameraX) * Zoomlevel;
        screenY = (imgY - CameraY) * Zoomlevel;
    }

    // אם מחוץ למסך
    if (screenX < 0 || screenY < 0 || screenX >= displayW || screenY >= displayH)
        return false;

    return true;
}
int gif_load_scale = 1; // חדש: 1=מלא, 2=חצי, 4=רבע
void GIFToPixelsArrayCallback(GIFDRAW *pDraw)
{
    uint16_t *palette = pDraw->pPalette;
    uint8_t *s = pDraw->pPixels;
    int scale = gif_load_scale;

    int srcY = pDraw->iY + pDraw->y;
    int startX = pDraw->iX;

    // טיפול נכון ב-disposal=2
    if (pDraw->ucDisposalMethod == 2)
    {
        for (int x = 0; x < pDraw->iWidth; x++)
        {
            if (s[x] == pDraw->ucTransparent)
                s[x] = pDraw->ucBackground;
        }
        pDraw->ucHasTransparency = 0;
    }

    // טעינה למערך פיקסלים
    if (pixelArrayPointer != nullptr && srcY % scale == 0)
    {
        int dstY = srcY / scale;

        for (int x = 0; x < pDraw->iWidth; x += scale)
        {
            if (pDraw->ucHasTransparency && s[x] == pDraw->ucTransparent)
                continue;

            int dstX = (startX + x) / scale;
            int targetIndex = dstY * PicSizeX + dstX;

            if (targetIndex >= 0 && targetIndex < PicSizeX * PicSizeY)
                pixelArrayPointer[targetIndex] = palette[s[x]];
        }
    }

    // ציור למסך
    int step = 1 << pic_zoom_out;
    if (srcY % step != 0)
        return;

    int drawY = 12 + (srcY / step);
    if (drawY < 0 || drawY >= SCREEN_H)
        return;

    int baseX = pDraw->iX / step;

    if (pDraw->ucHasTransparency)
    {
        uint8_t ucTransparent = pDraw->ucTransparent;
        for (int i = 0; i < pDraw->iWidth; i += step)
        {
            int drawX = baseX + (i / step);
            if (drawX < 0 || drawX >= SCREEN_W) continue;
            if (s[i] == ucTransparent) continue;
            M5Cardputer.Display.drawPixel(drawX, drawY, palette[s[i]]);
        }
    }
    else
    {
        for (int i = 0; i < pDraw->iWidth; i += step)
        {
            int drawX = baseX + (i / step);
            if (drawX < 0 || drawX >= SCREEN_W) continue;
            M5Cardputer.Display.drawPixel(drawX, drawY, palette[s[i]]);
        }
    }
}
void Painter::LoadGIF_To_PixelsArray(String path)
{
    SetPixelsArrayWHITE_Blank_canvas();

    gif.open(path.c_str(),
             GIFOpenFile, GIFCloseFile,
             GIFReadFile, GIFSeekFile,
             GIFToPixelsArrayCallback);

    int rawW = gif.getCanvasWidth();
    int rawH = gif.getCanvasHeight();

    // --- חישוב scale ---
    gif_load_scale = 1;
    if ((long)rawW * rawH > MAX_PIXEL_FOR_PAINTER)
        gif_load_scale = 2;
    if ((long)(rawW / 2) * (rawH / 2) > MAX_PIXEL_FOR_PAINTER)
        gif_load_scale = 4;

    // גודל התמונה הסופי אחרי הקטנה
    PicSizeX = rawW / gif_load_scale;
    PicSizeY = rawH / gif_load_scale;

    // אם גם ברבע עדיין גדול מדי — לא ניתן לטעון
    if ((long)PicSizeX * PicSizeY > MAX_PIXEL_FOR_PAINTER)
    {
        mainOS->ShowOnScreenMessege("GIF too large even at 1/4 size!", 1000);
        gif.close();
        gifFile.close();
        mainOS->ChangeMenu(new FileBrowser(mainOS));
        return;
    }

    if (gif_load_scale > 1)
    {
        String msg = String("GIF large, loading at 1/") +
                     String(gif_load_scale * gif_load_scale) +
                     String(" size (") + String(PicSizeX) +
                     String("x") + String(PicSizeY) + String(")");
        mainOS->ShowOnScreenMessege(msg, 1500);
    }

    M5Cardputer.Display.fillScreen(BLACK);
    gif.playFrame(true, NULL);
    M5Cardputer.Display.setCursor(0, 2);
    M5Cardputer.Display.setTextColor(BLACK, WHITE);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.unloadFont();
    M5Cardputer.Display.print("PRESS -> to next frame,\n Press ENTER to choose this frame\n Press ESC to Exit");

    bool Finished = false;
    while (!Finished)
    {

        M5Cardputer.update();

        if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
        {
            Finished = true;
        }
        if (mainOS->NewKey.ifKeyJustPress('`'))
        {
            mainOS->ChangeMenu(new FileBrowser(mainOS));
            Finished = true;
            gif.close();
            gifFile.close();
            return;
        }
        if (mainOS->NewKey.ifKeyJustPress('/')) // next frame

        {
            SetPixelsArrayWHITE_Blank_canvas();
            M5Cardputer.Display.fillScreen(BLACK);

            if (!gif.playFrame(true, NULL))
            { // re open the gif
                gif.open(
                    path.c_str(),
                    GIFOpenFile,
                    GIFCloseFile,
                    GIFReadFile,
                    GIFSeekFile,
                    GIFToPixelsArrayCallback // ה-Callback החדש שלנו
                );
                // gif.playFrame(true, NULL);
            }
            else
            {
            }
            M5Cardputer.Display.setCursor(0, 2);

            M5Cardputer.Display.print("PRESS -> to next frame,\n Press ENTER to choose this frame\n Press ESC to Exit");
        }
    }

    gif.close();
    //  mainOS->ShowOnScreenMessege("3",100);

    gifFile.close(); // סגירת הקובץ הפיזי ב-SD
                     // mainOS->ShowOnScreenMessege("4",100);

    pixelArrayPointer = nullptr; // איפוס לביטחון
                                 // mainOS->ShowOnScreenMessege("5",100);

    // mainOS->ShowOnScreenMessege("GIF frame 0 loaded to PixelsArray");
}

void Painter::LoadPNG_To_PixelsArray(String PNG_PATH)
{
    // 1. פתיחת הקובץ באמצעות הפונקציות הקיימות שלך
    int16_t rc = png.open(
        PNG_PATH.c_str(),
        pngOpen,
        pngClose,
        pngRead,
        pngSeek,
        pngToPixelsArrayCallback // שימוש ב-Callback החדש שלנו
    );

    if (rc != PNG_SUCCESS)
    {
        mainOS->ShowOnScreenMessege("Error: Could not open PNG");
        return;
    }

    // 2. עדכון מימדי התמונה בצייר
    PicSizeX = png.getWidth();
    PicSizeY = png.getHeight();

    // 3. בדיקת זיכרון (חשוב מאוד!)
    if (PicSizeX * PicSizeY > MAX_PIXEL_FOR_PAINTER)
    {
        mainOS->ShowOnScreenMessege("Error: PNG too large for Cardputer RAM!");
        png.close();
        return;
    }

    // 4. פענוח התמונה (זה מפעיל את ה-Callback לכל שורה)
    png.decode(NULL, 0);

    // 5. סגירת הקובץ
    png.close();
    // mainOS->ShowOnScreenMessege("PNG loaded successfully to PixelsArray");
}

void Painter::DrawSidePanels()
{
    updateSideBar = false;
    int screenH = 134; // M5.Lcd.height()
    int screenW = 240; // M5.Lcd.width()

    bool toolsActive = (uiState == UI_PANEL_SELECT && activePanel == PANEL_TOOLS);
    bool paletteActive = (uiState == UI_PANEL_SELECT && activePanel == PANEL_PALETTE);

    // ── פאנל שמאל: כלים ──────────────────────────────────────────
    M5.Lcd.fillRect(0, 0, TOOL_PANEL_W, screenH, BACK_COLOR); // left side panel color

    static const ShapeMode TOOL_ORDER[] = {
        NONE, LINE_MODE,
        CIRCLE_MODE, RECT_MODE,
        ELLIPSE_MODE, TRIANGLE_MODE,
        BUCKET_MODE, OUTLINE_MODE,
        EYEDROPPER_MODE, WORD_MODE,
        ZOOM_IN, ZOOM_OUT,
        BRUSH_P, BRUSH_M, SAVE, LOAD,
        (ShapeMode)99};

    int cols = 2;
    int cellW = TOOL_PANEL_W / cols; // 16px per cell
    int cellH = 16;

    // מרכוז אנכי
    int totalToolsH = (TOOL_COUNT / cols) * cellH;
    int startY = (screenH - totalToolsH) / 2;

    for (int i = 0; i < TOOL_COUNT; i++)
    {
        int col = i % cols;
        int row = i / cols;
        int x = col * cellW;
        int y = startY + row * cellH;
        int cx = x + cellW / 2;
        int cy = y + cellH / 2;

        bool isSelected = (TOOL_ORDER[i] == shapeMode);
        bool isCursor = toolsActive && (i == panelCursor);

        uint16_t bg = 0x0C14;
        if (isCursor)
            bg = 0x8410;
        else if (isSelected)
            bg = 0x2945;

        uint16_t iconColor = (isSelected || isCursor) ? TFT_WHITE : BLACK;
        DrawToolIcon(TOOL_ORDER[i], cx, cy, iconColor);

        if (isCursor)
        {
            M5.Lcd.drawRect(x, y, cellW, cellH, TFT_YELLOW);
            M5.Lcd.drawRect(x - 1, y - 1, cellW + 2, cellH + 2, BLACK);
        }
        else if (isSelected)
        {
            M5.Lcd.drawRect(x, y, cellW, cellH, TFT_CYAN);
        }
    }

    if (toolsActive)
    {
        M5.Lcd.drawRect(0, 0, TOOL_PANEL_W, screenH, TFT_YELLOW);
        M5.Lcd.drawRect(1, 1, TOOL_PANEL_W - 2, screenH - 2, BLACK);
    }

    // ── פאנל ימין: פלטה ──────────────────────────────────────────
    int px = screenW - PALETTE_PANEL_W;
    M5.Lcd.fillRect(px, 0, PALETTE_PANEL_W, screenH, BACK_COLOR); // right si

    int swatchW = 10;
    int swatchH = 10;
    int swatchGap = 0;
    int previewH = swatchW + 4; // שמור מקום לתצוגת הצבע

    int totalPaletteH = (PALETTE_COUNT / 2) * (swatchH + swatchGap) - swatchGap;
    int startPX = px + (PALETTE_PANEL_W - 2 * swatchW - swatchGap) / 2;
    int startPY = (screenH - previewH - totalPaletteH) / 2;

    // לולאה ראשונה: רק הריבועים
    for (int i = 0; i < PALETTE_COUNT; i++)
    {
        int col = i % 2;
        int row = i / 2;
        int cx = startPX + col * (swatchW + swatchGap);
        int cy = startPY + row * (swatchH + swatchGap);
        M5.Lcd.fillRect(cx, cy, swatchW, swatchH, PALETTE[i]);
    }

    // לולאה שנייה: רק המסגרות — מעל הכל
    for (int i = 0; i < PALETTE_COUNT; i++)
    {
        int col = i % 2;
        int row = i / 2;
        int cx = startPX + col * (swatchW + swatchGap);
        int cy = startPY + row * (swatchH + swatchGap);

        bool isCurrent = (PALETTE[i] == currentColor);
        bool isCursor = paletteActive && (i == panelCursor);

        if (isCursor)
        {
            M5.Lcd.drawRect(cx - 1, cy - 1, swatchW + 2, swatchH + 2, TFT_YELLOW);
            M5.Lcd.drawRect(cx - 2, cy - 2, swatchW + 4, swatchH + 4, BLACK);
        }

        else if (isCurrent)
        {
            M5.Lcd.drawRect(cx - 1, cy - 1, swatchW + 2, swatchH + 2, TFT_WHITE);
            M5.Lcd.drawRect(cx - 2, cy - 2, swatchW + 4, swatchH + 4, BLACK);
        }
    }

    // תצוגת צבע נוכחי — ממורכזת בתחתית
    int previewW = swatchW * 2 + swatchGap;
    int previewX = startPX;
    int previewY = screenH - previewH + 2;
    M5.Lcd.fillRect(previewX, previewY, previewW, swatchH, currentColor);
    M5.Lcd.drawRect(previewX - 1, previewY - 1, previewW + 2, swatchH + 2, TFT_WHITE);

    if (paletteActive)
    {
        M5.Lcd.drawRect(px, 0, PALETTE_PANEL_W, screenH, TFT_YELLOW);
        M5.Lcd.drawRect(px + 1, 1, PALETTE_PANEL_W - 2, screenH - 2, BLACK);
    }
}
void Painter::DrawToolIcon(ShapeMode mode, int cx, int cy, uint16_t color)
{
    int s = 8; // scale factor — כל האיקונים בתוך קופסת ~16px
    int HalfOfSize = 8;
    int posX = cx - (HalfOfSize);
    int posY = cy - (HalfOfSize);
    M5.Lcd.setSwapBytes(true);
    switch (mode)
    {
    case NONE: // עפרון
    {
        //  M5.Lcd.fillRect(posX, posY, 15, 15, RED);
        //  M5.Lcd.pushImage(posX, posY, 16, 16, pencil);

        drawImageTransparent(posX, posY, 16, 16, pencil, [&](int x, int y, uint16_t c)
                             { M5.Lcd.drawPixel(x, y, c); }, 0x5ACB);
        break;
    }
    case LINE_MODE: // קו
    {

        M5.Lcd.drawLine(cx - s, cy + s, cx + s, cy - s, color);
        //  M5.Lcd.fillCircle(cx - s, cy + s, 2, color);
        //  M5.Lcd.fillCircle(cx + s, cy - s, 2, color);
        break;
    }
    case CIRCLE_MODE: // עיגול
    {
        M5.Lcd.drawCircle(cx, cy, s, color);
        break;
    }
    case RECT_MODE: // מלבן
    {
        M5.Lcd.drawRect(cx - s, cy - s + 3, s * 2, s + 3, color);
        break;
    }
    case ELLIPSE_MODE: // אליפסה
    {
        M5.Lcd.drawEllipse(cx, cy, s, s / 2 + 2, color);
        break;
    }
    case TRIANGLE_MODE: // משולש
    {
        M5.Lcd.drawLine(cx, cy - s, cx - s, cy + s, color);     // שמאל
        M5.Lcd.drawLine(cx - s, cy + s, cx + s, cy + s, color); // תחתון
        M5.Lcd.drawLine(cx + s, cy + s, cx, cy - s, color);     // ימין
        break;
    }
    case BUCKET_MODE: // דלי
    {
        drawImageTransparent(posX, posY, 16, 16, Bucket, [&](int x, int y, uint16_t c)
                             { M5.Lcd.drawPixel(x, y, c); }, 0x0000);
        // M5.Lcd.pushImage(posX, posY, 16, 16, Bucket);

        break;
    }
    case OUTLINE_MODE: // מסגרת אוטומטית
    {
        // מלבן מקווקו
        for (int i = 0; i < s * 2; i += 3)
        {
            M5.Lcd.drawPixel(cx - s + i, cy - s + 2, color);
            M5.Lcd.drawPixel(cx - s + i, cy + s - 2, color);
        }
        for (int i = 0; i < (s - 2) * 2; i += 3)
        {
            M5.Lcd.drawPixel(cx - s + 2, cy - s + 2 + i, color);
            M5.Lcd.drawPixel(cx + s - 2, cy - s + 2 + i, color);
        }
        // סמן/זכוכית מגדלת בפינה
        M5.Lcd.drawCircle(cx + s - 3, cy + s - 5, 3, 0xFD20);
        M5.Lcd.drawLine(cx + s, cy + s - 2, cx + s + 2, cy + s, 0xFD20);
        break;
    }
    case EYEDROPPER_MODE: // טפטפת
    {
        drawImageTransparent(posX, posY, 16, 16, eyeDropper, [&](int x, int y, uint16_t c)
                             { M5.Lcd.drawPixel(x, y, c); }, 0x0000);
        //  M5.Lcd.pushImage(posX, posY, 16, 16, eyeDropper);

        break;
    }
    case ZOOM_IN: // TEXT — האות T
    {
        // M5.Lcd.pushImage(posX, posY, 15, 15, PAzoom_in);
        drawImageTransparent(posX, posY, 15, 15, PAzoom_in, [&](int x, int y, uint16_t c)
                             { M5.Lcd.drawPixel(x, y, c); }, 0x0000);
        break;
    }
    case ZOOM_OUT: // TEXT — האות T
    {
        //  M5.Lcd.pushImage(posX, posY, 15, 15, PA_zoom_out);
        drawImageTransparent(posX, posY, 15, 15, PA_zoom_out, [&](int x, int y, uint16_t c)
                             { M5.Lcd.drawPixel(x, y, c); }, 0x0000);
        break;
    }
    case WORD_MODE: // TEXT — האות T
    {
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(color);
        M5.Lcd.setCursor(cx - 3, cy - 4);
        M5.Lcd.print("T");
        // קו תחתון
        M5.Lcd.drawLine(cx - 5, cy + 5, cx + 5, cy + 5, 0xFD20);
        break;
    }
    case BRUSH_P: // +
    {
        int t = 1; // thickness

        for (int i = -t; i <= t; i++)
        {
            M5.Lcd.drawLine(cx - s, cy + i, cx + s, cy + i, color);
        }

        for (int i = -t; i <= t; i++)
        {
            M5.Lcd.drawLine(cx + i, cy - s, cx + i, cy + s, color);
        }
        break;
    }

    case BRUSH_M: // -
    {
        int t = 1; // thickness

        for (int i = -t; i <= t; i++)
        {
            M5.Lcd.drawLine(cx - s, cy + i, cx + s, cy + i, color);
        }

        break;
    }
    case SAVE:
    {
        drawImageTransparent(posX, posY, 16, 16, OLD_DISK, [&](int x, int y, uint16_t c)
                             { M5.Lcd.drawPixel(x, y, c); }, 0xF800);

        int arrowTop = cy - 10; // קצה עליון של החץ
        int arrowBot = cy + 0;  // קצה תחתון של החץ

        // חץ למטה (שמירה)
        M5.Lcd.drawLine(cx, arrowTop, cx, arrowBot, color);
        M5.Lcd.drawLine(cx - 3, arrowBot - 3, cx, arrowBot, color);
        M5.Lcd.drawLine(cx + 3, arrowBot - 3, cx, arrowBot, color);
        break;
    }
    case LOAD:
    {
        drawImageTransparent(posX, posY, 16, 16, OLD_DISK, [&](int x, int y, uint16_t c)
                             { M5.Lcd.drawPixel(x, y, c); }, 0xF800);

        int arrowTop = cy - 10; // קצה עליון של החץ
        int arrowBot = cy + 0;  // קצה תחתון של החץ

        // חץ למעלה (טעינה)
        M5.Lcd.drawLine(cx, arrowBot, cx, arrowTop, color);
        M5.Lcd.drawLine(cx - 3, arrowTop + 3, cx, arrowTop, color);
        M5.Lcd.drawLine(cx + 3, arrowTop + 3, cx, arrowTop, color);
        break;
    }
    }
    M5.Lcd.setSwapBytes(false);
}

void Painter::DrawCanvasSizeMenu()
{
    // פינה ימנית עליונה - קומפקטי
    int mw = 100, mh = 88;
    int mx = 1;
    int my = 1;
    int radius = 5;

    M5.Lcd.fillRoundRect(mx, my, mw, mh, radius, YELLOW);
    M5.Lcd.drawRoundRect(mx, my, mw, mh, radius, BLACK);

    // כותרת
    M5.Lcd.fillRoundRect(mx + 1, my + 1, mw - 2, 11, radius, 0xFD20);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Display.setFont(nullptr);
    M5.Lcd.setCursor(mx + 4, my + 3);
    M5.Lcd.print("Resize Canvas");

    // פריטים
    const char *labels[] = {"Apply to Up", "Apply to Down", "Apply to Right", "Apply to Left"};
    int rowH = 11;
    int startY = my + 14;

    for (int i = 0; i < 4; i++)
    {
        int rowY = startY + i * rowH;

        if (canvasSizeMenuSelection == i)
        {
            M5.Lcd.fillRoundRect(mx + 2, rowY - 1, mw - 4, rowH, 3, 0xFD20);
            M5.Lcd.setTextColor(WHITE);
        }
        else
        {
            M5.Lcd.setTextColor(BLACK);
        }

        M5.Lcd.setCursor(mx + 5, rowY + 2);
        M5.Lcd.print(labels[i]);

        /*     M5.Lcd.setCursor(mx + 30, rowY + 2);
            M5.Lcd.print((i < 2) ? String(PicSizeY) : String(PicSizeX)); */
    }

    // שורת Amount
    int amountY = startY + 4 * rowH + 1;
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setCursor(mx + 4, amountY);
    String label = "";
        label += (canvasSizeAmount >= 0 ? "ADD " : "REMOVE ");

    label += (canvasSizeAmount >= 0 ? "+" : "") + String(canvasSizeAmount);
    label+=" Pixels";
    M5.Lcd.print(label);
     M5.Lcd.setCursor(mx + 4, amountY + 9);
    M5.Lcd.print("Press-> or <-\n To Change value"); 
}
void Painter::HandleCanvasSizeMenuInput()
{
    bool changed = false;

    // ניווט מעלה/מטה בין הכיוונים
    if (mainOS->NewKey.ifKeyJustPress('.'))
    {
        canvasSizeMenuSelection = (canvasSizeMenuSelection + 1) % 4;
        changed = true;
    }
    if (mainOS->NewKey.ifKeyJustPress(';'))
    {
        canvasSizeMenuSelection = (canvasSizeMenuSelection - 1 + 4) % 4;
        changed = true;
    }

    // ימין/שמאל — שינוי ה-amount
    if (mainOS->NewKey.ifKeyJustPress('/'))
    {
        canvasSizeAmount += 1;
        changed = true;
    }
    if (mainOS->NewKey.ifKeyJustPress(','))
    {
        canvasSizeAmount -= 1;
        int minAmount = -(min(PicSizeX, PicSizeY) - 1);
        if (canvasSizeAmount < minAmount)
            canvasSizeAmount = minAmount;
        changed = true;
    }

    // Enter — ביצוע שינוי
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        bool up = (canvasSizeMenuSelection == 0);
        bool down = (canvasSizeMenuSelection == 1);
        bool right = (canvasSizeMenuSelection == 2);
        bool left = (canvasSizeMenuSelection == 3);

        ChangeCanvasSize(up, down, right, left, canvasSizeAmount);
        changed = true;
    }

    // Backtick — יציאה מהתפריט
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        inCanvasSizeMenu = false;
        menuOpen = false;
        updateSideBar = true;
        Draw();
        return;
    }

    if (changed)
    {
        Draw();
        DrawCanvasSizeMenu();
    }
    if(ImageTooBigToMemory)
    {
        ImageTooBigToMemory=false;
        ShowQuickMessege("Can't resize-exceeds memory limit");

    }
}