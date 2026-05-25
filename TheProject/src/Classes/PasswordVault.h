#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <ArduinoJson.h>












class PasswordVault : public GlobalParentClass
{
public:
String Password;
int selectedIndex = 0;
JsonDocument doc;
    PasswordVault(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void encryptDecrypt(uint8_t *input, size_t inputLen, uint8_t *output, String key, bool encrypt);

    void SaveVault(String userPassword);

    bool LoadVault(String userPassword);

    void Loop() override;
    void Draw() override;
String AskForPassword();
void CreateEntry();

void DeleteEntry();

private:
};
