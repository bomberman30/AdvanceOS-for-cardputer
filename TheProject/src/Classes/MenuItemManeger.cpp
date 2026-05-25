#include "MenuItemManeger.h"
#include "MyOS.h"
#include "MainMenuV2.h"
void MenuItemManeger::Begin()
{
    selectedIndex = 0;
    showTopBar=false;
}
void MenuItemManeger::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
        return;
    }

    bool curUp    = mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(';',700,50);
    bool curDown  = mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('.',700,50);
    bool curLeft  = mainOS->NewKey.ifKeyJustPress(',');
    bool curRight = mainOS->NewKey.ifKeyJustPress('/');
    bool curEnter = mainOS->NewKey.ifKeyJustPress(KEY_ENTER);

    // ── בניית drawOrder (אותו סדר כמו ב-Draw) ────────────────
    drawOrder.clear();
    for (int idx : mainOS->mainScreenIndices)
        drawOrder.push_back(idx);
    for (int i = 0; i < (int)mainOS->allApps.size(); i++)
        if (findPosition(i) < 0)
            drawOrder.push_back(i);

    int total = (int)drawOrder.size();

    // ── מציאת מיקום הנוכחי ב-drawOrder ───────────────────────
    int currentDrawPos = 0;
    for (int i = 0; i < total; i++)
        if (drawOrder[i] == selectedIndex) { currentDrawPos = i; break; }

    // ── ניווט לפי drawOrder ───────────────────────────────────
    if (curUp && !prevUp)
    {
        currentDrawPos--;
        if (currentDrawPos < 0) currentDrawPos = total - 1;
        selectedIndex = drawOrder[currentDrawPos];
    }
    if (curDown && !prevDown)
    {
        currentDrawPos++;
        if (currentDrawPos >= total) currentDrawPos = 0;
        selectedIndex = drawOrder[currentDrawPos];
    }

    if (curEnter && !prevEnter)
        toggleVisible(selectedIndex);

    if (curRight && !prevRight)
        moveDown(selectedIndex);

    if (curLeft && !prevLeft)
        moveUp(selectedIndex);

    prevUp    = curUp;
    prevDown  = curDown;
    prevLeft  = curLeft;
    prevRight = curRight;
    prevEnter = curEnter;

    Draw();
}
void MenuItemManeger::Draw()
{
    auto &apps    = mainOS->allApps;
    auto &indices = mainOS->mainScreenIndices;

    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H);
    mainOS->sprite.fillSprite(TFT_BLACK);

    // ── כותרת ─────────────────────────────────────────────────
    mainOS->sprite.fillRect(0, 0, SCREEN_W, 14, TFT_BLUE);
    mainOS->sprite.setTextColor(TFT_WHITE, TFT_BLUE);
    mainOS->sprite.setTextDatum(MC_DATUM);
    mainOS->sprite.drawString("Menu Manager", SCREEN_W / 2, 7);

    // ── בניית רשימת הצגה ממוינת ───────────────────────────────
    // קודם מה שמופיע (לפי סדר mainScreenIndices), אחר כך השאר
    std::vector<int> drawOrder;
    for (int idx : indices)
        drawOrder.push_back(idx);
    for (int i = 0; i < (int)apps.size(); i++)
        if (findPosition(i) < 0)
            drawOrder.push_back(i);

    // ── מציאת מיקום selectedIndex ב-drawOrder ─────────────────
    int selectedDrawPos = 0;
    for (int i = 0; i < (int)drawOrder.size(); i++)
        if (drawOrder[i] == selectedIndex) { selectedDrawPos = i; break; }

    // ── רשימה ─────────────────────────────────────────────────
const int rowH    = 14;
const int startY  = 18;
const int footerH = 12;
const int visible = (SCREEN_H - footerH - startY) / rowH;  // במקום 8

    int scrollOffset = 0;
    if (selectedDrawPos >= visible)
        scrollOffset = selectedDrawPos - visible + 1;

    for (int di = 0; di < (int)drawOrder.size(); di++)
    {
        int drawPos = di - scrollOffset;
        if (drawPos < 0 || drawPos >= visible) continue;

        int appIdx   = drawOrder[di];
        int y        = startY + drawPos * rowH;
        bool sel     = (appIdx == selectedIndex);
        bool isVis   = findPosition(appIdx) >= 0;

        if (sel)
            mainOS->sprite.fillRect(0, y, SCREEN_W, rowH, TFT_DARKGREY);

        mainOS->sprite.setTextDatum(ML_DATUM);
        mainOS->sprite.setTextSize(1);

if (isVis)
{
    mainOS->sprite.setTextColor(appIdx == 24 ? TFT_CYAN : TFT_GREEN);
    mainOS->sprite.drawString("V", 4, y + rowH / 2);
}
else
{
    mainOS->sprite.setTextColor(TFT_RED);
    mainOS->sprite.drawString("X", 4, y + rowH / 2);
}

        mainOS->sprite.setTextColor(sel ? TFT_WHITE : TFT_LIGHTGREY);
        mainOS->sprite.drawString(apps[appIdx].name, 18, y + rowH / 2);

        int pos = findPosition(appIdx);
        if (pos >= 0)
        {
            mainOS->sprite.setTextColor(TFT_YELLOW);
            mainOS->sprite.setTextDatum(MR_DATUM);
            mainOS->sprite.drawString("#" + String(pos + 1), SCREEN_W - 4, y + rowH / 2);
        }
    }

    // ── Footer ──────────────────────────────────────────
    mainOS->sprite.fillRect(0, SCREEN_H - 12, SCREEN_W, 12, TFT_NAVY);
    mainOS->sprite.setTextColor(TFT_WHITE, TFT_NAVY);
    mainOS->sprite.setTextDatum(MC_DATUM);
    mainOS->sprite.drawString("ENTER=show/hide  </>=move", SCREEN_W / 2, SCREEN_H - 6);

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

void MenuItemManeger::OnExit()
{
    mainOS->saveSettings();
}

// ══════════════════════════════════════════════════════════════
//  עוזרים פנימיים
// ══════════════════════════════════════════════════════════════

bool MenuItemManeger::isVisible(int appIndex)
{
    return findPosition(appIndex) >= 0;
}

void MenuItemManeger::toggleVisible(int appIndex)
{
    if (appIndex == 24) return; // settings - מוגן תמיד

    if (isVisible(appIndex))
    {
        auto &v = mainOS->mainScreenIndices;
        v.erase(std::remove(v.begin(), v.end(), appIndex), v.end());
    }
    else
    {
        mainOS->mainScreenIndices.push_back(appIndex);
    }
}

void MenuItemManeger::moveUp(int appIndex)
{
    int pos = findPosition(appIndex);
    if (pos > 0)
        std::swap(mainOS->mainScreenIndices[pos], mainOS->mainScreenIndices[pos - 1]);
}

void MenuItemManeger::moveDown(int appIndex)
{
    auto &v = mainOS->mainScreenIndices;
    int pos = findPosition(appIndex);
    if (pos >= 0 && pos < (int)v.size() - 1)
        std::swap(v[pos], v[pos + 1]);
}

int MenuItemManeger::findPosition(int appIndex)
{
    auto &v = mainOS->mainScreenIndices;
    for (int i = 0; i < (int)v.size(); i++)
        if (v[i] == appIndex) return i;
    return -1;
}