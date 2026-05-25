#include "MassStorage.h"
#include "MyOS.h"
// ══════════════════════════════════════════════════════════════════
// SD RAW
// ══════════════════════════════════════════════════════════════════

uint8_t MassStorage::sdCmd(uint8_t cmd, uint32_t arg) {
    sdDeselect();
    sdTransfer(0xFF);
    sdSelect();
    sdTransfer(0x40 | cmd);
    sdTransfer((arg >> 24) & 0xFF);
    sdTransfer((arg >> 16) & 0xFF);
    sdTransfer((arg >>  8) & 0xFF);
    sdTransfer((arg      ) & 0xFF);
    uint8_t crc = 0xFF;
    if (cmd == 0) crc = 0x95;
    if (cmd == 8) crc = 0x87;
    sdTransfer(crc);
    uint8_t r = 0xFF;
    for (int i = 0; i < 8; i++) {
        r = sdTransfer(0xFF);
        if (!(r & 0x80)) break;
    }
    return r;
}

bool MassStorage::sdRawInit() {
   /*  sdSPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
    sdDeselect();
    for (int i = 0; i < 10; i++) sdTransfer(0xFF);

    if (sdCmd(0, 0) != 0x01) { sdSPI.endTransaction(); return false; }

    bool v2 = false;
    if (sdCmd(8, 0x000001AA) == 0x01) {
        uint8_t r7[4];
        for (int i = 0; i < 4; i++) r7[i] = sdTransfer(0xFF);
        if (r7[2] == 0x01 && r7[3] == 0xAA) v2 = true;
    }

    uint32_t deadline = millis() + 2000;
    uint8_t r;
    do {
        sdCmd(55, 0);
        r = sdCmd(41, v2 ? 0x40000000 : 0);
        if (millis() > deadline) { sdSPI.endTransaction(); return false; }
    } while (r != 0x00);

    if (v2) {
        if (sdCmd(58, 0) == 0x00) {
            uint8_t ocr[4];
            for (int i = 0; i < 4; i++) ocr[i] = sdTransfer(0xFF);
            sdIsHC = (ocr[0] & 0x40) != 0;
        }
    }

    if (!sdIsHC) {
        if (sdCmd(16, 512) != 0x00) { sdSPI.endTransaction(); return false; }
    }

    sdDeselect();
    sdSPI.endTransaction(); */
    return true;
}

// ══════════════════════════════════════════════════════════════════
// SECTOR I/O
// ══════════════════════════════════════════════════════════════════

bool MassStorage::sdReadSectors(uint8_t* buf, uint32_t lba, uint32_t count) {
    sdSPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    for (uint32_t i = 0; i < count; i++) {
        uint32_t addr = sdIsHC ? (lba + i) : ((lba + i) * SECTOR_SIZE);
        sdDeselect(); sdTransfer(0xFF); sdSelect();
        sdTransfer(0x40 | 17);
        sdTransfer((addr >> 24) & 0xFF); sdTransfer((addr >> 16) & 0xFF);
        sdTransfer((addr >>  8) & 0xFF); sdTransfer((addr      ) & 0xFF);
        sdTransfer(0xFF);
        uint8_t r1 = 0xFF;
        for (int t = 0; t < 10; t++) { r1 = sdTransfer(0xFF); if (!(r1 & 0x80)) break; }
        if (r1 != 0x00) { sdDeselect(); sdSPI.endTransaction(); return false; }
        uint8_t token = 0xFF;
        uint32_t dl = millis() + 500;
        while (millis() < dl) { token = sdTransfer(0xFF); if (token != 0xFF) break; }
        if (token != 0xFE) { sdDeselect(); sdSPI.endTransaction(); return false; }
        for (int b = 0; b < SECTOR_SIZE; b++) buf[i * SECTOR_SIZE + b] = sdTransfer(0xFF);
        sdTransfer(0xFF); sdTransfer(0xFF);
        sdDeselect();
    }
    sdSPI.endTransaction();
    return true;
}

bool MassStorage::sdWriteSectors(const uint8_t* buf, uint32_t lba, uint32_t count) {
    sdSPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    for (uint32_t i = 0; i < count; i++) {
        uint32_t addr = sdIsHC ? (lba + i) : ((lba + i) * SECTOR_SIZE);
        sdDeselect(); sdTransfer(0xFF); sdSelect();
        sdTransfer(0x40 | 24);
        sdTransfer((addr >> 24) & 0xFF); sdTransfer((addr >> 16) & 0xFF);
        sdTransfer((addr >>  8) & 0xFF); sdTransfer((addr      ) & 0xFF);
        sdTransfer(0xFF);
        uint8_t r1 = 0xFF;
        for (int t = 0; t < 10; t++) { r1 = sdTransfer(0xFF); if (!(r1 & 0x80)) break; }
        if (r1 != 0x00) { sdDeselect(); sdSPI.endTransaction(); return false; }
        sdTransfer(0xFF); sdTransfer(0xFE);
        for (int b = 0; b < SECTOR_SIZE; b++) sdTransfer(buf[i * SECTOR_SIZE + b]);
        sdTransfer(0xFF); sdTransfer(0xFF);
        uint8_t dresp = sdTransfer(0xFF);
        if ((dresp & 0x1F) != 0x05) { sdDeselect(); sdSPI.endTransaction(); return false; }
        uint32_t dl = millis() + 2000;
        while (millis() < dl) { if (sdTransfer(0xFF) != 0x00) break; }
        sdDeselect();
    }
    sdSPI.endTransaction();
    return true;
}

// ══════════════════════════════════════════════════════════════════
// USB MSC CALLBACKS (static → delegate to instance)
// ══════════════════════════════════════════════════════════════════

int32_t MassStorage::onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    (void)offset;
    uint32_t count = bufsize / SECTOR_SIZE;
    if (count == 0 || !_instance) return -1;
    return _instance->sdReadSectors((uint8_t*)buffer, lba, count) ? (int32_t)bufsize : -1;
}

int32_t MassStorage::onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    (void)offset;
    uint32_t count = bufsize / SECTOR_SIZE;
    if (count == 0 || !_instance) return -1;
    _instance->mscWriteActive = true;
    bool ok = _instance->sdWriteSectors(buffer, lba, count);
    _instance->mscWriteActive = false;
    return ok ? (int32_t)bufsize : -1;
}

bool MassStorage::onStartStop(uint8_t power_condition, bool start, bool load_eject) {
    (void)power_condition; (void)start; (void)load_eject;
    return true;
}

// ══════════════════════════════════════════════════════════════════
// DRAWING
// ══════════════════════════════════════════════════════════════════

void MassStorage::drawSDCardShape(int x, int y, int w, int h, uint16_t col) {
    M5Cardputer.Display.fillRoundRect(x, y, w, h, 4, col);
    int cutSize = 14;
    M5Cardputer.Display.fillTriangle(
        x + w - cutSize, y, x + w, y, x + w, y + cutSize, C_BLACK);
    M5Cardputer.Display.drawLine(
        x + w - cutSize, y, x + w, y + cutSize, col);
    int pinY    = y + h - 10;
    int pinGap  = 6;
    int pinStart= x + 6;
    for (int i = 0; i < 6; i++)
        M5Cardputer.Display.fillRect(pinStart + i * pinGap, pinY, 4, 10, C_BLACK);
}

void MassStorage::drawMainScreen() {
    // Use sprite as required by the OS pattern
    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H);

    mainOS->sprite.fillSprite(C_BLACK);
    mainOS->sprite.drawFastHLine(0, 18, 240, C_DGRAY);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextColor(C_GRAY);
    mainOS->sprite.setCursor(4, 5);
    mainOS->sprite.print("USB Mass Storage");
    mainOS->sprite.setTextColor(C_DGRAY);
    mainOS->sprite.setCursor(178, 5);
    mainOS->sprite.print("by ");
    mainOS->sprite.setTextColor(C_ACCENT);
    mainOS->sprite.print("MOY");

    if (!sdMounted) {
        mainOS->sprite.setTextColor(C_RED);
        mainOS->sprite.setTextSize(2);
        mainOS->sprite.setCursor(50, 55);
        mainOS->sprite.print("SD ERROR");
        mainOS->sprite.setTextSize(1);
        mainOS->sprite.setTextColor(C_GRAY);
        mainOS->sprite.setCursor(42, 85);
        mainOS->sprite.print("Check SD card");
        mainOS->sprite.setCursor(50, 98);
        mainOS->sprite.print("[R] to retry");
        mainOS->sprite.pushSprite(0, 0);
        mainOS->sprite.deleteSprite();
        return;
    }

    // SD card shape — drawn directly on display (uses fillTriangle/fillRect tricks)
    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
    drawSDCardShape(168, 28, 60, 80, C_DGRAY);

    // Redraw sprite on top of SD shape for text layer
    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H);
    mainOS->sprite.fillSprite(TFT_TRANSPARENT);
    mainOS->sprite.setTextColor(C_GRAY);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(184, 48);
    mainOS->sprite.print("SD");

    if (usbActive) {
        mainOS->sprite.setTextColor(C_GREEN);
        mainOS->sprite.setTextSize(2);
        mainOS->sprite.setCursor(8, 42);
        mainOS->sprite.print("CONNECTED");
        mainOS->sprite.setTextSize(1);
        mainOS->sprite.setTextColor(C_GRAY);
        mainOS->sprite.setCursor(8, 72);
        mainOS->sprite.print("Device ready");
        mainOS->sprite.setTextColor(C_DGRAY);
        mainOS->sprite.setCursor(8, 86);
        mainOS->sprite.printf("%s  %u sectors", sdIsHC ? "SDHC" : "SD", sdSectorCount);
    } else {
        mainOS->sprite.setTextColor(C_YELLOW);
        mainOS->sprite.setTextSize(2);
        mainOS->sprite.setCursor(8, 42);
        mainOS->sprite.print("WAITING");
        mainOS->sprite.setTextSize(1);
        mainOS->sprite.setTextColor(C_GRAY);
        mainOS->sprite.setCursor(8, 72);
        mainOS->sprite.print("Plug in USB cable");
    }

    mainOS->sprite.drawFastHLine(0, 118, 240, C_DGRAY);
    mainOS->sprite.setTextColor(C_DGRAY);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(4, 122);
    mainOS->sprite.print("[R] Refresh  [DEL] Safe Eject");

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

// ══════════════════════════════════════════════════════════════════
// ANIMATIONS (run directly on display — no redraw needed)
// ══════════════════════════════════════════════════════════════════

void MassStorage::animSDPulse() {
    if (millis() - sdPulseLastT < 30) return;
    sdPulseLastT = millis();

    if (sdPulseDir) sdPulseBright += 4; else sdPulseBright -= 4;
    if (sdPulseBright >= 120) sdPulseDir = false;
    if (sdPulseBright <= 30)  sdPulseDir = true;

    uint16_t col = usbActive
        ? M5Cardputer.Display.color565(0, sdPulseBright, 0)
        : M5Cardputer.Display.color565(sdPulseBright >> 1, sdPulseBright >> 1, sdPulseBright >> 1);

    M5Cardputer.Display.drawRoundRect(168, 28, 60, 80, 4, col);
}

void MassStorage::animDataFlow() {
    if (!usbActive) return;
    if (millis() - dataFlowLastT < 80) return;
    dataFlowLastT = millis();

    M5Cardputer.Display.fillRect(6, 99, 155, 8, C_BLACK);
    for (int i = 0; i < 6; i++) {
        int x = 10 + (dataFlowPos + i * 24) % 150;
        uint8_t br = (uint8_t)(200 - i * 30);
        uint16_t c = M5Cardputer.Display.color565(0, br / 5, br / 2);
        M5Cardputer.Display.fillCircle(x, 103, 3, c);
    }
    dataFlowPos = (dataFlowPos + 6) % 150;
}

void MassStorage::animWaitDots() {
    if (usbActive) return;
    if (millis() - waitDotsLastT < 500) return;
    waitDotsLastT = millis();

    M5Cardputer.Display.fillRect(6, 86, 155, 12, C_BLACK);
    M5Cardputer.Display.setTextColor(C_DGRAY);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(8, 88);
    M5Cardputer.Display.print("Waiting");
    for (int i = 0; i < waitDotsPhase; i++)
        M5Cardputer.Display.print(".");
    waitDotsPhase = (waitDotsPhase + 1) % 4;
}

// ══════════════════════════════════════════════════════════════════
// LIFECYCLE
// ══════════════════════════════════════════════════════════════════
void MassStorage::Begin() {
    _instance = this;

    // SD already initialized by OS — just get sector count via raw init
    sdSectorCount = (uint32_t)(SD.totalBytes() / SECTOR_SIZE);
    SD.end();
    delay(20);

    if (sdRawInit()) {
        sdMounted = true;

        MSC.vendorID("M5Stack");
        MSC.productID("Cardputer");
        MSC.productRevision("2.1");
        MSC.onRead(onRead);
        MSC.onWrite(onWrite);
        MSC.onStartStop(onStartStop);
        MSC.mediaPresent(true);
        MSC.begin(sdSectorCount, SECTOR_SIZE);

        USB.manufacturerName("M5Stack");
        USB.productName("Cardputer SD");
        USB.serialNumber("MSC00003");
        USB.begin();
    }

    Draw();
}

void MassStorage::Loop() {
    M5Cardputer.update();

    if (sdMounted) {
        bool nowConn = tud_mounted();
        if (nowConn != usbActive) {
            usbActive  = nowConn;
            needRedraw = true;
        }
    }

    if (needRedraw) {
        Draw();
        needRedraw = false;
    }

    if (sdMounted) {
        animSDPulse();
        animDataFlow();
        animWaitDots();
    }

    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        auto st = M5Cardputer.Keyboard.keysState();

        for (char c : st.word) {
            if (c == 'r' || c == 'R') { needRedraw = true; break; }
        }

        if (st.del) {
            MSC.mediaPresent(false);
            delay(300);
            M5Cardputer.Display.fillRect(0, 119, 240, 16, C_BLACK);
            M5Cardputer.Display.setTextColor(C_GREEN);
            M5Cardputer.Display.setTextSize(1);
            M5Cardputer.Display.setCursor(8, 122);
            M5Cardputer.Display.print("Safely ejected!  Re-plug to use.");
            delay(3000);
            MSC.mediaPresent(true);
            needRedraw = true;
        }
    }

    delay(8);
}

void MassStorage::Draw() {
    drawMainScreen();
}

void MassStorage::OnExit() {
    MSC.mediaPresent(false);
    _instance = nullptr;
}