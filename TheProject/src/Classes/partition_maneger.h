#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

class partition_maneger : public GlobalParentClass
{
public:
struct PartitionInfo {
    char label[17];
    size_t size;
    uint32_t address;
    uint16_t color;
};


    partition_maneger(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;

private:
std::vector<PartitionInfo> _partitions;
    size_t _totalFlash = 0;
    uint16_t getPartitionColor(int index);


    void SetAdvanceOS_partition();
};
