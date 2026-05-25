#include "wifiSpectrum.h"
#include "MyOS.h"
#include "./Classes/MainMenuV2.h"

#define C_BG       0x0000
#define C_GRID     0x0841
#define C_TEAL     0x07F5
#define C_TEAL_DIM 0x03EA
#define C_AMBER    0xFD20
#define C_RED      0xF800

static const uint16_t CHAN_COLORS[] = {
    0,
    0xF800,0xFB40,0xFD20,0xFFE0,0x87E0,
    0x07E0,0x07F5,0x07FF,0x027F,0x001F,
    0x601F,0xC81F,0xF81F
};

static int   channelRSSI[15];
static int   totalNets   = 0;
static int   bestChannel = 1;
static int   scanCount   = 0;
static int   scanlineY   = 0;
static unsigned long lastScanlineMs = 0;
static bool  blinkState  = false;
static unsigned long lastBlinkMs = 0;

void wifiSpectrum::Begin()
{
    showTopBar=false;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    for (int i = 0; i < 15; i++) {
        channelHeights[i] = 0;
        channelRSSI[i]    = -100;
        memset(ssidNames[i], 0, 33);
    }
    performScan();
}

void wifiSpectrum::performScan()
{
    WiFi.scanNetworks(true);
}

void wifiSpectrum::Loop()
{
    int n = WiFi.scanComplete();

    if (n >= 0) {
        for (int i = 0; i < 15; i++) {
            channelHeights[i] = 0;
            channelRSSI[i]    = -100;
            memset(ssidNames[i], 0, 33);
        }

        totalNets = n;
        int chanLoad[14] = {0};

        for (int i = 0; i < n; i++) {
            int ch   = WiFi.channel(i);
            int rssi = WiFi.RSSI(i);
            int val  = map(rssi, -100, -30, 0, 80);
            val      = constrain(val, 0, 80);

            if (ch >= 1 && ch <= 13) {
                chanLoad[ch]++;
                if (val > channelHeights[ch]) {
                    channelHeights[ch] = val;
                    channelRSSI[ch]    = rssi;
                    String name = WiFi.SSID(i);
                    name.substring(0, 10).toCharArray(ssidNames[ch], 11);
                }
            }
        }

        // Best channel from 1, 6, 11
        int preferred[] = {1, 6, 11};
        bestChannel = 1;
        int minLoad = 999;
        for (int p : preferred) {
            if (chanLoad[p] < minLoad) {
                minLoad = chanLoad[p];
                bestChannel = p;
            }
        }

        WiFi.scanDelete();
        WiFi.scanNetworks(true);
        scanCount++;
    }

    if (mainOS->NewKey.ifKeyJustPress('`')) {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
    }
if (DrawTime>500)
{
    DrawTime=0;
     Draw();
}
   
}

void wifiSpectrum::Draw()
{
    //mainOS->sprite.createSprite(240, 135);
    M5Cardputer.Display.fillScreen(C_BG);

    // ── Scanline ──────────────────────────────────────────
    unsigned long now = millis();
    if (now - lastScanlineMs > 20) {
        scanlineY = (scanlineY + 10) % 135;
        lastScanlineMs = now;
    }
    for (int x = 0; x < 240; x++)
        M5Cardputer.Display.drawPixel(x, scanlineY, C_TEAL_DIM);

    // ── HUD corners ───────────────────────────────────────
    M5Cardputer.Display.drawFastHLine(0, 0, 7, C_TEAL);
    M5Cardputer.Display.drawFastVLine(0, 0, 7, C_TEAL);
    M5Cardputer.Display.drawFastHLine(233, 0, 7, C_TEAL);
    M5Cardputer.Display.drawFastVLine(239, 0, 7, C_TEAL);
    M5Cardputer.Display.drawFastHLine(0, 134, 7, C_TEAL);
    M5Cardputer.Display.drawFastVLine(0, 128, 7, C_TEAL);
    M5Cardputer.Display.drawFastHLine(233, 134, 7, C_TEAL);
    M5Cardputer.Display.drawFastVLine(239, 128, 7, C_TEAL);

    // ── Header ────────────────────────────────────────────
    if (now - lastBlinkMs > 500) {
        blinkState = !blinkState;
        lastBlinkMs = now;
    }
    M5Cardputer.Display.fillCircle(5, 6, 2, blinkState ? C_TEAL : C_TEAL_DIM);
    M5Cardputer.Display.setTextColor(C_TEAL);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(10, 2);
    M5Cardputer.Display.print("RF SPECTRUM 2.4GHz");
    M5Cardputer.Display.setTextColor(C_TEAL_DIM);
    M5Cardputer.Display.setCursor(190, 2);
    char buf[8];
    snprintf(buf, sizeof(buf), "#%04d", scanCount);
    M5Cardputer.Display.print(buf);
    M5Cardputer.Display.drawFastHLine(0, 12, 240, C_GRID);

    // ── Grid lines ────────────────────────────────────────
    int baselineY = 105;
    for (int step = 1; step <= 3; step++)
        M5Cardputer.Display.drawFastHLine(2, baselineY - step * 25, 178, C_GRID);
    M5Cardputer.Display.drawFastHLine(2, baselineY, 178, C_TEAL_DIM);

    // ── Spectrum bars ─────────────────────────────────────
    for (int ch = 1; ch <= 13; ch++) {
        int x = 4 + (ch - 1) * 14;
        int h = channelHeights[ch];
        uint16_t col = CHAN_COLORS[ch];

        // channel number
        M5Cardputer.Display.setTextColor(h > 0 ? col : C_TEAL_DIM);
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setCursor(x - 2, baselineY + 3);
        M5Cardputer.Display.printf("%d", ch);

        if (h == 0) continue;

        // filled triangle (bell body)
        M5Cardputer.Display.fillTriangle(
            x - 13, baselineY,
            x + 13, baselineY,
            x,      baselineY - h,
            col);
        // dark outline
        M5Cardputer.Display.drawTriangle(
            x - 13, baselineY,
            x + 13, baselineY,
            x,      baselineY - h,
            C_BG);
        // bright tip
        M5Cardputer.Display.drawPixel(x, baselineY - h,     C_BG + 0xFFFF);
        M5Cardputer.Display.drawPixel(x, baselineY - h + 1, C_BG + 0xFFFF);

        // SSID label
        if (ssidNames[ch][0] != '\0') {
            int labelY = baselineY - h - 9;
            if (labelY < 14) labelY = 14;
            M5Cardputer.Display.setTextColor(col);
            M5Cardputer.Display.setCursor(x - 10, labelY);
            M5Cardputer.Display.print(ssidNames[ch]);
        }
    }

    // ── Sidebar divider ───────────────────────────────────
    M5Cardputer.Display.drawFastVLine(182, 0, 135, C_GRID);

    // ── Sidebar content ───────────────────────────────────
    M5Cardputer.Display.setTextColor(C_TEAL_DIM);
    M5Cardputer.Display.setCursor(184, 2);
    M5Cardputer.Display.print("NETS");
    M5Cardputer.Display.drawFastHLine(182, 12, 58, C_GRID);

    int listed = 0;
    for (int ch = 1; ch <= 13 && listed < 5; ch++) {
        if (channelHeights[ch] == 0) continue;
        int y = 15 + listed * 17;
        uint16_t col = CHAN_COLORS[ch];
        // SSID
        M5Cardputer.Display.setTextColor(col);
        M5Cardputer.Display.setCursor(184, y);
        M5Cardputer.Display.print(ssidNames[ch]);
        // mini signal bar
        int barW = map(channelRSSI[ch], -100, -30, 0, 52);
        barW = constrain(barW, 1, 52);
        M5Cardputer.Display.fillRect(184, y + 8, barW, 2, col);
        M5Cardputer.Display.fillRect(184 + barW, y + 8, 52 - barW, 2, C_GRID);
        listed++;
    }

    // Best channel box
    M5Cardputer.Display.drawRect(183, 101, 56, 14, C_TEAL_DIM);
    M5Cardputer.Display.setTextColor(C_TEAL);
    M5Cardputer.Display.setCursor(185, 104);
    M5Cardputer.Display.print("BEST:");
    M5Cardputer.Display.setTextColor(C_AMBER);
    M5Cardputer.Display.printf("CH%d", bestChannel);

    // ── Footer ────────────────────────────────────────────
    M5Cardputer.Display.drawFastHLine(0, 125, 240, C_GRID);

    // congestion segments
    M5Cardputer.Display.setTextColor(C_TEAL_DIM);
    M5Cardputer.Display.setCursor(2, 127);
    M5Cardputer.Display.print("LOAD");
    int fill = constrain(totalNets * 8 / 12, 0, 8);
    for (int i = 0; i < 8; i++) {
        bool on = (i < fill);
        uint16_t sc = (i >= 6) ? C_RED : (i >= 4) ? C_AMBER : C_TEAL;
        M5Cardputer.Display.fillRect(24 + i * 7, 126, 5, 8, on ? sc : C_GRID);
    }

    M5Cardputer.Display.setTextColor(C_TEAL);
    M5Cardputer.Display.setCursor(90, 127);
    M5Cardputer.Display.printf("N:%d", totalNets);

    M5Cardputer.Display.setTextColor(C_AMBER);
    M5Cardputer.Display.setCursor(115, 127);
    M5Cardputer.Display.printf("BEST:CH%d", bestChannel);

 /*    M5Cardputer.Display.setTextColor(C_TEAL_DIM);
    M5Cardputer.Display.setCursor(195, 127);
    M5Cardputer.Display.print("[`]BCK"); */


}

void wifiSpectrum::OnExit()
{
    WiFi.mode(WIFI_OFF);
    mainOS->WifiConnected = false;
}