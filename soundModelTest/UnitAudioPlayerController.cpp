#include "UnitAudioPlayerController.h"
#include "../ExtraMenu.h"
#include <MyOS.h>

// ─── Sabit Veri ───────────────────────────────────────────────
const char UnitAudioPlayerController::matrixChars[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ@#$%&*";

// ═════════════════════════════════════════════════════════════
//  YAŞAM DÖNGÜSÜ
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::Begin()
{
    showTopBar = false;

    // ── Bağlantı Ekranı ───────────────────────────────────────
    M5.Display.fillScreen(AP_COLOR_BG);
    drawCreamLine(0, 0.9f);
    drawCreamLine(134, 0.9f);

    M5.Display.setTextSize(2);
    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_BG);
    M5.Display.setCursor(30, 40);
    M5.Display.print("CONNECTING");

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_BG);
    M5.Display.setCursor(42, 60);
    M5.Display.print("Audio Player Module");

    // ── Port Pinlerini Al ─────────────────────────────────────
    int8_t pin1 = M5.getPin(m5::pin_name_t::port_a_pin1);
    int8_t pin2 = M5.getPin(m5::pin_name_t::port_a_pin2);
    Serial.printf("[AudioPlayer] Port A: RX=%d TX=%d\n", pin1, pin2);

    M5.Display.setTextColor(0x4208, AP_COLOR_BG);
    M5.Display.setCursor(50, 72);
    M5.Display.printf("Port A: G%d / G%d", pin1, pin2);

    // ── Bağlantı Döngüsü ──────────────────────────────────────
    int  counter = 0;
    int  dotX    = 60;

    while (!audioplayer.begin(&Serial1, pin1, pin2))
    {
        for (int i = 0; i < 5; i++) {
            uint8_t rv = (uint8_t)(10 + i * 4);
            uint8_t gv = (uint8_t)(8  + i * 3);
            M5.Display.fillRect(dotX + i * 8, 88, 6, 8,
                M5.Display.color565(rv, gv, 0));
        }
        dotX += 8;
        if (dotX > 180) {
            M5.Display.fillRect(55, 88, 180, 10, AP_COLOR_BG);
            dotX = 60;
        }
        delay(400);

        counter++;
        if (counter > 12) {
            M5.Display.fillRect(30, 84, 180, 18, AP_COLOR_BG);
            M5.Display.setTextColor(AP_COLOR_RED, AP_COLOR_BG);
            M5.Display.setTextSize(1);
            M5.Display.setCursor(40, 90);
            M5.Display.print("CONNECTION FAILED!");
            delay(1500);
            mainOS->ChangeMenu(new Extra(mainOS));
            return;
        }
    }

    // ── Bağlandı ──────────────────────────────────────────────
    M5.Display.fillRect(30, 84, 180, 18, AP_COLOR_BG);
    M5.Display.fillRoundRect(40, 86, 160, 14, 3,
        M5.Display.color565(15, 10, 1));
    M5.Display.drawRoundRect(40, 86, 160, 14, 3, AP_COLOR_CREAM1);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM1,
        M5.Display.color565(15, 10, 1));
    M5.Display.setCursor(70, 90);
    M5.Display.print("CONNECTED!");
    delay(600);

    // ── Modül Ayarları ────────────────────────────────────────
    audioplayer.setVolume(currentVolume);
    audioplayer.setPlayMode(AUDIO_PLAYER_MODE_SINGLE_STOP);
    delay(300);

    totalTracks        = audioplayer.getTotalAudioNumber();
    currentTrack       = 1;
    lastDisplayedTrack = 1;
    audioplayer.selectAudioNum(1);

    initialized     = true;
    lastDiskPlaying = !isPlaying;

    // ── Ana Ekranı Çiz ────────────────────────────────────────
    drawMainScreen();
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::Loop()
{
    if (!initialized) return;

    M5.update();

    // ── Info Ekranı ───────────────────────────────────────────
    if (isInfoScreen) {
        handleKeyboard();
        checkAutoNext();
        return;
    }

    // ── Normal Mod ────────────────────────────────────────────
    updateMainDisplay();
    handleKeyboard();

    if (!isInputMode) {
        handleButtonA();
        checkAutoNext();
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::Draw()
{
    // Çizim Loop() içinde yönetiliyor
}

// ═════════════════════════════════════════════════════════════
//  KLAVYE / BUTON İŞLEME
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::handleKeyboard()
{
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;

    Keyboard_Class::KeysState st = M5Cardputer.Keyboard.keysState();

    if (isInfoScreen) {
        for (auto k : st.word) {
            if (k == 'i' || k == 'I' || k == '`' || k == '~') {
                exitInfoScreen();
                return;
            }
        }
        return;
    }

    if (isInputMode) {
        handleKeyboardInput();
        return;
    }

    handleKeyboardNormal();
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::handleKeyboardNormal()
{
    Keyboard_Class::KeysState st = M5Cardputer.Keyboard.keysState();

    for (auto k : st.word) {
        switch (k) {
            case '+': case '=': volumeUp();      break;
            case '-':           volumeDown();     break;
            case 'l': case 'L': toggleLoop();     break;
            case 's': case 'S': toggleShuffle();  break;
            case 'r': case 'R': randomTrack();    break;
            case 'g': case 'G':
                isInputMode = true;
                inputTrackNumber = "";
                drawInputMode();
                return;
            case 'i': case 'I': enterInfoScreen(); return;
            case '`': case '~':
                mainOS->ChangeMenu(new Extra(mainOS));
                return;
            case ',': executeAction(3); break;
            case '/': executeAction(2); break;
            case ';': goToTrack(min((int)currentTrack + 10, (int)totalTracks)); break;
            case '.': goToTrack(max((int)currentTrack - 10, 1));                break;
        }
    }

    if (st.space) executeAction(1);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::handleKeyboardInput()
{
    Keyboard_Class::KeysState st = M5Cardputer.Keyboard.keysState();

    if (st.enter) {
        if (inputTrackNumber.length() > 0)
            goToTrack(inputTrackNumber.toInt());
        isInputMode      = false;
        inputTrackNumber = "";
        lastDiskPlaying  = !isPlaying;
        drawMainScreen();
        return;
    }

    if (st.del && inputTrackNumber.length() > 0) {
        inputTrackNumber.remove(inputTrackNumber.length() - 1);
        drawInputMode();
        return;
    }

    for (auto k : st.word) {
        if (k == 'g' || k == 'G') {
            isInputMode      = false;
            inputTrackNumber = "";
            lastDiskPlaying  = !isPlaying;
            drawMainScreen();
            return;
        }
        if (k >= '0' && k <= '9' && inputTrackNumber.length() < 5) {
            inputTrackNumber += k;
            drawInputMode();
        }
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::handleButtonA()
{
    if (M5.BtnA.wasPressed()) {
        unsigned long ct = millis();
        clickCount = (ct - lastClickTime < CLICK_TIMEOUT)
                     ? clickCount + 1 : 1;
        lastClickTime = ct;
    }
    if (clickCount > 0 && millis() - lastClickTime > CLICK_TIMEOUT) {
        executeAction(clickCount);
        clickCount = 0;
    }
}

// ═════════════════════════════════════════════════════════════
//  YARDIMCI: RENK
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawCreamLine(int y, float br)
{
    for (int x = 0; x < 240; x++) {
        float    t  = (float)x / 239.0f;
        uint8_t rv = (uint8_t)(31 * br * (0.5f + 0.5f * t));
        uint8_t gv = (uint8_t)(28 * br * (0.4f + 0.6f * t));
        uint8_t bv = (uint8_t)(10 * br * t);
        M5.Display.drawPixel(x, y,
            M5.Display.color565(rv, gv, bv));
    }
}

// Eski rainbow satırı - geriye dönük uyumluluk için bırakıldı
void UnitAudioPlayerController::drawRainbowLine(int y, float br, float off)
{
    drawCreamLine(y, br);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawNeonText(
        int x, int y, const char* txt, uint16_t col, uint8_t sz)
{
    M5.Display.setTextSize(sz);
    // Krem gölge
    uint16_t shadow = M5.Display.color565(12, 8, 0);
    M5.Display.setTextColor(shadow, AP_COLOR_BG);
    M5.Display.setCursor(x + 1, y + 1);
    M5.Display.print(txt);
    M5.Display.setTextColor(col, AP_COLOR_BG);
    M5.Display.setCursor(x, y);
    M5.Display.print(txt);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawGlowRect(
        int x, int y, int w, int h, uint16_t col, uint16_t inner)
{
    M5.Display.fillRoundRect(x, y, w, h, 3, inner);
    M5.Display.drawRoundRect(x, y, w, h, 3, col);
    M5.Display.drawRoundRect(x - 1, y - 1, w + 2, h + 2, 4,
        M5.Display.color565(
            ((col >> 11) & 0x1F) << 1,
            ((col >>  5) & 0x3F) >> 1,
            0));
}

// ═════════════════════════════════════════════════════════════
//  DİSK
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawDiskBackground()
{
    // Dış glow - amber/krem
    for (int r = DISK_R + 4; r > DISK_R; r--) {
        float   br = 0.10f + 0.10f * (DISK_R + 4 - r);
        uint8_t rv = (uint8_t)(20 * br);
        uint8_t gv = (uint8_t)(15 * br);
        M5.Display.drawCircle(DISK_CX, DISK_CY, r,
            M5.Display.color565(rv, gv, 0));
    }

    // Ana disk - koyu kahve
    M5.Display.fillCircle(DISK_CX, DISK_CY, DISK_R, AP_COLOR_CREAM5);

    // Yivler - krem tonları
    int rr[] = {38, 34, 30, 26, 22, 18, 14};
    for (int i = 0; i < 7; i++) {
        uint8_t rv = (uint8_t)(6 + i * 5);
        uint8_t gv = (uint8_t)(4 + i * 4);
        M5.Display.drawCircle(DISK_CX, DISK_CY, rr[i],
            M5.Display.color565(rv, gv, 0));
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawDiskDynamic(float angle)
{
    // Dönen amber/krem dış halka
    for (int r = DISK_R - 1; r > DISK_R - 7; r--) {
        for (int a = 0; a < 360; a += 3) {
            float   rad = a * 3.14159f / 180.0f;
            float   rot = fmod((float)a / 360.0f + angle / 360.0f, 1.0f);
            uint8_t rv  = (uint8_t)(15 + rot * 16);
            uint8_t gv  = (uint8_t)(10 + rot * 12);
            int     px  = DISK_CX + (int)(r * cosf(rad));
            int     py  = DISK_CY + (int)(r * sinf(rad));
            M5.Display.drawPixel(px, py,
                M5.Display.color565(rv, gv, 0));
        }
    }

    // Dönen parlak nokta
    static float lastSA = 0;
    {
        float lr = lastSA * 3.14159f / 180.0f;
        M5.Display.fillCircle(
            DISK_CX + (int)((DISK_R - 8) * cosf(lr)),
            DISK_CY + (int)((DISK_R - 8) * sinf(lr)),
            3, AP_COLOR_CREAM5);
    }
    lastSA    = angle;
    float rad = angle * 3.14159f / 180.0f;
    int   sx  = DISK_CX + (int)((DISK_R - 8) * cosf(rad));
    int   sy  = DISK_CY + (int)((DISK_R - 8) * sinf(rad));
    M5.Display.fillCircle(sx, sy, 3, AP_COLOR_CREAM1);
    M5.Display.fillCircle(sx, sy, 1, AP_COLOR_AMBER);

    // Merkez etiket
    M5.Display.fillCircle(DISK_CX, DISK_CY, 14, AP_COLOR_CREAM5);
    if (isPlaying) {
        float   t  = fmod(angle / 360.0f, 1.0f);
        uint8_t rv = (uint8_t)(16 + t * 15);
        uint8_t gv = (uint8_t)(10 + t * 10);
        for (int r2 = 14; r2 >= 12; r2--)
            M5.Display.drawCircle(DISK_CX, DISK_CY, r2,
                M5.Display.color565(rv, gv, 0));
        M5.Display.fillCircle(DISK_CX, DISK_CY, 11, AP_COLOR_BG);
        M5.Display.fillCircle(DISK_CX, DISK_CY,  8, AP_COLOR_CREAM2);
        M5.Display.fillCircle(DISK_CX, DISK_CY,  5, AP_COLOR_BG);
        M5.Display.fillCircle(DISK_CX, DISK_CY,  3, AP_COLOR_CREAM3);
    } else {
        M5.Display.drawCircle(DISK_CX, DISK_CY, 13, 0x4208);
        M5.Display.fillCircle(DISK_CX, DISK_CY, 11, 0x2945);
        M5.Display.fillCircle(DISK_CX, DISK_CY,  7, 0x4208);
        M5.Display.fillCircle(DISK_CX, DISK_CY,  4, AP_COLOR_CREAM5);
    }
    M5.Display.fillCircle(DISK_CX, DISK_CY, 2, AP_COLOR_BG);
    M5.Display.drawCircle(DISK_CX, DISK_CY, 2, 0x8410);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawFullDisk(float angle)
{
    M5.Display.fillRect(136, 18, 104, 100, AP_COLOR_BG);
    drawDiskBackground();
    drawDiskDynamic(angle);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateDiskRotation()
{
    targetDiskSpeed = isPlaying ? 5.0f : 0.0f;
    if (diskSpeed < targetDiskSpeed) {
        diskSpeed += 0.3f;
        if (diskSpeed > targetDiskSpeed) diskSpeed = targetDiskSpeed;
    } else if (diskSpeed > targetDiskSpeed) {
        diskSpeed -= 0.15f;
        if (diskSpeed < 0) diskSpeed = 0;
    }
    diskAngle = fmod(diskAngle + diskSpeed, 360.0f);
}

// ═════════════════════════════════════════════════════════════
//  VU METRE
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::updateVuMeter()
{
    if (!isPlaying) {
        for (int i = 0; i < 12; i++) {
            if (vuLeft[i]  > 0) vuLeft[i]--;
            if (vuRight[i] > 0) vuRight[i]--;
        }
        return;
    }
    int peak = random(5, 12);
    for (int i = 0; i < 12; i++)
        vuLeft[i]  = (i < peak) ? 1 : max(0, vuLeft[i] - 1);
    peak = random(4, 11);
    for (int i = 0; i < 12; i++)
        vuRight[i] = (i < peak) ? 1 : max(0, vuRight[i] - 1);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawVuMeter()
{
    const int bW = 7, bH = 5, bG = 1, sx = 4;
    const int lyY = 82, ryY = 90;

    for (int i = 0; i < 12; i++) {
        uint16_t on_c, off_c = AP_COLOR_CREAM5;

        if      (i < 6)  on_c = AP_COLOR_CREAM2;
        else if (i < 9)  on_c = AP_COLOR_AMBER;
        else if (i < 11) on_c = AP_COLOR_ORANGE;
        else             on_c = AP_COLOR_RED;

        // Sol kanal
        M5.Display.fillRect(sx + i * (bW + bG), lyY, bW, bH,
            vuLeft[i] ? on_c : off_c);
        if (vuLeft[i])
            M5.Display.drawFastHLine(sx + i * (bW + bG), lyY, bW, 0xFFFF);

        // Sağ kanal
        M5.Display.fillRect(sx + i * (bW + bG), ryY, bW, bH,
            vuRight[i] ? on_c : off_c);
        if (vuRight[i])
            M5.Display.drawFastHLine(sx + i * (bW + bG), ryY, bW, 0xFFFF);
    }
}

// ═════════════════════════════════════════════════════════════
//  ÜST BAR
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawTopBar()
{
    // Krem gradient
    for (int y = 0; y < 18; y++) {
        float   t  = (float)y / 17.0f;
        uint8_t rv = (uint8_t)(24 * (1.0f - t * 0.5f));
        uint8_t gv = (uint8_t)(18 * (1.0f - t * 0.5f));
        uint8_t bv = (uint8_t)(2  * (1.0f - t * 0.5f));
        M5.Display.drawFastHLine(0, y, 240,
            M5.Display.color565(rv, gv, bv));
    }
    drawCreamLine(17, 0.7f);

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM1, 0x0000);
    M5.Display.setCursor(4, 5);
    M5.Display.print("MOY");
    M5.Display.setTextColor(AP_COLOR_AMBER, 0x0000);
    M5.Display.print(" MUSIC");
    M5.Display.setTextColor(0x4208, 0x0000);
    M5.Display.print(" | Andy+AI Cardputer");
}

// ═════════════════════════════════════════════════════════════
//  SOL PANEL
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawLeftPanel()
{
    M5.Display.fillRect(0, 18, LEFT_W, 100, AP_COLOR_BG);

    // ── Track Header ──
    M5.Display.drawFastHLine(0, 18, LEFT_W, AP_COLOR_CREAM2);
    M5.Display.fillRect(0, 19, LEFT_W, 16, AP_COLOR_CREAM5);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_CREAM5);
    M5.Display.setCursor(4, 23);
    M5.Display.print("\x0E TRACK");

    char tot[12]; sprintf(tot, "/%d", totalTracks);
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_CREAM5);
    M5.Display.setCursor(82, 23);
    M5.Display.print(tot);
    M5.Display.drawFastHLine(0, 35, LEFT_W, AP_COLOR_CREAM3);

    // ── Büyük Track Numarası ──
    M5.Display.fillRect(0, 36, LEFT_W, 28, AP_COLOR_BG);
    char ts[6]; sprintf(ts, "%03d", currentTrack);
    uint16_t numCol = isPlaying ? AP_COLOR_CREAM1 : AP_COLOR_CREAM2;
    M5.Display.setTextSize(3);

    if (isPlaying) {
        M5.Display.setTextColor(
            M5.Display.color565(12, 8, 0), AP_COLOR_BG);
        M5.Display.setCursor(5, 39);
        M5.Display.print(ts);
        M5.Display.setTextColor(
            M5.Display.color565(20, 15, 4), AP_COLOR_BG);
        M5.Display.setCursor(4, 38);
        M5.Display.print(ts);
    }
    M5.Display.setTextColor(numCol, AP_COLOR_BG);
    M5.Display.setCursor(4, 37);
    M5.Display.print(ts);

    // TIME etiketi
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_BG);
    M5.Display.setCursor(76, 37);
    M5.Display.print("TIME");

    unsigned long el = isPlaying
        ? (millis() - trackStartTime - totalPausedTime) / 1000 : 0;
    M5.Display.setTextColor(0xFFFF, AP_COLOR_BG);
    M5.Display.setCursor(74, 47);
    M5.Display.printf("%02d:%02d", (int)(el / 60), (int)(el % 60));

    M5.Display.drawFastHLine(0, 64, LEFT_W, 0x4208);

    // ── Play Status ──
    M5.Display.fillRect(0, 65, LEFT_W, 15, AP_COLOR_BG);
    if (isPlaying) {
        M5.Display.fillRoundRect(2, 66, 66, 12, 2,
            M5.Display.color565(20, 14, 2));
        M5.Display.drawRoundRect(2, 66, 66, 12, 2, AP_COLOR_CREAM1);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(AP_COLOR_CREAM1,
            M5.Display.color565(20, 14, 2));
        M5.Display.setCursor(6, 69);
        M5.Display.print(">> PLAYING");
    } else {
        M5.Display.fillRoundRect(2, 66, 66, 12, 2,
            M5.Display.color565(10, 8, 1));
        M5.Display.drawRoundRect(2, 66, 66, 12, 2, AP_COLOR_CREAM3);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(AP_COLOR_CREAM2,
            M5.Display.color565(10, 8, 1));
        M5.Display.setCursor(6, 69);
        M5.Display.print("|| PAUSED");
    }

    // Mod Rozeti
    M5.Display.fillRect(72, 65, LEFT_W - 72, 15, AP_COLOR_BG);
    if (isLoopEnabled) {
        M5.Display.fillRoundRect(72, 66, 56, 12, 2,
            M5.Display.color565(20, 14, 2));
        M5.Display.drawRoundRect(72, 66, 56, 12, 2, AP_COLOR_AMBER);
        M5.Display.setTextColor(AP_COLOR_AMBER,
            M5.Display.color565(20, 14, 2));
        M5.Display.setCursor(75, 69);
        M5.Display.print("LOOP ON");
    } else if (isShuffleEnabled) {
        M5.Display.fillRoundRect(72, 66, 56, 12, 2,
            M5.Display.color565(12, 8, 1));
        M5.Display.drawRoundRect(72, 66, 56, 12, 2, AP_COLOR_WARM);
        M5.Display.setTextColor(AP_COLOR_WARM,
            M5.Display.color565(12, 8, 1));
        M5.Display.setCursor(75, 69);
        M5.Display.print("SHUFFLE");
    } else {
        M5.Display.setTextColor(0x4208, AP_COLOR_BG);
        M5.Display.setCursor(75, 69);
        M5.Display.print("SEQ");
    }

    M5.Display.drawFastHLine(0, 78, LEFT_W, 0x2945);

    // ── VU Metre ──
    M5.Display.fillRect(0, 79, LEFT_W, 21, AP_COLOR_BG);
    drawVuMeter();
    M5.Display.drawFastHLine(0, 100, LEFT_W, 0x2945);

    // ── Kısayol İpuçları ──
    M5.Display.fillRect(0, 101, LEFT_W, 17, AP_COLOR_BG);
    M5.Display.setTextSize(1);

    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_BG);
    M5.Display.setCursor(2, 103);
    M5.Display.print("SPC");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Ply ");
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_BG);
    M5.Display.print("+/-");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Vol ");
    M5.Display.setTextColor(AP_COLOR_AMBER, AP_COLOR_BG);
    M5.Display.print("R");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Rnd");

    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_BG);
    M5.Display.setCursor(2, 112);
    M5.Display.print(",");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Prv ");
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_BG);
    M5.Display.print("/");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Nxt ");
    M5.Display.setTextColor(AP_COLOR_AMBER, AP_COLOR_BG);
    M5.Display.print("I");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Info");
}

// ═════════════════════════════════════════════════════════════
//  ALT BAR
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawBottomBar()
{
    M5.Display.fillRect(0, 118, 240, 17, AP_COLOR_CREAM5);
    drawCreamLine(118, 0.6f);

    int vp = (currentVolume * 100) / 30;

    // VOL etiketi
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_CREAM5);
    M5.Display.setCursor(3, 123);
    M5.Display.print("VOL");

    // Ses barı - 18 segment
    const int bX = 26, bY = 121, sW = 4, sH = 9, sG = 1, totB = 18;
    int act = (vp * totB) / 100;
    for (int i = 0; i < totB; i++) {
        if (i < act) {
            uint16_t c;
            if      (i < 8)  c = AP_COLOR_CREAM2;
            else if (i < 13) c = AP_COLOR_AMBER;
            else if (i < 16) c = AP_COLOR_ORANGE;
            else             c = AP_COLOR_RED;
            M5.Display.fillRect(bX + i * (sW + sG), bY, sW, sH, c);
            M5.Display.drawFastHLine(bX + i * (sW + sG), bY, sW, 0xFFFF);
        } else {
            M5.Display.fillRect(bX + i * (sW + sG), bY, sW, sH,
                M5.Display.color565(6, 4, 0));
        }
    }

    // Yüzde
    char vstr[6]; sprintf(vstr, "%3d%%", vp);
    M5.Display.setTextColor(0xFFFF, AP_COLOR_CREAM5);
    M5.Display.setCursor(122, 123);
    M5.Display.print(vstr);

    // Dikey ayırıcı
    M5.Display.drawFastVLine(148, 119, 16, 0x4208);

    // Track bilgisi
    M5.Display.fillRect(149, 119, 91, 16, AP_COLOR_CREAM5);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_CREAM5);
    M5.Display.setCursor(153, 123);
    char trk[16]; sprintf(trk, "TRACK #%03d", currentTrack);
    M5.Display.print(trk);
}

// ═════════════════════════════════════════════════════════════
//  ANA EKRAN
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawMainScreen()
{
    M5.Display.fillScreen(AP_COLOR_BG);
    drawTopBar();
    drawLeftPanel();

    // Dikey ayırıcı - krem tonu
    for (int y = 18; y < 118; y++) {
        float   t  = (float)(y - 18) / 99.0f;
        uint8_t rv = (uint8_t)(8 + t * 10);
        uint8_t gv = (uint8_t)(6 + t *  8);
        M5.Display.drawPixel(LEFT_W, y,
            M5.Display.color565(rv, gv, 0));
    }

    drawFullDisk(diskAngle);
    drawBottomBar();
}

// ═════════════════════════════════════════════════════════════
//  KISMİ EKRAN GÜNCELLEMELERİ
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::updateMainDisplay()
{
    unsigned long now = millis();

    // Disk
    if (now - lastDiskUpdate >= DISK_UPDATE_MS) {
        lastDiskUpdate = now;
        updateDiskRotation();
        bool pChg = (lastDiskPlaying != isPlaying);
        if (diskSpeed > 0.01f || pChg) {
            if (pChg) { drawFullDisk(diskAngle); lastDiskPlaying = isPlaying; }
            else       drawDiskDynamic(diskAngle);
        }
    }

    // VU
    if (now - lastVuUpdate >= VU_UPDATE_MS) {
        lastVuUpdate = now;
        updateVuMeter();
        drawVuMeter();
    }

    // Zaman
    if (now - lastTimeUpdate >= TIME_UPDATE_MS) {
        lastTimeUpdate = now;
        updateTimeDisplay();
    }

    // Geçici mesaj süre kontrolü
    if (tempMessageActive && (now - tempMessageTime > TEMP_MSG_DURATION)) {
        tempMessageActive = false;
        clearTempArea();
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateTrackDisplay()
{
    M5.Display.fillRect(0, 36, LEFT_W, 28, AP_COLOR_BG);
    char ts[6]; sprintf(ts, "%03d", currentTrack);
    uint16_t numCol = isPlaying ? AP_COLOR_CREAM1 : AP_COLOR_CREAM2;
    M5.Display.setTextSize(3);

    if (isPlaying) {
        M5.Display.setTextColor(
            M5.Display.color565(12, 8, 0), AP_COLOR_BG);
        M5.Display.setCursor(5, 39);
        M5.Display.print(ts);
        M5.Display.setTextColor(
            M5.Display.color565(20, 15, 4), AP_COLOR_BG);
        M5.Display.setCursor(4, 38);
        M5.Display.print(ts);
    }
    M5.Display.setTextColor(numCol, AP_COLOR_BG);
    M5.Display.setCursor(4, 37);
    M5.Display.print(ts);

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_BG);
    M5.Display.setCursor(76, 37);
    M5.Display.print("TIME");

    // Alt barda track
    M5.Display.fillRect(149, 119, 91, 16, AP_COLOR_CREAM5);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_CREAM5);
    M5.Display.setCursor(153, 123);
    char trk[16]; sprintf(trk, "TRACK #%03d", currentTrack);
    M5.Display.print(trk);

    // Header toplam
    M5.Display.fillRect(78, 19, LEFT_W - 78, 15, AP_COLOR_CREAM5);
    char tot[12]; sprintf(tot, "/%d", totalTracks);
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_CREAM5);
    M5.Display.setCursor(82, 23);
    M5.Display.print(tot);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateStatusDisplay()
{
    M5.Display.fillRect(0, 65, 72, 14, AP_COLOR_BG);
    if (isPlaying) {
        M5.Display.fillRoundRect(2, 66, 66, 12, 2,
            M5.Display.color565(20, 14, 2));
        M5.Display.drawRoundRect(2, 66, 66, 12, 2, AP_COLOR_CREAM1);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(AP_COLOR_CREAM1,
            M5.Display.color565(20, 14, 2));
        M5.Display.setCursor(6, 69);
        M5.Display.print(">> PLAYING");
    } else {
        M5.Display.fillRoundRect(2, 66, 66, 12, 2,
            M5.Display.color565(10, 8, 1));
        M5.Display.drawRoundRect(2, 66, 66, 12, 2, AP_COLOR_CREAM3);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(AP_COLOR_CREAM2,
            M5.Display.color565(10, 8, 1));
        M5.Display.setCursor(6, 69);
        M5.Display.print("|| PAUSED");
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateModeDisplay()
{
    M5.Display.fillRect(72, 65, LEFT_W - 72, 14, AP_COLOR_BG);
    M5.Display.setTextSize(1);

    if (isLoopEnabled) {
        M5.Display.fillRoundRect(72, 66, 56, 12, 2,
            M5.Display.color565(20, 14, 2));
        M5.Display.drawRoundRect(72, 66, 56, 12, 2, AP_COLOR_AMBER);
        M5.Display.setTextColor(AP_COLOR_AMBER,
            M5.Display.color565(20, 14, 2));
        M5.Display.setCursor(75, 69);
        M5.Display.print("LOOP ON");
    } else if (isShuffleEnabled) {
        M5.Display.fillRoundRect(72, 66, 56, 12, 2,
            M5.Display.color565(12, 8, 1));
        M5.Display.drawRoundRect(72, 66, 56, 12, 2, AP_COLOR_WARM);
        M5.Display.setTextColor(AP_COLOR_WARM,
            M5.Display.color565(12, 8, 1));
        M5.Display.setCursor(75, 69);
        M5.Display.print("SHUFFLE");
    } else {
        M5.Display.setTextColor(0x4208, AP_COLOR_BG);
        M5.Display.setCursor(75, 69);
        M5.Display.print("SEQ");
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateTimeDisplay()
{
    unsigned long el = isPlaying
        ? (millis() - trackStartTime - totalPausedTime) / 1000 : 0;
    M5.Display.fillRect(72, 46, 58, 12, AP_COLOR_BG);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0xFFFF, AP_COLOR_BG);
    M5.Display.setCursor(74, 47);
    M5.Display.printf("%02d:%02d", (int)(el / 60), (int)(el % 60));
}

// ─────────────────────────────────────────────────────────────
// Eski batarya fonksiyonu - artık kullanılmıyor, boş bırakıldı

void UnitAudioPlayerController::updateBatteryDisplay()
{
    // Krem temada pil göstergesi kaldırıldı
}

// ═════════════════════════════════════════════════════════════
//  GEÇİCİ MESAJ
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawTempMsg()
{
    int mw = LEFT_W - 4;
    M5.Display.fillRoundRect(2, 102, mw, 14, 3,
        M5.Display.color565(15, 10, 1));
    M5.Display.drawRoundRect(2, 102, mw, 14, 3, tempMessageColor);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0xFFFF,
        M5.Display.color565(15, 10, 1));
    int tx = (mw - (int)tempMessageText.length() * 6) / 2 + 2;
    if (tx < 5) tx = 5;
    M5.Display.setCursor(tx, 105);
    M5.Display.print(tempMessageText);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::clearTempArea()
{
    M5.Display.fillRect(0, 101, LEFT_W, 17, AP_COLOR_BG);
    M5.Display.setTextSize(1);

    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_BG);
    M5.Display.setCursor(2, 103);
    M5.Display.print("SPC");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Ply ");
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_BG);
    M5.Display.print("+/-");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Vol ");
    M5.Display.setTextColor(AP_COLOR_AMBER, AP_COLOR_BG);
    M5.Display.print("R");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Rnd");

    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_BG);
    M5.Display.setCursor(2, 112);
    M5.Display.print(",");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Prv ");
    M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_BG);
    M5.Display.print("/");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Nxt ");
    M5.Display.setTextColor(AP_COLOR_AMBER, AP_COLOR_BG);
    M5.Display.print("I");
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.print(":Info");
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::showTempMsg(const String& msg, uint16_t col)
{
    tempMessageText   = msg;
    tempMessageColor  = col;
    tempMessageTime   = millis();
    tempMessageActive = true;
    drawTempMsg();
}

// ═════════════════════════════════════════════════════════════
//  INPUT MODU
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawInputMode()
{
    M5.Display.fillScreen(AP_COLOR_BG);
    drawCreamLine(0, 0.9f);
    drawCreamLine(1, 0.5f);

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_BG);
    M5.Display.setCursor(68, 6);
    M5.Display.print("\x0E GO TO TRACK \x0E");
    drawCreamLine(15, 0.6f);

    // Ana kutu
    M5.Display.fillRoundRect(20, 24, 200, 72, 5, AP_COLOR_CREAM5);
    M5.Display.drawRoundRect(20, 24, 200, 72, 5, AP_COLOR_CREAM2);
    M5.Display.drawRoundRect(19, 23, 202, 74, 6,
        M5.Display.color565(20, 14, 2));

    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_CREAM5);
    M5.Display.setCursor(35, 30);
    M5.Display.print("ENTER TRACK NUMBER:");

    M5.Display.fillRoundRect(35, 40, 170, 40, 4, AP_COLOR_BG);
    M5.Display.drawRoundRect(35, 40, 170, 40, 4, 0x4208);

    M5.Display.setTextSize(3);
    if (inputTrackNumber.length() > 0) {
        // Gölge
        M5.Display.setTextColor(
            M5.Display.color565(12, 8, 0), AP_COLOR_BG);
        int tw = inputTrackNumber.length() * 18;
        M5.Display.setCursor(120 - tw / 2 + 1, 48);
        M5.Display.print(inputTrackNumber);
        M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_BG);
        M5.Display.setCursor(120 - tw / 2, 47);
        M5.Display.print(inputTrackNumber);
        // İmleç
        M5.Display.fillRect(120 - tw / 2 + tw + 2, 47, 3, 20, AP_COLOR_AMBER);
    } else {
        M5.Display.setTextColor(AP_COLOR_CREAM5, AP_COLOR_BG);
        M5.Display.setCursor(82, 47);
        M5.Display.print("_ _ _");
    }

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.setCursor(30, 100);
    M5.Display.print("[ENTER] Confirm  [DEL] Delete  [G] Exit");
    M5.Display.setTextColor(0x4208, AP_COLOR_BG);
    M5.Display.setCursor(70, 112);
    M5.Display.printf("Library: %d tracks", totalTracks);

    drawCreamLine(121, 0.6f);
    drawCreamLine(122, 0.3f);
}

// ═════════════════════════════════════════════════════════════
//  INFO EKRANI
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::enterInfoScreen()
{
    isInfoScreen = true;
    M5.Display.fillScreen(AP_COLOR_BG);
    drawCreamLine(0, 0.9f);

    M5.Display.fillRect(0, 1, 240, 14, AP_COLOR_CREAM5);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_CREAM5);
    M5.Display.setCursor(60, 4);
    M5.Display.print("MOY MUSIC - KEYBINDS");
    drawCreamLine(15, 0.5f);

    struct KeyInfo { const char* key; const char* desc; };
    KeyInfo keys[] = {
        {"SPACE",   "Play/Pause"},
        {"BtnA x1", "Play/Pause"},
        {"BtnA x2", "Next Track"},
        {"BtnA x3", "Prev Track"},
        {"+  / -",  "Vol Up/Down"},
        {"R",       "Random"},
        {"L",       "Loop On/Off"},
        {"S",       "Shuffle"},
        {",",       "Prev Track"},
        {"/",       "Next Track"},
        {".",       "-10 Tracks"},
        {";",       "+10 Tracks"},
        {"I / `",   "Info/Exit"},
    };

    int total = sizeof(keys) / sizeof(keys[0]);
    int half  = (total + 1) / 2;
    int ky    = 19;

    for (int i = 0; i < half; i++) {
        // Sol kolon
        M5.Display.fillRoundRect(2, ky, 36, 9, 1, AP_COLOR_CREAM5);
        M5.Display.drawRoundRect(2, ky, 36, 9, 1, AP_COLOR_CREAM2);
        M5.Display.setTextColor(AP_COLOR_CREAM1, AP_COLOR_CREAM5);
        M5.Display.setCursor(4, ky + 1);
        M5.Display.print(keys[i].key);
        M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_BG);
        M5.Display.setCursor(41, ky + 1);
        M5.Display.print(keys[i].desc);

        // Sağ kolon
        int j = i + half;
        if (j < total) {
            M5.Display.fillRoundRect(122, ky, 36, 9, 1, AP_COLOR_CREAM5);
            M5.Display.drawRoundRect(122, ky, 36, 9, 1, AP_COLOR_AMBER);
            M5.Display.setTextColor(AP_COLOR_AMBER, AP_COLOR_CREAM5);
            M5.Display.setCursor(124, ky + 1);
            M5.Display.print(keys[j].key);
            M5.Display.setTextColor(AP_COLOR_CREAM2, AP_COLOR_BG);
            M5.Display.setCursor(161, ky + 1);
            M5.Display.print(keys[j].desc);
        }
        ky += 11;
    }

    drawCreamLine(126, 0.4f);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0x6B4D, AP_COLOR_BG);
    M5.Display.setCursor(52, 129);
    M5.Display.print("Press  I  or  `  to exit");
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::exitInfoScreen()
{
    isInfoScreen    = false;
    lastDiskPlaying = !isPlaying;
    drawMainScreen();
}

// ═════════════════════════════════════════════════════════════
//  KONTROL / EYLEMLER
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::volumeUp()
{
    if (currentVolume >= 30) return;
    currentVolume++;
    audioplayer.setVolume(currentVolume);
    char m[20]; sprintf(m, "VOL %d%%", (currentVolume * 100) / 30);
    drawBottomBar();
    showTempMsg(m, AP_COLOR_CREAM1);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::volumeDown()
{
    if (currentVolume <= 0) return;
    currentVolume--;
    audioplayer.setVolume(currentVolume);
    char m[20]; sprintf(m, "VOL %d%%", (currentVolume * 100) / 30);
    drawBottomBar();
    showTempMsg(m, AP_COLOR_CREAM2);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::toggleLoop()
{
    isLoopEnabled = !isLoopEnabled;
    if (isLoopEnabled && isShuffleEnabled) isShuffleEnabled = false;
    String   msg = isLoopEnabled ? "LOOP ON" : "LOOP OFF";
    uint16_t c   = AP_COLOR_AMBER;
    updateModeDisplay();
    showTempMsg(msg, c);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::toggleShuffle()
{
    isShuffleEnabled = !isShuffleEnabled;
    if (isShuffleEnabled && isLoopEnabled) isLoopEnabled = false;
    String   msg = isShuffleEnabled ? "SHUFFLE ON" : "SHUFFLE OFF";
    uint16_t c   = AP_COLOR_WARM;
    updateModeDisplay();
    showTempMsg(msg, c);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::goToTrack(int n)
{
    if (n < 1 || n > (int)totalTracks) {
        char m[25]; sprintf(m, "BAD! (1-%d)", totalTracks);
        showTempMsg(m, AP_COLOR_RED);
        return;
    }
    audioplayer.selectAudioNum(n);
    delay(100);
    audioplayer.playAudio();
    isPlaying         = true;
    trackStartTime    = millis();
    totalPausedTime   = 0;
    currentTrack      = n;
    lastDisplayedTrack = n;

    char m[15]; sprintf(m, "GOTO %d", n);
    updateTrackDisplay();
    updateStatusDisplay();
    showTempMsg(m, AP_COLOR_CREAM1);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::randomTrack()
{
    if (totalTracks == 0) return;
    uint16_t rnd = random(1, totalTracks + 1);
    audioplayer.selectAudioNum(rnd);
    currentTrack       = rnd;
    lastDisplayedTrack = rnd;
    trackStartTime     = millis();
    totalPausedTime    = 0;

    if (lastPlayStatus == AUDIO_PLAYER_STATUS_PLAYING) {
        audioplayer.playAudio();
        isPlaying = true;
    }
    char m[15]; sprintf(m, "RND %d!", rnd);
    updateTrackDisplay();
    showTempMsg(m, AP_COLOR_AMBER);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::executeAction(int clicks)
{
    if (clicks == 1) {
        uint8_t st = audioplayer.checkPlayStatus();
        if (st == AUDIO_PLAYER_STATUS_PLAYING) {
            audioplayer.pauseAudio();
            pausedTime = millis();
            isPlaying  = false;
            updateStatusDisplay();
            showTempMsg("PAUSED", AP_COLOR_CREAM2);
        } else {
            audioplayer.playAudio();
            if (st == AUDIO_PLAYER_STATUS_STOPPED) {
                trackStartTime  = millis();
                totalPausedTime = 0;
            } else {
                totalPausedTime += (millis() - pausedTime);
            }
            isPlaying = true;
            updateStatusDisplay();
            showTempMsg("PLAYING", AP_COLOR_CREAM1);
        }
    }
    else if (clicks == 2) {
        audioplayer.nextAudio(); delay(100); audioplayer.playAudio();
        isPlaying = true; trackStartTime = millis(); totalPausedTime = 0;
        updateStatusDisplay();
        showTempMsg("NEXT >>", AP_COLOR_CREAM1);
    }
    else if (clicks == 3) {
        audioplayer.previousAudio(); delay(100); audioplayer.playAudio();
        isPlaying = true; trackStartTime = millis(); totalPausedTime = 0;
        updateStatusDisplay();
        showTempMsg("<< PREV", AP_COLOR_CREAM1);
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::checkAutoNext()
{
    static unsigned long lc = 0;
    if (millis() - lc < 500) return;
    lc = millis();

    uint8_t cs = audioplayer.checkPlayStatus();

    if (lastPlayStatus == AUDIO_PLAYER_STATUS_PLAYING &&
        cs == AUDIO_PLAYER_STATUS_STOPPED)
    {
        if (isLoopEnabled) {
            delay(100); audioplayer.playAudio(); delay(100);
            trackStartTime = millis(); totalPausedTime = 0; isPlaying = true;
            if (!isInfoScreen) showTempMsg("LOOP", AP_COLOR_AMBER);
        }
        else if (isShuffleEnabled) {
            uint16_t r = random(1, totalTracks + 1);
            audioplayer.selectAudioNum(r); delay(200);
            audioplayer.playAudio(); delay(100);
            trackStartTime = millis(); totalPausedTime = 0; isPlaying = true;
            if (!isInfoScreen) showTempMsg("SHUFFLE", AP_COLOR_WARM);
        }
        else {
            audioplayer.nextAudio(); delay(200);
            audioplayer.playAudio(); delay(100);
            trackStartTime = millis(); totalPausedTime = 0; isPlaying = true;
            if (!isInfoScreen) showTempMsg("AUTO NEXT", AP_COLOR_CREAM1);
        }
    }

    uint16_t rt = audioplayer.getCurrentAudioNumber();
    if (rt < 1 || rt > totalTracks) return;

    if (rt != lastDisplayedTrack) {
        currentTrack       = rt;
        lastDisplayedTrack = rt;
        trackStartTime     = millis();
        totalPausedTime    = 0;
        if (!isInfoScreen) updateTrackDisplay();
    }

    if (lastPlayStatus != cs) {
        if (cs == AUDIO_PLAYER_STATUS_PLAYING) {
            if (lastPlayStatus == AUDIO_PLAYER_STATUS_PAUSED)
                totalPausedTime += (millis() - pausedTime);
            isPlaying = true;
        } else if (cs == AUDIO_PLAYER_STATUS_PAUSED) {
            pausedTime = millis();
            isPlaying  = false;
        } else {
            isPlaying = false;
        }
        if (!isInfoScreen) updateStatusDisplay();
    }
    lastPlayStatus = cs;
}
/* 
// ═════════════════════════════════════════════════════════════
//  VİZUALİZER (Stub - krem temada devre dışı)
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::enterFullscreenVisualizer() {}
void UnitAudioPlayerController::exitFullscreenVisualizer()  {}
void UnitAudioPlayerController::switchVisualizerMode()      {}
void UnitAudioPlayerController::handleKeyboardVisualizer()  {}
void UnitAudioPlayerController::updateFullscreenVisualizer(){}
void UnitAudioPlayerController::drawFullscreenBars()        {}
void UnitAudioPlayerController::drawFullscreenWaves()       {}
void UnitAudioPlayerController::drawMatrixRain()            {}
void UnitAudioPlayerController::drawVisualizerHeader()      {}
void UnitAudioPlayerController::initMatrixColumns()         {}
void UnitAudioPlayerController::drawFullscreenTempMessage(
        const String&, uint16_t)                            {}
void UnitAudioPlayerController::showSplash()                {}
uint16_t UnitAudioPlayerController::hsvToRgb565(
        float, float, float) { return 0; } */