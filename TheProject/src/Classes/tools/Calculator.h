#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class Calculator : public GlobalParentClass
{
public:
    Calculator(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop() override;
    void Draw() override;

private:
    String inputBuffer = "";  
    String result = "";        
    void Evaluate();        
};
