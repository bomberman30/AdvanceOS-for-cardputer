#include "IRsend.h"
// #include <ArduinoJson.h>
#include "../../MyOS.h"
#include "../FileBrowser.h"
#include "../../Function.h"
const char qwertyMap[] = "qwertyuioasdfghjklzxcvbn"; // p and m removed becose of POwer and mute commands
void IRSenderApp::Begin()
{
    showTopBar = false;
    ir.begin();

    CommandFromFile = getAllCommandNamesFromFile(mainOS->FileSelectedInFS);
    DeviceID = mainOS->getFileNameFromPath(mainOS->FileSelectedInFS);

    // איפוס משתנים חדשים
    tempUp = -1;
    tempDown = -1;
    for (int i = 0; i < 10; i++)
        idxNum[i] = -1;

    letterCmdIndices.clear();
    int letterIdx = 0;

    for (int i = 0; i < CommandFromFile.size(); i++)
    {
        String name = CommandFromFile[i];
        name.toLowerCase();

        bool isSpecial = false; // דגל לבדיקה אם הפקודה שויכה למשהו מיוחד

        // בדיקת פקודות מיוחדות
        if (name == "up")
        {
            idxUp = i;
            isSpecial = true;
        }
        else if (name == "down")
        {
            idxDown = i;
            isSpecial = true;
        }
        else if (name == "left")
        {
            idxLeft = i;
            isSpecial = true;
        }
        else if (name == "right")
        {
            idxRight = i;
            isSpecial = true;
        }
        else if (name == "enter" || name == "ok")
        {
            idxEnter = i;
            isSpecial = true;
        }
        else if (name == "power")
        {
            idxPower = i;
            isSpecial = true;
        }
        else if (name == "mute")
        {
            idxMute = i;
            isSpecial = true;
        }
        else if (name == "vol+" || name == "volumeup")
        {
            idVolUp = i;
            isSpecial = true;
        }
        else if (name == "vol-" || name == "volumedown")
        {
            idVolDown = i;
            isSpecial = true;
        }
        else if (name == "channelup")
        {
            idChannelUp = i;
            isSpecial = true;
        }
        else if (name == "channeldown")
        {
            idChannelDown = i;
            isSpecial = true;
        }
        // הוספת טמפרטורה
        else if (name == "temp+" || name == "tempup")
        {
            tempUp = i;
            isSpecial = true;
        }
        else if (name == "temp-" || name == "tempdown")
        {
            tempDown = i;
            isSpecial = true;
        }

        // בדיקת מספרים
        for (int n = 0; n <= 9; n++)
        {
            if (name == String(n))
            {
                idxNum[n] = i;
                isSpecial = true;
                break;
            }
        }

        // הוספה ל-QWERTY רק אם זה לא "מיוחד"
        if (!isSpecial)
        {
            if (letterIdx < strlen(qwertyMap))
            {
                letterCmdIndices.push_back(i);
                letterMap[letterIdx] = qwertyMap[letterIdx];
                letterIdx++;
            }
        }
    }
}

void IRSenderApp::Loop()
{
    if (mainOS->screenOff)
    {
        return;
    }
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new FileBrowser(mainOS));
        return;
    }

    int toSend = -1;

    // חיצים
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(';',700, 100) && idxUp != -1)
        toSend = idxUp;
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('.',700, 100) && idxDown != -1)
        toSend = idxDown;
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(',',700, 100) && idxLeft != -1)
        toSend = idxLeft;
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('/',700, 100) && idxRight != -1)
        toSend = idxRight;
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER) && idxEnter != -1)
        toSend = idxEnter;

    // מספרים
    for (int i = 0; i <= 9; i++)
    {
        if (mainOS->NewKey.ifKeyJustPress('0' + i) && idxNum[i] != -1)
        {
            toSend = idxNum[i];
        }
    }

    // אותיות
    for (int i = 0; i < letterCmdIndices.size(); i++)
    {
        if (mainOS->NewKey.ifKeyJustPress(letterMap[i]))
        {
            toSend = letterCmdIndices[i];
        }
    }
    // POWER
    if (mainOS->NewKey.ifKeyJustPress('p') && idxPower != -1)
        toSend = idxPower;

    // MUTE
    if (mainOS->NewKey.ifKeyJustPress('m') && idxMute != -1)
        toSend = idxMute;

    // VOL
    if (mainOS->NewKey.ifKeyJustPress('=') && idVolUp != -1)

    {
        toSend = idVolUp;
    }

    if (mainOS->NewKey.ifKeyJustPress('-') && idVolDown != -1)
        toSend = idVolDown;

    // CHANNEL
    if (mainOS->NewKey.ifKeyJustPress(']') && idChannelUp != -1)
        toSend = idChannelUp;

    if (mainOS->NewKey.ifKeyJustPress('[') && idChannelDown != -1)
        toSend = idChannelDown;

    // TEMP (קיצורים לדוגמה: ' ו- ;)
    if (mainOS->NewKey.ifKeyJustPress('\'') && tempUp != -1)
        toSend = tempUp;

    if (mainOS->NewKey.ifKeyJustPress(';') && tempDown != -1)
        toSend = tempDown;

    if (toSend != -1)
    {
        InCorrentCommand = toSend;
        loadAndSendFromFile(CommandFromFile[toSend], mainOS->FileSelectedInFS);
        Draw();
        return;
    }

    Draw();
}

/* 
void IRSenderApp::Draw()
{
    mainOS->sprite.createSprite(240, 135);
    mainOS->sprite.fillSprite(TFT_BLACK);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextWrap(false);

    // בדיקה אם קיימים מקשי חיצים
    bool hasArrows = (idxUp != -1 || idxDown != -1 || idxLeft != -1 || idxRight != -1 || idxEnter != -1);

    int rowHeight = 11;
    // TopOffset לא מוגדר בקוד שסיפקת, השתמשתי בערך ברירת מחדל 0. שנה בהתאם לצורך.
    int currentTopOffset = 0;
    int maxRows = (135 - currentTopOffset - 5) / rowHeight;
    int numCols = hasArrows ? 2 : 3;
    int colWidth = 240 / numCols;

    // ===== בניית רשימה מסוננת (בלי כפתורים מיוחדים) =====
    std::vector<int> displayList;

    for (int i = 0; i < CommandFromFile.size(); i++)
    {
        bool isSpecial = false;

        // בדיקה אם האינדקס שייך למספר
        for (int n = 0; n <= 9; n++)
        {
            if (idxNum[n] == i)
            {
                isSpecial = true;
                break;
            }
        }
        if (isSpecial)
            continue;

        // בדיקת שאר הכפתורים המיוחדים
        if (i == idxUp || i == idxDown || i == idxLeft || i == idxRight || i == idxEnter ||
            i == idxPower || i == idxMute ||
            i == idVolUp || i == idVolDown ||
            i == idChannelUp || i == idChannelDown ||
            i == tempUp || i == tempDown) // הוספת הטמפרטורה לסינון
        {
            isSpecial = true;
        }

        if (!isSpecial)
            displayList.push_back(i);
    }

    // ===== ציור רשימת הפקודות הכלליות (עם QWERTY) =====
    for (int k = 0; k < displayList.size(); k++)
    {
        int i = displayList[k];

        int col = k / maxRows;
        int row = k % maxRows;
        if (col >= numCols)
            break;

        int x = 5 + (col * colWidth);
        int y = currentTopOffset + 5 + (row * rowHeight);

        uint16_t txtCol = (i == InCorrentCommand) ? TFT_GREEN : TFT_WHITE;

        // ===== HOTKEY QWERTY =====
        for (int j = 0; j < letterCmdIndices.size(); j++)
        {
            if (letterCmdIndices[j] == i)
            {
                mainOS->sprite.setTextColor(TFT_YELLOW);
                mainOS->sprite.setCursor(x, y);
                mainOS->sprite.printf("%c:", letterMap[j]);
                x += 15;
                break;
            }
        }

        // ===== שם הפקודה =====
        mainOS->sprite.setTextColor(txtCol);
        mainOS->sprite.setCursor(x, y);

        String dName = CommandFromFile[i];
        // חישוב דינמי של אורך הטקסט המקסימלי
        int maxChars = (colWidth - (letterMap[0] ? 15 : 0)) / 6 - 1;
        if (dName.length() > maxChars && maxChars > 3)
            dName = dName.substring(0, maxChars - 2) + "..";

        mainOS->sprite.print(dName);
    }

    // ===== ציור מקשי חיצים (D-Pad) =====
    if (hasArrows)
    {
        int padX = 195;
        int padY = 95;
        int s = 22; // מרווח בין כפתורים

        auto drawPadBtn = [&](int x, int y, int idx, String label, uint16_t color)
        {
            if (idx == -1)
                return;
            bool isPressed = (InCorrentCommand == idx);
            // צבע כהה יותר כברירת מחדל אם color לא סופק
            uint16_t btnColor = (idx == idxEnter) ? 0x4208 : 0x2104;
            mainOS->sprite.fillRoundRect(x, y, 20, 18, 3, isPressed ? TFT_GREEN : btnColor);
            mainOS->sprite.drawRoundRect(x, y, 20, 18, 3, TFT_WHITE);
            mainOS->sprite.setTextColor(TFT_WHITE);

            // מרכוס הטקסט (U, D, L, R, OK)
            int labelX = x + (20 - label.length() * 6) / 2 + 1;
            mainOS->sprite.setCursor(labelX, y + 5);
            mainOS->sprite.print(label);
        };

        drawPadBtn(padX, padY - s, idxUp, "U", 0); // השתמשתי ב-0 כברירת מחדל
        drawPadBtn(padX, padY + s, idxDown, "D", 0);
        drawPadBtn(padX - s, padY, idxLeft, "L", 0);
        drawPadBtn(padX + s, padY, idxRight, "R", 0);
        drawPadBtn(padX, padY, idxEnter, "OK", 0);

        // כיתוב מקשי קיצור לחיצים

    }

    // ===== כפתור POWER =====
    if (idxPower != -1)
    {
        bool isPressed = (InCorrentCommand == idxPower);
        mainOS->sprite.fillRoundRect(210, 5, 25, 16, 3, isPressed ? TFT_GREEN : TFT_RED);
        mainOS->sprite.drawRoundRect(210, 5, 25, 16, 3, TFT_WHITE);
        mainOS->sprite.setTextColor(TFT_WHITE);
        mainOS->sprite.setCursor(212, 9);
        mainOS->sprite.print("ON");

        mainOS->sprite.setTextColor(TFT_YELLOW);
        mainOS->sprite.setCursor(200, 9); // מיקום התווית 'p'
        mainOS->sprite.print("P");
    }

    // ===== כפתורי בקרה (VOL / CHANNEL / TEMP / MUTE) =====
    int baseX = 155; // הוזז מעט שמאלה כדי לפנות מקום
    int baseY = 15;

    auto drawSmallBtn = [&](int x, int y, int idx, String label)
    {
        if (idx == -1)
            return;
        bool isPressed = (InCorrentCommand == idx);
        mainOS->sprite.fillRoundRect(x, y, 24, 14, 3, isPressed ? TFT_GREEN : 0x3186);
        mainOS->sprite.drawRoundRect(x, y, 24, 14, 3, TFT_WHITE);
        mainOS->sprite.setTextColor(TFT_WHITE);
        // מרכוס הטקסט
        int labelX = x + (24 - label.length() * 6) / 2 + 1;
        mainOS->sprite.setCursor(labelX, y + 3);
        mainOS->sprite.print(label);
    };

    // --- Volume ---
    if (idVolUp != -1 || idVolDown != -1)
    {
        drawSmallBtn(baseX, baseY, idVolUp, "V+");
        drawSmallBtn(baseX, baseY + 16, idVolDown, "V-");
        mainOS->sprite.setTextColor(TFT_YELLOW);
        mainOS->sprite.setCursor(baseX + 6, baseY + 32);
        mainOS->sprite.print("+ - KEYS"); // מקשי קיצור לווליום
    }

    // --- Channel ---
    if (idChannelUp != -1 || idChannelDown != -1)
    {
        drawSmallBtn(baseX + 28, baseY, idChannelUp, "C+");
        drawSmallBtn(baseX + 28, baseY + 16, idChannelDown, "C-");
        mainOS->sprite.setTextColor(TFT_YELLOW);
        mainOS->sprite.setCursor(baseX + 28 + 6, baseY + 32);
        mainOS->sprite.print("[ ]"); // מקשי קיצור לערוצים
    }

    // --- Temperature (חדש!) ---
    if (tempUp != -1 || tempDown != -1)
    {
        drawSmallBtn(baseX + 56, baseY, tempUp, "T+");
        drawSmallBtn(baseX + 56, baseY + 16, tempDown, "T-");
        mainOS->sprite.setTextColor(TFT_YELLOW);
        mainOS->sprite.setCursor(baseX + 56 + 6, baseY + 32);
        mainOS->sprite.print("' ;"); // מקשי קיצור לטמפרטורה
    }

    // --- Mute ---
    if (idxMute != -1)
    {
        drawSmallBtn(baseX + 28, baseY + 45, idxMute, "MUTE");
        mainOS->sprite.setTextColor(TFT_YELLOW);
        mainOS->sprite.setCursor(baseX + 28 - 10, baseY + 48);
        mainOS->sprite.print("M"); // מקש קיצור להשתקה
    }
    // ===== שורת הסבר תחתונה =====
    mainOS->sprite.setTextColor(TFT_DARKGREY);
    mainOS->sprite.setCursor(5, 125);
    // ===== שורת הסבר דינמית =====
    String footerHint = "";

    // בדיקה אם קיימים מספרים
    bool hasNums = false;
    for (int n = 0; n <= 9; n++)
    {
        if (idxNum[n] != -1)
        {
            hasNums = true;
            break;
        }
    }
    if (hasNums)
        footerHint += "[0-9]:Numbers ";

    // בדיקת Power
    if (idxPower != -1)
        footerHint += "P:power ";

    // בדיקת Mute
    if (idxMute != -1)
        footerHint += "M:mute ";
    if (hasArrows)
    {
        footerHint += "Arrow Keys And ENTER ";
    }
    // בדיקת יציאה (תמיד קיים אצלך ב-Loop)
    // footerHint += "`:exit";

    // ציור השורה התחתונה
    mainOS->sprite.setTextColor(TFT_DARKGREY);
    mainOS->sprite.setCursor(5, 125);
    mainOS->sprite.print(footerHint);

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
} */

/* 
void IRSenderApp::Draw()
{
    mainOS->sprite.createSprite(240, 135);
    mainOS->sprite.fillSprite(0x080C); // #0d0d1a - כחול-שחור עמוק

    bool hasArrows = (idxUp != -1 || idxDown != -1 || idxLeft != -1 || idxRight != -1 || idxEnter != -1);

    // ===== צבעי NEON =====
    uint16_t COL_BG       = 0x080C; // #0d0d1a רקע
    uint16_t COL_BG2      = 0x1092; // #111130 תאים
    uint16_t COL_BORDER   = 0x190C; // #1e1e3a גבולות
    uint16_t COL_TEXT     = 0xC618; // #c0c0e0 טקסט רגיל
    uint16_t COL_KEY      = 0xF7E0; // #f0c040 מקש QWERTY (צהוב)
    uint16_t COL_ACTIVE   = 0x37E6; // #30ff80 פקודה פעילה (ירוק ניאון)
    uint16_t COL_ACTIVE_BG= 0x0291; // #021818 רקע פקודה פעילה
    uint16_t COL_HINT     = 0x318C; // #303060 טקסט hint תחתון
    uint16_t COL_DPAD     = 0x8C10; // #8080c0 D-pad כחול-לבנדר
    uint16_t COL_DPAD_BG  = 0x1092; // #111130
    uint16_t COL_DPAD_OK  = 0x37E6; // #30ff80 ירוק
    uint16_t COL_DPAD_OKBG= 0x0291; // #082818
    uint16_t COL_PWR_BG   = 0x8000; // #880000 אדום כהה
    uint16_t COL_PWR      = 0xF800; // #ff0000 אדום
    uint16_t COL_PWR_KEY  = 0xF7E0; // צהוב P
    uint16_t COL_VOL      = 0x047F; // #0080ff כחול ניאון
    uint16_t COL_VOL_BG   = 0x000C; // #00001a
    uint16_t COL_CH       = 0x07E8; // #00ff40 ירוק ניאון
    uint16_t COL_CH_BG    = 0x0001; // #000010
    uint16_t COL_TEMP_UP  = 0xFC20; // #ff8000 כתום
    uint16_t COL_TEMP_DN  = 0x04FF; // #0088ff כחול
    uint16_t COL_MUTE     = 0xFEA0; // #ffa000 ענבר
    uint16_t COL_SEC      = 0x318C; // #303060 משני

    // ===== שורת כותרת עליונה =====
    mainOS->sprite.fillRect(0, 0, 240, 10, COL_BORDER);
    mainOS->sprite.drawFastHLine(0, 10, 240, COL_BORDER);

    // שם המכשיר - מרכז
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextColor(0x7BCF); // #7c7caa
    mainOS->sprite.setTextWrap(false);
    // מרכז הטקסט של שם המכשיר
    int devNameLen = DeviceID.length();
    int devX = (240 - devNameLen * 6) / 2;
    mainOS->sprite.setCursor(devX, 2);
    mainOS->sprite.print(DeviceID);

    // ===== POWER button - פינה עליונה ימנית =====
    if (idxPower != -1)
    {
        bool pwrPressed = (InCorrentCommand == idxPower);
        uint16_t pwrFill = pwrPressed ? COL_PWR : COL_PWR_BG;
        mainOS->sprite.fillRoundRect(209, 1, 28, 8, 2, pwrFill);
        mainOS->sprite.drawRoundRect(209, 1, 28, 8, 2, COL_PWR);
        mainOS->sprite.setTextColor(COL_PWR_KEY);
        mainOS->sprite.setCursor(211, 3);
        mainOS->sprite.print("P");
        mainOS->sprite.setTextColor(pwrPressed ? TFT_WHITE : COL_PWR);
        mainOS->sprite.setCursor(218, 3);
        mainOS->sprite.print("ON");
    }

    // ===== בניית רשימת פקודות כלליות (ללא מיוחדות) =====
    std::vector<int> displayList;
    for (int i = 0; i < (int)CommandFromFile.size(); i++)
    {
        bool isSpecial = false;
        for (int n = 0; n <= 9; n++)
            if (idxNum[n] == i) { isSpecial = true; break; }
        if (!isSpecial &&
            i != idxUp && i != idxDown && i != idxLeft && i != idxRight &&
            i != idxEnter && i != idxPower && i != idxMute &&
            i != idVolUp && i != idVolDown &&
            i != idChannelUp && i != idChannelDown &&
            i != tempUp && i != tempDown)
        {
            displayList.push_back(i);
        }
    }

    // ===== עמודות פקודות =====
    int listX      = 2;
    int listTop    = 13;
    int rowH       = 11;
    int listW      = hasArrows ? 148 : 200; // צמצם אם יש D-pad
    int numCols    = 2;
    int colW       = listW / numCols;
    int maxRows    = (135 - listTop - 10) / rowH; // שמור מקום לפוטר

    for (int k = 0; k < (int)displayList.size(); k++)
    {
        int i   = displayList[k];
        int col = k / maxRows;
        int row = k % maxRows;
        if (col >= numCols) break;

        int x = listX + col * colW;
        int y = listTop + row * rowH;

        bool isActive = (i == InCorrentCommand);

        // רקע שורה פעילה
        if (isActive)
        {
            mainOS->sprite.fillRect(x - 1, y - 1, colW - 2, rowH, COL_ACTIVE_BG);
            mainOS->sprite.drawRect(x - 1, y - 1, colW - 2, rowH, COL_ACTIVE);
        }

        // מקש QWERTY
        for (int j = 0; j < (int)letterCmdIndices.size(); j++)
        {
            if (letterCmdIndices[j] == i)
            {
                mainOS->sprite.setTextColor(isActive ? COL_KEY : COL_KEY);
                mainOS->sprite.setCursor(x, y);
                char kbuf[3] = { (char)toupper(letterMap[j]), ':', 0 };
                mainOS->sprite.print(kbuf);
                x += 13;
                break;
            }
        }

        // שם פקודה
        mainOS->sprite.setTextColor(isActive ? COL_ACTIVE : COL_TEXT);
        mainOS->sprite.setCursor(x, y);
        String dName = CommandFromFile[i];
        int maxCh = (colW - 14) / 6;
        if ((int)dName.length() > maxCh && maxCh > 3)
            dName = dName.substring(0, maxCh - 2) + "..";
        mainOS->sprite.print(dName);

        // נקודת ACTIVE בצד
        if (isActive)
        {
            mainOS->sprite.fillCircle(listX + colW - 4, y + 4, 2, COL_ACTIVE);
        }
    }

    // ===== קו מפריד אנכי בין רשימה לבקרות =====
    if (hasArrows)
        mainOS->sprite.drawFastVLine(listW + 2, 12, 110, COL_BORDER);

    // ===== D-PAD =====
    if (hasArrows)
    {
        int px = 195; // מרכז D-pad
        int py = 85;
        int s  = 20;  // מרווח

        auto drawDpad = [&](int x, int y, int idx, const char* label)
        {
            if (idx == -1) return;
            bool pressed = (InCorrentCommand == idx);
            bool isOK    = (idx == idxEnter);
            uint16_t bg  = isOK ? (pressed ? COL_DPAD_OK : COL_DPAD_OKBG)
                                : (pressed ? COL_ACTIVE   : COL_DPAD_BG);
            uint16_t brd = isOK ? COL_DPAD_OK : (pressed ? COL_ACTIVE : COL_BORDER);
            uint16_t txt = isOK ? (pressed ? TFT_BLACK : COL_DPAD_OK)
                                : (pressed ? TFT_BLACK : COL_DPAD);

            mainOS->sprite.fillRoundRect(x, y, 18, 14, 2, bg);
            mainOS->sprite.drawRoundRect(x, y, 18, 14, 2, brd);
            mainOS->sprite.setTextColor(txt);
            int lx = x + (18 - (int)strlen(label) * 6) / 2;
            mainOS->sprite.setCursor(lx, y + 4);
            mainOS->sprite.print(label);
        };

        drawDpad(px - 9,    py - s,  idxUp,    "U");
        drawDpad(px - 9,    py + s,  idxDown,  "D");
        drawDpad(px - 9-s,  py,      idxLeft,  "L");
        drawDpad(px - 9+s,  py,      idxRight, "R");
        drawDpad(px - 9,    py,      idxEnter, "OK");
    }

    // ===== כפתורי בקרה קטנים: VOL / CH / TEMP / MUTE =====
    int bx = 154;
    int by = 13;

    auto drawCtrlBtn = [&](int x, int y, int idx, const char* label,
                           uint16_t fg, uint16_t bg_idle)
    {
        if (idx == -1) return;
        bool pressed = (InCorrentCommand == idx);
        uint16_t bg  = pressed ? fg : bg_idle;
        uint16_t brd = fg;
        uint16_t txt = pressed ? TFT_BLACK : fg;

        mainOS->sprite.fillRoundRect(x, y, 22, 12, 2, bg);
        mainOS->sprite.drawRoundRect(x, y, 22, 12, 2, brd);
        mainOS->sprite.setTextColor(txt);
        int lx = x + (22 - (int)strlen(label) * 6) / 2;
        mainOS->sprite.setCursor(lx, y + 3);
        mainOS->sprite.print(label);
    };

    // -- VOL --
    if (idVolUp != -1 || idVolDown != -1)
    {
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 2, by);
        mainOS->sprite.print("V");
        drawCtrlBtn(bx, by + 3,  idVolUp,   "V+", COL_VOL,     COL_VOL_BG);
        drawCtrlBtn(bx, by + 17, idVolDown, "V-", COL_VOL,     COL_VOL_BG);
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 1, by + 31);
        mainOS->sprite.print("+-");
    }

    // -- CH --
    if (idChannelUp != -1 || idChannelDown != -1)
    {
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 26, by);
        mainOS->sprite.print("C");
        drawCtrlBtn(bx + 24, by + 3,  idChannelUp,   "C+", COL_CH, COL_CH_BG);
        drawCtrlBtn(bx + 24, by + 17, idChannelDown, "C-", COL_CH, COL_CH_BG);
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 24, by + 31);
        mainOS->sprite.print("[]");
    }

    // -- TEMP --
    if (tempUp != -1 || tempDown != -1)
    {
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 49, by);
        mainOS->sprite.print("T");
        drawCtrlBtn(bx + 48, by + 3,  tempUp,   "T+", COL_TEMP_UP, 0x1000);
        drawCtrlBtn(bx + 48, by + 17, tempDown, "T-", COL_TEMP_DN, 0x0001);
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 48, by + 31);
        mainOS->sprite.print("';");
    }

    // -- MUTE --
    if (idxMute != -1)
    {
        bool mutePr = (InCorrentCommand == idxMute);
        uint16_t mg = mutePr ? COL_MUTE : 0x1000;
        mainOS->sprite.fillRoundRect(bx + 24, by + 43, 22, 12, 2, mg);
        mainOS->sprite.drawRoundRect(bx + 24, by + 43, 22, 12, 2, COL_MUTE);
        mainOS->sprite.setTextColor(mutePr ? TFT_BLACK : COL_MUTE);
        mainOS->sprite.setCursor(bx + 25, by + 46);
        mainOS->sprite.print("MUT");
        mainOS->sprite.setTextColor(COL_KEY);
        mainOS->sprite.setCursor(bx + 18, by + 46);
        mainOS->sprite.print("M");
    }

    // ===== מספרים 0-9 =====
    bool hasNums = false;
    for (int n = 0; n <= 9; n++) if (idxNum[n] != -1) { hasNums = true; break; }

    if (hasNums)
    {
        int numY = 118;
        int numX = 2;
        mainOS->sprite.drawFastHLine(0, numY - 3, 150, COL_BORDER);
        for (int n = 0; n <= 9; n++)
        {
            if (idxNum[n] == -1) continue;
            bool pr = (InCorrentCommand == idxNum[n]);
            uint16_t bg = pr ? COL_ACTIVE : COL_BG2;
            uint16_t fg = pr ? TFT_BLACK  : COL_DPAD;
            mainOS->sprite.fillRect(numX + n * 14, numY, 12, 10, bg);
            mainOS->sprite.drawRect(numX + n * 14, numY, 12, 10, COL_BORDER);
            mainOS->sprite.setTextColor(fg);
            mainOS->sprite.setCursor(numX + n * 14 + 3, numY + 2);
            mainOS->sprite.print(n);
        }
    }

    // ===== שורת Footer =====
    mainOS->sprite.drawFastHLine(0, 124, 240, COL_BORDER);
    mainOS->sprite.setTextColor(COL_HINT);
    mainOS->sprite.setCursor(2, 126);
    String footer = "";
    if (hasNums)       footer += "0-9 ";
    if (idxPower != -1) footer += "P:pwr ";
    if (idxMute  != -1) footer += "M:mute ";
    if (hasArrows)      footer += "arrows+ENT";
    mainOS->sprite.print(footer);

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}


*/

void IRSenderApp::OnExit()
{
}

bool IRSenderApp::SendCommand(const String &commandName, const String &filePathh)
{
    return loadAndSendFromFile(commandName, filePathh);
}

std::vector<String> IRSenderApp::getAllCommandNamesFromFile(const String &filePath)
{

    std::vector<String> names;

    File f = SD.open(filePath, "r");
    if (!f)
    {
        Serial.println("Failed to open file: " + filePath);
        return names;
    }

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.startsWith("name: "))
        {
            names.push_back(line.substring(6));
        }
    }

    f.close();
    return names;
}

uint32_t IRSenderApp::getFrequencyFromFile(const String &name, const String &filePath)
{
    File f = SD.open(filePath, "r");
    if (!f)
    {
        return 38000;
    }

    bool found = false;
    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line == "name: " + name)
        {
            found = true;
        }
        else if (found && line.startsWith("frequency: "))
        {
            uint32_t frq = line.substring(11).toInt();
            // uint32_t frq = line.substring(String("frequency: ").length()).toInt();
            // uint32_t frq = line.substring(6).toInt();
            f.close();

            return (frq > 0) ? frq : 38000;
        }
        else if (found && line.startsWith("name: "))
        { // התחלנו בלוק חדש בלי למצוא freq
            break;
        }
    }

    f.close();
    return 38000;
}

bool IRSenderApp::loadAndSendFromFile(const String &name, const String &filePath)
{
    File f = SD.open(filePath, "r");
    if (!f)
        return false;

    bool found = false;
    // uint32_t freq = 38000; // דיפולט
    uint32_t freq = getFrequencyFromFile(CommandFromFile[InCorrentCommand], mainOS->FileSelectedInFS);

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();

        if (line == "name: " + name)
        {
            found = true;
        }
        else if (found && line.startsWith("frequency: "))
        {
            freq = line.substring(11).toInt();
        }
        else if (found && line.startsWith("data: "))
        {
            line.remove(0, 6);
            uint16_t raw[400];
            int count = 0;
            char buf[line.length() + 1];
            line.toCharArray(buf, sizeof(buf));
            char *tok = strtok(buf, " ");
            while (tok && count < 400)
            {
                raw[count++] = (uint16_t)atoi(tok);
                tok = strtok(nullptr, " ");
            }

            // השליחה האמיתית עם התדר מהקובץ!
            ir.sendRaw(raw, count, freq);
            f.close();
            return true;
        }
    }
    f.close();
    return false;
} 

void IRSenderApp::Draw()
{
    mainOS->sprite.createSprite(240, 135);
    mainOS->sprite.fillSprite(0x080C); // #0d0d1a - כחול-שחור עמוק

    bool hasArrows = (idxUp != -1 || idxDown != -1 || idxLeft != -1 || idxRight != -1 || idxEnter != -1);

    // ===== צבעי NEON =====
    uint16_t COL_BG       = 0x080C; // #0d0d1a רקע
    uint16_t COL_BG2      = 0x1092; // #111130 תאים
    uint16_t COL_BORDER   = 0x190C; // #1e1e3a גבולות
    uint16_t COL_TEXT     = 0xC618; // #c0c0e0 טקסט רגיל
    uint16_t COL_KEY      = 0xF7E0; // #f0c040 מקש QWERTY (צהוב)
    uint16_t COL_ACTIVE   = 0x37E6; // #30ff80 פקודה פעילה (ירוק ניאון)
    uint16_t COL_ACTIVE_BG= 0x0291; // #021818 רקע פקודה פעילה
    uint16_t COL_HINT     = 0x318C; // #303060 טקסט hint תחתון
    uint16_t COL_DPAD     = 0x8C10; // #8080c0 D-pad כחול-לבנדר
    uint16_t COL_DPAD_BG  = 0x1092; // #111130
    uint16_t COL_DPAD_OK  = 0x37E6; // #30ff80 ירוק
    uint16_t COL_DPAD_OKBG= 0x0291; // #082818
    uint16_t COL_PWR_BG   = 0x8000; // #880000 אדום כהה
    uint16_t COL_PWR      = 0xF800; // #ff0000 אדום
    uint16_t COL_PWR_KEY  = 0xF7E0; // צהוב P
    uint16_t COL_VOL      = 0x047F; // #0080ff כחול ניאון
    uint16_t COL_VOL_BG   = 0x000C; // #00001a
    uint16_t COL_CH       = 0x07E8; // #00ff40 ירוק ניאון
    uint16_t COL_CH_BG    = 0x0001; // #000010
    uint16_t COL_TEMP_UP  = 0xFC20; // #ff8000 כתום
    uint16_t COL_TEMP_DN  = 0x04FF; // #0088ff כחול
    uint16_t COL_MUTE     = 0xFEA0; // #ffa000 ענבר
    uint16_t COL_SEC      = 0x318C; // #303060 משני

    // ===== שורת כותרת עליונה =====
    mainOS->sprite.fillRect(0, 0, 240, 10, COL_BORDER);
    mainOS->sprite.drawFastHLine(0, 10, 240, COL_BORDER);

    // שם המכשיר - מרכז
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextColor(0x7BCF); // #7c7caa
    mainOS->sprite.setTextWrap(false);
    // מרכז הטקסט של שם המכשיר
    int devNameLen = DeviceID.length();
    int devX = (240 - devNameLen * 6) / 2;
    mainOS->sprite.setCursor(devX, 2);
    mainOS->sprite.print(DeviceID);

    // ===== POWER button - פינה עליונה ימנית =====
    if (idxPower != -1)
    {
        bool pwrPressed = (InCorrentCommand == idxPower);
        uint16_t pwrFill = pwrPressed ? COL_PWR : COL_PWR_BG;
        mainOS->sprite.fillRoundRect(209, 1, 28, 8, 2, pwrFill);
        mainOS->sprite.drawRoundRect(209, 1, 28, 8, 2, COL_PWR);
        mainOS->sprite.setTextColor(COL_PWR_KEY);
        mainOS->sprite.setCursor(211, 3);
        mainOS->sprite.print("P");
        mainOS->sprite.setTextColor(pwrPressed ? TFT_WHITE : COL_PWR);
        mainOS->sprite.setCursor(218, 3);
        mainOS->sprite.print("ON");
    }

    // ===== בניית רשימת פקודות כלליות (ללא מיוחדות) =====
    std::vector<int> displayList;
    for (int i = 0; i < (int)CommandFromFile.size(); i++)
    {
        bool isSpecial = false;
        for (int n = 0; n <= 9; n++)
            if (idxNum[n] == i) { isSpecial = true; break; }
        if (!isSpecial &&
            i != idxUp && i != idxDown && i != idxLeft && i != idxRight &&
            i != idxEnter && i != idxPower && i != idxMute &&
            i != idVolUp && i != idVolDown &&
            i != idChannelUp && i != idChannelDown &&
            i != tempUp && i != tempDown)
        {
            displayList.push_back(i);
        }
    }

    // ===== עמודות פקודות =====
    int listX      = 2;
    int listTop    = 13;
    int rowH       = 11;
    int listW      = hasArrows ? 148 : 200; // צמצם אם יש D-pad
    int numCols    = 2;
    int colW       = listW / numCols;
    int maxRows    = 9;//(135 - listTop - 10) / rowH; // שמור מקום לפוטר

    for (int k = 0; k < (int)displayList.size(); k++)
    {
        int i   = displayList[k];
        int col = k / maxRows;
        int row = k % maxRows;
        if (col >= numCols) break;

        int x = listX + col * colW;
        int y = listTop + row * rowH;

        bool isActive = (i == InCorrentCommand);

        // רקע שורה פעילה
        if (isActive)
        {
            mainOS->sprite.fillRect(x - 1, y - 1, colW - 2, rowH, COL_ACTIVE_BG);
            mainOS->sprite.drawRect(x - 1, y - 1, colW - 2, rowH, COL_ACTIVE);
        }

        // מקש QWERTY
        for (int j = 0; j < (int)letterCmdIndices.size(); j++)
        {
            if (letterCmdIndices[j] == i)
            {
                mainOS->sprite.setTextColor(isActive ? COL_KEY : COL_KEY);
                mainOS->sprite.setCursor(x, y);
                char kbuf[3] = { (char)toupper(letterMap[j]), ':', 0 };
                mainOS->sprite.print(kbuf);
                x += 13;
                break;
            }
        }

        // שם פקודה
        mainOS->sprite.setTextColor(isActive ? COL_ACTIVE : COL_TEXT);
        mainOS->sprite.setCursor(x, y);
        String dName = CommandFromFile[i];
        int maxCh = (colW - 14) / 6;
        if ((int)dName.length() > maxCh && maxCh > 3)
            dName = dName.substring(0, maxCh - 2) + "..";
        mainOS->sprite.print(dName);

        // נקודת ACTIVE בצד
        if (isActive)
        {
            mainOS->sprite.fillCircle(listX + colW - 4, y + 4, 2, COL_ACTIVE);
        }
    }

    // ===== קו מפריד אנכי בין רשימה לבקרות =====
    if (hasArrows)
        mainOS->sprite.drawFastVLine(listW + 2, 12, 110, COL_BORDER);

    // ===== D-PAD עם חיצים מצוירים =====
    if (hasArrows)
    {
        int px = 195; // מרכז כפתור האמצע
        int py = 95;  // הורד יותר למטה כדי לא לחפוף עם הבקרות
        int s  = 19;  // מרווח בין כפתורים

        // ציור כפתור D-pad עם חץ גרפי
        // dir: 0=UP, 1=DOWN, 2=LEFT, 3=RIGHT, 4=OK
        auto drawDpadArrow = [&](int x, int y, int idx, int dir)
        {
            if (idx == -1) return;
            bool pressed = (InCorrentCommand == idx);
            bool isOK    = (dir == 4);

            uint16_t bg  = isOK ? (pressed ? COL_DPAD_OK : COL_DPAD_OKBG)
                                : (pressed ? COL_ACTIVE   : COL_DPAD_BG);
            uint16_t brd = isOK ? COL_DPAD_OK : (pressed ? COL_ACTIVE : COL_BORDER);
            uint16_t arrowCol = isOK ? (pressed ? TFT_BLACK : COL_DPAD_OK)
                                     : (pressed ? TFT_BLACK : COL_DPAD);

            mainOS->sprite.fillRoundRect(x, y, 17, 13, 2, bg);
            mainOS->sprite.drawRoundRect(x, y, 17, 13, 2, brd);

            // מרכז הכפתור
            int cx = x + 8;
            int cy = y + 6;

            if (isOK)
            {
                // כפתור OK - טקסט קטן
                mainOS->sprite.setTextColor(arrowCol);
                mainOS->sprite.setCursor(x + 2, y + 4);
                mainOS->sprite.print("OK");
            }
            else if (dir == 0) // UP - משולש למעלה
            {
                mainOS->sprite.fillTriangle(cx, cy - 4, cx - 4, cy + 3, cx + 4, cy + 3, arrowCol);
            }
            else if (dir == 1) // DOWN - משולש למטה
            {
                mainOS->sprite.fillTriangle(cx, cy + 4, cx - 4, cy - 3, cx + 4, cy - 3, arrowCol);
            }
            else if (dir == 2) // LEFT - משולש שמאל
            {
                mainOS->sprite.fillTriangle(cx - 4, cy, cx + 3, cy - 4, cx + 3, cy + 4, arrowCol);
            }
            else if (dir == 3) // RIGHT - משולש ימין
            {
                mainOS->sprite.fillTriangle(cx + 4, cy, cx - 3, cy - 4, cx - 3, cy + 4, arrowCol);
            }
        };

        drawDpadArrow(px - 8,     py - s,  idxUp,    0);
        drawDpadArrow(px - 8,     py + s,  idxDown,  1);
        drawDpadArrow(px - 8 - s, py,      idxLeft,  2);
        drawDpadArrow(px - 8 + s, py,      idxRight, 3);
        drawDpadArrow(px - 8,     py,      idxEnter, 4);
    }

    // ===== כפתורי בקרה קטנים: VOL / CH / TEMP / MUTE =====
    int bx = 154;
    int by = 13;

    auto drawCtrlBtn = [&](int x, int y, int idx, const char* label,
                           uint16_t fg, uint16_t bg_idle)
    {
        if (idx == -1) return;
        bool pressed = (InCorrentCommand == idx);
        uint16_t bg  = pressed ? fg : bg_idle;
        uint16_t brd = fg;
        uint16_t txt = pressed ? TFT_BLACK : fg;

        mainOS->sprite.fillRoundRect(x, y, 22, 12, 2, bg);
        mainOS->sprite.drawRoundRect(x, y, 22, 12, 2, brd);
        mainOS->sprite.setTextColor(txt);
        int lx = x + (22 - (int)strlen(label) * 6) / 2;
        mainOS->sprite.setCursor(lx, y + 3);
        mainOS->sprite.print(label);
    };

    // -- VOL --
    if (idVolUp != -1 || idVolDown != -1)
    {
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 2, by);
        mainOS->sprite.print("V");
        drawCtrlBtn(bx, by + 3,  idVolUp,   "V+", COL_VOL,     COL_VOL_BG);
        drawCtrlBtn(bx, by + 17, idVolDown, "V-", COL_VOL,     COL_VOL_BG);
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 1, by + 31);
        mainOS->sprite.print("+-");
    }

    // -- CH --
    if (idChannelUp != -1 || idChannelDown != -1)
    {
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 26, by);
        mainOS->sprite.print("C");
        drawCtrlBtn(bx + 24, by + 3,  idChannelUp,   "C+", COL_CH, COL_CH_BG);
        drawCtrlBtn(bx + 24, by + 17, idChannelDown, "C-", COL_CH, COL_CH_BG);
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 24, by + 31);
        mainOS->sprite.print("[]");
    }

    // -- TEMP --
    if (tempUp != -1 || tempDown != -1)
    {
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 49, by);
        mainOS->sprite.print("T");
        drawCtrlBtn(bx + 48, by + 3,  tempUp,   "T+", COL_TEMP_UP, 0x1000);
        drawCtrlBtn(bx + 48, by + 17, tempDown, "T-", COL_TEMP_DN, 0x0001);
        mainOS->sprite.setTextColor(COL_SEC);
        mainOS->sprite.setCursor(bx + 48, by + 31);
        mainOS->sprite.print("';");
    }

    // -- MUTE -- מתחת לכל כפתורי הבקרה, לא חופף D-pad
    if (idxMute != -1)
    {
        bool mutePr = (InCorrentCommand == idxMute);
        uint16_t mg = mutePr ? COL_MUTE : 0x1000;
        mainOS->sprite.fillRoundRect(bx, by + 48, 70, 12, 2, mg);
        mainOS->sprite.drawRoundRect(bx, by + 48, 70, 12, 2, COL_MUTE);
        mainOS->sprite.setTextColor(COL_KEY);
        mainOS->sprite.setCursor(bx + 2, by + 51);
        mainOS->sprite.print("M");
        mainOS->sprite.setTextColor(mutePr ? TFT_BLACK : COL_MUTE);
        mainOS->sprite.setCursor(bx + 10, by + 51);
        mainOS->sprite.print("MUTE");
    }

    // ===== מספרים 0-9 — מתחת לרשימה, מעל הפוטר =====
    bool hasNums = false;
    for (int n = 0; n <= 9; n++) if (idxNum[n] != -1) { hasNums = true; break; }

    // חשב כמה שורות תפסה הרשימה בפועל
    int usedRows = (int)displayList.size() > maxRows ? maxRows : (int)displayList.size();
    int listBottom = listTop + usedRows * rowH + 2;
    // רצפה: הניחה תמיד שיש מקום ל-footer ב-125, numbers ב-113
    int numsY   = 113;
    int footerY = 126;

    if (hasNums)
    {
        mainOS->sprite.drawFastHLine(0, numsY - 2, listW + 2, COL_BORDER);
        for (int n = 0; n <= 9; n++)
        {
            if (idxNum[n] == -1) continue;
            bool pr = (InCorrentCommand == idxNum[n]);
            uint16_t bg = pr ? COL_ACTIVE : COL_BG2;
            uint16_t fg = pr ? TFT_BLACK  : COL_DPAD;
            int nx = 2 + n * 14;
            mainOS->sprite.fillRect(nx, numsY, 12, 10, bg);
            mainOS->sprite.drawRect(nx, numsY, 12, 10, COL_BORDER);
            mainOS->sprite.setTextColor(fg);
            mainOS->sprite.setCursor(nx + 3, numsY + 2);
            mainOS->sprite.print(n);
        }
    }

    // ===== שורת Footer =====
    mainOS->sprite.drawFastHLine(0, footerY - 1, 240, COL_BORDER);
    mainOS->sprite.setTextColor(COL_HINT);
    mainOS->sprite.setCursor(2, footerY);
    String footer = "";
    if (hasNums)        footer += "0-9 ";
    if (idxPower != -1) footer += "P:pwr ";
    if (idxMute  != -1) footer += "M:mute ";
    if (hasArrows)      footer += "arr+ENTER";
    mainOS->sprite.print(footer);

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}