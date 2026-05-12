#include "Cardputer_Remote.h"
#include "MyOS.h"
#include "MainMenuV2.h"
void Cardputer_Remote::Begin()
{
    showTopBar = false;

    // mainOS->sprite.setColorDepth(16);
    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H);

    irRecv.enableIRIn();
    irSend.begin();

    initSD();
    if (sdAvailable)
        loadLibrary();

    animTimer = millis();
    drawScreen();
}

void Cardputer_Remote::Loop()
{

    if (mainOS->NewKey.ifKeyJustPress('`'))
    {

        if (currentState == STATE_MAIN_MENU)
        {
            mainOS->ChangeMenu(new MainMenuV2(mainOS));
        }
    }
        handleIRReceive();

    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed())
    {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        char key = 0;

        if (!status.word.empty())
            key = status.word[0];

        if (!status.hid_keys.empty())
        {
            switch (status.hid_keys[0])
            {
            case 0x28:
                key = '\n';
                break; // Enter
            case 0x2A:
                key = 127;
                break; // Backspace
            case 0x29:
                key = '`';
                break; // ESC
            default:
                break;
            }
        }

        if (key != 0)
        {
            switch (currentState)
            {
            case STATE_MAIN_MENU:
                handleMainMenu(key);
                break;
            case STATE_CAPTURE:
                handleCapture(key);
                break;
            case STATE_SAVE_PROMPT:
                handleSavePrompt(key);
                break;
            case STATE_CAPTURE_SAVE:
                handleSaveDialog(key);
                break;
            case STATE_LIBRARY:
                handleLibrary(key);
                break;
            case STATE_VIEW_CODE:
                handleViewCode(key);
                break;
            case STATE_SD_BROWSER:
                handleSDBrowser(key);
                break;
            case STATE_SEND_CONFIRM:
                handleSendConfirm(key);
                break;
            }
            needsRedraw = true;
        }
    }

    // Sürekli yeniden çizim gerektiren durumlar
    if (currentState == STATE_MAIN_MENU ||
        (currentState == STATE_CAPTURE && !signalCaptured) ||
        currentState == STATE_CAPTURE_SAVE ||
        currentState == STATE_SEND_CONFIRM)
        needsRedraw = true;

    if (statusMsg.length() > 0 && millis() - statusTime < 2500)
        needsRedraw = true;

    if (needsRedraw)
    {
        drawScreen();
        needsRedraw = false;
    }
    delay(16);
}

void Cardputer_Remote::Draw()
{
}

void Cardputer_Remote::OnExit()
{
}

String Cardputer_Remote::myU64ToStr(uint64_t val)
{

    if (val == 0)
        return "0";
    String r = "";
    uint64_t t = val;
    while (t > 0)
    {
        r = String((char)('0' + (t % 10))) + r;
        t /= 10;
    }
    return r;
}

String Cardputer_Remote::myU64ToHex(uint64_t val)
{

    if (val == 0)
        return "0x00000000";
    String h = "";
    uint64_t t = val;
    while (t > 0)
    {
        int n = t & 0xF;
        h = String((char)(n < 10 ? '0' + n : 'A' + n - 10)) + h;
        t >>= 4;
    }
    while (h.length() < 8)
        h = "0" + h;
    return "0x" + h;
}

String Cardputer_Remote::protoName(decode_type_t p)
{

    switch (p)
    {
    case NEC:
        return "NEC";
    case SONY:
        return "SONY";
    case RC5:
        return "RC5";
    case RC6:
        return "RC6";
    case SAMSUNG:
        return "SAMSUNG";
    case LG:
        return "LG";
    case PANASONIC:
        return "PANASONIC";
    case JVC:
        return "JVC";
    case SHARP:
        return "SHARP";
    case DENON:
        return "DENON";
    case PIONEER:
        return "PIONEER";
    case MITSUBISHI:
        return "MITSUBISHI";
    default:
        return "RAW";
    }
}

void Cardputer_Remote::setStatus(String msg)
{
    statusMsg = msg;
    statusTime = millis();
}

bool Cardputer_Remote::initSD()
{
    sdAvailable = mainOS->haveSDcard;
    if (!SD.exists("/IR_Signals"))
        SD.mkdir("/IR_Signals");
    return sdAvailable;
}

bool Cardputer_Remote::saveSignalToSD(IRSignal &sig)
{
    if (!sdAvailable)
        return false;
    // .ir uzantısı kullan
    String fn = "/IR_Signals/" + sig.name + ".ir";
    fn.replace(" ", "_");
    sig.filename = fn;
    if (SD.exists(fn))
        SD.remove(fn);
    File f = SD.open(fn, FILE_WRITE);
    if (!f)
        return false;
    DynamicJsonDocument doc(4096);
    doc["name"] = sig.name;
    doc["protocol"] = (int)sig.protocol;
    doc["protocol_name"] = protoName(sig.protocol);
    doc["value"] = myU64ToStr(sig.value);
    doc["bits"] = sig.bits;
    doc["frequency"] = sig.frequency;
    doc["is_raw"] = sig.isRaw;
    JsonArray ra = doc.createNestedArray("raw_data");
    for (auto &v : sig.rawData)
        ra.add(v);
    serializeJsonPretty(doc, f);
    f.close();
    return true;
}

bool Cardputer_Remote::loadSignalFromSD(String filename, IRSignal &sig)
{
    File f = SD.open(filename, FILE_READ);
    if (!f)
        return false;
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, f))
    {
        f.close();
        return false;
    }
    f.close();
    sig.name = doc["name"].as<String>();
    sig.protocol = (decode_type_t)(int)doc["protocol"];
    sig.bits = doc["bits"];
    sig.frequency = doc["frequency"] | 38000;
    sig.isRaw = doc["is_raw"] | false;
    sig.filename = filename;
    String vs = doc["value"].as<String>();
    sig.value = 0;
    for (char c : vs)
        if (c >= '0' && c <= '9')
            sig.value = sig.value * 10 + (c - '0');
    sig.rawData.clear();
    if (doc.containsKey("raw_data"))
    {
        JsonArray ra = doc["raw_data"];
        for (auto v : ra)
            sig.rawData.push_back((uint16_t)v.as<int>());
    }
    return true;
}

void Cardputer_Remote::scanSDFiles()
{
    sdFiles.clear();
    if (!sdAvailable)
        return;
    File dir = SD.open("/IR_Signals");
    if (!dir)
        return;
    while (true)
    {
        File e = dir.openNextFile();
        if (!e)
            break;
        String nm = String(e.name());
        // .ir uzantılı dosyaları tara
        if (nm.endsWith(".ir"))
            sdFiles.push_back("/IR_Signals/" + nm);
        e.close();
    }
    dir.close();
}

void Cardputer_Remote::loadLibrary()
{
    library.clear();
    scanSDFiles();
    for (auto &f : sdFiles)
    {
        IRSignal s;
        if (loadSignalFromSD(f, s))
            library.push_back(s);
    }
}

bool Cardputer_Remote::deleteFromSD(String fn)
{
    if (!sdAvailable)
        return false;
    return SD.remove(fn);
}

//==============================================================
// IR SEND
//==============================================================
bool Cardputer_Remote::sendIR(IRSignal &sig)
{
    irSend.begin();
    delay(10);
    if (sig.rawData.size() > 0 && (sig.isRaw || sig.protocol == UNKNOWN))
    {
        irSend.sendRaw(sig.rawData.data(), sig.rawData.size(),
                       sig.frequency > 0 ? sig.frequency / 1000 : 38);
        return true;
    }
    switch (sig.protocol)
    {
    case NEC:
        irSend.sendNEC(sig.value, sig.bits);
        break;
    case SONY:
        irSend.sendSony(sig.value, sig.bits);
        break;
    case RC5:
        irSend.sendRC5(sig.value, sig.bits);
        break;
    case RC6:
        irSend.sendRC6(sig.value, sig.bits);
        break;
    case SAMSUNG:
        irSend.sendSAMSUNG(sig.value, sig.bits);
        break;
    case LG:
        irSend.sendLG(sig.value, sig.bits);
        break;
    case PANASONIC:
        irSend.sendPanasonic(sig.bits, sig.value);
        break;
    case JVC:
        irSend.sendJVC(sig.value, sig.bits, 0);
        break;
    case SHARP:
        irSend.sendSharpRaw(sig.value, sig.bits);
        break;
    case DENON:
        irSend.sendDenon(sig.value, sig.bits);
        break;
    default:
        if (sig.rawData.size() > 0)
        {
            irSend.sendRaw(sig.rawData.data(), sig.rawData.size(), 38);
            return true;
        }
        return false;
    }
    return true;
}

//==============================================================
// DRAW HELPERS
//==============================================================
void Cardputer_Remote::drawTopBar(String title)
{
    mainOS->sprite.fillRect(0, 0, SCREEN_W, 22, 0x111111);
    mainOS->sprite.drawFastHLine(0, 22, SCREEN_W, 0x333333);
    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);
    // Sol: uygulama adı
    mainOS->sprite.setTextColor(0x4488FF);
    mainOS->sprite.setCursor(4, 7);
    mainOS->sprite.print("IR REMOTE");
    // Orta: başlık
    mainOS->sprite.setTextColor(COLOR_TEXT);
    int tx = (SCREEN_W - (int)title.length() * 6) / 2;
    mainOS->sprite.setCursor(tx, 7);
    mainOS->sprite.print(title);
    // Sağ: SD göstergesi
    mainOS->sprite.fillCircle(SCREEN_W - 8, 11, 4, sdAvailable ? 0x004400 : 0x440000);
    mainOS->sprite.fillCircle(SCREEN_W - 8, 11, 2, sdAvailable ? COLOR_SUCCESS : COLOR_ACCENT);
}

void Cardputer_Remote::drawToast()
{
    if (statusMsg.length() == 0)
        return;
    if (millis() - statusTime > 2500)
    {
        statusMsg = "";
        return;
    }
    int w = min((int)statusMsg.length() * 6 + 16, SCREEN_W - 20);
    int x = (SCREEN_W - w) / 2, y = SCREEN_H - 28;
    mainOS->sprite.fillRoundRect(x, y, w, 16, 4, 0x220011);
    mainOS->sprite.drawRoundRect(x, y, w, 16, 4, 0x884466);
    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextColor(COLOR_TEXT);
    mainOS->sprite.setCursor(x + 8, y + 4);
    mainOS->sprite.print(statusMsg);
}

void Cardputer_Remote::drawBottomBar(String hint)
{
    mainOS->sprite.fillRect(0, SCREEN_H - 13, SCREEN_W, 13, 0x0A0A0A);
    mainOS->sprite.drawFastHLine(0, SCREEN_H - 13, SCREEN_W, 0x222222);
    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setTextColor(0x555555);
    mainOS->sprite.setCursor(3, SCREEN_H - 10);
    mainOS->sprite.print(hint);
}

//==============================================================
// MAIN MENU — sade, animasyonsuz
//==============================================================
void Cardputer_Remote::drawMainMenu()
{
    mainOS->sprite.fillScreen(COLOR_BG);
    drawTopBar("MAIN MENU");

    for (int i = 0; i < MAIN_MENU_COUNT; i++)
    {
        int y = MENU_START_Y + i * ITEM_H;
        bool sel = (i == menuIndex);
        uint32_t col = menuColors[i];

        if (sel)
        {
            // Seçili satır: renkli arka plan
            mainOS->sprite.fillRoundRect(4, y, SCREEN_W - 8, ITEM_H - 3, 4, col);
            mainOS->sprite.fillRect(4, y, 4, ITEM_H - 3, TFT_WHITE);
            mainOS->sprite.setTextColor(TFT_WHITE);
        }
        else
        {
            // Normal satır: koyu arka plan
            mainOS->sprite.fillRoundRect(4, y, SCREEN_W - 8, ITEM_H - 3, 4, 0x111111);
            mainOS->sprite.drawRoundRect(4, y, SCREEN_W - 8, ITEM_H - 3, 4, 0x222222);
            // Sol renkli çizgi
            mainOS->sprite.fillRect(4, y, 3, ITEM_H - 3, col);
            mainOS->sprite.setTextColor(0x888888);
        }

        mainOS->sprite.setFont(&fonts::FreeSansBold9pt7b);
        mainOS->sprite.setTextSize(1);
        mainOS->sprite.setCursor(14, y + 5);
        mainOS->sprite.print(mainMenuItems[i]);

        if (sel)
        {
            mainOS->sprite.setFont(&fonts::Font0);
            mainOS->sprite.setTextSize(1);
            mainOS->sprite.setTextColor(TFT_WHITE);
            mainOS->sprite.setCursor(SCREEN_W - 14, y + 9);
            mainOS->sprite.print(">");
        }
    }

    drawBottomBar("Use Up or Down Key, Enter: Select");
}

//==============================================================
// CAPTURE
//==============================================================
void Cardputer_Remote::drawCaptureScreen()
{
    mainOS->sprite.fillScreen(COLOR_BG);
    drawTopBar("CAPTURE IR");

    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);

    if (!signalCaptured)
    {
        // Basit animasyon halkası
        uint32_t t = millis() - animTimer;
        int r = 8 + (t / 80) % 16;
        mainOS->sprite.drawCircle(30, 78, r, 0x001866);
        mainOS->sprite.fillCircle(30, 78, 7, 0x0033BB);
        mainOS->sprite.fillCircle(30, 78, 3, TFT_WHITE);

        mainOS->sprite.setTextColor(COLOR_WARNING);
        mainOS->sprite.setCursor(50, 42);
        mainOS->sprite.print("Waiting for IR...");

        mainOS->sprite.setTextColor(0x444444);
        mainOS->sprite.setCursor(50, 58);
        mainOS->sprite.print("Point remote at");
        mainOS->sprite.setCursor(50, 70);
        mainOS->sprite.print("the device and");
        mainOS->sprite.setCursor(50, 82);
        mainOS->sprite.print("press a button.");

        drawBottomBar("`/ESC: Back");
        needsRedraw = true;
    }
    else
    {
        // Sinyal yakalandı
        mainOS->sprite.fillCircle(14, 42, 7, COLOR_SUCCESS);
        mainOS->sprite.setTextColor(COLOR_BG);
        mainOS->sprite.setCursor(9, 38);
        mainOS->sprite.print("OK");

        mainOS->sprite.fillRoundRect(26, 28, SCREEN_W - 30, 70, 5, 0x080808);
        mainOS->sprite.drawRoundRect(26, 28, SCREEN_W - 30, 70, 5, COLOR_SUCCESS);

        String proto = protoName(capturedSignal.protocol);
        mainOS->sprite.fillRoundRect(30, 31, proto.length() * 6 + 8, 12, 3, 0x002288);
        mainOS->sprite.setTextColor(TFT_WHITE);
        mainOS->sprite.setCursor(34, 34);
        mainOS->sprite.print(proto);

        mainOS->sprite.setTextColor(0x555555);
        mainOS->sprite.setCursor(30, 50);
        mainOS->sprite.print("HEX: ");
        mainOS->sprite.setTextColor(COLOR_SUCCESS);
        String hx = myU64ToHex(capturedSignal.value);
        if (hx.length() > 14)
            hx = hx.substring(0, 14) + "..";
        mainOS->sprite.print(hx);

        mainOS->sprite.setTextColor(0x555555);
        mainOS->sprite.setCursor(30, 62);
        mainOS->sprite.print("Bits: ");
        mainOS->sprite.setTextColor(COLOR_WARNING);
        mainOS->sprite.print(capturedSignal.bits);

        mainOS->sprite.setTextColor(0x555555);
        mainOS->sprite.setCursor(30, 74);
        mainOS->sprite.print("Raw: ");
        mainOS->sprite.setTextColor(0xAAAAAA);
        mainOS->sprite.print(capturedSignal.rawData.size());
        mainOS->sprite.print(" pulses");

        // Alt buton satırı
        mainOS->sprite.fillRoundRect(4, 102, 54, 14, 3, 0x003300);
        mainOS->sprite.drawRoundRect(4, 102, 54, 14, 3, COLOR_SUCCESS);
        mainOS->sprite.setTextColor(COLOR_SUCCESS);
        mainOS->sprite.setCursor(8, 105);
        mainOS->sprite.print("S: Save");

        mainOS->sprite.fillRoundRect(62, 102, 54, 14, 3, 0x000033);
        mainOS->sprite.drawRoundRect(62, 102, 54, 14, 3, 0x4444FF);
        mainOS->sprite.setTextColor(0x4444FF);
        mainOS->sprite.setCursor(66, 105);
        mainOS->sprite.print("T: Test");

        mainOS->sprite.fillRoundRect(120, 102, 54, 14, 3, 0x221100);
        mainOS->sprite.drawRoundRect(120, 102, 54, 14, 3, COLOR_WARNING);
        mainOS->sprite.setTextColor(COLOR_WARNING);
        mainOS->sprite.setCursor(124, 105);
        mainOS->sprite.print("V: View");

        mainOS->sprite.fillRoundRect(178, 102, 54, 14, 3, 0x220000);
        mainOS->sprite.drawRoundRect(178, 102, 54, 14, 3, COLOR_ACCENT);
        mainOS->sprite.setTextColor(COLOR_ACCENT);
        mainOS->sprite.setCursor(182, 105);
        mainOS->sprite.print("R: Redo");

        drawBottomBar("`/ESC: Back (save prompt)");
    }
}

//==============================================================
// SAVE PROMPT
//==============================================================
void Cardputer_Remote::drawSavePrompt()
{
    mainOS->sprite.fillScreen(COLOR_BG);
    drawTopBar("SAVE SIGNAL?");

    mainOS->sprite.fillRoundRect(10, 32, SCREEN_W - 20, 68, 6, 0x0A0A0A);
    mainOS->sprite.drawRoundRect(10, 32, SCREEN_W - 20, 68, 6, COLOR_WARNING);

    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);

    mainOS->sprite.setTextColor(COLOR_TEXT);
    mainOS->sprite.setCursor(18, 42);
    mainOS->sprite.print("Save before exiting?");

    mainOS->sprite.setTextColor(0x555555);
    mainOS->sprite.setCursor(18, 56);
    mainOS->sprite.print("Proto: ");
    mainOS->sprite.setTextColor(0x4488FF);
    mainOS->sprite.print(protoName(capturedSignal.protocol));
    mainOS->sprite.setTextColor(0x555555);
    mainOS->sprite.print("  ");
    mainOS->sprite.print(capturedSignal.bits);
    mainOS->sprite.print("b");

    mainOS->sprite.setTextColor(0x444444);
    mainOS->sprite.setCursor(18, 68);
    mainOS->sprite.print("Code: ");
    mainOS->sprite.setTextColor(0x666666);
    String hx = myU64ToHex(capturedSignal.value);
    if (hx.length() > 14)
        hx = hx.substring(0, 14);
    mainOS->sprite.print(hx);

    mainOS->sprite.fillRoundRect(18, 82, 84, 16, 4, 0x003300);
    mainOS->sprite.drawRoundRect(18, 82, 84, 16, 4, COLOR_SUCCESS);
    mainOS->sprite.setTextColor(COLOR_SUCCESS);
    mainOS->sprite.setCursor(28, 86);
    mainOS->sprite.print("Y - Yes, Save");

    mainOS->sprite.fillRoundRect(118, 82, 100, 16, 4, 0x220000);
    mainOS->sprite.drawRoundRect(118, 82, 100, 16, 4, COLOR_ACCENT);
    mainOS->sprite.setTextColor(COLOR_ACCENT);
    mainOS->sprite.setCursor(128, 86);
    mainOS->sprite.print("N - No, Discard");
}

//==============================================================
// SAVE DIALOG
//==============================================================
void Cardputer_Remote::drawSaveDialog()
{
    mainOS->sprite.fillScreen(COLOR_BG);
    drawTopBar("NAME SIGNAL");

    mainOS->sprite.fillRoundRect(6, 26, SCREEN_W - 12, 90, 6, 0x080808);
    mainOS->sprite.drawRoundRect(6, 26, SCREEN_W - 12, 90, 6, 0x0055FF);

    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);

    mainOS->sprite.setTextColor(0x777777);
    mainOS->sprite.setCursor(14, 34);
    mainOS->sprite.print("Enter signal name:");

    // Giriş kutusu
    mainOS->sprite.fillRoundRect(14, 46, SCREEN_W - 28, 18, 3, 0x050505);
    mainOS->sprite.drawRoundRect(14, 46, SCREEN_W - 28, 18, 3, 0x0055FF);
    mainOS->sprite.setTextColor(TFT_WHITE);
    mainOS->sprite.setCursor(18, 50);
    String disp = inputText;
    if (disp.length() > 24)
        disp = disp.substring(disp.length() - 24);
    mainOS->sprite.print(disp);
    // Cursor yanıp sönme
    if ((millis() / 400) % 2 == 0)
    {
        int cx = 18 + disp.length() * 6;
        if (cx < SCREEN_W - 18)
            mainOS->sprite.drawFastVLine(cx, 48, 12, 0x0088FF);
    }

    mainOS->sprite.setTextColor(0x444444);
    mainOS->sprite.setCursor(14, 72);
    mainOS->sprite.print("Proto: ");
    mainOS->sprite.setTextColor(0x4488FF);
    mainOS->sprite.print(protoName(capturedSignal.protocol));
    mainOS->sprite.setTextColor(0x444444);
    mainOS->sprite.print("  Bits: ");
    mainOS->sprite.setTextColor(0x4488FF);
    mainOS->sprite.print(capturedSignal.bits);

    mainOS->sprite.setTextColor(0x333333);
    mainOS->sprite.setCursor(14, 84);
    mainOS->sprite.print("Saved as: ");
    mainOS->sprite.setTextColor(0x555555);
    String fn = inputText.length() > 0 ? inputText : "?";
    fn.replace(" ", "_");
    mainOS->sprite.print(fn + ".ir");

    mainOS->sprite.fillRoundRect(14, 100, 80, 14, 3, 0x003300);
    mainOS->sprite.drawRoundRect(14, 100, 80, 14, 3, COLOR_SUCCESS);
    mainOS->sprite.setTextColor(COLOR_SUCCESS);
    mainOS->sprite.setCursor(20, 103);
    mainOS->sprite.print("ENTER: Save");

    mainOS->sprite.fillRoundRect(102, 100, 124, 14, 3, 0x220000);
    mainOS->sprite.drawRoundRect(102, 100, 124, 14, 3, COLOR_ACCENT);
    mainOS->sprite.setTextColor(COLOR_ACCENT);
    mainOS->sprite.setCursor(108, 103);
    mainOS->sprite.print("` / ESC: Cancel");

    needsRedraw = true;
}

//==============================================================
// LIBRARY
//==============================================================
void Cardputer_Remote::drawLibraryScreen()
{
    mainOS->sprite.fillScreen(COLOR_BG);
    drawTopBar("MY LIBRARY");

    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);

    if (library.empty())
    {
        mainOS->sprite.setTextColor(0x444444);
        mainOS->sprite.setCursor(SCREEN_W / 2 - 52, 55);
        mainOS->sprite.print("No saved signals yet");
        mainOS->sprite.setCursor(SCREEN_W / 2 - 58, 68);
        mainOS->sprite.print("Capture a signal first!");
        drawBottomBar("` : Back");
        return;
    }

    int visN = 4, itemH = 24, startY = 26;
    if (libraryIndex < scrollOffset)
        scrollOffset = libraryIndex;
    if (libraryIndex >= scrollOffset + visN)
        scrollOffset = libraryIndex - visN + 1;

    for (int i = 0; i < visN; i++)
    {
        int idx = i + scrollOffset;
        if (idx >= (int)library.size())
            break;
        int y = startY + i * itemH;
        bool sel = (idx == libraryIndex);

        if (sel)
        {
            mainOS->sprite.fillRoundRect(2, y, SCREEN_W - 4, itemH - 2, 3, 0x001244);
            mainOS->sprite.drawRoundRect(2, y, SCREEN_W - 4, itemH - 2, 3, 0x0044FF);
            mainOS->sprite.fillRect(2, y, 3, itemH - 2, 0x0088FF);
        }
        else
        {
            mainOS->sprite.fillRoundRect(2, y, SCREEN_W - 4, itemH - 2, 3, 0x080808);
            mainOS->sprite.drawRoundRect(2, y, SCREEN_W - 4, itemH - 2, 3, 0x1A1A1A);
        }

        mainOS->sprite.setTextColor(sel ? TFT_WHITE : 0x777777);
        mainOS->sprite.setCursor(10, y + 4);
        String nm = library[idx].name;
        if (nm.length() > 18)
            nm = nm.substring(0, 18) + "..";
        mainOS->sprite.print(nm);

        String pt = protoName(library[idx].protocol);
        mainOS->sprite.setTextColor(sel ? 0x66AAFF : 0x444444);
        mainOS->sprite.setCursor(10, y + 14);
        String hx = myU64ToHex(library[idx].value);
        if (hx.length() > 12)
            hx = hx.substring(0, 12) + "..";
        mainOS->sprite.print(pt + " | " + hx);
    }

    // Scroll bar
    if ((int)library.size() > visN)
    {
        int sbH = visN * itemH, sbX = SCREEN_W - 3, sbY = startY;
        mainOS->sprite.fillRect(sbX, sbY, 2, sbH, 0x111111);
        int th = sbH * visN / library.size();
        int ty = sbY + sbH * scrollOffset / library.size();
        mainOS->sprite.fillRect(sbX, ty, 2, th, 0x0055FF);
    }

    drawBottomBar(",/.: Nav  Enter:Send  V:View  D:Del  `:Back");
}

//==============================================================
// VIEW CODE
//==============================================================
void Cardputer_Remote::drawViewCodeScreen()
{
    mainOS->sprite.fillScreen(COLOR_BG);
    drawTopBar("IR CODE DETAIL");

    IRSignal &sig = selectedSignal;
    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);

    // Protokol kartı
    mainOS->sprite.fillRoundRect(2, 26, 88, 26, 4, 0x090909);
    mainOS->sprite.drawRoundRect(2, 26, 88, 26, 4, 0x0033AA);
    mainOS->sprite.setTextColor(0x334466);
    mainOS->sprite.setCursor(6, 29);
    mainOS->sprite.print("Protocol");
    mainOS->sprite.setTextColor(0x4488FF);
    mainOS->sprite.setCursor(6, 40);
    mainOS->sprite.print(protoName(sig.protocol));

    // Bits kartı
    mainOS->sprite.fillRoundRect(94, 26, 56, 26, 4, 0x090909);
    mainOS->sprite.drawRoundRect(94, 26, 56, 26, 4, 0x003300);
    mainOS->sprite.setTextColor(0x224422);
    mainOS->sprite.setCursor(98, 29);
    mainOS->sprite.print("Bits");
    mainOS->sprite.setTextColor(COLOR_SUCCESS);
    mainOS->sprite.setCursor(98, 40);
    mainOS->sprite.print(sig.bits);

    // Frekans kartı
    mainOS->sprite.fillRoundRect(154, 26, 82, 26, 4, 0x090909);
    mainOS->sprite.drawRoundRect(154, 26, 82, 26, 4, 0x332200);
    mainOS->sprite.setTextColor(0x443322);
    mainOS->sprite.setCursor(158, 29);
    mainOS->sprite.print("Freq");
    mainOS->sprite.setTextColor(COLOR_WARNING);
    mainOS->sprite.setCursor(158, 40);
    mainOS->sprite.print(String(sig.frequency / 1000) + " kHz");

    // HEX satırı
    mainOS->sprite.fillRoundRect(2, 56, SCREEN_W - 4, 16, 3, 0x080808);
    mainOS->sprite.drawRoundRect(2, 56, SCREEN_W - 4, 16, 3, 0x222222);
    mainOS->sprite.setTextColor(0x444444);
    mainOS->sprite.setCursor(6, 60);
    mainOS->sprite.print("HEX:");
    mainOS->sprite.setTextColor(COLOR_SUCCESS);
    mainOS->sprite.setCursor(36, 60);
    mainOS->sprite.print(myU64ToHex(sig.value));

    // DEC satırı
    mainOS->sprite.fillRoundRect(2, 76, SCREEN_W - 4, 16, 3, 0x080808);
    mainOS->sprite.drawRoundRect(2, 76, SCREEN_W - 4, 16, 3, 0x222222);
    mainOS->sprite.setTextColor(0x444444);
    mainOS->sprite.setCursor(6, 80);
    mainOS->sprite.print("DEC:");
    mainOS->sprite.setTextColor(0xCCCCCC);
    mainOS->sprite.setCursor(36, 80);
    String dc = myU64ToStr(sig.value);
    if (dc.length() > 18)
        dc = dc.substring(0, 18) + "..";
    mainOS->sprite.print(dc);

    // İsim / raw satırı
    mainOS->sprite.setTextColor(0x333333);
    mainOS->sprite.setCursor(6, 100);
    if (sig.rawData.size() > 0)
    {
        mainOS->sprite.print("Raw: ");
        mainOS->sprite.setTextColor(COLOR_WARNING);
        mainOS->sprite.print(sig.rawData.size());
        mainOS->sprite.print(" pulses");
    }
    else
    {
        mainOS->sprite.print("Name: ");
        mainOS->sprite.setTextColor(0xAAAAAA);
        String nm = sig.name;
        if (nm.length() > 18)
            nm = nm.substring(0, 18) + "..";
        mainOS->sprite.print(nm);
    }

    drawBottomBar("Enter/S: Send    `: Back");
}

//==============================================================
// SD BROWSER
//==============================================================
void Cardputer_Remote::drawSDBrowserScreen()
{
    mainOS->sprite.fillScreen(COLOR_BG);
    drawTopBar("SD BROWSER (.ir)");

    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);

    if (!sdAvailable)
    {
        mainOS->sprite.setTextColor(COLOR_ACCENT);
        mainOS->sprite.setCursor(SCREEN_W / 2 - 48, 52);
        mainOS->sprite.print("SD Card not found!");
        mainOS->sprite.setTextColor(0x444444);
        mainOS->sprite.setCursor(SCREEN_W / 2 - 54, 66);
        mainOS->sprite.print("Insert card & restart");
        drawBottomBar("` : Back");
        return;
    }

    if (sdFiles.empty())
    {
        mainOS->sprite.setTextColor(0x444444);
        mainOS->sprite.setCursor(SCREEN_W / 2 - 48, 52);
        mainOS->sprite.print("No .ir files found");
        mainOS->sprite.setCursor(SCREEN_W / 2 - 52, 66);
        mainOS->sprite.print("in /IR_Signals/ folder");
        drawBottomBar("R: Refresh   ` : Back");
        return;
    }

    int visN = 4, itemH = 24, startY = 26;
    if (sdFileIndex < scrollOffset)
        scrollOffset = sdFileIndex;
    if (sdFileIndex >= scrollOffset + visN)
        scrollOffset = sdFileIndex - visN + 1;

    for (int i = 0; i < visN; i++)
    {
        int idx = i + scrollOffset;
        if (idx >= (int)sdFiles.size())
            break;
        int y = startY + i * itemH;
        bool sel = (idx == sdFileIndex);

        if (sel)
        {
            mainOS->sprite.fillRoundRect(2, y, SCREEN_W - 4, itemH - 2, 3, 0x001800);
            mainOS->sprite.drawRoundRect(2, y, SCREEN_W - 4, itemH - 2, 3, 0x00AA44);
            mainOS->sprite.fillRect(2, y, 3, itemH - 2, COLOR_SUCCESS);
        }
        else
        {
            mainOS->sprite.fillRoundRect(2, y, SCREEN_W - 4, itemH - 2, 3, 0x080808);
            mainOS->sprite.drawRoundRect(2, y, SCREEN_W - 4, itemH - 2, 3, 0x1A1A1A);
        }

        String fn = sdFiles[idx];
        int ls = fn.lastIndexOf('/');
        if (ls >= 0)
            fn = fn.substring(ls + 1);
        if (fn.endsWith(".ir"))
            fn = fn.substring(0, fn.length() - 3);
        fn.replace("_", " ");
        if (fn.length() > 26)
            fn = fn.substring(0, 26) + "..";

        mainOS->sprite.setTextColor(sel ? COLOR_SUCCESS : 0x337733);
        mainOS->sprite.setCursor(8, y + 4);
        mainOS->sprite.print(".ir ");
        mainOS->sprite.setTextColor(sel ? TFT_WHITE : 0x777777);
        mainOS->sprite.print(fn);

        mainOS->sprite.setTextColor(sel ? 0x338844 : 0x2A2A2A);
        String ctr = String(idx + 1) + "/" + String(sdFiles.size());
        mainOS->sprite.setCursor(SCREEN_W - ctr.length() * 6 - 6, y + 14);
        mainOS->sprite.print(ctr);
    }

    // Scroll bar
    if ((int)sdFiles.size() > visN)
    {
        int sbH = visN * itemH, sbX = SCREEN_W - 3, sbY = startY;
        mainOS->sprite.fillRect(sbX, sbY, 2, sbH, 0x111111);
        int th = sbH * visN / sdFiles.size();
        int ty = sbY + sbH * scrollOffset / sdFiles.size();
        mainOS->sprite.fillRect(sbX, ty, 2, th, COLOR_SUCCESS);
    }

    drawBottomBar("R:Refresh  ESC:Back");
}

//==============================================================
// SEND CONFIRM
//==============================================================
void Cardputer_Remote::drawSendConfirm(bool sending)
{
    mainOS->sprite.fillScreen(COLOR_BG);
    drawTopBar("TRANSMIT");

    mainOS->sprite.fillRoundRect(6, 26, SCREEN_W - 12, 80, 6, 0x080808);
    mainOS->sprite.drawRoundRect(6, 26, SCREEN_W - 12, 80, 6, sending ? COLOR_SUCCESS : 0x0044FF);

    mainOS->sprite.setFont(&fonts::Font0);
    mainOS->sprite.setTextSize(1);

    if (sending)
    {
        mainOS->sprite.setTextSize(2);
        mainOS->sprite.setTextColor(COLOR_SUCCESS);
        mainOS->sprite.setCursor(80, 52);
        mainOS->sprite.print(">>>");
        mainOS->sprite.setTextSize(1);
        mainOS->sprite.setTextColor(0x777777);
        mainOS->sprite.setCursor(SCREEN_W / 2 - 36, 82);
        mainOS->sprite.print("Transmitting...");
        needsRedraw = true;
    }
    else
    {
        mainOS->sprite.setTextColor(0x555555);
        mainOS->sprite.setCursor(14, 34);
        mainOS->sprite.print("Ready to send:");
        mainOS->sprite.setTextColor(TFT_WHITE);
        mainOS->sprite.setCursor(14, 46);
        String nm = selectedSignal.name;
        if (nm.length() > 26)
            nm = nm.substring(0, 26) + "..";
        mainOS->sprite.print(nm);

        String pt = protoName(selectedSignal.protocol);
        int pw = pt.length() * 6 + 8;
        mainOS->sprite.fillRoundRect(14, 58, pw, 12, 3, 0x001A44);
        mainOS->sprite.setTextColor(0x4488FF);
        mainOS->sprite.setCursor(18, 61);
        mainOS->sprite.print(pt);

        mainOS->sprite.setTextColor(0x333333);
        mainOS->sprite.setCursor(14, 74);
        mainOS->sprite.print("Code: ");
        mainOS->sprite.setTextColor(0x555555);
        String hx = myU64ToHex(selectedSignal.value);
        if (hx.length() > 14)
            hx = hx.substring(0, 14) + "..";
        mainOS->sprite.print(hx);

        mainOS->sprite.fillRoundRect(14, 92, 88, 14, 4, 0x003300);
        mainOS->sprite.drawRoundRect(14, 92, 88, 14, 4, COLOR_SUCCESS);
        mainOS->sprite.setTextColor(COLOR_SUCCESS);
        mainOS->sprite.setCursor(22, 95);
        mainOS->sprite.print("Enter: SEND");

        mainOS->sprite.fillRoundRect(114, 92, 110, 14, 4, 0x1A0000);
        mainOS->sprite.drawRoundRect(114, 92, 110, 14, 4, COLOR_ACCENT);
        mainOS->sprite.setTextColor(COLOR_ACCENT);
        mainOS->sprite.setCursor(120, 95);
        mainOS->sprite.print("` / ESC: Cancel");
    }
}

//==============================================================
// MASTER DRAW
//==============================================================
void Cardputer_Remote::drawScreen()
{
    switch (currentState)
    {
    case STATE_MAIN_MENU:
        drawMainMenu();
        break;
    case STATE_CAPTURE:
        drawCaptureScreen();
        break;
    case STATE_SAVE_PROMPT:
        drawSavePrompt();
        break;
    case STATE_CAPTURE_SAVE:
        drawSaveDialog();
        break;
    case STATE_LIBRARY:
        drawLibraryScreen();
        break;
    case STATE_VIEW_CODE:
        drawViewCodeScreen();
        break;
    case STATE_SD_BROWSER:
        drawSDBrowserScreen();
        break;
    case STATE_SEND_CONFIRM:
        drawSendConfirm(false);
        break;
    }
    drawToast();
    mainOS->sprite.pushSprite(0, 0);
}

//==============================================================
// IR RECEIVE
//==============================================================
void Cardputer_Remote::handleIRReceive()
{
    if (!capturing || signalCaptured)
        return;
    if (!irRecv.decode(&irResults))
        return;
    if (irResults.value != 0xFFFFFFFF && irResults.value != 0x0 && millis() - lastCapture > 300)
    {
        capturedSignal.protocol = irResults.decode_type;
        capturedSignal.value = irResults.value;
        capturedSignal.bits = irResults.bits;
        capturedSignal.frequency = 38000;
        capturedSignal.name = "IR_" + String(millis() % 100000);
        capturedSignal.isRaw = (irResults.decode_type == UNKNOWN);
        capturedSignal.rawData.clear();
        for (int i = 1; i < irResults.rawlen && i < 300; i++)
            capturedSignal.rawData.push_back(irResults.rawbuf[i] * RAWTICK);
        signalCaptured = true;
        lastCapture = millis();
        capturing = false;
        irRecv.pause();
        needsRedraw = true;
    }
    irRecv.resume();
}

//==============================================================
// KEY HANDLERS
//==============================================================
void Cardputer_Remote::handleMainMenu(char key)
{
    if (key == ';')
        menuIndex = (menuIndex - 1 + MAIN_MENU_COUNT) % MAIN_MENU_COUNT;
    else if (key == '.')
        menuIndex = (menuIndex + 1) % MAIN_MENU_COUNT;
    else if (key == '\n' || key == '\r' || key == ' ')
    {
        scrollOffset = 0;
        switch (menuIndex)
        {
        case 0:
            currentState = STATE_CAPTURE;
            signalCaptured = false;
            capturing = true;
            irRecv.resume();
            animTimer = millis();
            break;
        case 1:
            loadLibrary();
            currentState = STATE_LIBRARY;
            libraryIndex = 0;
            scrollOffset = 0;
            break;
        case 2:
            scanSDFiles();
            currentState = STATE_SD_BROWSER;
            sdFileIndex = 0;
            scrollOffset = 0;
            break;
        }
    }
}

void Cardputer_Remote::handleCapture(char key)
{
    if (key == '`' || key == 27)
    {

        if (signalCaptured)
        {
            currentState = STATE_SAVE_PROMPT;
        }
        else
        {
            capturing = false;
            irRecv.pause();
            currentState = STATE_MAIN_MENU;
        }
    }
    else if (signalCaptured)
    {
        if (key == 's' || key == 'S')
        {
            if (sdAvailable)
            {
                currentState = STATE_CAPTURE_SAVE;
                inputText = "";
            }
            else
                setStatus("SD Card not available!");
        }
        else if (key == 't' || key == 'T')
        {
            selectedSignal = capturedSignal;
            bool ok = sendIR(selectedSignal);
            setStatus(ok ? "Signal sent!" : "Send failed!");
        }
        else if (key == 'v' || key == 'V')
        {
            selectedSignal = capturedSignal;
            previousState = STATE_CAPTURE;
            currentState = STATE_VIEW_CODE;
        }
        else if (key == 'r' || key == 'R')
        {
            signalCaptured = false;
            capturing = true;
            irRecv.resume();
            animTimer = millis();
        }
    }
}

void Cardputer_Remote::handleSavePrompt(char key)
{
    if (key == 'y' || key == 'Y')
    {
        currentState = STATE_CAPTURE_SAVE;
        inputText = "";
    }
    else if (key == 'n' || key == 'N' || key == '`' || key == 27)
    {
        capturing = false;
        irRecv.pause();
        signalCaptured = false;
        currentState = STATE_MAIN_MENU;
    }
}

void Cardputer_Remote::handleSaveDialog(char key)
{
    if (key == '`' || key == 27)
    {
        currentState = STATE_CAPTURE;
    }
    else if (key == '\n' || key == '\r')
    {
        if (inputText.length() == 0)
        {
            setStatus("Name cannot be empty!");
            return;
        }
        capturedSignal.name = inputText;
        if (saveSignalToSD(capturedSignal))
        {
            library.push_back(capturedSignal);
            setStatus("Saved: " + inputText + ".ir");
            currentState = STATE_MAIN_MENU;
            signalCaptured = false;
            capturing = false;
            inputText = "";
        }
        else
            setStatus("Save failed!");
    }
    else if (key == 8 || key == 127)
    {
        if (inputText.length() > 0)
            inputText.remove(inputText.length() - 1);
    }
    else if (key >= 32 && key <= 126 && inputText.length() < 20)
    {
        inputText += key;
    }
}

void Cardputer_Remote::handleLibrary(char key)
{
    if (key == '`' || key == 27)
        currentState = STATE_MAIN_MENU;
    else if (key == ';' && libraryIndex > 0)
        libraryIndex--;
    else if (key == '.' && libraryIndex < (int)library.size() - 1)
        libraryIndex++;
    else if ((key == '\n' || key == '\r' || key == ' ') && !library.empty())
    {
        selectedSignal = library[libraryIndex];
        previousState = STATE_LIBRARY;
        currentState = STATE_SEND_CONFIRM;
    }
    else if ((key == 'v' || key == 'V') && !library.empty())
    {
        selectedSignal = library[libraryIndex];
        previousState = STATE_LIBRARY;
        currentState = STATE_VIEW_CODE;
    }
    else if ((key == 'd' || key == 'D') && !library.empty())
    {
        if (deleteFromSD(library[libraryIndex].filename))
        {
            library.erase(library.begin() + libraryIndex);
            if (libraryIndex >= (int)library.size() && libraryIndex > 0)
                libraryIndex--;
            setStatus("Signal deleted");
        }
        else
            setStatus("Delete failed!");
    }
}

void Cardputer_Remote::handleViewCode(char key)
{
    if (key == '`' || key == 27)
        currentState = previousState;
    else if (key == 's' || key == 'S' || key == '\n' || key == '\r')
    {
        bool ok = sendIR(selectedSignal);
        setStatus(ok ? "Signal sent!" : "Send failed!");
    }
}

void Cardputer_Remote::handleSDBrowser(char key)
{
    if (key == '`' || key == 27)
        currentState = STATE_MAIN_MENU;
    else if (key == ';' && sdFileIndex > 0)
        sdFileIndex--;
    else if (key == '.' && sdFileIndex < (int)sdFiles.size() - 1)
        sdFileIndex++;
    else if ((key == '\n' || key == '\r' || key == ' ') && !sdFiles.empty())
    {
        IRSignal sig;
        if (loadSignalFromSD(sdFiles[sdFileIndex], sig))
        {
            selectedSignal = sig;
            previousState = STATE_SD_BROWSER;
            currentState = STATE_SEND_CONFIRM;
        }
        else
            setStatus("Failed to load!");
    }
    else if (key == 'r' || key == 'R')
    {
        scanSDFiles();
        sdFileIndex = 0;
        scrollOffset = 0;
        setStatus("SD refreshed");
    }
}

void Cardputer_Remote::handleSendConfirm(char key)
{
    if (key == '`' || key == 27)
    {
        currentState = previousState;
    }
    else if (key == '\n' || key == '\r' || key == ' ' || key == 's' || key == 'S')
    {
        // Gönderme animasyonu
        mainOS->sprite.fillScreen(COLOR_BG);
        drawTopBar("TRANSMIT");
        drawSendConfirm(true);
        drawToast();
        mainOS->sprite.pushSprite(0, 0);
        bool ok = sendIR(selectedSignal);
        delay(400);
        setStatus(ok ? "Sent successfully!" : "Send failed!");
        currentState = previousState;
    }
}
