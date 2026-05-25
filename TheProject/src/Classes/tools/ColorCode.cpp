#include "ColorCode.h"
#include "MyOS.h"
#include "./Classes/MainMenuV2.h"
void ColorPicker::Begin()
{
    r = 255;
    g = 255;
    b = 255;
  //  a = 255;
}

void ColorPicker::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
        // delay(500);
    }
    HandleInput();
    if (!mainOS->screenOff)
    {
        Draw();
    }
}

void ColorPicker::HandleInput()
{

    if (mainOS->NewKey.ifKeyJustPress('.'))
    {
        selectedBar += 1;
    }
    if (mainOS->NewKey.ifKeyJustPress(';'))
    {
        selectedBar -= 1;
    }
    if (selectedBar < 0)
    {
        selectedBar = 0;
    }
    if (selectedBar > 2)
    {
        selectedBar = 2;
    }
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('/', 500, 1))
    {
        uint8_t *v[] = {&r, &g, &b, &a};
        uint8_t val = *v[selectedBar];
        (val) += 2;
        if (val > 255)
        {
            val = 255;
        }
        *v[selectedBar] = val;
    }

    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(',', 500, 1))
    {
        uint8_t *v[] = {&r, &g, &b, &a};
        uint8_t val = *v[selectedBar];
        (val) -= 2;
        if (val < 0)
        {
            val = 0;
        }
        *v[selectedBar] = val;
    }
}

uint32_t ColorPicker::GetColor() const
{
    return M5Cardputer.Display.color888(r, g, b);
}

String ColorPicker::GetHexString() const
{
    char buf[10];
    sprintf(buf, "#%02X%02X%02X", r, g, b);
    return String(buf);
}

void ColorPicker::Draw()
{
    auto gfx = &mainOS->sprite;

    gfx->createSprite(240, 135 - TopOffset);
    gfx->setTextSize(1);
    gfx->fillSprite(GetColor());

    const char *names[4] = {"R", "G", "B", "A"};
    uint8_t values[4] = {r, g, b, a};
    uint16_t colors[4] = {RED, GREEN, BLUE, WHITE};

    for (int i = 0; i < 3; i++)
    {
        int y = 5 + i * 15;

        gfx->setTextColor(i == selectedBar ? YELLOW : WHITE, BLACK);
        gfx->setCursor(5, y);
        gfx->printf("%s: %3d", names[i], values[i]);

        gfx->fillRect(45, y + 5, values[i], 10, colors[i]);
        gfx->drawRect(45, y + 5, values[i], 10, BLACK);
    }

    gfx->fillRect(10, 150, 100, 40, GetColor());
    gfx->drawRect(10, 150, 100, 40, WHITE);

    gfx->setCursor(10, 75);
    gfx->setTextColor(WHITE, BLACK);
    gfx->print("Hex: " + GetHexString());
    gfx->setCursor(10, 90);

    gfx->print("Int: " + String(GetColor()));

    gfx->pushSprite(0, 0);
    gfx->deleteSprite();
}
