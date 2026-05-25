#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class LoanCalculator : public GlobalParentClass
{
public:
    LoanCalculator(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop() override;
    void Draw() override;

private:
    // ===== מצבים =====
    enum class State
    {
        INPUTT,  // הכנסת נתונים
        RESULT  // הצגת תוצאה
    };

    State currentState = State::INPUTT;

    // ===== 4 שדות =====
    // 0 = סכום הלוואה
    // 1 = ריבית שנתית (%)
    // 2 = משך בחודשים
    // 3 = תשלום חודשי
    static const int FIELD_COUNT = 4;

    float fieldValues[FIELD_COUNT] = {0, 0, 0, 0};
    bool  fieldFilled[FIELD_COUNT] = {false, false, false, false};

    int   selectedField = 0;  // שדה מסומן
    int   solvedField   = -1; // השדה שחושב (מוצג בצבע שונה)

    // תוצאות נוספות
    float totalPayment  = 0;
    float totalInterest = 0;

    // ===== פונקציות עיקריות =====
    void EditSelectedField();
    bool TrySolve();

    void SolveForAmount();
    void SolveForRate();
    void SolveForMonths();
    void SolveForMonthly();
    void CalcTotals();

    void ClearField(int idx);
    void ClearAll();

    // ===== ציור =====
    void DrawInput();
    void DrawResult();

    // ===== עזר =====
    const char* FieldLabel(int idx);
    const char* FieldUnit(int idx);
    String      FieldStr(int idx);
    String      FloatToStr(float v, int dec = 2);
};