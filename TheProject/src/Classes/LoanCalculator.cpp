#include "LoanCalculator.h"
#include <math.h>
#include "MyOS.h"
#include "./Classes/MainMenuV2.h"

// ─────────────────────────────────────────────
//  צבעים וקבועים
// ─────────────────────────────────────────────
#define SCREEN_W 240
#define SCREEN_H 135

#define CLR_BG 0x0841       // רקע כחול-שחור
#define CLR_PANEL 0x2104    // פאנל כהה
#define CLR_SELECTED 0x2D7F // תכלת – שדה מסומן
#define CLR_FILLED 0x1A45   // ירקרק כהה – שדה מלא
#define CLR_SOLVED 0xFD20   // כתום – השדה שחושב
#define CLR_EMPTY 0x18C3    // אפור-כחול – שדה ריק
#define CLR_BTN 0x07E0      // ירוק – כפתור חשב
#define CLR_BTN_TXT 0x0000  // שחור על כפתור
#define CLR_WHITE 0xFFFF
#define CLR_GRAY 0x8410
#define CLR_YELLOW 0xFFE0
#define CLR_RED 0xF800
#define CLR_CYAN 0x07FF
#define CLR_ACCENT 0x07E0

// ─────────────────────────────────────────────
//  Begin
// ─────────────────────────────────────────────
void LoanCalculator::Begin()
{
    showTopBar = false;
    currentState = State::INPUTT;
    selectedField = 0;
    solvedField = -1;
    ClearAll();
}

// ─────────────────────────────────────────────
//  Loop
// ─────────────────────────────────────────────
void LoanCalculator::Loop()
{
        if (mainOS->NewKey.ifKeyJustPress('`')) // esc
{
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
        return;

}
    if (currentState == State::INPUTT)
    {
        // ── ניווט בין שדות ──────────────────
        if (mainOS->NewKey.ifKeyJustPress(';')) // UP
        {
            selectedField = (selectedField - 1 + FIELD_COUNT) % FIELD_COUNT;
            delay(220);
        }
        if (M5Cardputer.Keyboard.isKeyPressed('.')) // DOWN
        {
            selectedField = (selectedField + 1) % FIELD_COUNT;
            delay(220);
        }

        // ── עריכה / מחיקה ──────────────────
        if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER)) // P = עריכת שדה
        {
            delay(150);
            EditSelectedField();
        }

        if (mainOS->NewKey.ifKeyJustPress(KEY_BACKSPACE)) // LEFT = נקה שדה
        {
            delay(150);
            ClearField(selectedField);
            solvedField = -1;
        }

        // ── כפתור חשב (RIGHT = '/') ─────────

        if (mainOS->NewKey.ifKeyJustPress('/')) // RIGHT = חשב
        {
            delay(150);
            if (TrySolve())
            {
                CalcTotals();
                currentState = State::RESULT;
            }
        }
    }
    else // RESULT
    {
        // כל כפתור → חזרה לעריכה
        if (M5Cardputer.Keyboard.isKeyPressed('p') ||
            M5Cardputer.Keyboard.isKeyPressed(',') ||
            M5Cardputer.Keyboard.isKeyPressed('/'))
        {
            delay(150);
            currentState = State::INPUTT;
        }
    }

    Draw();
}

// ─────────────────────────────────────────────
//  Draw – dispatcher
// ─────────────────────────────────────────────
void LoanCalculator::Draw()
{
    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H);
    mainOS->sprite.fillSprite(CLR_BG);

    if (currentState == State::INPUTT)
        DrawInput();
    else
        DrawResult();

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

// ─────────────────────────────────────────────
//  DrawInput – 4 שדות + כפתור חשב
// ─────────────────────────────────────────────
void LoanCalculator::DrawInput()
{
    // כותרת
    mainOS->sprite.fillRoundRect(0, 0, SCREEN_W, 18, 0, CLR_SELECTED);
    mainOS->sprite.setTextColor(CLR_WHITE);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(55, 5);
    mainOS->sprite.print("LOAN  CALCULATOR");

    // ── 4 שדות ──────────────────────────────
    // כל שדה: גובה 22px, רווח 2px, מתחיל מ-y=22
    int fieldW = 176;
    int fieldH = 22;
    int fieldX = 2;
    int startY = 21;
    int spacing = 2;

    for (int i = 0; i < FIELD_COUNT; i++)
    {
        int y = startY + i * (fieldH + spacing);

        // צבע רקע לפי מצב
        uint16_t bgColor;
        if (i == solvedField)
            bgColor = CLR_SOLVED;
        else if (i == selectedField)
            bgColor = CLR_SELECTED;
        else if (fieldFilled[i])
            bgColor = CLR_FILLED;
        else
            bgColor = CLR_EMPTY;

        mainOS->sprite.fillRoundRect(fieldX, y, fieldW, fieldH, 4, bgColor);

        // label
        mainOS->sprite.setTextColor(CLR_WHITE);
        mainOS->sprite.setTextSize(1);
        mainOS->sprite.setCursor(fieldX + 4, y + 4);
        mainOS->sprite.print(FieldLabel(i));

        // ערך
        mainOS->sprite.setTextSize(1);
        mainOS->sprite.setCursor(fieldX + 4, y + 13);
        if (fieldFilled[i])
        {
            mainOS->sprite.setTextColor((i == solvedField) ? CLR_BG : CLR_YELLOW);
            mainOS->sprite.print(FieldStr(i));
            mainOS->sprite.print(" ");
            mainOS->sprite.print(FieldUnit(i));
        }
        else
        {
            mainOS->sprite.setTextColor(CLR_GRAY);
            mainOS->sprite.print("---");
        }

        // חץ בשדה מסומן
        if (i == selectedField)
        {
            mainOS->sprite.setTextColor(CLR_WHITE);
            mainOS->sprite.setCursor(fieldX + fieldW - 10, y + 7);
            mainOS->sprite.print(">");
        }
    }

    // ── כפתור חשב ────────────────────────────
    int btnX = fieldX + fieldW + 4;
    int btnW = SCREEN_W - btnX - 2;

    // ספירת שדות מלאים
    int filled = 0;
    for (int i = 0; i < FIELD_COUNT; i++)
        if (fieldFilled[i])
            filled++;

    bool canCalc = (filled == 3);
    uint16_t btnColor = canCalc ? CLR_BTN : CLR_GRAY;

    mainOS->sprite.fillRoundRect(btnX, startY, btnW, (fieldH + spacing) * FIELD_COUNT - spacing, 5, btnColor);
    mainOS->sprite.setTextColor(canCalc ? CLR_BTN_TXT : CLR_BG);
    mainOS->sprite.setTextSize(1);

    // טקסט מרכזי
    int bCenterY = startY + ((fieldH + spacing) * FIELD_COUNT) / 2 - 16;
    mainOS->sprite.setCursor(btnX + 4, bCenterY);
    mainOS->sprite.print("CALC");
    mainOS->sprite.setCursor(btnX + 6, bCenterY + 12);
    mainOS->sprite.print(canCalc ? ">>>" : String(filled) + "/3");

    // ── רצועת עזרה תחתונה ────────────────────
    mainOS->sprite.setTextColor(CLR_GRAY);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(2, SCREEN_H - 10);
    mainOS->sprite.print("ENTER:edit  DEL:clear  ->:calc");
}

// ─────────────────────────────────────────────
//  DrawResult – תוצאות מפורטות
// ─────────────────────────────────────────────
void LoanCalculator::DrawResult()
{
    // כותרת
    mainOS->sprite.fillRoundRect(0, 0, SCREEN_W, 18, 0, CLR_BTN);
    mainOS->sprite.setTextColor(CLR_BG);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(70, 5);
    mainOS->sprite.print("LOAN  SUMMARY");

    int y = 22;
    int lh = 14; // line height

    auto row = [&](const char *label, String val, uint16_t col)
    {
        mainOS->sprite.setTextColor(CLR_GRAY);
        mainOS->sprite.setCursor(4, y);
        mainOS->sprite.print(label);
        mainOS->sprite.setTextColor(col);
        mainOS->sprite.setCursor(130, y);
        mainOS->sprite.print(val);
        y += lh;
    };

    row("Loan Amount:", FloatToStr(fieldValues[0]) + " ", CLR_WHITE);
    row("Annual Rate:", FloatToStr(fieldValues[1]) + " %", CLR_WHITE);
    row("Duration:", String((int)fieldValues[2]) + " Month", CLR_WHITE);
    row("Monthly Pay:", FloatToStr(fieldValues[3]) + " ", CLR_CYAN);

    // קו
    mainOS->sprite.drawLine(4, y + 1, SCREEN_W - 4, y + 1, CLR_GRAY);
    y += 6;

    row("Total Payment:", FloatToStr(totalPayment) + " ", CLR_YELLOW);
    row("Total Interest:", FloatToStr(totalInterest) + " ", CLR_RED);

    // אחוז ריבית אפקטיבית
    float effRate = (fieldValues[0] > 0) ? (totalInterest / fieldValues[0] * 100.0f) : 0;
    row("Interest/Principal:", FloatToStr(effRate) + " %", CLR_RED);

    // חזרה
    mainOS->sprite.setTextColor(CLR_GRAY);
    mainOS->sprite.setCursor(4, SCREEN_H - 10);
    mainOS->sprite.print("[any key] back to edit");
}

// ─────────────────────────────────────────────
//  EditSelectedField – פותח AskFromUser
// ─────────────────────────────────────────────
void LoanCalculator::EditSelectedField()
{
    String question;
    bool onlyDigit = false;

    switch (selectedField)
    {
    case 0:
        question = "Loan Amount :";
        onlyDigit = false;
        break;
    case 1:
        question = "Annual Interest Rate (%):";
        onlyDigit = false;
        break;
    case 2:
        question = "Duration (months) HINT Years * 12:";
        onlyDigit = true;
        break;
    case 3:
        question = "Monthly Payment :";
        onlyDigit = false;
        break;
    }

    String result = mainOS->AskFromUserForString(question, false, false, true);

    if (result.length() > 0)
    {
        float val = result.toFloat();
        if (val > 0)
        {
            fieldValues[selectedField] = val;
            fieldFilled[selectedField] = true;
            solvedField = -1; // איפוס שדה מחושב
        }
    }
}

// ─────────────────────────────────────────────
//  TrySolve – מוצא איזה שדה חסר ומחשב
// ─────────────────────────────────────────────
bool LoanCalculator::TrySolve()
{
    // ספור כמה שדות מלאים ומי ריק
    int filledCount = 0;
    int emptyIdx = -1;

    for (int i = 0; i < FIELD_COUNT; i++)
    {
        if (fieldFilled[i])
            filledCount++;
        else
            emptyIdx = i;
    }

     if (filledCount != 3 || emptyIdx == -1) return false;
/*     if (filledCount < 3)
        return false; */
    solvedField = emptyIdx;

    switch (emptyIdx)
    {
    case 0:
        SolveForAmount();
        break;
    case 1:
        SolveForRate();
        break;
    case 2:
        SolveForMonths();
        break;
    case 3:
        SolveForMonthly();
        break;
    }

    fieldFilled[solvedField] = true;
    return true;
}

// ─────────────────────────────────────────────
//  SolveForMonthly  →  PMT = P*r*(1+r)^n / ((1+r)^n-1)
// ─────────────────────────────────────────────
void LoanCalculator::SolveForMonthly()
{
    float P = fieldValues[0];
    float r = (fieldValues[1] / 100.0f) / 12.0f;
    int n = (int)fieldValues[2];

    if (r == 0.0f)
        fieldValues[3] = P / n;
    else
    {
        float factor = powf(1.0f + r, (float)n);
        fieldValues[3] = P * r * factor / (factor - 1.0f);
    }
}

// ─────────────────────────────────────────────
//  SolveForAmount  →  P = PMT * ((1+r)^n-1) / (r*(1+r)^n)
// ─────────────────────────────────────────────
void LoanCalculator::SolveForAmount()
{
    float PMT = fieldValues[3];
    float r = (fieldValues[1] / 100.0f) / 12.0f;
    int n = (int)fieldValues[2];

    if (r == 0.0f)
        fieldValues[0] = PMT * n;
    else
    {
        float factor = powf(1.0f + r, (float)n);
        fieldValues[0] = PMT * (factor - 1.0f) / (r * factor);
    }
}

// ─────────────────────────────────────────────
//  SolveForMonths  →  n = -ln(1 - P*r/PMT) / ln(1+r)
// ─────────────────────────────────────────────
void LoanCalculator::SolveForMonths()
{
    float P = fieldValues[0];
    float r = (fieldValues[1] / 100.0f) / 12.0f;
    float PMT = fieldValues[3];

    if (r == 0.0f)
    {
        fieldValues[2] = (PMT > 0) ? roundf(P / PMT) : 0;
        return;
    }

    float inner = 1.0f - (P * r / PMT);
    if (inner <= 0)
    {
        // התשלום החודשי לא מספיק לכסות ריבית – שגיאה
        fieldValues[2] = 0;
        return;
    }

    fieldValues[2] = (float)(-log(inner) / log(1.0f + r));
    fieldValues[2] = ceilf(fieldValues[2]); // עיגול למעלה
}

// ─────────────────────────────────────────────
//  SolveForRate  →  איטרציה (אין נוסחה סגורה)
//  מחפש r כך ש-PMT(P,r,n) = PMT_target
//  בשיטת Newton-Raphson
// ─────────────────────────────────────────────
void LoanCalculator::SolveForRate()
{
    float P = fieldValues[0];
    float n = fieldValues[2];
    float PMT = fieldValues[3];

    // בדיקה: אם PMT*n <= P → ריבית שלילית / בלתי אפשרית
    if (PMT * n <= P)
    {
        fieldValues[1] = 0;
        return;
    }

    // ניחוש התחלתי – ריבית חודשית
    float r = 0.01f; // 1% לחודש = 12% שנתי

    for (int iter = 0; iter < 100; iter++)
    {
        float factor = powf(1.0f + r, n);
        float f = P * r * factor / (factor - 1.0f) - PMT;
        float df = P * (factor * (1.0f + r * n) - factor - r * n * factor) /
                   ((factor - 1.0f) * (factor - 1.0f) * (1.0f + r));
        // אם הנגזרת קטנה מדי – עצור
        if (fabsf(df) < 1e-10f)
            break;
        float rNew = r - f / df;
        if (rNew < 0)
            rNew = r / 2.0f; // אל תלך שלילי
        if (fabsf(rNew - r) < 1e-7f)
        {
            r = rNew;
            break;
        }
        r = rNew;
    }

    fieldValues[1] = r * 12.0f * 100.0f; // → ריבית שנתית באחוזים
}

// ─────────────────────────────────────────────
//  CalcTotals
// ─────────────────────────────────────────────
void LoanCalculator::CalcTotals()
{
    float PMT = fieldValues[3];
    int n = (int)fieldValues[2];
    float P = fieldValues[0];

    totalPayment = PMT * n;
    totalInterest = totalPayment - P;
    if (totalInterest < 0)
        totalInterest = 0;
}

// ─────────────────────────────────────────────
//  עזר
// ─────────────────────────────────────────────
void LoanCalculator::ClearField(int idx)
{
    fieldValues[idx] = 0;
    fieldFilled[idx] = false;
}

void LoanCalculator::ClearAll()
{
    for (int i = 0; i < FIELD_COUNT; i++)
        ClearField(i);
    totalPayment = 0;
    totalInterest = 0;
}

const char *LoanCalculator::FieldLabel(int idx)
{
    switch (idx)
    {
    case 0:
        return "Loan Amount";
    case 1:
        return "Annual Rate";
    case 2:
        return "Months";
    case 3:
        return "Monthly Pay";
    default:
        return "";
    }
}

const char *LoanCalculator::FieldUnit(int idx)
{
    switch (idx)
    {
    case 0:
        return "";
    case 1:
        return "%";
    case 2:
        return "mo";
    case 3:
        return "";
    default:
        return "";
    }
}

String LoanCalculator::FieldStr(int idx)
{
    if (idx == 2)
        return String((int)fieldValues[idx]);
    return FloatToStr(fieldValues[idx]);
}

String LoanCalculator::FloatToStr(float v, int dec)
{
    char buf[20];
    dtostrf(v, 1, dec, buf);
    return String(buf);
}