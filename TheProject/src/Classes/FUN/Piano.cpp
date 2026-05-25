#include "Piano.h"
#include "MyOS.h"
#include "../MainMenuV2.h"
void Piano::Begin() {
    // Optional: Set speaker volume
    M5Cardputer.Speaker.setVolume(128);
    showTopBar=false;
}

void Piano::Loop() {
    
    for (int i = 0; i < 19; i++) {
        if (M5Cardputer.Keyboard.isKeyPressed(keys[i].keyChar)) {
            // Play sound only on initial press to avoid stuttering
            if (!keys[i].isPressed) {
                M5Cardputer.Speaker.tone(keys[i].noteFreq, 50);
            }
            keys[i].isPressed = true;
        } else {
            keys[i].isPressed = false;
        }
    }
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
    }

    Draw();
}

void Piano::Draw() {
    // 240x135 is the Cardputer resolution
    mainOS->sprite.createSprite(240, 135);
    mainOS->sprite.fillScreen(TFT_BLACK);

    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextColor(TFT_DARKGREY);
    mainOS->sprite.drawString("Cardputer Synth: Q to L", 5, 5);

    for (int i = 0; i < 19; i++) {
        // Calculate X position
        int xPos = startX + (i * (keyWidth + 1));
        
        // Color changes to Green when pressed
        uint16_t color = keys[i].isPressed ? TFT_GREEN : TFT_WHITE;

        // 1. Draw the white key body
        mainOS->sprite.fillRoundRect(xPos, 20, keyWidth, keyHeight, 2, color);
        
        // 2. Draw the black outline (thin)
        mainOS->sprite.drawRoundRect(xPos, 20, keyWidth, keyHeight, 2, TFT_BLACK);

        // 3. Draw the key label at the bottom
        mainOS->sprite.setTextColor(TFT_BLACK);
        mainOS->sprite.drawChar(keys[i].keyChar, xPos + 2, 105);
    }

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}