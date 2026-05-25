#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <SPI.h>
#include <SD.h>
#include "USB.h"
#include "USBMSC.h"
#include "tusb.h"

#define SD_CS_PIN   12
#define SD_SCK_PIN  40
#define SD_MOSI_PIN 14
#define SD_MISO_PIN 39
#define SECTOR_SIZE 512

#define C_BLACK  0x0000
#define C_WHITE  0xFFFF
#define C_GREEN  0x07E0
#define C_YELLOW 0xFFE0
#define C_RED    0xF800
#define C_GRAY   0x7BEF
#define C_DGRAY  0x2104
#define C_ACCENT 0x07FF

class MassStorage : public GlobalParentClass
{
public:
    MassStorage(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop() override;
    void Draw() override;
    void OnExit() override;

private:
    // State
    volatile bool mscWriteActive = false;
    bool     sdMounted     = false;
    bool     usbActive     = false;
    bool     needRedraw    = true;
    uint32_t sdSectorCount = 0;
    bool     sdIsHC        = false;

    // Animation state
    uint32_t sdPulseLastT  = 0;
    uint8_t  sdPulseBright = 30;
    bool     sdPulseDir    = true;

    uint32_t dataFlowLastT = 0;
    int      dataFlowPos   = 0;

    uint32_t waitDotsLastT = 0;
    int      waitDotsPhase = 0;

    // SPI
    SPIClass sdSPI{HSPI};
    USBMSC   MSC;

    // SPI helpers
    uint8_t sdTransfer(uint8_t b) { return sdSPI.transfer(b); }
    void sdSelect()   { digitalWrite(SD_CS_PIN, LOW);  delayMicroseconds(1); }
    void sdDeselect() { digitalWrite(SD_CS_PIN, HIGH); sdSPI.transfer(0xFF); }

    // SD raw init
    uint8_t sdCmd(uint8_t cmd, uint32_t arg);
    bool    sdRawInit();

    // Sector I/O
    bool sdReadSectors(uint8_t* buf, uint32_t lba, uint32_t count);
    bool sdWriteSectors(const uint8_t* buf, uint32_t lba, uint32_t count);

    // USB MSC callbacks (static wrappers needed for the API)
    static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
    static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
    static bool    onStartStop(uint8_t power_condition, bool start, bool load_eject);

    // Static instance pointer for callbacks
    static MassStorage* _instance;

    // Drawing helpers
    void drawSDCardShape(int x, int y, int w, int h, uint16_t col);
    void drawMainScreen();

    // Animations
    void animSDPulse();
    void animDataFlow();
    void animWaitDots();
};

inline MassStorage* MassStorage::_instance = nullptr;