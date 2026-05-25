#include "GameKeyRemapper.h"
#include <ArduinoJson.h>
#include "MyOS.h"
#include "MainMenuV2.h"
// ─── Constants ────────────────────────────────────────────────────────────────
#define CONFIG_PATH "/AdvanceOS/GameKeyConfig.json"

#define COL_BG 0x1082      // dark navy
#define COL_PANEL 0x2104   // slightly lighter panel
#define COL_ACCENT 0xFD20  // orange-yellow
#define COL_ACTIVE 0x07FF  // cyan highlight (currently editing)
#define COL_TEXT 0xFFFF    // white
#define COL_DIM 0x8410     // grey
#define COL_SUCCESS 0x07E0 // green
// בראש הקובץ
static const int ROW_H = 14;
static const int Y0 = 24;
static const int FOOTER_H = 14;
static const int VISIBLE_H = SCREEN_H - Y0 - FOOTER_H - 4; // אזור השורות הנראה
static const char *KEY_NAMES[] = {
    "A", "B", "START", "SELECT",
    "UP", "DOWN", "RIGHT", "LEFT"};
static const int KEY_COUNT = 8;

// ─── Helpers ──────────────────────────────────────────────────────────────────

// Blocks until the user presses exactly one character key; returns it.
// Shows a live "Press any key..." screen while waiting.

// Pretty-print a char for display (handles non-printable symbols)
static void charLabel(char c, char *out, size_t sz)
{
    if (c == KEY_ENTER)
        snprintf(out, sz, "ENTER");
    else if (c == KEY_BACKSPACE)
        snprintf(out, sz, "BKSP");
    else if (c == KEY_TAB)
        snprintf(out, sz, "TAB");
    else if (c == ' ')
        snprintf(out, sz, "SPACE");
    else
        snprintf(out, sz, "%c", c);
}
void GameKeyRemapper::UpdateCamera()
{
    int rowY = selectedIndex * ROW_H;

    // גלול למעלה אם הבחירה מעל הצפייה
    if (rowY < cameraY)
        cameraY = rowY;

    // גלול למטה אם הבחירה מתחת
    int visBottom = cameraY + VISIBLE_H;
    if (rowY + ROW_H > visBottom)
        cameraY = rowY + ROW_H - VISIBLE_H;
}
// ─── File I/O ─────────────────────────────────────────────────────────────────

void GameKeyRemapper::LoadConfig()
{
    // Defaults
    keys.A = 'z';
    keys.B = 'x';
    keys.Start = KEY_ENTER;
    keys.Select = KEY_BACKSPACE;
    keys.Up = ';';
    keys.Down = '.';
    keys.Right = '/';
    keys.Left = ',';

    if (!SD.exists(CONFIG_PATH))
        return;

    File f = SD.open(CONFIG_PATH, FILE_READ);
    if (!f)
        return;

    JsonDocument doc;
    if (deserializeJson(doc, f) != DeserializationError::Ok)
    {
        f.close();
        return;
    }
    f.close();

    if (doc["A"].is<int>())
        keys.A = (char)(int)doc["A"];
    if (doc["B"].is<int>())
        keys.B = (char)(int)doc["B"];
    if (doc["START"].is<int>())
        keys.Start = (char)(int)doc["START"];
    if (doc["SELECT"].is<int>())
        keys.Select = (char)(int)doc["SELECT"];
    if (doc["UP"].is<int>())
        keys.Up = (char)(int)doc["UP"];
    if (doc["DOWN"].is<int>())
        keys.Down = (char)(int)doc["DOWN"];
    if (doc["RIGHT"].is<int>())
        keys.Right = (char)(int)doc["RIGHT"];
    if (doc["LEFT"].is<int>())
        keys.Left = (char)(int)doc["LEFT"];
}

void GameKeyRemapper::SaveConfig()
{
    // Make sure directory exists
    if (!SD.exists("/AdvanceOS"))
        SD.mkdir("/AdvanceOS");

    File f = SD.open(CONFIG_PATH, FILE_WRITE);
    if (!f)
    {
        mainOS->ShowOnScreenMessege("Save failed!");
        return;
    }

    JsonDocument doc;
    doc["A"] = (int)keys.A;
    doc["B"] = (int)keys.B;
    doc["START"] = (int)keys.Start;
    doc["SELECT"] = (int)keys.Select;
    doc["UP"] = (int)keys.Up;
    doc["DOWN"] = (int)keys.Down;
    doc["RIGHT"] = (int)keys.Right;
    doc["LEFT"] = (int)keys.Left;

    serializeJson(doc, f);
    f.close();
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

void GameKeyRemapper::Begin()
{
    cameraY = 0;

    showTopBar = false;
    LoadConfig();
    selectedIndex = 0;
    editingMode = false;
    dirty = false;
}

void GameKeyRemapper::Loop()
{

    if (editingMode)
        return; // Draw handles the blocking WaitForSingleKey flow

    /*     if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed())
        {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            // Navigate up/down with the arrow keys or ; and .
            if (M5Cardputer.Keyboard.isKeyPressed(';'))          // physical up
                selectedIndex = (selectedIndex - 1 + KEY_COUNT) % KEY_COUNT;
            else if (M5Cardputer.Keyboard.isKeyPressed('.'))     // physical down
                selectedIndex = (selectedIndex + 1) % KEY_COUNT;
             else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER) || status.word.size() && status.word[0] == '\n')
            {
                // Enter edit mode for selected key
                editingMode = true;
            }
        }
     */
    // ENTER pressed via NewKey helper (if your OS provides it)
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        if (dirty)
            SaveConfig();
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER) && !editingMode)
    {
        editingMode = true;
    }
    if (mainOS->NewKey.ifKeyJustPress(';') && !editingMode)
    {
        selectedIndex = (selectedIndex - 1 + KEY_COUNT) % KEY_COUNT;
    }

    if (mainOS->NewKey.ifKeyJustPress('.') && !editingMode)
    {
        selectedIndex = (selectedIndex + 1) % KEY_COUNT;
    }

    // ESC = back / exit (if your OS has that helper)
    if (mainOS->NewKey.ifKeyJustPress(KEY_BACKSPACE))
    {
        if (dirty)
            SaveConfig();
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
    }
    if (!mainOS->screenOff)
    {
        UpdateCamera();

        Draw();
    }
}

void GameKeyRemapper::Draw()
{
    // ── If we just entered edit mode, do the blocking key-capture now ──
    if (editingMode)
    {
        char c = mainOS->AskForAnyKey(KEY_NAMES[selectedIndex]);
        if (c != 0)
        {
            // Write back to the correct slot
            switch (selectedIndex)
            {
            case 0:
                keys.A = c;
                break;
            case 1:
                keys.B = c;
                break;
            case 2:
                keys.Start = c;
                break;
            case 3:
                keys.Select = c;
                break;
            case 4:
                keys.Up = c;
                break;
            case 5:
                keys.Down = c;
                break;
            case 6:
                keys.Right = c;
                break;
            case 7:
                keys.Left = c;
                break;
            }
            dirty = true;
        }
        editingMode = false;
        // fall through to normal draw
    }

    // ── Gather current mapping as an array ───────────────────────────────────
    char mapping[KEY_COUNT];
    mapping[0] = keys.A;
    mapping[1] = keys.B;
    mapping[2] = keys.Start;
    mapping[3] = keys.Select;
    mapping[4] = keys.Up;
    mapping[5] = keys.Down;
    mapping[6] = keys.Right;
    mapping[7] = keys.Left;

    // ── Build sprite ─────────────────────────────────────────────────────────
    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H);
    mainOS->sprite.fillSprite(COL_BG);

    // Title bar
    mainOS->sprite.fillRect(0, 0, SCREEN_W, 18, COL_ACCENT);
    mainOS->sprite.setTextColor(COL_BG);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(6, 5);
    mainOS->sprite.print("Game Key Remapper");

    // Two-column layout
    // Left col: action names  |  Right col: assigned key
    const int ROW_H = 14;
    const int Y0 = 24;
    const int COL1_X = 8;
    const int COL2_X = 140;
    const int BOX_W = 88;
    const int BOX_H = 12;

    for (int i = 0; i < KEY_COUNT; i++)
    {
        int y = Y0 + i * ROW_H;
        bool sel = (i == selectedIndex);

        // Row highlight
        if (sel)
            mainOS->sprite.fillRoundRect(COL1_X - 2, y - 1, SCREEN_W - 10, BOX_H + 2, 3, COL_ACTIVE);

        // Action label
        mainOS->sprite.setTextColor(sel ? COL_BG : COL_TEXT);
        mainOS->sprite.setCursor(COL1_X, y);
        mainOS->sprite.print(KEY_NAMES[i]);

        // Key box
        uint16_t boxColor = sel ? COL_ACCENT : COL_PANEL;
        mainOS->sprite.fillRoundRect(COL2_X, y - 1, BOX_W, BOX_H, 2, boxColor);
        mainOS->sprite.drawRoundRect(COL2_X, y - 1, BOX_W, BOX_H, 2, sel ? COL_BG : COL_DIM);

        char lbl[8];
        charLabel(mapping[i], lbl, sizeof(lbl));
        mainOS->sprite.setTextColor(sel ? COL_BG : COL_ACCENT);
        mainOS->sprite.setCursor(COL2_X + 4, y);
        mainOS->sprite.print(lbl);
    }

    // Footer hints
    mainOS->sprite.setTextColor(COL_DIM);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(4, SCREEN_H - 11);
    //mainOS->sprite.print("ENTER Edit  BKSP Save+Exit");

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

void GameKeyRemapper::OnExit()
{
    if (dirty)
        SaveConfig();
}