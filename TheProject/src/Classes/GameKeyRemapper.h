#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <ArduinoJson.h>

// ─── Screen dimensions (adjust if your project defines them elsewhere) ────────
#ifndef SCREEN_W
#define SCREEN_W 240
#endif
#ifndef SCREEN_H
#define SCREEN_H 135
#endif

// ─── Key struct ───────────────────────────────────────────────────────────────
struct GameKeyMapping
{
    char A      = 'z';
    char B      = 'x';
    char Start  = KEY_ENTER;      // KEY_ENTER from M5Cardputer
    char Select = KEY_BACKSPACE;  // KEY_BACKSPACE from M5Cardputer
    char Up     = ';';
    char Down   = '.';
    char Right  = '/';
    char Left   = ',';
};

// ─── Class ────────────────────────────────────────────────────────────────────
class GameKeyRemapper : public GlobalParentClass
{
public:
    GameKeyRemapper(MyOS *os) : GlobalParentClass(os) {}

    void Begin()   override;
    void Loop()    override;
    void Draw()    override;
    void OnExit()  override;

    // Call this from anywhere to get the current mapping
    const GameKeyMapping &GetMapping() const { return keys; }

private:
    GameKeyMapping keys;
    int  selectedIndex = 0;   // which row the cursor is on (0-7)
    bool editingMode   = false;
    bool dirty         = false;
int cameraY = 0;

void UpdateCamera();

void LoadConfig();
void SaveConfig();

// Blocks until one key is pressed; returns the char (0 = cancelled)
};