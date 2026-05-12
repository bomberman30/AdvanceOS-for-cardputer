#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <vector>

#define IR_RECV_PIN 1
#define IR_SEND_PIN 44

#define COLOR_BG TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_SUCCESS 0x00FF66
#define COLOR_WARNING 0xFFCC00
#define COLOR_ACCENT 0xFF4422
#define ITEM_H 28
#define MENU_START_Y 28
enum AppState
{
    STATE_MAIN_MENU,
    STATE_CAPTURE,
    STATE_SAVE_PROMPT,
    STATE_CAPTURE_SAVE,
    STATE_LIBRARY,
    STATE_VIEW_CODE,
    STATE_SD_BROWSER,
    STATE_SEND_CONFIRM
};

struct IRSignal
{
    String name;
    decode_type_t protocol;
    uint64_t value;
    uint16_t bits;
    uint32_t frequency;
    std::vector<uint16_t> rawData;
    bool isRaw;
    String filename;
};

class Cardputer_Remote : public GlobalParentClass
{
public:
    Cardputer_Remote(MyOS *os)
        : GlobalParentClass(os),
          irRecv(IR_RECV_PIN),
          irSend(IR_SEND_PIN)
    {
    }
    void Begin() override;

    void Loop() override;
    void Draw() override;
    void OnExit() override;

private:
      IRrecv irRecv;
    IRsend irSend;
    decode_results irResults;
    AppState currentState = STATE_MAIN_MENU;
    AppState previousState = STATE_MAIN_MENU;
    IRSignal capturedSignal;
    IRSignal selectedSignal;
    std::vector<IRSignal> library;
    std::vector<String> sdFiles;

    int menuIndex = 0;
    int libraryIndex = 0;
    int sdFileIndex = 0;
    int scrollOffset = 0;
    bool capturing = false;
    bool signalCaptured = false;
    String inputText = "";
    String statusMsg = "";
    unsigned long statusTime = 0;
    unsigned long lastCapture = 0;
    unsigned long animTimer = 0;
    bool needsRedraw = true;

    const char *mainMenuItems[3] = {
        "Capture IR Signal",
        "My Library",
        "SD Card Browser"};
    const uint32_t menuColors[3] = {
        0xFF4422,
        0x0088FF,
        0x00CC44};
    const int MAIN_MENU_COUNT = 3;

    String myU64ToStr(uint64_t val);

    String myU64ToHex(uint64_t val);

    String protoName(decode_type_t p);
    void setStatus(String msg);
    bool initSD();
    bool saveSignalToSD(IRSignal &sig);
    bool loadSignalFromSD(String filename, IRSignal &sig);
    void scanSDFiles();
    void loadLibrary();
    bool deleteFromSD(String fn);
    bool sendIR(IRSignal &sig);
    void drawTopBar(String title);
    void drawToast();
    void drawBottomBar(String hint);
    void drawMainMenu();
    void drawCaptureScreen();
    void drawSavePrompt();
    void drawSaveDialog();
    void drawLibraryScreen();
    void drawViewCodeScreen();
    void drawSDBrowserScreen();
    void drawSendConfirm(bool sending);
    void drawScreen();
    void handleIRReceive();
    void handleMainMenu(char key);
    void handleCapture(char key);
    void handleSavePrompt(char key);
    void handleSaveDialog(char key);
    void handleLibrary(char key);
    void handleViewCode(char key);
    void handleSDBrowser(char key);
    void handleSendConfirm(char key);
    bool sdAvailable = true;
};

/* M5Cardputer.Keyboard.isKeyPressed('/');//right
M5Cardputer.Keyboard.isKeyPressed(',');//left
M5Cardputer.Keyboard.isKeyPressed(';');//up
M5Cardputer.Keyboard.isKeyPressed('.');//down
M5Cardputer.Keyboard.isKeyPressed('p');//P button

M5Cardputer.Display.fillRoundRect()// fil round rect

M5Cardputer screen size is 240x135
example for Draw

    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H);
    mainOS->sprite.pushSprite(0, 0);
    // draw login here
    mainOS->sprite.deleteSprite();

*/
