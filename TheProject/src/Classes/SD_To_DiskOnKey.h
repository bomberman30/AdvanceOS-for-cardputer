#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class SD_To_DiskOnKey : public GlobalParentClass
{
public:
    SD_To_DiskOnKey(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;
private:





};
