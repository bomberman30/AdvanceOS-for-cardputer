



#pragma once
#include <unit_audioplayer.hpp>
#include "./GlobalParentClass.h"

// ═════════════════════════════════════════════════════════════
//  GLOBAL PLAYBACK STATE  (menüler arası taşınan durum)
// ═════════════════════════════════════════════════════════════
struct AudioPlaybackState {
    bool     active          = false;   // modül başlatıldı mı?
    bool     isPlaying       = false;
    uint16_t currentTrack    = 1;
    uint16_t totalTracks     = 0;
    int      currentVolume   = 25;
    bool     isLoopEnabled   = false;
    bool     isShuffleEnabled= false;
    uint8_t  lastPlayStatus  = 0;      // AUDIO_PLAYER_STATUS_*

    unsigned long trackStartTime  = 0;
    unsigned long pausedTime      = 0;
    unsigned long totalPausedTime = 0;
};
// Tek global örnek – tüm Translation Unit'lerde paylaşılır
extern AudioPlaybackState g_audioState;
// Modül nesnesi de global tutulmalı ki Serial1 bağlantısı korunsun
extern AudioPlayerUnit    g_audioPlayer;
// ─── Krem Renk Paleti ─────────────────────────────────────────
#define AP_COLOR_BG       0x0000
#define AP_COLOR_CREAM1   0xF7D6   // Ana krem (açık)
#define AP_COLOR_CREAM2   0xDECB   // Orta krem
#define AP_COLOR_CREAM3   0xC5A3   // Koyu krem / sepia
#define AP_COLOR_CREAM4   0x2945   // Çok koyu kahve (panel bg)
#define AP_COLOR_CREAM5   0x18C3   // Derin kahverengi
#define AP_COLOR_AMBER    0xFD20   // Amber / turuncu vurgu
#define AP_COLOR_WARM     0xFC80   // Sıcak sarı
#define AP_COLOR_GREEN    0x67E0   // Soluk yeşil (VU yeşil)
#define AP_COLOR_ORANGE   0xFD20   // Turuncu (VU orta)
#define AP_COLOR_RED      0xF800   // Kırmızı (VU peak)
#define AP_COLOR_WHITE    0xFFFF   // Beyaz

// ─── Layout Sabitleri ─────────────────────────────────────────
#define LEFT_W    130
#define DISK_CX   184
#define DISK_CY    66
#define DISK_R     42

// ─── Ana Sınıf ────────────────────────────────────────────────
class UnitAudioPlayerController : public GlobalParentClass {

public:
    // Yaşam döngüsü
    void Begin();
    void Loop();
    void Draw();
    void handleExitRequest();
    UnitAudioPlayerController(MyOS *os) : GlobalParentClass(os) {}

private:
    // ── Çekirdek ──────────────────────────────────────────────
    AudioPlayerUnit audioplayer;
    bool            initialized = false;
    static const char matrixChars[];
    // ── Parça / Çalma Durumu ──────────────────────────────────
    uint8_t  lastPlayStatus     = AUDIO_PLAYER_STATUS_STOPPED;
    uint16_t currentTrack       = 1;
    uint16_t lastDisplayedTrack = 1;
    uint16_t totalTracks        = 0;

    unsigned long trackStartTime  = 0;
    unsigned long pausedTime      = 0;
    unsigned long totalPausedTime = 0;
    bool          isPlaying       = false;

    // ── Mod Bayrakları ────────────────────────────────────────
    bool isLoopEnabled    = false;
    bool isShuffleEnabled = false;

    // ── Ses ───────────────────────────────────────────────────
    int currentVolume = 25;

    // ── Ekran Durumu ──────────────────────────────────────────
    bool isInfoScreen = false;

    // ── Disk Animasyonu ───────────────────────────────────────
    float         diskAngle       = 0.0f;
    float         diskSpeed       = 0.0f;
    float         targetDiskSpeed = 0.0f;
    bool          lastDiskPlaying = false;
    unsigned long lastDiskUpdate  = 0;
    static const unsigned long DISK_UPDATE_MS = 50;

    // ── VU Metre ──────────────────────────────────────────────
    int           vuLeft[12]   = {0};
    int           vuRight[12]  = {0};
    unsigned long lastVuUpdate = 0;
    static const unsigned long VU_UPDATE_MS = 80;

    // ── Zamanlayıcılar ────────────────────────────────────────
    unsigned long lastTimeUpdate = 0;
    static const unsigned long TIME_UPDATE_MS = 1000;

    // ── Geçici Mesaj ──────────────────────────────────────────
    bool          tempMessageActive = false;
    unsigned long tempMessageTime   = 0;
    String        tempMessageText   = "";
    uint16_t      tempMessageColor  = 0xFFFF;
    static const unsigned long TEMP_MSG_DURATION = 2000;

    // ── Buton Çoklu Tıklama ───────────────────────────────────
    unsigned long lastClickTime = 0;
    int           clickCount    = 0;
    static const unsigned long CLICK_TIMEOUT = 500;

    // ─────────────────────────────────────────────────────────
    //  Yardımcı / Çizim Metodları (private)
    // ─────────────────────────────────────────────────────────
    bool isInputMode      = false;

    // Genel çizim araçları
    void drawCreamLine(int y, float br);

    // Ana ekran bileşenleri
    void drawMainScreen();
    void drawTopBar();
    void drawLeftPanel();
    void drawBottomBar();

    // Disk
    void drawDiskBackground();
    void drawDiskDynamic(float angle);
    void drawFullDisk(float angle);
    void updateDiskRotation();

    // VU Metre
    void updateVuMeter();
    void drawVuMeter();

    // Güncelleme (kısmi yeniden çizim)
    void updateMainDisplay();
    void updateTrackDisplay();
    void updateStatusDisplay();
    void updateModeDisplay();
    void updateTimeDisplay();

    // Geçici mesaj
    void showTempMsg(const String& msg, uint16_t col);
    void drawTempMsg();
    void clearTempArea();

    // Info ekranı
    void enterInfoScreen();
    void exitInfoScreen();

    // Kontrol / Eylemler
    void volumeUp();
    void volumeDown();
    void toggleLoop();
    void toggleShuffle();
    void goToTrack(int n);
    void randomTrack();
    void executeAction(int clicks);
    void checkAutoNext();
    String inputTrackNumber = "";

    // Klavye / Buton işleme
    void handleKeyboard();
    void handleButtonA();
        void handleKeyboardInput();
    void handleKeyboardNormal();
        void drawInputMode();
       // void drawCreamLine(int y, float br, float);
        void drawRainbowLine(int y, float brightness = 0.9f, float offset = 0.0f);
        void drawNeonText(int x, int y, const char *txt, uint16_t col, uint8_t sz = 1);
        void drawGlowRect(int x, int y, int w, int h,
                          uint16_t borderCol, uint16_t innerCol);
        void updateBatteryDisplay();

};



/* 




#pragma once
//#include <M5Cardputer.h>
#include <unit_audioplayer.hpp>
#include "./GlobalParentClass.h"
// ─── Renk Sabitleri ───────────────────────────────────────────
#define AP_COLOR_BG       0x0000
#define AP_COLOR_ACCENT1  0x07FF   // Cyan
#define AP_COLOR_ACCENT2  0xF81F   // Magenta
#define AP_COLOR_ACCENT3  0xFFE0   // Yellow
#define AP_COLOR_GREEN    0x07E0
#define AP_COLOR_ORANGE   0xFD20
#define AP_COLOR_RED      0xF800
#define AP_COLOR_PINK     0xF81F
#define AP_COLOR_DARKBG   0x0821
#define AP_COLOR_PANEL    0x0410

// ─── Disk Sabitleri ───────────────────────────────────────────
#define DISK_CX 182
#define DISK_CY  66
#define DISK_R   43

// ─── Görselleştirici Modu Sabitleri ───────────────────────────
#define VIZ_NONE     0
#define VIZ_SPECTRUM 1
#define VIZ_WAVE     2
#define VIZ_MATRIX   3

// ─── Matrix Kolonu Yapısı ─────────────────────────────────────
struct MatrixColumn {
    int    y;
    int    speed;
    int    trailLength;
    char   character;
};

// ─── Ana Sınıf ────────────────────────────────────────────────

class UnitAudioPlayerController : public GlobalParentClass{


public:
    // Yaşam döngüsü
    void Begin();
    void Loop();
    void Draw();
    UnitAudioPlayerController(MyOS *os) : GlobalParentClass(os) {}

private:
    // ── Çekirdek ──────────────────────────────────────────────
    AudioPlayerUnit audioplayer;
    bool            initialized = false;

    // ── Parça / Çalma Durumu ──────────────────────────────────
    uint8_t  lastPlayStatus     = AUDIO_PLAYER_STATUS_STOPPED;
    uint16_t currentTrack       = 1;
    uint16_t lastDisplayedTrack = 1;
    uint16_t totalTracks        = 0;

    unsigned long trackStartTime  = 0;
    unsigned long pausedTime      = 0;
    unsigned long totalPausedTime = 0;
    bool          isPlaying       = false;

    // ── Mod Bayrakları ────────────────────────────────────────
    bool isLoopEnabled    = false;
    bool isShuffleEnabled = false;
    bool isInputMode      = false;
    String inputTrackNumber = "";

    // ── Ses ───────────────────────────────────────────────────
    int currentVolume = 25;

    // ── Ekran Durumu ──────────────────────────────────────────
    bool isFullscreenVisualizer = false;
    int  visualizerMode         = VIZ_NONE;
    bool isInfoScreen           = false;

    // ── Disk Animasyonu ───────────────────────────────────────
    float         diskAngle       = 0.0f;
    float         diskSpeed       = 0.0f;
    float         targetDiskSpeed = 0.0f;
    bool          lastDiskPlaying = false;
    unsigned long lastDiskUpdate  = 0;
    static const unsigned long DISK_UPDATE_MS = 50;

    // ── VU Metre ──────────────────────────────────────────────
    int           vuLeft[12]   = {0};
    int           vuRight[12]  = {0};
    unsigned long lastVuUpdate = 0;
    static const unsigned long VU_UPDATE_MS = 80;

    // ── Rainbow / Animasyon ───────────────────────────────────
    float         rainbowOffset = 0.0f;
    float         wavePhase     = 0.0f;

    // ── Zamanlayıcılar ────────────────────────────────────────
    unsigned long lastBatteryUpdate = 0;
    unsigned long lastTimeUpdate    = 0;
    unsigned long lastRainbowUpdate = 0;
    unsigned long lastMatrixUpdate  = 0;
    static const unsigned long BATTERY_UPDATE_MS = 5000;
    static const unsigned long TIME_UPDATE_MS    = 1000;
    static const unsigned long RAINBOW_UPDATE_MS = 100;

    // ── Geçici Mesaj ──────────────────────────────────────────
    bool          tempMessageActive   = false;
    unsigned long tempMessageTime     = 0;
    String        tempMessageText     = "";
    uint16_t      tempMessageColor    = 0xFFFF;
    static const unsigned long TEMP_MSG_DURATION = 2000;

    // ── Buton Çoklu Tıklama ───────────────────────────────────
    unsigned long lastClickTime = 0;
    int           clickCount    = 0;
    static const unsigned long CLICK_TIMEOUT = 500;

    // ── Tam Ekran Görselleştirici ─────────────────────────────
    int           fullscreenBarHeights[20] = {0};
    MatrixColumn  matrixColumns[40];
    static const char matrixChars[];

    // ─────────────────────────────────────────────────────────
    //  Yardımcı / Çizim Metodları (private)
    // ─────────────────────────────────────────────────────────

    // Renk
    uint16_t hsvToRgb565(float h, float s, float v);

    // Genel çizim araçları
    void drawRainbowLine(int y, float brightness = 0.9f, float offset = 0.0f);
    void drawNeonText(int x, int y, const char* txt, uint16_t col, uint8_t sz = 1);
    void drawGlowRect(int x, int y, int w, int h,
                      uint16_t borderCol, uint16_t innerCol);

    // Splash
    void showSplash();

    // Ana ekran bileşenleri
    void drawMainScreen();
    void drawTopBar();
    void drawLeftPanel();
    void drawBottomBar();

    // Disk
    void drawDiskBackground();
    void drawDiskDynamic(float angle);
    void drawFullDisk(float angle);
    void updateDiskRotation();

    // VU Metre
    void updateVuMeter();
    void drawVuMeter();

    // Güncelleme (kısmi yeniden çizim)
    void updateMainDisplay();
    void updateTrackDisplay();
    void updateStatusDisplay();
    void updateModeDisplay();
    void updateTimeDisplay();
    void updateBatteryDisplay();

    // Geçici mesaj
    void showTempMsg(const String& msg, uint16_t col);
    void drawTempMsg();
    void clearTempArea();

    // Input modu
    void drawInputMode();

    // Info ekranı
    void enterInfoScreen();
    void exitInfoScreen();

    // Tam ekran görselleştirici
    void enterFullscreenVisualizer();
    void exitFullscreenVisualizer();
    void switchVisualizerMode();
    void drawVisualizerHeader();
    void updateFullscreenVisualizer();
    void drawFullscreenBars();
    void drawFullscreenWaves();
    void drawMatrixRain();
    void initMatrixColumns();
    void drawFullscreenTempMessage(const String& msg, uint16_t col);

    // Kontrol / Eylemler
    void volumeUp();
    void volumeDown();
    void toggleLoop();
    void toggleShuffle();
    void goToTrack(int n);
    void randomTrack();
    void executeAction(int clicks);
    void checkAutoNext();

    // Klavye / Buton işleme
    void handleKeyboard();
    void handleKeyboardVisualizer();
    void handleKeyboardInput();
    void handleKeyboardNormal();
    void handleButtonA();
}; */