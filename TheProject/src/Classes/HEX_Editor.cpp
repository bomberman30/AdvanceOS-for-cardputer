#include "HEX_Editor.h"
#include <SD.h>
#include <M5Unified.h>
#include "MyOS.h"
#include "FileBrowser.h"

void HEX_Editor::Begin()
{
    loadFile(mainOS->FileSelectedInFS.c_str());
}

void HEX_Editor::Loop()
{
    if(mainOS->screenOff)
    {
        return;
    }
    handleInput();
    Draw();
}

void HEX_Editor::loadFile(const char *path)
{
    File f = SD.open(path, FILE_READ);
    if (!f)
        return;

    data.clear();
    while (f.available())
    {
        data.push_back(f.read());
    }
    f.close();
}

void HEX_Editor::Draw()
{
    mainOS->sprite.createSprite(240, 135 - TopOffset);
    mainOS->sprite.fillSprite(WHITE);
    mainOS->sprite.setTextColor(BLACK);
    mainOS->sprite.setCursor(0, 0);
    mainOS->sprite.setTextSize(1);

    const int bytesPerLine = 7;
    size_t start = (cursor / bytesPerLine) * bytesPerLine;

    for (int row = 0; row < 14; row++)
    {
        size_t offset = start + row * bytesPerLine;
        if (offset >= data.size())
            break;

        // offset
        mainOS->sprite.printf("%06X: ", offset);

        // HEX
        for (int i = 0; i < bytesPerLine; i++)
        {
            if (offset + i < data.size())
                mainOS->sprite.printf("%02X ", data[offset + i]);
            else
                mainOS->sprite.print("   ");
        }

        // ASCII
        mainOS->sprite.print(" |");
        for (int i = 0; i < bytesPerLine; i++)
        {
            if (offset + i < data.size())
            {
                char c = data[offset + i];
                mainOS->sprite.print((c >= 32 && c <= 126) ? c : '.');
            }
        }
        mainOS->sprite.println("|");
    }
    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}


void HEX_Editor::handleInput()
{
    M5.update();

    const int bytesPerLine = 16;


   if (mainOS->NewKey.ifKeyJustPress('`')) {
               mainOS->ChangeMenu(new FileBrowser(mainOS));

    }
    
    if (mainOS->NewKey.ifKeyJustPress('/')) {
        if (cursor + 1 < data.size())
            cursor++;
    }

    if (mainOS->NewKey.ifKeyJustPress(',')) {
        if (cursor > 0)
            cursor--;
    }

    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('.',700,50)) {
        if (cursor + bytesPerLine < data.size())
            cursor += bytesPerLine;
    }

    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(';',700,50)) {
        if (cursor >= bytesPerLine)
            cursor -= bytesPerLine;
    }

    if (mainOS->NewKey.ifKeyJustPress(KEY_LEFT_SHIFT)) {
        cursor = (cursor + 256 < data.size()) ? cursor + 256 : data.size() - 1;
    }
}