#include "ESPnowChat.h"
#include "../../MyOS.h"
String ESPnowChat::receivedMsg = "";

ESPnowChat::ESPnowChat(MyOS *mainOS)
    : GlobalParentClass(mainOS)
{
}

void ESPnowChat::Begin()
{
    auto gfx = &mainOS->sprite; // Get reference to Cardputer's display object
    gfx->createSprite(SCREEN_W, SCREEN_H - TopOffset);
    gfx->fillSprite(TFT_BLACK);
    gfx->setTextSize(1);
    gfx->setCursor(0, 18);
    gfx->println("ESP-NOW Chat ready...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK)
    {
        M5Cardputer.Display.println("ESP-NOW init failed");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);

    // broadcast peer
    esp_now_peer_info_t peer{};
    memset(peer.peer_addr, 0xFF, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
    gfx->pushSprite(0, TopOffset);
}

void ESPnowChat::Loop()
{
    M5Cardputer.update();

    // keyboard input
    if (M5Cardputer.Keyboard.isChange())
    {
        if (M5Cardputer.Keyboard.isPressed())
        {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            for (auto c : status.word)
            {
                if (inputBuffer.length() < ESPNOW_MAX_MSG)
                {
                    inputBuffer += c;
                    drawInputBar();
                }
            }

            // BACKSPACE
            if (status.del)
            {
                if (!inputBuffer.isEmpty())
                {
                    inputBuffer.remove(inputBuffer.length() - 1);
                    drawInputBar();
                }
            }

            // ENTER
            if (status.enter)
            {
                sendMessage(inputBuffer);
                inputBuffer = "";
                drawInputBar();
            }

            // SPACE
            if (status.space)
            {
                if (inputBuffer.length() < ESPNOW_MAX_MSG)
                {
                    inputBuffer += ' ';
                    drawInputBar();
                }
            }
        }
    }
    // received message
    if (receivedMsg.length())
    {
        addMessage(receivedMsg, false);
        receivedMsg = "";
    }

    // cursor blink
    if (millis() - cursorTimer > 500)
    {
        cursorTimer = millis();
        cursorState = !cursorState;
        drawInputBar();
    }
}

void ESPnowChat::Draw()
{
   
}

void ESPnowChat::sendMessage(const String &msg)
{
    if (msg.isEmpty())
        return;

    esp_now_send(nullptr, (uint8_t *)msg.c_str(), msg.length() + 1);
    addMessage(msg, true);
}

void ESPnowChat::addMessage(const String &msg, bool sent)
{
    M5Cardputer.Display.setTextColor(sent ? TFT_GREEN : TFT_CYAN, TFT_BLACK);
    M5Cardputer.Display.print(sent ? "<< " : ">> ");
    M5Cardputer.Display.println(msg);
}

void ESPnowChat::drawInputBar()
{
    auto gfx = &mainOS->sprite; // Get reference to Cardputer's display object
    gfx->createSprite(SCREEN_W, SCREEN_H - TopOffset);
    gfx->fillRect(0, 0, 240, 16, TFT_DARKGREY);
    gfx->setCursor(4, 4);
    gfx->setTextColor(TFT_WHITE, TFT_DARKGREY);

    String line = ">>> " + inputBuffer;
    line += cursorState ? "_" : " ";
    gfx->print(line);
    gfx->pushSprite(0, TopOffset);
}

void ESPnowChat::OnDataRecv(const uint8_t *mac, const uint8_t *data, int len)
{
    receivedMsg = String((char *)data);
}
