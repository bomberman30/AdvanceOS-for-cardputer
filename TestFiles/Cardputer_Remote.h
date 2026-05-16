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
#define IR_SEND_PIN 2
#define COLOR_BG      0x0000
#define COLOR_BAR     0x1082
#define COLOR_GREEN   0x07E0
#define COLOR_WHITE   0xFFFF
#define COLOR_GRAY    0x8410
#define COLOR_YELLOW  0xFFE0
#define COLOR_RED     0xF800
#define COLOR_DARK    0x18C3

#define TOP_BAR_H    18
#define SCREEN_W     240
#define SCREEN_H     135
#define SIGNAL_BOX_Y (TOP_BAR_H + 2)
#define SIGNAL_BOX_H 44
#define LIST_BOX_Y   (SIGNAL_BOX_Y + SIGNAL_BOX_H + 6)
#define LIST_BOX_H   (SCREEN_H - LIST_BOX_Y - 3)
#define LIST_START_Y (LIST_BOX_Y + 14)
#define LIST_ROW_H   13
#define LIST_VISIBLE ((LIST_BOX_H - 16) / LIST_ROW_H)

struct IRFile {
  String name;
  String path;
};
class Cardputer_Remote : public GlobalParentClass
{
public:
    Cardputer_Remote(MyOS *os)
        : GlobalParentClass(os),
          irrecv(IR_RECV_PIN),
          irsend(IR_SEND_PIN)
    {
    }
    void Begin() override;

    void Loop() override;
    void Draw() override;
    void OnExit() override;
decode_results results;

private:
IRrecv irrecv;
IRsend irsend;

bool keyPressed(const Keyboard_Class::KeysState &ks, char c);
// M5Cardputer'da OK/Enter tuşu tespiti
// Klavyede ortadaki büyük tuş: key code 0x0D veya özel OK tuşu
bool enterPressed(const Keyboard_Class::KeysState &ks);

bool mountSD();

void scanIRFiles();

bool saveIRSignalNamed(const String &hex, const String &protocol,
                       uint64_t value, const String &customName);




// ─── IR ──────────────────────────────────────────────────────────────────────

String decodeToHex(decode_results *res);

bool sendIRFile(const String &path);

// ─── Draw ────────────────────────────────────────────────────────────────────

void drawTopBar();
void drawStaticFrames();

void redrawSignalContent();

void redrawListContent();

void fullRedraw();

void drawSendFlash();

void drawSaveFlash(bool ok);
// ─── Naming screen ───────────────────────────────────────────────────────────

void drawNamingScreenFull();

void updateNamingInputBox();

// ─── Setup ───────────────────────────────────────────────────────────────────


































IRFile   irFiles[50];
int      fileCount         = 0;
int      selectedFile      = 0;
int      scrollOffset      = 0;
String   lastHex           = "";
String   lastProtocol      = "";
uint64_t lastValue         = 0;
bool     newSignalReceived = false;
bool     sdMounted         = false;

unsigned long lastNewSignalTime = 0;

String pendingSaveName = "";
bool   namingMode      = false;

unsigned long lastBlinkTime = 0;
bool          blinkState    = false;
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
