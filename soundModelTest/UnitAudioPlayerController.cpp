#include "UnitAudioPlayerController.h"
#include "../ExtraMenu.h"   // Geri dönüş menüsü - projenize göre ayarlayın
#include <MyOS.h>           // mainOS pointer - projenize göre ayarlayın

// ─── Sabit Veri ───────────────────────────────────────────────
const char UnitAudioPlayerController::matrixChars[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ@#$%&*";

// ═════════════════════════════════════════════════════════════
//  YAŞAM DÖNGÜSÜ
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::Begin()
{
    showTopBar = false;

    // ── Splash ────────────────────────────────────────────────
    showSplash();

    // ── Port Pinlerini Al ─────────────────────────────────────
    int8_t pin1 = M5.getPin(m5::pin_name_t::port_a_pin1);
    int8_t pin2 = M5.getPin(m5::pin_name_t::port_a_pin2);
    Serial.printf("[AudioPlayer] Port A: RX=%d TX=%d\n", pin1, pin2);

    // ── Bağlantı Ekranı ───────────────────────────────────────
    M5.Display.fillScreen(AP_COLOR_BG);
    drawRainbowLine(0, 0.9f);
    drawRainbowLine(134, 0.9f);

    M5.Display.setTextSize(2);
    M5.Display.setTextColor(AP_COLOR_ACCENT2, AP_COLOR_BG);
    M5.Display.setCursor(30, 40);
    M5.Display.print("CONNECTING");

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT1, AP_COLOR_BG);
    M5.Display.setCursor(42, 60);
    M5.Display.print("Audio Player Module");

    M5.Display.setTextColor(0x4208, AP_COLOR_BG);
    M5.Display.setCursor(50, 72);
    M5.Display.printf("Port A: G%d / G%d", pin1, pin2);

    // ── Bağlantı Döngüsü ──────────────────────────────────────
    int  counter = 0;
    int  dotX    = 60;

    while (!audioplayer.begin(&Serial1, pin1, pin2))
    {
        // Animasyonlu ilerleme noktaları
        for (int i = 0; i < 5; i++) {
            uint16_t dc = hsvToRgb565((float)(dotX + i * 8) / 240.0f, 1.0f, 0.9f);
            M5.Display.fillRect(dotX + i * 8, 88, 6, 8, dc);
        }
        dotX += 8;
        if (dotX > 180) {
            M5.Display.fillRect(55, 88, 180, 10, AP_COLOR_BG);
            dotX = 60;
        }
        delay(400);

        counter++;
        if (counter > 12) {   // ~5 saniye sonra vazgeç
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
        M5.Display.color565(0, 40, 0));
    M5.Display.drawRoundRect(40, 86, 160, 14, 3, AP_COLOR_GREEN);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_GREEN,
        M5.Display.color565(0, 40, 0));
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

    initialized    = true;
    lastDiskPlaying = !isPlaying;

    // ── Ana Ekranı Çiz ────────────────────────────────────────
    drawMainScreen();
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::Loop()
{
    if (!initialized) return;

    M5.update();   // M5Stack güncelle

    // ── ESC → Geri Dön ────────────────────────────────────────
    // (Escape tuşu '`' olarak atanmış - info/viz modlarında içeride handle ediliyor)

    // ── Info Ekranı ───────────────────────────────────────────
    if (isInfoScreen) {
        handleKeyboard();   // sadece 'i' / '`' dinlenir
        checkAutoNext();
        return;
    }

    // ── Tam Ekran Görselleştirici ─────────────────────────────
    if (isFullscreenVisualizer) {
        unsigned long now = millis();
        rainbowOffset = fmod(rainbowOffset + 0.003f, 1.0f);

        unsigned long iv = (visualizerMode == VIZ_MATRIX) ? 80 : 90;
        if (now - lastMatrixUpdate >= iv) {
            lastMatrixUpdate = now;
            updateFullscreenVisualizer();

            switch (visualizerMode) {
                case VIZ_SPECTRUM: drawFullscreenBars();  drawVisualizerHeader(); break;
                case VIZ_WAVE:     drawFullscreenWaves(); drawVisualizerHeader(); break;
                case VIZ_MATRIX:   drawMatrixRain();      drawVisualizerHeader(); break;
            }
            if (tempMessageActive && (now - tempMessageTime > TEMP_MSG_DURATION)) {
                tempMessageActive = false;
                M5.Display.fillRect(0, 128, 240, 7, AP_COLOR_BG);
            }
        }
        handleKeyboardVisualizer();
        handleButtonA();
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
    // Çizim Loop() içinde zaten yönetiliyor
}

// ═════════════════════════════════════════════════════════════
//  KLAVYE / BUTON İŞLEME
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::handleKeyboard()
{
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;

    Keyboard_Class::KeysState st = M5Cardputer.Keyboard.keysState();

    // ── Info ekranındayken ────────────────────────────────────
    if (isInfoScreen) {
        for (auto k : st.word) {
            if (k == 'i' || k == 'I' || k == '`' || k == '~') {
                exitInfoScreen();
                return;
            }
        }
        return;
    }

    // ── Input modundayken ─────────────────────────────────────
    if (isInputMode) {
        handleKeyboardInput();
        return;
    }

    // ── Normal mod ────────────────────────────────────────────
    handleKeyboardNormal();
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::handleKeyboardNormal()
{
    Keyboard_Class::KeysState st = M5Cardputer.Keyboard.keysState();

    for (auto k : st.word) {
        switch (k) {
            // Ses
            case '+': case '=': volumeUp();   break;
            case '-':           volumeDown();  break;

            // Mod
            case 'l': case 'L': toggleLoop();    break;
            case 's': case 'S': toggleShuffle(); break;

            // Rastgele
            case 'r': case 'R': randomTrack(); break;

            // Go To Track
            case 'g': case 'G':
                isInputMode = true;
                inputTrackNumber = "";
                drawInputMode();
                return;

            // Görselleştirici
            case 'v': case 'V': enterFullscreenVisualizer(); return;

            // Info
            case 'i': case 'I': enterInfoScreen(); return;

            // Çıkış → ExtraMenu
            case '`': case '~':
                mainOS->ChangeMenu(new Extra(mainOS));
                return;

            // Navigasyon
            case ',': executeAction(3); break;   // Önceki
            case '/': executeAction(2); break;   // Sonraki
            case ';': goToTrack(min((int)currentTrack + 10, (int)totalTracks)); break;
            case '.': goToTrack(max((int)currentTrack - 10, 1));                break;
        }
    }

    if (st.space) executeAction(1);   // Play / Pause
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::handleKeyboardVisualizer()
{
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;

    Keyboard_Class::KeysState st = M5Cardputer.Keyboard.keysState();

    for (auto k : st.word) {
        switch (k) {
            case 'v': case 'V': switchVisualizerMode();   break;
            case '`': case '~': exitFullscreenVisualizer(); break;
            case '+': case '=': volumeUp();   break;
            case '-':           volumeDown();  break;
            case 'r': case 'R': randomTrack(); break;
            case 'l': case 'L': toggleLoop();    break;
            case 's': case 'S': toggleShuffle(); break;
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
        isInputMode = false;
        inputTrackNumber = "";
        lastDiskPlaying = !isPlaying;
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
            isInputMode = false;
            inputTrackNumber = "";
            lastDiskPlaying = !isPlaying;
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

uint16_t UnitAudioPlayerController::hsvToRgb565(float h, float s, float v)
{
    float r, g, b;
    int   i = (int)(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s),
          q = v * (1 - f * s),
          t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r=v; g=t; b=p; break;
        case 1: r=q; g=v; b=p; break;
        case 2: r=p; g=v; b=t; break;
        case 3: r=p; g=q; b=v; break;
        case 4: r=t; g=p; b=v; break;
        default:r=v; g=p; b=q; break;
    }
    return M5.Display.color565(
        (uint8_t)(r * 255),
        (uint8_t)(g * 255),
        (uint8_t)(b * 255));
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawRainbowLine(int y, float br, float off)
{
    for (int x = 0; x < 240; x++)
        M5.Display.drawPixel(x, y,
            hsvToRgb565(fmod((float)x / 240.0f + off, 1.0f), 1.0f, br));
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawNeonText(
        int x, int y, const char* txt, uint16_t col, uint8_t sz)
{
    M5.Display.setTextSize(sz);
    uint16_t gc = M5.Display.color565(
        (((col >> 11) & 0x1F) << 2) >> 2,
        (((col >>  5) & 0x3F) << 1) >> 2,
        ((col & 0x1F) << 2) >> 2);
    M5.Display.setTextColor(gc, AP_COLOR_BG);
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
    M5.Display.drawRoundRect(x-1, y-1, w+2, h+2, 4,
        M5.Display.color565(
            ((col >> 11) & 0x1F) << 2,
            ((col >>  5) & 0x3F) << 1,
            (col & 0x1F) << 2));
}

// ═════════════════════════════════════════════════════════════
//  SPLASH
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::showSplash()
{
    M5.Display.fillScreen(AP_COLOR_BG);

    // Arka plan ızgara
    for (int y = 40; y < 135; y += 8) {
        uint16_t gc = M5.Display.color565(0, 0, (uint8_t)(15 * (y - 40) / 95));
        M5.Display.drawFastHLine(0, y, 240, gc);
    }
    for (int x = 0; x < 240; x += 20)
        M5.Display.drawFastVLine(x, 40, 95,
            M5.Display.color565(0, 0, 10));

    // Neon üst çizgiler
    for (int i = 0; i < 3; i++)
        drawRainbowLine(i, 0.9f, (float)i * 0.05f);

    // Logo
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(AP_COLOR_ACCENT2, AP_COLOR_BG);
    M5.Display.setCursor(10, 50);
    M5.Display.print("MOY");
    M5.Display.setTextColor(AP_COLOR_ACCENT1, AP_COLOR_BG);
    M5.Display.setCursor(64, 50);
    M5.Display.print("MUSIC");

    // Alt çizgiler
    M5.Display.fillRect(0, 76, 240, 2, AP_COLOR_ACCENT2);
    drawRainbowLine(77, 1.0f);
    M5.Display.fillRect(0, 78, 240, 2, AP_COLOR_ACCENT1);

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT3, AP_COLOR_BG);
    M5.Display.setCursor(58, 84);
    M5.Display.print("[ DJ AUDIO PLAYER v6.0 ]");

    M5.Display.setTextColor(0x528A, AP_COLOR_BG);
    M5.Display.setCursor(74, 94);
    M5.Display.print("by Andy + AI");

    // EQ çubukları
    int eqH[] = {8,14,20,16,22,18,12,24,20,16,14,10,18,22,16};
    for (int i = 0; i < 15; i++) {
        uint16_t ec = hsvToRgb565((float)i / 15.0f, 1.0f, 0.9f);
        M5.Display.fillRect(8 + i * 15, 120 - eqH[i], 12, eqH[i], ec);
        M5.Display.drawRect(8 + i * 15, 120 - eqH[i], 12, eqH[i],
            M5.Display.color565(255, 255, 255));
    }

    for (int i = 0; i < 3; i++)
        drawRainbowLine(120 + i, 0.8f, (float)i * 0.1f);

    M5.Display.setTextColor(0x4208, AP_COLOR_BG);
    M5.Display.setCursor(52, 126);
    M5.Display.print(">> Press any key to DROP <<");

    // Tuşa bas veya 4 saniye bekle
    unsigned long t = millis();
    while (millis() - t < 4000) {
        M5.update();
        if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) break;
        delay(50);
    }
    M5.Display.fillScreen(AP_COLOR_BG);
}

// ═════════════════════════════════════════════════════════════
//  DİSK
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawDiskBackground()
{
    for (int r = DISK_R + 3; r >= DISK_R; r--)
        M5.Display.drawCircle(DISK_CX, DISK_CY, r,
            hsvToRgb565(0.75f, 1.0f, 0.2f + (float)(DISK_R + 3 - r) * 0.15f));

    M5.Display.fillCircle(DISK_CX, DISK_CY, DISK_R, 0x0821);

    int rr[] = {38, 35, 32, 29, 26, 23, 20, 17};
    for (int i = 0; i < 8; i++)
        M5.Display.drawCircle(DISK_CX, DISK_CY, rr[i],
            hsvToRgb565((float)i / 8.0f * 0.6f + 0.6f, 0.5f, 0.2f));
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawDiskDynamic(float angle)
{
    // Dönen gökkuşağı dış halka
    for (int r = DISK_R - 1; r > DISK_R - 7; r--) {
        for (int a = 0; a < 360; a += 3) {
            float rad = a * 3.14159f / 180.0f;
            float hue = fmod((float)a / 360.0f + angle / 360.0f, 1.0f);
            int   px  = DISK_CX + (int)(r * cosf(rad));
            int   py  = DISK_CY + (int)(r * sinf(rad));
            M5.Display.drawPixel(px, py, hsvToRgb565(hue, 1.0f, 0.6f));
        }
    }

    // Dönen parlak nokta
    static float lastSA = 0;
    {
        float lr = lastSA * 3.14159f / 180.0f;
        M5.Display.fillCircle(
            DISK_CX + (int)((DISK_R - 8) * cosf(lr)),
            DISK_CY + (int)((DISK_R - 8) * sinf(lr)), 3, 0x0821);
    }
    lastSA = angle;
    float rad = angle * 3.14159f / 180.0f;
    int   sx  = DISK_CX + (int)((DISK_R - 8) * cosf(rad));
    int   sy  = DISK_CY + (int)((DISK_R - 8) * sinf(rad));
    M5.Display.fillCircle(sx, sy, 3, 0xFFFF);
    M5.Display.fillCircle(sx, sy, 1, AP_COLOR_ACCENT1);

    // Merkez etiket
    M5.Display.fillCircle(DISK_CX, DISK_CY, 15, 0x0821);
    if (isPlaying) {
        float hue = fmod(angle / 360.0f, 1.0f);
        for (int r = 15; r >= 13; r--)
            M5.Display.drawCircle(DISK_CX, DISK_CY, r,
                hsvToRgb565(hue, 1.0f, 0.9f));
        M5.Display.fillCircle(DISK_CX, DISK_CY, 12, AP_COLOR_BG);
        M5.Display.fillCircle(DISK_CX, DISK_CY,  9,
            hsvToRgb565(fmod(hue + 0.5f, 1.0f), 0.9f, 1.0f));
        M5.Display.fillCircle(DISK_CX, DISK_CY,  6, AP_COLOR_BG);
        M5.Display.fillCircle(DISK_CX, DISK_CY,  3,
            hsvToRgb565(fmod(hue + 0.25f, 1.0f), 1.0f, 1.0f));
    } else {
        M5.Display.drawCircle(DISK_CX, DISK_CY, 14, 0x2945);
        M5.Display.drawCircle(DISK_CX, DISK_CY, 12, 0x1082);
        M5.Display.fillCircle(DISK_CX, DISK_CY,  9, 0x1082);
        M5.Display.fillCircle(DISK_CX, DISK_CY,  4, 0x2945);
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
    const int lyY = 80, ryY = 88;

    auto segColor = [](int i) -> uint16_t {
        if (i < 6)  return AP_COLOR_GREEN;
        if (i < 9)  return AP_COLOR_ACCENT3;
        if (i < 11) return AP_COLOR_ORANGE;
        return AP_COLOR_RED;
    };

    for (int ch = 0; ch < 2; ch++) {
        int  baseY = (ch == 0) ? lyY : ryY;
        int* vu    = (ch == 0) ? vuLeft : vuRight;

        for (int i = 0; i < 12; i++) {
            uint16_t onC  = segColor(i);
            uint16_t offC = 0x0841;
            M5.Display.fillRect(sx + i * (bW + bG), baseY, bW, bH,
                vu[i] ? onC : offC);
            if (vu[i])
                M5.Display.drawFastHLine(sx + i * (bW + bG), baseY, bW, 0xFFFF);
        }
    }
}

// ═════════════════════════════════════════════════════════════
//  ÜST BAR
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawTopBar()
{
    for (int y = 0; y < 18; y++) {
        float t = (float)y / 18.0f;
        M5.Display.drawFastHLine(0, y, 240,
            M5.Display.color565(
                (uint8_t)(20 * t),
                (uint8_t)(10 * t),
                (uint8_t)(40 * (1-t) + 10 * t)));
    }
    drawRainbowLine(17, 0.8f, rainbowOffset);

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT2, 0x0000);
    M5.Display.setCursor(4, 5);
    M5.Display.print("MOY");
    M5.Display.setTextColor(AP_COLOR_ACCENT1, 0x0000);
    M5.Display.print(" MUSIC");
    M5.Display.setTextColor(0x4208, 0x0000);
    M5.Display.print(" v6.0");

    // DJ rozeti
    M5.Display.fillRoundRect(186, 2, 50, 12, 2, 0x2000);
    M5.Display.drawRoundRect(186, 2, 50, 12, 2, AP_COLOR_ACCENT2);
    M5.Display.setTextColor(AP_COLOR_ACCENT2, 0x2000);
    M5.Display.setCursor(190, 5);
    M5.Display.print("DJ MODE");
}

// ═════════════════════════════════════════════════════════════
//  SOL PANEL
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawLeftPanel()
{
    // Degrade arka plan
    for (int y = 18; y < 118; y++) {
        float t = (float)(y - 18) / 100.0f;
        M5.Display.drawFastHLine(0, y, 136,
            M5.Display.color565(
                (uint8_t)(4 * t),
                (uint8_t)(8 * (1-t) * t * 4),
                (uint8_t)(16 * (1-t) * t * 4)));
    }

    // ── Track header ──
    M5.Display.drawFastHLine(0, 18, 136, AP_COLOR_ACCENT2);
    M5.Display.fillRect(0, 19, 136, 16, 0x0821);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT2, 0x0821);
    M5.Display.setCursor(4, 23);
    M5.Display.print("\x0E TRACK");

    char tot[10]; sprintf(tot, "/%d", totalTracks);
    M5.Display.setTextColor(0x528A, 0x0821);
    M5.Display.setCursor(90, 23);
    M5.Display.print(tot);
    M5.Display.drawFastHLine(0, 35, 136, AP_COLOR_ACCENT2);

    // ── Büyük track numarası ──
    M5.Display.fillRect(0, 36, 136, 28, AP_COLOR_BG);
    char ts[6]; sprintf(ts, "%03d", currentTrack);
    uint16_t numCol = isPlaying ? AP_COLOR_ACCENT1 : 0x4208;
    M5.Display.setTextSize(3);

    if (isPlaying) {
        M5.Display.setTextColor(
            M5.Display.color565(0, 80, 120), AP_COLOR_BG);
        M5.Display.setCursor(5, 39);
        M5.Display.print(ts);
        M5.Display.setTextColor(
            M5.Display.color565(0, 180, 220), AP_COLOR_BG);
        M5.Display.setCursor(4, 38);
        M5.Display.print(ts);
    }
    M5.Display.setTextColor(numCol, AP_COLOR_BG);
    M5.Display.setCursor(4, 37);
    M5.Display.print(ts);

    // Zaman etiketi
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT3, AP_COLOR_BG);
    M5.Display.setCursor(72, 37);
    M5.Display.print("TIME");

    unsigned long el = isPlaying
        ? (millis() - trackStartTime - totalPausedTime) / 1000 : 0;
    M5.Display.setTextColor(0xFFFF, AP_COLOR_BG);
    M5.Display.setCursor(68, 47);
    M5.Display.printf("%02d:%02d", (int)(el / 60), (int)(el % 60));

    M5.Display.drawFastHLine(0, 64, 136, 0x2945);

    // ── Play status ──
    M5.Display.fillRect(0, 65, 136, 15, AP_COLOR_BG);
    if (isPlaying) {
        M5.Display.fillRoundRect(2, 66, 58, 12, 2,
            M5.Display.color565(0, 60, 0));
        M5.Display.drawRoundRect(2, 66, 58, 12, 2, AP_COLOR_GREEN);
        M5.Display.setTextColor(AP_COLOR_GREEN,
            M5.Display.color565(0, 60, 0));
        M5.Display.setCursor(7, 69);
        M5.Display.print("\x10 PLAYING");
    } else {
        M5.Display.fillRoundRect(2, 66, 55, 12, 2,
            M5.Display.color565(60, 60, 0));
        M5.Display.drawRoundRect(2, 66, 55, 12, 2, AP_COLOR_ACCENT3);
        M5.Display.setTextColor(AP_COLOR_ACCENT3,
            M5.Display.color565(60, 60, 0));
        M5.Display.setCursor(7, 69);
        M5.Display.print("|| PAUSED");
    }

    // Mod rozeti
    M5.Display.fillRect(64, 66, 70, 12, AP_COLOR_BG);
    if (isLoopEnabled) {
        M5.Display.fillRoundRect(64, 66, 68, 12, 2,
            M5.Display.color565(60, 60, 0));
        M5.Display.drawRoundRect(64, 66, 68, 12, 2, AP_COLOR_ACCENT3);
        M5.Display.setTextColor(AP_COLOR_ACCENT3,
            M5.Display.color565(60, 60, 0));
        M5.Display.setCursor(68, 69);
        M5.Display.print("\x1D LOOP");
    } else if (isShuffleEnabled) {
        M5.Display.fillRoundRect(64, 66, 68, 12, 2,
            M5.Display.color565(0, 0, 60));
        M5.Display.drawRoundRect(64, 66, 68, 12, 2, AP_COLOR_ACCENT1);
        M5.Display.setTextColor(AP_COLOR_ACCENT1,
            M5.Display.color565(0, 0, 60));
        M5.Display.setCursor(68, 69);
        M5.Display.print("~ SHUF");
    } else {
        M5.Display.setTextColor(0x2945, AP_COLOR_BG);
        M5.Display.setCursor(68, 69);
        M5.Display.print("= SEQ");
    }

    M5.Display.drawFastHLine(0, 78, 136, 0x1082);

    // ── VU Metre ──
    M5.Display.fillRect(0, 79, 136, 19, AP_COLOR_BG);
    drawVuMeter();
    M5.Display.drawFastHLine(0, 98, 136, 0x1082);

    // ── Kısayol ipuçları ──
    M5.Display.fillRect(0, 99, 136, 19, AP_COLOR_BG);
    M5.Display.setTextSize(1);

    M5.Display.setTextColor(AP_COLOR_ACCENT2, AP_COLOR_BG);
    M5.Display.setCursor(4, 101);
    M5.Display.print("SPC");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":Play ");
    M5.Display.setTextColor(AP_COLOR_ACCENT1, AP_COLOR_BG);
    M5.Display.print("+/-");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":Vol");

    M5.Display.setTextColor(AP_COLOR_ACCENT3, AP_COLOR_BG);
    M5.Display.setCursor(4, 110);
    M5.Display.print("G");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":GoTo ");
    M5.Display.setTextColor(AP_COLOR_GREEN, AP_COLOR_BG);
    M5.Display.print("V");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":Viz ");
    M5.Display.setTextColor(AP_COLOR_ORANGE, AP_COLOR_BG);
    M5.Display.print("I");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":Info");
}

// ═════════════════════════════════════════════════════════════
//  ALT BAR
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawBottomBar()
{
    M5.Display.fillRect(0, 118, 240, 17, AP_COLOR_BG);
    drawRainbowLine(118, 0.7f, rainbowOffset);
    M5.Display.fillRect(0, 119, 240, 16, AP_COLOR_PANEL);

    int vp = (currentVolume * 100) / 30;

    // VOL etiketi
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT2, AP_COLOR_PANEL);
    M5.Display.setCursor(3, 123);
    M5.Display.print("VOL");

    // Ses barı
    const int bX = 26, bY = 121, sW = 4, sH = 9, sG = 1, segTotal = 16;
    int act = (vp * segTotal) / 100;
    for (int i = 0; i < segTotal; i++) {
        uint16_t c;
        if (i < act) {
            if      (i <  7) c = AP_COLOR_GREEN;
            else if (i < 11) c = AP_COLOR_ACCENT3;
            else if (i < 14) c = AP_COLOR_ORANGE;
            else             c = AP_COLOR_RED;
            M5.Display.fillRect(bX + i * (sW + sG), bY, sW, sH, c);
            M5.Display.drawFastHLine(bX + i * (sW + sG), bY, sW, 0xFFFF);
        } else {
            M5.Display.fillRect(bX + i * (sW + sG), bY, sW, sH, 0x0820);
        }
    }

    char vstr[6]; sprintf(vstr, "%3d%%", vp);
    M5.Display.setTextColor(0xFFFF, AP_COLOR_PANEL);
    M5.Display.setCursor(110, 123);
    M5.Display.print(vstr);

    M5.Display.drawFastVLine(135, 119, 16, 0x2945);

    // ── Batarya ──
    int  lv  = M5.Power.getBatteryLevel();
    int  vol2 = M5.Power.getBatteryVoltage();
    bool ch  = (vol2 > 4200);
    uint16_t bc = (lv > 60) ? AP_COLOR_GREEN
                : (lv > 20) ? AP_COLOR_ACCENT3 : AP_COLOR_RED;

    int bx = 140, by = 121;
    M5.Display.fillRect(bx, by, 30, 11, AP_COLOR_PANEL);
    M5.Display.drawRect(bx, by, 24, 11, 0x8410);
    M5.Display.fillRect(bx + 24, by + 3, 3, 5, 0x8410);

    int fi = (lv * 20) / 100; if (fi > 22) fi = 22;
    if (fi > 0) M5.Display.fillRect(bx + 2, by + 2, fi, 7, bc);
    if (ch) {
        M5.Display.setTextColor(AP_COLOR_ACCENT3, AP_COLOR_PANEL);
        M5.Display.setCursor(bx + 7, by + 1);
        M5.Display.print("~");
    }

    char bstr[6]; sprintf(bstr, "%d%%", lv);
    M5.Display.setTextColor(bc, AP_COLOR_PANEL);
    M5.Display.setCursor(169, 123);
    M5.Display.print(bstr);

    M5.Display.drawFastVLine(195, 119, 16, 0x2945);

    // Track kısa bilgi
    M5.Display.setTextColor(0x528A, AP_COLOR_PANEL);
    M5.Display.setCursor(198, 123);
    char trk[10]; sprintf(trk, "#%03d", currentTrack);
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

    // Dikey ayırıcı - neon
    for (int y = 18; y < 118; y++) {
        float t = (float)(y - 18) / 100.0f;
        M5.Display.drawPixel(136, y,
            hsvToRgb565(fmod(0.75f + t * 0.5f, 1.0f), 1.0f, 0.5f));
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

    rainbowOffset = fmod(rainbowOffset + 0.002f, 1.0f);

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

    // Geçici mesaj süresi dolduysa temizle
    if (tempMessageActive && (now - tempMessageTime > TEMP_MSG_DURATION)) {
        tempMessageActive = false;
        clearTempArea();
    }

    // Batarya
    if (now - lastBatteryUpdate >= BATTERY_UPDATE_MS) {
        lastBatteryUpdate = now;
        updateBatteryDisplay();
    }

    // Rainbow çizgileri güncelle
    if (now - lastRainbowUpdate > RAINBOW_UPDATE_MS) {
        lastRainbowUpdate = now;
        drawRainbowLine(17,  0.8f, rainbowOffset);
        drawRainbowLine(118, 0.7f, fmod(rainbowOffset + 0.5f, 1.0f));
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateTrackDisplay()
{
    M5.Display.fillRect(0, 36, 136, 28, AP_COLOR_BG);
    char ts[6]; sprintf(ts, "%03d", currentTrack);
    uint16_t numCol = isPlaying ? AP_COLOR_ACCENT1 : 0x4208;
    M5.Display.setTextSize(3);

    if (isPlaying) {
        M5.Display.setTextColor(M5.Display.color565(0, 80, 120), AP_COLOR_BG);
        M5.Display.setCursor(5, 39); M5.Display.print(ts);
        M5.Display.setTextColor(M5.Display.color565(0, 180, 220), AP_COLOR_BG);
        M5.Display.setCursor(4, 38); M5.Display.print(ts);
    }
    M5.Display.setTextColor(numCol, AP_COLOR_BG);
    M5.Display.setCursor(4, 37); M5.Display.print(ts);

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT3, AP_COLOR_BG);
    M5.Display.setCursor(72, 37);
    M5.Display.print("TIME");

    // Alt barda track güncelle
    M5.Display.fillRect(196, 119, 44, 16, AP_COLOR_PANEL);
    M5.Display.setTextColor(0x528A, AP_COLOR_PANEL);
    M5.Display.setCursor(198, 123);
    char trk[10]; sprintf(trk, "#%03d", currentTrack);
    M5.Display.print(trk);

    // Header toplam
    M5.Display.fillRect(80, 19, 55, 16, 0x0821);
    char tot[10]; sprintf(tot, "/%d", totalTracks);
    M5.Display.setTextColor(0x528A, 0x0821);
    M5.Display.setCursor(90, 23);
    M5.Display.print(tot);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateStatusDisplay()
{
    M5.Display.fillRect(0, 65, 136, 14, AP_COLOR_BG);
    if (isPlaying) {
        M5.Display.fillRoundRect(2, 66, 58, 12, 2,
            M5.Display.color565(0, 60, 0));
        M5.Display.drawRoundRect(2, 66, 58, 12, 2, AP_COLOR_GREEN);
        M5.Display.setTextColor(AP_COLOR_GREEN,
            M5.Display.color565(0, 60, 0));
        M5.Display.setCursor(7, 69);
        M5.Display.print("\x10 PLAYING");
    } else {
        M5.Display.fillRoundRect(2, 66, 55, 12, 2,
            M5.Display.color565(60, 60, 0));
        M5.Display.drawRoundRect(2, 66, 55, 12, 2, AP_COLOR_ACCENT3);
        M5.Display.setTextColor(AP_COLOR_ACCENT3,
            M5.Display.color565(60, 60, 0));
        M5.Display.setCursor(7, 69);
        M5.Display.print("|| PAUSED");
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateModeDisplay()
{
    M5.Display.fillRect(64, 65, 72, 14, AP_COLOR_BG);
    M5.Display.setTextSize(1);

    if (isLoopEnabled) {
        M5.Display.fillRoundRect(64, 66, 68, 12, 2,
            M5.Display.color565(60, 60, 0));
        M5.Display.drawRoundRect(64, 66, 68, 12, 2, AP_COLOR_ACCENT3);
        M5.Display.setTextColor(AP_COLOR_ACCENT3,
            M5.Display.color565(60, 60, 0));
        M5.Display.setCursor(68, 69);
        M5.Display.print("\x1D LOOP");
    } else if (isShuffleEnabled) {
        M5.Display.fillRoundRect(64, 66, 68, 12, 2,
            M5.Display.color565(0, 0, 60));
        M5.Display.drawRoundRect(64, 66, 68, 12, 2, AP_COLOR_ACCENT1);
        M5.Display.setTextColor(AP_COLOR_ACCENT1,
            M5.Display.color565(0, 0, 60));
        M5.Display.setCursor(68, 69);
        M5.Display.print("~ SHUF");
    } else {
        M5.Display.setTextColor(0x2945, AP_COLOR_BG);
        M5.Display.setCursor(68, 69);
        M5.Display.print("= SEQ");
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateTimeDisplay()
{
    unsigned long el = isPlaying
        ? (millis() - trackStartTime - totalPausedTime) / 1000 : 0;
    M5.Display.fillRect(64, 46, 70, 12, AP_COLOR_BG);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0xFFFF, AP_COLOR_BG);
    M5.Display.setCursor(68, 47);
    M5.Display.printf("%02d:%02d", (int)(el / 60), (int)(el % 60));
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateBatteryDisplay()
{
    M5.Display.fillRect(136, 119, 60, 16, AP_COLOR_PANEL);
    M5.Display.drawFastVLine(135, 119, 16, 0x2945);

    int  lv   = M5.Power.getBatteryLevel();
    int  vol2 = M5.Power.getBatteryVoltage();
    bool ch   = (vol2 > 4200);
    uint16_t bc = (lv > 60) ? AP_COLOR_GREEN
                : (lv > 20) ? AP_COLOR_ACCENT3 : AP_COLOR_RED;

    int bx = 140, by = 121;
    M5.Display.drawRect(bx, by, 24, 11, 0x8410);
    M5.Display.fillRect(bx + 24, by + 3, 3, 5, 0x8410);
    int fi = (lv * 20) / 100; if (fi > 22) fi = 22;
    if (fi > 0) M5.Display.fillRect(bx + 2, by + 2, fi, 7, bc);
    if (ch) {
        M5.Display.setTextColor(AP_COLOR_ACCENT3, AP_COLOR_PANEL);
        M5.Display.setCursor(bx + 7, by + 1);
        M5.Display.print("~");
    }
    char bstr[6]; sprintf(bstr, "%d%%", lv);
    M5.Display.setTextColor(bc, AP_COLOR_PANEL);
    M5.Display.setCursor(169, 123);
    M5.Display.print(bstr);
}

// ═════════════════════════════════════════════════════════════
//  GEÇİCİ MESAJ
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawTempMsg()
{
    uint16_t bg = M5.Display.color565(
        ((tempMessageColor >> 11) & 0x1F),
        ((tempMessageColor >>  5) & 0x3F) >> 2,
        (tempMessageColor & 0x1F));
    M5.Display.fillRoundRect(2, 99, 132, 17, 3, bg);
    M5.Display.drawRoundRect(2, 99, 132, 17, 3, tempMessageColor);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0xFFFF, bg);
    int tx = (134 - (int)tempMessageText.length() * 6) / 2;
    if (tx < 5) tx = 5;
    M5.Display.setCursor(tx, 104);
    M5.Display.print(tempMessageText);
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::clearTempArea()
{
    M5.Display.fillRect(0, 98, 136, 20, AP_COLOR_BG);

    // Kısayol ipuçlarını yeniden çiz
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT2, AP_COLOR_BG);
    M5.Display.setCursor(4, 101); M5.Display.print("SPC");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":Play ");
    M5.Display.setTextColor(AP_COLOR_ACCENT1, AP_COLOR_BG);
    M5.Display.print("+/-");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":Vol");

    M5.Display.setTextColor(AP_COLOR_ACCENT3, AP_COLOR_BG);
    M5.Display.setCursor(4, 110); M5.Display.print("G");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":GoTo ");
    M5.Display.setTextColor(AP_COLOR_GREEN, AP_COLOR_BG);
    M5.Display.print("V");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":Viz ");
    M5.Display.setTextColor(AP_COLOR_ORANGE, AP_COLOR_BG);
    M5.Display.print("I");
    M5.Display.setTextColor(0x8410, AP_COLOR_BG);
    M5.Display.print(":Info");
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::showTempMsg(const String& msg, uint16_t col)
{
    tempMessageText  = msg;
    tempMessageColor = col;
    tempMessageTime  = millis();
    tempMessageActive = true;
    drawTempMsg();
}

// ═════════════════════════════════════════════════════════════
//  INPUT MODU
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::drawInputMode()
{
    M5.Display.fillScreen(AP_COLOR_BG);
    drawRainbowLine(0, 0.9f);
    drawRainbowLine(1, 0.5f);

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT2, AP_COLOR_BG);
    M5.Display.setCursor(68, 6);
    M5.Display.print("\x0E GO TO TRACK \x0E");
    drawRainbowLine(15, 0.6f);

    // Ana kutu
    M5.Display.fillRoundRect(20, 24, 200, 72, 5, 0x0821);
    M5.Display.drawRoundRect(20, 24, 200, 72, 5, AP_COLOR_ACCENT2);
    M5.Display.drawRoundRect(19, 23, 202, 74, 6,
        M5.Display.color565(80, 0, 80));

    M5.Display.setTextColor(AP_COLOR_ACCENT1, 0x0821);
    M5.Display.setCursor(35, 30);
    M5.Display.print("ENTER TRACK NUMBER:");

    M5.Display.fillRoundRect(35, 40, 170, 40, 4, AP_COLOR_BG);
    M5.Display.drawRoundRect(35, 40, 170, 40, 4, 0x2945);

    M5.Display.setTextSize(3);
    if (inputTrackNumber.length() > 0) {
        M5.Display.setTextColor(
            M5.Display.color565(0, 100, 150), AP_COLOR_BG);
        int tw = inputTrackNumber.length() * 18;
        M5.Display.setCursor(120 - tw / 2 + 1, 48);
        M5.Display.print(inputTrackNumber);
        M5.Display.setTextColor(AP_COLOR_ACCENT1, AP_COLOR_BG);
        M5.Display.setCursor(120 - tw / 2, 47);
        M5.Display.print(inputTrackNumber);
        M5.Display.fillRect(120 - tw / 2 + tw + 2, 47, 3, 20, AP_COLOR_ACCENT1);
    } else {
        M5.Display.setTextColor(0x1082, AP_COLOR_BG);
        M5.Display.setCursor(82, 47);
        M5.Display.print("_ _ _");
    }

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0x528A, AP_COLOR_BG);
    M5.Display.setCursor(30, 100);
    M5.Display.print("[ENTER] Confirm  [DEL] Delete  [G] Exit");
    M5.Display.setTextColor(0x2945, AP_COLOR_BG);
    M5.Display.setCursor(70, 112);
    M5.Display.printf("Library: %d tracks", totalTracks);

    drawRainbowLine(121, 0.6f);
    drawRainbowLine(122, 0.3f);
}

// ═════════════════════════════════════════════════════════════
//  INFO EKRANI
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::enterInfoScreen()
{
    isInfoScreen = true;
    M5.Display.fillScreen(AP_COLOR_BG);

    drawRainbowLine(0, 0.9f);
    drawRainbowLine(1, 0.5f);

    M5.Display.fillRoundRect(0, 3, 240, 14, 0, 0x0821);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_ACCENT3, 0x0821);
    M5.Display.setCursor(70, 6);
    M5.Display.print("[ ABOUT & KEYBINDS ]");
    drawRainbowLine(17, 0.6f);

    // Logo
    M5.Display.fillRect(0, 18, 240, 26, AP_COLOR_PANEL);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(AP_COLOR_ACCENT2, AP_COLOR_PANEL);
    M5.Display.setCursor(6, 21); M5.Display.print("MOY");
    M5.Display.setTextColor(AP_COLOR_ACCENT1, AP_COLOR_PANEL);
    M5.Display.print(" MUSIC");
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(AP_COLOR_GREEN, AP_COLOR_PANEL);
    M5.Display.setCursor(174, 21); M5.Display.print("DJ v6.0");
    M5.Display.setTextColor(0x528A, AP_COLOR_PANEL);
    M5.Display.setCursor(6, 33);
    M5.Display.print("by Andy + AI  |  M5Stack Cardputer");
    M5.Display.drawFastHLine(0, 44, 240, 0x2945);

    // Keybind tablosu
    struct KeyInfo { const char* key; const char* desc; uint16_t kc; };
    KeyInfo keys[] = {
        {"SPACE",   "Play / Pause",          AP_COLOR_GREEN},
        {"BtnA",    "1x=Pause 2x=>> 3x=<<", AP_COLOR_ACCENT1},
        {"+  / -",  "Volume Up / Down",      AP_COLOR_ACCENT3},
        {"R",       "Random Track",          AP_COLOR_ORANGE},
        {"G",       "Go To Track #",         AP_COLOR_ACCENT2},
        {"L",       "Loop Toggle",           AP_COLOR_ACCENT3},
        {"S",       "Shuffle Toggle",        AP_COLOR_ACCENT1},
        {"V",       "Visualizer",            AP_COLOR_GREEN},
        {"I / `",   "Info / Exit",           AP_COLOR_PINK},
        {",  /  /", "Prev / Next Track",     AP_COLOR_ACCENT1},
        {".  / ;",  "-10 / +10 Tracks",     AP_COLOR_ORANGE},
    };

    int ky = 47;
    for (auto& k : keys) {
        M5.Display.fillRoundRect(4, ky, 32, 8, 1, 0x0821);
        M5.Display.drawRoundRect(4, ky, 32, 8, 1, k.kc);
        M5.Display.setTextColor(k.kc, 0x0821);
        M5.Display.setCursor(6, ky + 1);
        M5.Display.print(k.key);
        M5.Display.setTextColor(0xBDF7, AP_COLOR_BG);
        M5.Display.setCursor(40, ky + 1);
        M5.Display.print(k.desc);
        ky += 10;
    }

    drawRainbowLine(133, 0.5f);
    M5.Display.setTextColor(0x4208, AP_COLOR_BG);
    M5.Display.setCursor(60, 125);
    M5.Display.print("[ Press I or ` to exit ]");
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::exitInfoScreen()
{
    isInfoScreen    = false;
    lastDiskPlaying = !isPlaying;
    drawMainScreen();
}

// ═════════════════════════════════════════════════════════════
//  TAM EKRAN GÖRSELLEŞTİRİCİ
// ═════════════════════════════════════════════════════════════

void UnitAudioPlayerController::initMatrixColumns()
{
    for (int i = 0; i < 40; i++) {
        matrixColumns[i].y           = random(-50, 0);
        matrixColumns[i].speed       = random(1, 4);
        matrixColumns[i].trailLength = random(5, 15);
        matrixColumns[i].character   = matrixChars[random(0, strlen(matrixChars))];
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawVisualizerHeader()
{
    M5.Display.fillRect(0, 0, 240, 17, AP_COLOR_PANEL);
    drawRainbowLine(0, 0.7f, rainbowOffset);
    M5.Display.fillRect(0, 1, 240, 16, AP_COLOR_PANEL);

    const char* mn = ""; uint16_t mc = AP_COLOR_ACCENT1;
    if      (visualizerMode == VIZ_SPECTRUM) { mn = "SPECTRUM BARS"; mc = AP_COLOR_ACCENT3; }
    else if (visualizerMode == VIZ_WAVE)     { mn = "WAVE FORM";     mc = AP_COLOR_GREEN;   }
    else if (visualizerMode == VIZ_MATRIX)   { mn = "MATRIX RAIN";   mc = AP_COLOR_GREEN;   }

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(mc, AP_COLOR_PANEL);
    M5.Display.setCursor(5, 5);
    M5.Display.print(mn);

    M5.Display.setTextColor(0x4208, AP_COLOR_PANEL);
    M5.Display.setCursor(120, 5);
    M5.Display.print("V:Next  ESC:Exit  +/-:Vol");
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::updateFullscreenVisualizer()
{
    if (visualizerMode == VIZ_SPECTRUM) {
        if (!isPlaying) {
            for (int i = 0; i < 20; i++) fullscreenBarHeights[i] = 0;
            return;
        }
        for (int i = 0; i < 20; i++) {
            float c = 9.5f, d = abs(i - c) / c;
            int mH = (int)(106 * (1.0f - d * 0.3f));
            fullscreenBarHeights[i] = random(mH / 3, mH + 5);
        }
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawFullscreenBars()
{
    const int bW = 11, bG = 1, maxH = 110, baseY = 127;
    for (int i = 0; i < 20; i++) {
        int x = i * (bW + bG);
        M5.Display.fillRect(x, 17, bW, maxH, AP_COLOR_BG);
        if (fullscreenBarHeights[i] > 0) {
            for (int y = 0; y < fullscreenBarHeights[i]; y++) {
                float t = (float)y / maxH;
                M5.Display.drawFastHLine(x, baseY - y, bW,
                    hsvToRgb565(t * 0.4f, 1.0f, 0.95f));
            }
            M5.Display.drawFastHLine(x,
                baseY - fullscreenBarHeights[i], bW, 0xFFFF);
        }
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawFullscreenWaves()
{
    M5.Display.fillRect(0, 17, 240, 111, AP_COLOR_BG);
    wavePhase += 0.12f;
    if (wavePhase > 6.28f) wavePhase = 0.0f;

    float amp  = isPlaying ? random(25, 50) : 6.0f;
    float amp2 = amp * 0.6f;
    int   cy   = 72;

    for (int x = 0; x < 239; x++) {
        int y1 = cy + (int)(amp  * sinf(x       * 0.04f + wavePhase));
        int y2 = cy + (int)(amp  * sinf((x + 1) * 0.04f + wavePhase));
        int y3 = cy + (int)(amp2 * sinf(x       * 0.07f - wavePhase * 1.3f));
        int y4 = cy + (int)(amp2 * sinf((x + 1) * 0.07f - wavePhase * 1.3f));
        y1 = constrain(y1, 17, 127); y2 = constrain(y2, 17, 127);
        y3 = constrain(y3, 17, 127); y4 = constrain(y4, 17, 127);

        float hue = (float)x / 240.0f;
        M5.Display.drawLine(x, y1, x + 1, y2,
            hsvToRgb565(hue, 1.0f, 0.95f));
        M5.Display.drawLine(x, y3, x + 1, y4,
            hsvToRgb565(fmod(hue + 0.5f, 1.0f), 1.0f, 0.6f));

        int mn = min(y1, cy), mx = max(y1, cy);
        if (mx > mn)
            M5.Display.drawFastVLine(x, mn, mx - mn,
                hsvToRgb565(hue, 1.0f, 0.25f));
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawMatrixRain()
{
    M5.Display.fillRect(0, 17, 240, 111, AP_COLOR_BG);
    M5.Display.setTextSize(1);
    int sm = isPlaying ? 2 : 1;

    for (int i = 0; i < 40; i++) {
        matrixColumns[i].y += matrixColumns[i].speed * sm;
        if (matrixColumns[i].y > 135 + matrixColumns[i].trailLength * 8) {
            matrixColumns[i].y           = random(-60, -5);
            matrixColumns[i].speed       = random(1, 4);
            matrixColumns[i].trailLength = random(5, 15);
            matrixColumns[i].character   = matrixChars[random(0, strlen(matrixChars))];
        }

        int x = i * 6, y = matrixColumns[i].y;
        if (y >= 17 && y < 128) {
            M5.Display.setTextColor(
                M5.Display.color565(220, 255, 220), AP_COLOR_BG);
            M5.Display.setCursor(x, y);
            M5.Display.print(matrixColumns[i].character);

            for (int j = 1; j < matrixColumns[i].trailLength; j++) {
                int ty = y - j * 8;
                if (ty >= 17 && ty < 128) {
                    int br = 200 - (j * 200 / matrixColumns[i].trailLength);
                    M5.Display.setTextColor(
                        M5.Display.color565(0, br, 0), AP_COLOR_BG);
                    M5.Display.setCursor(x, ty);
                    M5.Display.print(matrixChars[random(0, strlen(matrixChars))]);
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::drawFullscreenTempMessage(
        const String& msg, uint16_t col)
{
    tempMessageText  = msg;
    tempMessageColor = col;
    tempMessageTime  = millis();
    tempMessageActive = (msg.length() > 0);

    M5.Display.fillRect(0, 128, 240, 7, AP_COLOR_BG);
    if (msg.length() > 0) {
        int x = (240 - (int)msg.length() * 6) / 2;
        if (x < 0) x = 0;
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(col, AP_COLOR_BG);
        M5.Display.setCursor(x, 129);
        M5.Display.print(msg);
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::enterFullscreenVisualizer()
{
    isFullscreenVisualizer = true;
    visualizerMode = VIZ_SPECTRUM;
    M5.Display.fillScreen(AP_COLOR_BG);
    initMatrixColumns();
    drawVisualizerHeader();
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::switchVisualizerMode()
{
    visualizerMode++;
    if (visualizerMode > VIZ_MATRIX) {
        exitFullscreenVisualizer();
        return;
    }
    M5.Display.fillScreen(AP_COLOR_BG);
    drawVisualizerHeader();
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::exitFullscreenVisualizer()
{
    isFullscreenVisualizer = false;
    visualizerMode         = VIZ_NONE;
    lastDiskPlaying        = !isPlaying;
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
    if (isFullscreenVisualizer) drawFullscreenTempMessage(m, AP_COLOR_GREEN);
    else { drawBottomBar(); showTempMsg(m, AP_COLOR_GREEN); }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::volumeDown()
{
    if (currentVolume <= 0) return;
    currentVolume--;
    audioplayer.setVolume(currentVolume);
    char m[20]; sprintf(m, "VOL %d%%", (currentVolume * 100) / 30);
    if (isFullscreenVisualizer) drawFullscreenTempMessage(m, AP_COLOR_ACCENT3);
    else { drawBottomBar(); showTempMsg(m, AP_COLOR_ACCENT3); }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::toggleLoop()
{
    isLoopEnabled = !isLoopEnabled;
    if (isLoopEnabled && isShuffleEnabled) isShuffleEnabled = false;
    String   msg = isLoopEnabled ? "LOOP ON" : "LOOP OFF";
    uint16_t c   = isLoopEnabled ? AP_COLOR_ACCENT3 : 0x8410;
    if (isFullscreenVisualizer) drawFullscreenTempMessage(msg, c);
    else { updateModeDisplay(); showTempMsg(msg, c); }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::toggleShuffle()
{
    isShuffleEnabled = !isShuffleEnabled;
    if (isShuffleEnabled && isLoopEnabled) isLoopEnabled = false;
    String   msg = isShuffleEnabled ? "SHUFFLE ON" : "SHUFFLE OFF";
    uint16_t c   = isShuffleEnabled ? AP_COLOR_ACCENT1 : 0x8410;
    if (isFullscreenVisualizer) drawFullscreenTempMessage(msg, c);
    else { updateModeDisplay(); showTempMsg(msg, c); }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::goToTrack(int n)
{
    if (n < 1 || n > (int)totalTracks) {
        char m[25]; sprintf(m, "BAD! (1-%d)", totalTracks);
        if (isFullscreenVisualizer) drawFullscreenTempMessage(m, AP_COLOR_RED);
        else showTempMsg(m, AP_COLOR_RED);
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
    if (isFullscreenVisualizer) drawFullscreenTempMessage(m, AP_COLOR_ACCENT1);
    else { updateTrackDisplay(); updateStatusDisplay(); showTempMsg(m, AP_COLOR_ACCENT1); }
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
    if (isFullscreenVisualizer) drawFullscreenTempMessage(m, AP_COLOR_ACCENT2);
    else { updateTrackDisplay(); showTempMsg(m, AP_COLOR_ACCENT2); }
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
            if (isFullscreenVisualizer) drawFullscreenTempMessage("PAUSED", AP_COLOR_ACCENT3);
            else { updateStatusDisplay(); showTempMsg("PAUSED", AP_COLOR_ACCENT3); }
        } else {
            audioplayer.playAudio();
            if (st == AUDIO_PLAYER_STATUS_STOPPED) {
                trackStartTime  = millis();
                totalPausedTime = 0;
            } else {
                totalPausedTime += (millis() - pausedTime);
            }
            isPlaying = true;
            if (isFullscreenVisualizer) drawFullscreenTempMessage("PLAYING", AP_COLOR_GREEN);
            else { updateStatusDisplay(); showTempMsg("PLAYING", AP_COLOR_GREEN); }
        }
    }
    else if (clicks == 2) {
        audioplayer.nextAudio(); delay(100); audioplayer.playAudio();
        isPlaying = true; trackStartTime = millis(); totalPausedTime = 0;
        if (isFullscreenVisualizer) drawFullscreenTempMessage("NEXT >>", AP_COLOR_ACCENT1);
        else { updateStatusDisplay(); showTempMsg("NEXT >>", AP_COLOR_ACCENT1); }
    }
    else if (clicks == 3) {
        audioplayer.previousAudio(); delay(100); audioplayer.playAudio();
        isPlaying = true; trackStartTime = millis(); totalPausedTime = 0;
        if (isFullscreenVisualizer) drawFullscreenTempMessage("<< PREV", AP_COLOR_ACCENT1);
        else { updateStatusDisplay(); showTempMsg("<< PREV", AP_COLOR_ACCENT1); }
    }
}

// ─────────────────────────────────────────────────────────────

void UnitAudioPlayerController::checkAutoNext()
{
    static unsigned long lc = 0;
    if (millis() - lc < 500) return;
    lc = millis();

    uint8_t cs = audioplayer.checkPlayStatus();

    // ── Şarkı bitti → Otomatik geçiş ────────────────────────
    if (lastPlayStatus == AUDIO_PLAYER_STATUS_PLAYING &&
        cs == AUDIO_PLAYER_STATUS_STOPPED)
    {
        if (isLoopEnabled) {
            delay(100); audioplayer.playAudio(); delay(100);
            trackStartTime = millis(); totalPausedTime = 0; isPlaying = true;
            if (!isFullscreenVisualizer) showTempMsg("LOOP", AP_COLOR_ACCENT3);
            else drawFullscreenTempMessage("LOOP", AP_COLOR_ACCENT3);
        }
        else if (isShuffleEnabled) {
            uint16_t r = random(1, totalTracks + 1);
            audioplayer.selectAudioNum(r); delay(200);
            audioplayer.playAudio(); delay(100);
            trackStartTime = millis(); totalPausedTime = 0; isPlaying = true;
            if (!isFullscreenVisualizer) showTempMsg("SHUFFLE", AP_COLOR_ACCENT1);
            else drawFullscreenTempMessage("SHUFFLE", AP_COLOR_ACCENT1);
        }
        else {
            audioplayer.nextAudio(); delay(200);
            audioplayer.playAudio(); delay(100);
            trackStartTime = millis(); totalPausedTime = 0; isPlaying = true;
            if (!isFullscreenVisualizer) showTempMsg("AUTO NEXT", 0xFFFF);
            else drawFullscreenTempMessage("AUTO NEXT", 0xFFFF);
        }
    }

    // ── Güncel track numarasını kontrol et ──────────────────
    uint16_t rt = audioplayer.getCurrentAudioNumber();
    if (rt < 1 || rt > totalTracks) return;

    if (rt != lastDisplayedTrack) {
        currentTrack       = rt;
        lastDisplayedTrack = rt;
        trackStartTime     = millis();
        totalPausedTime    = 0;
        if (!isFullscreenVisualizer && !isInfoScreen)
            updateTrackDisplay();
    }

    // ── Play durumu değiştiyse güncelle ─────────────────────
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
        if (!isFullscreenVisualizer && !isInfoScreen)
            updateStatusDisplay();
    }
    lastPlayStatus = cs;
}