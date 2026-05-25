#include "Calculator.h"
#include "./MyOS.h"
#include <vector>
#include <math.h> 
#include "./Classes/MainMenuV2.h"

// הגדרת צבעים לעיצוב מודרני
#define COLOR_BG        0x0000 // שחור
#define COLOR_HEADER    0x0038 // כחול כהה
#define COLOR_FOOTER    0x001D // אפור כהה מאוד
#define COLOR_BTN       0x1234 // אפור-כחלל לכפתורים
#define COLOR_BTN_SEL   0x000F // כחול זוהר לכפתור נבחר
#define COLOR_TEXT      0x07FF // סייאן (Cyan)
#define COLOR_RESULT    0xFFFF // לבן לתוצאה

const char *opButtons[] = {"(", ")", "+", "-", "*", "/", ".", "s", "="};
const int opCount = 9;

int selIndex = 0;
String inputBuffer = "";
String result = "";

void Calculator::Begin()
{
    inputBuffer = "";
    result = "";
}

void Calculator::Loop()
{
    // GlobalParentClass::Loop();
    
    // ניווט לתפריט נוסף
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
    }

    // קליטת מקלדת פיזית
    if (M5Cardputer.Keyboard.isChange())
    {
        auto ks = M5Cardputer.Keyboard.keysState();
        for (auto k : ks.word)
            if ((k >= '0' && k <= '9') || k == '.')
            {
                inputBuffer += (char)k;
            }
    }

    // ניווט בין כפתורים
    if (mainOS->NewKey.ifKeyJustPress(',')) // חץ שמאל / מקש קודם
        selIndex = (selIndex - 1 + opCount) % opCount;

    if (mainOS->NewKey.ifKeyJustPress('/')) // חץ ימין / מקש הבא (שים לב: / משמש גם כפעולה, זה עשוי ליצור קונפליקט אם לא מטפלים בזה, מומלץ להשתמש בחיצים)
        selIndex = (selIndex + 1) % opCount;

    // לחיצה על Enter
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        String op = opButtons[selIndex];
        if (op == "=")
        {
            Evaluate();
        }
        else
        {
            inputBuffer += op;
        }
    }

    // BACKSPACE (מקש פיזי או כפתור וירטואלי אם תרצה להוסיף)
    if (mainOS->NewKey.ifKeyJustPress(KEY_BACKSPACE))
    {
        if (inputBuffer.length() > 0)
            inputBuffer.remove(inputBuffer.length() - 1);
    }
    
    // רישום
    if (!mainOS->screenOff)
    {
        Draw();
    }
}

void Calculator::Draw()
{
    // יצירת Sprite בגודל המסך המלא (240x135)
    mainOS->sprite.createSprite(240, 135-TopOffset);
    
    // 1. רקע כללי
    mainOS->sprite.fillScreen(COLOR_BG);

    // 2. Header (כותרת עליונה)
    mainOS->sprite.fillRoundRect(0, 0, 240, 25, 0, COLOR_HEADER); // פס כחול
    mainOS->sprite.setTextColor(TFT_WHITE, COLOR_HEADER);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(5, 8);
    mainOS->sprite.print("CALCULATOR");
    
    // אייקון קטן בצד ימין של ההדר
    mainOS->sprite.drawChar(230, 6, '=', 0xFFFF, COLOR_HEADER, 1);

    // 3. תצוגת התוצאה (החלק המרכזי והבולט)
    mainOS->sprite.setTextColor(COLOR_RESULT, COLOR_BG);
    mainOS->sprite.setTextSize(2);
    // מציג את התוצאה או הודעת שגיאה
    if (result == "") result = "0"; // ערך ברירת מחדל
    mainOS->sprite.setCursor(10, 45);
    mainOS->sprite.print(result);

    // 4. תצוגת הזיכרון (הקלט הנוכחי)
    mainOS->sprite.setTextColor(COLOR_TEXT, COLOR_BG);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(10, 30);
    mainOS->sprite.print(inputBuffer);

    // 5. רשת הכפתורים (החלק התחתון)
    // החלפנו את החישוב המתמטי הנוקשה לעיצוב גריד נקי יותר
    int startY = 85;
    int btnW = 24;  // רוחב כפתור מעט גדול יותר
    int btnH = 30;  // גובה כפתור
    int spacing = 1;
    int startX = 10; // מרווח מהצד

    for (int i = 0; i < opCount; i++)
    {
        int x = startX + (i * (btnW + spacing));
        bool sel = (i == selIndex);

        // צבע הכפתור: אם נבחר - זוהר, אחרת - אפור כהה
        uint16_t btnColor = sel ? COLOR_BTN_SEL : COLOR_BTN;
        
        // ציור הכפתור עם פינות עגולות
        mainOS->sprite.fillRoundRect(x, startY, btnW, btnH, 4, btnColor);
        
        // מסגרת עדינה לכפתור הנבחר
        if (sel) {
            mainOS->sprite.drawRoundRect(x, startY, btnW, btnH, 4, 0x00FF); // מסגרת ירוקה/צהובה
        }

        // טקסט הכפתור
        mainOS->sprite.setTextColor(TFT_WHITE, btnColor);
        mainOS->sprite.setTextSize(1);
        mainOS->sprite.setCursor(x + 8, startY + 10); // מרכז יחסית
        mainOS->sprite.print(opButtons[i]);
    }

    // 6. Footer (תפריט תחתון)
    // ציור פס תחתון אפור
/*     mainOS->sprite.fillRoundRect(0, 118, 240, 17, 0, COLOR_FOOTER);
    
    // כפתור Extra בצד שמאל
    mainOS->sprite.setTextColor(TFT_WHITE, COLOR_FOOTER);
    mainOS->sprite.setCursor(5, 122);
    mainOS->sprite.print("[`] Extra");

    // כפתור מחיקה (Backspace) בצד ימין (וירטואלי)
    mainOS->sprite.setTextColor(TFT_RED, COLOR_FOOTER); // אדום למחיקה
    mainOS->sprite.setCursor(170, 122);
    mainOS->sprite.print("DEL"); */

    // הדפסה למסך
    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

// --- שאר הפונקציות (Evaluate וכו') נשארות בדיוק כמו שהיו ---

float eval(const String &expr, bool &ok);
float evalFlat(const String &expr, bool &ok);

void Calculator::Evaluate()
{
    bool ok = true;
    float v = eval(inputBuffer, ok);

    if (!ok || isnan(v))
    {
        result = "ERR";
        inputBuffer = "";
        return;
    }

    if (v == (int)v)
        result = String((int)v);
    else
        result = String(v, 4);

    inputBuffer = "";
}

float eval(const String &expr, bool &ok)
{
    String s = "";
    int len = expr.length();

    for (int i = 0; i < len; i++)
    {
        char c = expr[i];

        if (c == 's')
        {
            int start = i + 1;
            float val = 0;

            if (start >= len) { ok = false; return NAN; }

            if (expr[start] == '(')
            {
                int depth = 1;
                int j = start + 1;
                while (j < len && depth > 0)
                {
                    if (expr[j] == '(') depth++;
                    else if (expr[j] == ')') depth--;
                    j++;
                }
                String inside = expr.substring(start + 1, j - 1);
                val = eval(inside, ok);
                i = j - 1;
            }
            else
            {
                int j = start;
                while (j < len && (isdigit(expr[j]) || expr[j] == '.')) j++;
                val = expr.substring(start, j).toFloat();
                i = j - 1; 
            }

            if (val < 0) { ok = false; return NAN; }
            s += String(sqrt(val));
        }
        else if (c == '(')
        {
            int depth = 1;
            int j = i + 1;
            while (j < len && depth > 0)
            {
                if (expr[j] == '(') depth++;
                else if (expr[j] == ')') depth--;
                j++;
            }
            float insideVal = eval(expr.substring(i + 1, j - 1), ok);
            s += String(insideVal);
            i = j - 1;
        }
        else
        {
            s += c;
        }
    }
    return evalFlat(s, ok);
}

float evalFlat(const String &expr, bool &ok)
{
    std::vector<float> nums;
    std::vector<char> ops;

    String cur = "";
    bool negative = false;

    for (int i = 0; i < expr.length(); i++)
    {
        char c = expr[i];

        if (c == '-' && (i == 0 || strchr("+-*/(", expr[i - 1])))
        {
            cur += '-';
        }
        else if ((c >= '0' && c <= '9') || c == '.')
        {
            cur += c;
        }
        else if (c == '+' || c == '-' || c == '*' || c == '/')
        {
            if (cur == "")
            {
                ok = false;
                return NAN;
            }

            nums.push_back(cur.toFloat());
            cur = "";
            ops.push_back(c);
        }
    }

    if (cur != "")
        nums.push_back(cur.toFloat());

    if (nums.size() == 0)
    {
        ok = false;
        return NAN;
    }

    for (int i = 0; i < ops.size(); i++)
    {
        if (ops[i] == '*' || ops[i] == '/')
        {
            float a = nums[i];
            float b = nums[i + 1];

            if (ops[i] == '*')
                nums[i] = a * b;
            else
            {
                if (b == 0)
                {
                    ok = false;
                    return NAN;
                }
                nums[i] = a / b;
            }

            nums.erase(nums.begin() + i + 1);
            ops.erase(ops.begin() + i);
            i--;
        }
    }

    float res = nums[0];
    for (int i = 0; i < ops.size(); i++)
    {
        if (ops[i] == '+')
            res += nums[i + 1];
        else if (ops[i] == '-')
            res -= nums[i + 1];
    }

    return res;
}
