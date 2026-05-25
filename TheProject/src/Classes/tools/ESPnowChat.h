#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <esp_now.h>
#include <WiFi.h>

#define ESPNOW_MAX_MSG 200

class ESPnowChat : public GlobalParentClass
{
public:
    ESPnowChat(MyOS *mainOS);

    void Begin() override;
    void Loop() override;
    void Draw() override;

private:
    static void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len);

    void sendMessage(const String &msg);
    void drawInputBar();
    void addMessage(const String &msg, bool sent);

    static String receivedMsg;
    String inputBuffer;
    unsigned long cursorTimer = 0;
    bool cursorState = true;
};