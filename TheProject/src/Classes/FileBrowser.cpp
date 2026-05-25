#include "FileBrowser.h"
#include "./MyOS.h"
// #include "MusicPlayerV2.h"
#include "./Function.h"
#include "TextEditor.h"
#include "My3dRenderer.h"
#include "Pic/TopBarIcons.c"
// #include <JPEGDecoder.h>
#if defined(COMPILE_WIFI_THINGS)
#include "EspProjectRC/ESP_RemoteControlFile.h"
#endif
#include "tools/IRsend.h"
#include "tools/IRedit.h"
#include "GamesV2.h"
// #include <AnimatedGIF.h>
// #include <JPEGDecoder.h>
// gif sector
#include <LittleFS.h>
#include "esp_ota_ops.h"
#include "MainMenuV2.h"
// #include "nvs_flash.h"
#include "HEX_Editor.h"
#include <Update.h>
#include <Preferences.h>
#include "Theme_Maneger.h"
#include "Painter.h"
#include <algorithm> // נדרש עבור std::sort
#include <vector>
#include "MusicCreator.h"
/* #include <USB.h>
#include <USBMSC.h>
USBMSC msc;
void drawLedIndicator(bool plugged)
{
    M5.Lcd.fillRoundRect(54, 60, 5, 30, 5, plugged ? GREEN : RED);
}
void displayMessage(std::string message)
{
    M5.Lcd.fillRect(63, 28, 10, 17, TFT_BLACK);
    M5.Lcd.setCursor(66, 30);
    M5.Lcd.print(message.c_str());
}
int32_t usbWriteCallback(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    // Verify freespace
    uint64_t freeSpace = SD.totalBytes() - SD.usedBytes();
    if (bufsize > freeSpace)
    {
        return -1; // no space available
    }

    // Verify sector size
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0)
        return -1; // disk error

    // Write blocs
    for (uint32_t x = 0; x < bufsize / secSize; ++x)
    {
        uint8_t blkBuffer[secSize];
        memcpy(blkBuffer, buffer + secSize * x, secSize);
        if (!SD.writeRAW(blkBuffer, lba + x))
        {
            return -1; // write error
        }
    }
    return bufsize;
}

int32_t usbReadCallback(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    // Verify sector size
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0)
        return -1; // disk error

    // Read blocs
    for (uint32_t x = 0; x < bufsize / secSize; ++x)
    {
        if (!SD.readRAW(reinterpret_cast<uint8_t *>(buffer) + (x * secSize), lba + x))
        {
            return -1; // read error
        }
    }
    return bufsize;
}

bool usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject)
{
    return true;
}
void drawUSBStickIcon()
{
    // Port USB
    M5.Lcd.fillRoundRect(155, 55, 40, 40, 5, LIGHTGREY);
    // Body
    M5.Lcd.fillRoundRect(45, 50, 112, 50, 5, DARKCYAN);
    // Small square on port
    M5.Lcd.fillRoundRect(164, 63, 20, 10, 5, DARKGREY);
    M5.Lcd.fillRoundRect(164, 78, 20, 10, 5, DARKGREY);
} */

Preferences prefs;

// File f;
static unsigned long gifNextFrame = 0;

bool gifLoopVar = false;
String GifPathToPlay;
bool RenderGif = false;

void drawJpeg(const char *filename)
{
    /*     M5Cardputer.Display.setSwapBytes(true);
        JpegDec.decodeSdFile(filename);

        int scale = 1 << pic_zoom_out; // 1,2,4,8...

        while (JpegDec.read())
        {
            uint16_t *pImg = JpegDec.pImage;

            int srcW = JpegDec.MCUWidth;
            int srcH = JpegDec.MCUHeight;

            int dstW = srcW / scale;
            int dstH = srcH / scale;

            static uint16_t lineBuf[240];

            for (int y = 0; y < dstH; y++)
            {
                for (int x = 0; x < dstW; x++)
                {
                    int srcX = x * scale;
                    int srcY = y * scale;
                    lineBuf[x] = pImg[srcY * srcW + srcX];
                }

                int drawX = IMG_x_POS + (JpegDec.MCUx * srcW) / scale;
                int drawY = IMG_y_POS + (JpegDec.MCUy * srcH) / scale + y;

                M5Cardputer.Display.pushImage(drawX, drawY, dstW, 1, lineBuf);
            }
        }

        M5Cardputer.Display.setSwapBytes(false); */
}

// end of gif sector

// #include "MusicPlayerV2.h"

// (Crop)

void FileBrowser::UndrawPic()
{
    mainOS->showTopBar = true;
    RenderGif = false;
    gifLoopVar = false;
    ShowPicOnScreen = false;
    gif.close();
    gifFile.close();
}

// end of PNG sector

void FileBrowser::Begin()
{
    // handleRunOnceRollback();

    if (BrowseLittleFS)
    {
        if (!LittleFS.begin())
        {
            mainOS->ShowOnScreenMessege("little FS filed", 1000);
            return;
        }
    }
    if (mainOS->FileOpenFromMainMenu)
    {
        // RenderFileOptionMenu = true;
        FileOfFolderSelected();
        return;
    }
    ShowCurrentFilesInDir(GetActiveFS(BrowseLittleFS), mainOS->currentPath.c_str());

    // ScanAndSort(GetActiveFS(BrowseLittleFS), mainOS->currentPath.c_str());
}

void FileBrowser::Loop()
{
    if (mainOS->screenOff)
    {
        return;
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_BACKSPACE))
    {
        if (!os.AskSomthing("Are you sure You want to delete file?"))
        {
            // CloseFileMenu();
            return;
        }
        if (!InFileMarkMode)

        {
            String full_Path_Of_The_Current_Selected = FileFound[mainOS->FileIndexSelected];
            if (mainOS->deleteFromSd(SD, full_Path_Of_The_Current_Selected))
            //   targetCameraY = 0;
            {
                ShowCurrentFilesInDir(GetActiveFS(BrowseLittleFS), mainOS->currentPath.c_str());
            }
        }

        else // in file mark mode
        {

            if (InFileMarkMode)
            {

                for (size_t i = 0; i < File_ID_Marked.size(); i++)
                {
                    String full_Path_Of_The_Current_Selected = File_ID_Marked[i];
                    mainOS->deleteFromSd(SD, full_Path_Of_The_Current_Selected);
                }
                InFileOptionMenuIndex = 0;
                ShowCurrentFilesInDir(GetActiveFS(BrowseLittleFS), mainOS->currentPath.c_str());
                InFileMarkMode = false;
                File_ID_Marked.clear();
                CloseFileMenu();
            }
        }
    }

    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(';', 700, 50))

    // if (M5Cardputer.Keyboard.isKeyPressed(';'))
    {
        mainOS->PlayCuteEvilTone();

        // UndrawPic();
        if (ShowPicOnScreen)
        {
            M5Cardputer.Display.fillScreen(BLACK);

            IMG_y_POS += 6;
            RefrashImage();
        }
        // FirsTimePressTimer++;
        else if (RenderFileOptionMenu)
        {
            InFileOptionMenuIndex--;
            if (InFileOptionMenuIndex < 0)
            {
                InFileOptionMenuIndex = FileOptionMenuItems.size() - 1;
            }
        }
        else
        {
            mainOS->FileIndexSelected--;
            //  targetCameraY--;
            if (mainOS->FileIndexSelected < 0)
                mainOS->FileIndexSelected = fileCount - 1;
        }
    }
    // if (M5Cardputer.Keyboard.isKeyPressed('.'))
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('.', 700, 50))

    {
        mainOS->PlayCuteEvilTone();

        if (ShowPicOnScreen)
        {
            M5Cardputer.Display.fillScreen(BLACK);

            IMG_y_POS -= 6;
            RefrashImage();
        }
        // FirsTimePressTimer++;

        else if (RenderFileOptionMenu)
        {
            InFileOptionMenuIndex++;

            if (InFileOptionMenuIndex > FileOptionMenuItems.size() - 1)
            {
                InFileOptionMenuIndex = 0;
            }
        }
        else
        {

            mainOS->FileIndexSelected++;
            //  targetCameraY++;
            if (mainOS->FileIndexSelected >= fileCount)
                mainOS->FileIndexSelected = 0;
        }
    }
    if (ShowPicOnScreen)
    {
        if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('/', 700, 50)) // right  arrow
        {
            mainOS->PlayCuteEvilTone();

            if (ShowPicOnScreen)
            {
                M5Cardputer.Display.fillScreen(BLACK);

                IMG_x_POS -= 6;
                RefrashImage();
            }
        }
        if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(',', 700, 50)) // left arrow
        {
            mainOS->PlayCuteEvilTone();

            if (ShowPicOnScreen)
            {
                M5Cardputer.Display.fillScreen(BLACK);

                IMG_x_POS += 6;
                RefrashImage();
            }
        }
    }
    if (mainOS->NewKey.ifKeyJustPress('-')) //-
    {
        mainOS->PlayCuteEvilTone();

        if (ShowPicOnScreen)
        {
            M5Cardputer.Display.fillScreen(BLACK);

            pic_zoom_out += 1;
            RefrashImage();
        }
    }
    if (mainOS->NewKey.ifKeyJustPress('=')) // =
    {
        mainOS->PlayCuteEvilTone();

        if (ShowPicOnScreen)
        {
            M5Cardputer.Display.fillScreen(BLACK);

            pic_zoom_out -= 1;
            if (pic_zoom_out <= 0)
            {
                pic_zoom_out = 0;
            }
            RefrashImage();
        }
    }

    // ESC
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        if (mainOS->FileOpenFromMainMenu)
        {
            mainOS->ChangeMenu(new MainMenuV2(mainOS));
            return;
        }
        mainOS->PlayCuteEvilTone();

        if (ShowPicOnScreen)
        {
            UndrawPic();
        }
        else if (RenderFileOptionMenu)
        {
            CloseFileMenu();
        }
        else
        {
            GoUpOneFolder();
        }
    }
    /*         if (M5Cardputer.Keyboard.isKeyPressed('/')) // if right arrow selected activate file without menu
            {
                // UndrawPic();
                ExecuteFileOrFolder();
            } */
    if (mainOS->NewKey.ifKeyJustPress('/'))
    {
        mainOS->PlayCuteEvilTone();

        if (InFileMarkMode)
        {
            Mark_UnmarkFile();
        }
    }

    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        mainOS->PlayCuteEvilTone();

        if (ShowPicOnScreen)
        {
            UndrawPic();
            // PNGypos += 1;
            // DrawPNG(FileFound[mainOS->FileIndexSelected]);
        }
        // UndrawPic();
        /*         else if (InFileMarkMode && !RenderFileOptionMenu)
                {
                    Mark_UnmarkFile();
                } */
        else if (RenderFileOptionMenu)
        {
            SelectMenuItem();
            return;
        }
        else
        {
            InFileOptionMenuIndex = 0;
            FileOfFolderSelected();
        }
    }

    // }

    // lerp camera
    float finalCameraYPos = FileListYoffset + mainOS->FileIndexSelected * (mainOS->sprite.fontHeight() + 3);
    cameraYpos = lerpFloat(cameraYpos, finalCameraYPos, 0.12);
    // cameraYpos = cameraYpos + (finalCameraYPos - cameraYpos) * 0.12f;
    if (!mainOS->screenOff)
    {
        if (!ShowPicOnScreen)
        {
            Draw();
        }
        else
        {

            // הוסף משתנים אלה ליד gifLoopVar

            // בתוך ה-loop:
            if (RenderGif)
            {
                if (!gifLoopVar)
                {
                    gifFile = SD.open(GifPathToPlay.c_str());
                    if (!gifFile)
                        return;

                    gif.open(GifPathToPlay.c_str(),
                             GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw);

                    gifLoopVar = true;
                    gifNextFrame = millis(); // אתחול טיימר
                }

                // המתן לזמן הנכון — ללא חסימה
                if (millis() >= gifNextFrame)
                {
                    int frameDelay = 0;
                    if (!gif.playFrame(false, &frameDelay))
                    {
                        gif.close();
                        gifLoopVar = false;
                        gifFile.close();
                    }
                    else
                    {
                        // frameDelay הוא במילישניות (הספרייה כבר מחזירה ms)
                        gifNextFrame = millis() + frameDelay;
                    }
                }

                yield();
            }
        }
    }
}
void FileBrowser::Draw()
{
    if (AddWillBeDeleted)
    {
        return;
    }
    int OffsetBetweenLines = 3;
    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H - TopOffset);
    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextDatum(TL_DATUM);

    // mainOS->sprite.loadFont(ariel12font);
    mainOS->sprite.setTextSize(mainOS->TextSizeInMenuesSAVE);
    mainOS->sprite.fillSprite(rgb565(20, 20, 20));
    // sprite.setFont(&fonts::lgfxJapanGothic_12);
    mainOS->sprite.setTextWrap(false);
    if (FolderEmpty)
    {
        mainOS->sprite.setCursor(50, 50);
        mainOS->sprite.print("Folder Empty!");
        // mainOS->ShowOnScreenMessege("empty",10);
    }
    // int visibleCount = (SCREEN_H - TopOffset) / (mainOS->sprite.fontHeight() + OffsetBetweenLines);
    int visibleCount = (SCREEN_H - TopOffset) / (mainOS->sprite.fontHeight() + OffsetBetweenLines);
    mainOS->SetFontForSprite(mainOS->sprite);

    for (int i = 0; i < fileCount; i++)
    {
        if (i < mainOS->FileIndexSelected - 7) // not render all the list to save calculation
        {
            continue;
        }
        if (i > mainOS->FileIndexSelected + 7) // not render all the list to save calculation
        {
            continue;
        }

        mainOS->sprite.setCursor(12, 10 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos); // set position for file list

        mainOS->sprite.setTextColor(WHITE); // normal file white color
        //  sprite.print(FileFound[i]);
        // draw file name
        String ext = mainOS->GetExtensionLower(FileFound[i].c_str());
        if (IsFileMarked(FileFound[i]))
        {

            drawImageTransparent(1, 9 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos, 10, 10, SelectV, [&](int x, int y, uint16_t c)
                                 { mainOS->sprite.drawPixel(x, y, c); }, 0xF800);
        }
        else if (ext == "obj" || ext == "mtl")
        {

            // ToPrint += "Music File ";
            drawImageTransparent(1, 9 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos, 10, 10, Icon3dOBJ, [&](int x, int y, uint16_t c)
                                 { mainOS->sprite.drawPixel(x, y, c); }, 0xF800);
        }
        else if (ext == "mp3")
        {

            // ToPrint += "Music File ";
            drawImageTransparent(1, 9 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos, 10, 10, miniNote8, [&](int x, int y, uint16_t c)
                                 { mainOS->sprite.drawPixel(x, y, c); }, 0xF800);
        }
        else if (ext == "png" || ext == "gif" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
        {

            // ToPrint += "Music File ";
            drawImageTransparent(1, 9 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos, 10, 10, MiniPic8, [&](int x, int y, uint16_t c)
                                 { mainOS->sprite.drawPixel(x, y, c); }, 0xF800);
        }
        else if (ext == "wav")
        {

            // ToPrint += "Music File ";
            drawImageTransparent(1, 9 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos, 10, 10, WAVfileICON, [&](int x, int y, uint16_t c)
                                 { mainOS->sprite.drawPixel(x, y, c); }, 0xF800);
        }
        else if (ext == "ard" || ext == "nes" || ext == "gb" || ext == "gbc")
        {
            drawImageTransparent(1, 9 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos, 10, 10, GameFile, [&](int x, int y, uint16_t c)
                                 { mainOS->sprite.drawPixel(x, y, c); }, 0xF800);
        }

        else if (ext == "none")
        {
            mainOS->sprite.setTextColor(YELLOW); // folders yellow color

            drawImageTransparent(1, 9 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos, 10, 10, Folder8, [&](int x, int y, uint16_t c)
                                 { mainOS->sprite.drawPixel(x, y, c); }, 0xF800);
        }
        else if (ext == "txt" || ext == "esprc" || ext == "cpp" || ext == "hpp" || ext == "h" || ext == "py" || ext == "conf" || ext == "json" || ext == "ir")
        {

            drawImageTransparent(1, 9 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos, 10, 10, TextFile, [&](int x, int y, uint16_t c)
                                 { mainOS->sprite.drawPixel(x, y, c); }, 0xF800);
        }
        else
        {
            mainOS->sprite.drawRect(1, 9 + i * (mainOS->sprite.fontHeight() + OffsetBetweenLines) - cameraYpos, 10, 10, MAROON);
        }
        if (i == mainOS->FileIndexSelected)
        {
            mainOS->sprite.setTextColor(ORANGE);
        } // selected file otrange color

        // from GPT if file name long scroll the name
        String ToPrint = "";

        if (i == mainOS->FileIndexSelected) // focus on long file if long scroll

        {

            String name = mainOS->getFileNameFromPath(FileFound[i]);
            int maxChars = 38;
            if (mainOS->TextSizeInMenuesSAVE == 2)
            {
                maxChars = 17;
            }
            static uint32_t lastMove = 0;
            static uint32_t pauseStart = 0;
            static int offset = 0;
            static int dir = 1;
            static bool inPause = false;

            if (name.length() > maxChars)
            {
                if (inPause)
                {
                    if (millis() - pauseStart >= 1000)
                    {
                        inPause = false;
                        lastMove = millis();
                    }
                }
                else if (millis() - lastMove >= 150)
                {
                    offset += dir;

                    if (offset <= 0 || offset >= name.length() - maxChars)
                    {
                        offset = constrain(offset, 0, name.length() - maxChars);
                        dir *= -1;
                        inPause = true;
                        pauseStart = millis();
                    }

                    lastMove = millis();
                }

                ToPrint = name.substring(offset, offset + maxChars);
            }
            else
            {
                ToPrint = name;
            }

            if (ext == "none") // if folder print like [folder]
            {
                // mainOS->sprite.print(String("[" + reverseHebrewOnly(ToPrint) + "]"));
                mainOS->sprite.print(String("[" + ToPrint + "]"));
            }
            else
            {
                // mainOS->sprite.print(reverseHebrewOnly(ToPrint));
                mainOS->sprite.print(ToPrint);
            }
        }
        else // file that is not in focus always not scroll
        {

            ToPrint += mainOS->getFileNameFromPath(FileFound[i]);
            if (ext == "none") // if folder print like [folder]
            {
                mainOS->sprite.print(String("[" + ToPrint + "]"));
            }
            else
            {
                mainOS->sprite.print(ToPrint);
            }
        }
        // end from GPT if file name long scroll the name
    }

    // draw file path with horizontal scroll if too long
    mainOS->sprite.setCursor(0, 2);
    mainOS->sprite.fillRect(0, 0, SCREEN_W, mainOS->sprite.fontHeight() + 4, BLACK);
    mainOS->sprite.setTextColor(ORANGE);

    String PathName = mainOS->currentPath;
    int maxChars = SCREEN_W / (6 * mainOS->TextSizeInMenuesSAVE);
    static int offsetPath = 0;
    static int dirPath = 1;
    static uint32_t lastMovePath = 0;
    static bool inPausePath = false;
    static uint32_t pauseStartPath = 0;

    if (PathName.length() > maxChars)
    {
        if (inPausePath)
        {
            if (millis() - pauseStartPath >= 1000)
            {
                inPausePath = false;
                lastMovePath = millis();
            }
        }
        else if (millis() - lastMovePath >= 200)
        {
            offsetPath += dirPath;
            if (offsetPath <= 0 || offsetPath >= PathName.length() - maxChars)
            {
                offsetPath = constrain(offsetPath, 0, PathName.length() - maxChars);
                dirPath *= -1;
                inPausePath = true;
                pauseStartPath = millis();
            }
            lastMovePath = millis();
        }

        mainOS->sprite.print(PathName.substring(offsetPath, offsetPath + maxChars));
    }
    else
    {
        mainOS->sprite.print(PathName);
    }

    mainOS->sprite.drawRect(0, 0, SCREEN_W, mainOS->sprite.fontHeight() + 4, MAROON);

    // end file path
    // --- Scroll Bar ---
    mainOS->sprite.fillRect(SCREEN_W - 6, 0, 6, SCREEN_H, DARKGREY);
    if (fileCount > visibleCount)
    {
        int barHeight = (visibleCount * (SCREEN_H - TopOffset)) / fileCount;
        int firstVisibleIndex = cameraYpos / (mainOS->sprite.fontHeight() + OffsetBetweenLines);
        int barY = (firstVisibleIndex * (SCREEN_H - TopOffset)) / fileCount;

        barY = constrain(barY, 0, (SCREEN_H - TopOffset) - barHeight);

        int barX = SCREEN_W - 6;
        mainOS->sprite.fillRect(barX, barY, 5, barHeight, ORANGE);
    }
    // draw menu if visable
    if (RenderFileOptionMenu)
    {
        DrawFileOptionMenu();
    }

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

void FileBrowser::Mark_UnmarkFile()
{
    const String &id = FileFound[mainOS->FileIndexSelected];

    auto it = std::find(File_ID_Marked.begin(),
                        File_ID_Marked.end(),
                        id);

    if (it == File_ID_Marked.end())
    {
        File_ID_Marked.push_back(id);
    }
    else
    {
        File_ID_Marked.erase(it);
    }
}
bool FileBrowser::IsFileMarked(const String &id)
{
    return std::find(File_ID_Marked.begin(),
                     File_ID_Marked.end(),
                     id) != File_ID_Marked.end();
}

void FileBrowser::DrawFileOptionMenu()
{
    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextSize(mainOS->TextSizeInMenuesSAVE);
    int YdistanceBetweenMenu = mainOS->sprite.fontHeight() + 2;
    int topOffset = 30;

    cameraYposMenu = InFileOptionMenuIndex * YdistanceBetweenMenu - topOffset;
    if (cameraYposMenu < 0)
        cameraYposMenu = 0;

    mainOS->sprite.fillRect(0, 0, 240, 135, ORANGE);
    mainOS->sprite.drawRect(0, 0, 240, 135, MAROON);
    mainOS->sprite.setTextWrap(false);

    int cursorX = 5;
    int cursorY = 3 - cameraYposMenu;
    mainOS->sprite.setCursor(cursorX, cursorY);
    File f;
    if (mainOS->FileOpenFromMainMenu)
    {
        f = SD.open(mainOS->FileSelectedInFS);
    }
    else
    {
        f = SD.open(FileFound[mainOS->FileIndexSelected]);
    }
    if (!f.isDirectory())
    {
        mainOS->sprite.setTextColor(MAROON);

        float sizeMB = f.size() / (1024.0 * 1024.0);
        mainOS->sprite.printf("Size: %.2f MB", sizeMB);

        mainOS->sprite.setCursor(
            cursorX,
            mainOS->sprite.getCursorY() + YdistanceBetweenMenu);
    }

    for (int i = 0; i < FileOptionMenuItems.size(); i++)
    {
        if (InFileOptionMenuIndex == i)
            mainOS->sprite.setTextColor(RED);
        else
            mainOS->sprite.setTextColor(BLACK);

        String menuName;
        if (FileOptionMenuItems[i] == "Move here")
        {
            menuName = "Move ";
            menuName += mainOS->getFileNameFromPath(FileOrFolderToMove);
            menuName += " Here";
        }
        else if (FileOptionMenuItems[i] == "Copy here")
        {
            menuName = "Copy ";
            menuName += mainOS->getFileNameFromPath(FileOrFolderToMove);
            menuName += " Here";
        }
        else
        {
            menuName = FileOptionMenuItems[i];
        }

        mainOS->sprite.print(menuName);

        mainOS->sprite.setCursor(
            cursorX,
            mainOS->sprite.getCursorY() + YdistanceBetweenMenu);
    }

    mainOS->sprite.setTextColor(YELLOW);
}

bool FileBrowser::hasExtension(const char *filename, const char *ext)
{
    size_t len = strlen(filename);
    size_t extLen = strlen(ext);
    if (len < extLen)
        return false;
    return strcasecmp(filename + len - extLen, ext) == 0;
}

bool FileBrowser::fileExists(const String &path)
{

    return SD.exists(path);
}

void FileBrowser::ShowCurrentFilesInDir(fs::FS &fs, const char *dirname)
{
    // Print the directory being listed to serial monitor for draw
    // Serial.printf("Listing directory: %s\n", dirname);
    // Open the root directory specified by dirname
    File root = fs.open(dirname);

    // Check if opening the directory failed
    if (!root)
    {
        Serial.println("Failed to open directory");
        return; // Exit function if directory can't be opened
    }

    // Check if the opened path is actually a directory
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        root.close(); // Close the file handle
        return;       // Exit function if it's not a directory
    }
    // Open the first file in the directory
    File file = root.openNextFile();
    fileCount = 0;
    // Loop through all files in the directory while fileCount is less than MAX_FILES
    while (file && fileCount < MAX_FILES)
    {
        // Check if current item is a directory
        if (file.isDirectory())
        {
            const char *name = file.name(); // Get the name of the directory
                                            // Serial.print("DIR : "); // Print "DIR :" prefix
                                            // Serial.println(name); // Print the directory name

            // Skip system directories that shouldn't be scanned
            if (strcmp(name, "System Volume Information") == 0 ||
                strcmp(name, "LOST.DIR") == 0 ||
                strcmp(name, "lost+found") == 0)
            {
                file.close();               // Close current file handle
                file = root.openNextFile(); // Get next file in directory
                continue;                   // Skip to next iteration of while loop
            }
            // if valid folder
            else
            {
                const char *name = file.name(); // Get the filename

                // String CutrrentScan = name;

                String fullPath = String(dirname); // Start with directory path
                if (!fullPath.endsWith("/"))       // If no trailing slash
                    fullPath += "/";               // Add slash
                fullPath += name;                  // Append filename

                // Store the complete file path in audioFiles array at current fileCount index
                FileFound[fileCount] = fullPath;

                // Increment file counter to track how many MP3 files we've found
                fileCount++;
            }
        }
        else // if normal file
        {
            // Current item is a file (not a directory)
            const char *name = file.name(); // Get the filename

            // Create full path to the file by combining directory path and filename
            String fullPath = String(dirname); // Start with directory path
            if (!fullPath.endsWith("/"))       // If no trailing slash
                fullPath += "/";               // Add slash
            fullPath += name;                  // Append filename

            // Store the complete file path in audioFiles array at current fileCount index
            FileFound[fileCount] = fullPath;

            // Increment file counter to track how many MP3 files we've found
            fileCount++;
            //}
        }

        // Close current file handle and get next file from directory
        file.close();
        file = root.openNextFile();
    }
    if (fileCount == 0)
    {
        // mainOS->ShowOnScreenMessege("folder empty", 100);
        FolderEmpty = true;
    }
    else
    {
        // mainOS->ShowOnScreenMessege("folder not empty", 100);

        FolderEmpty = false;
    }
    // Close the root directory when done scanning all files
    root.close();
    // SortFiles();
}
fs::FS &FileBrowser::GetActiveFS(bool browseLittleFS)
{
    return browseLittleFS ? (fs::FS &)LittleFS : (fs::FS &)SD;
}

bool FileBrowser::isFolderEmpty(File dir)
{
    if (!dir || !dir.isDirectory())
        return false;

    File entry = dir.openNextFile();

    if (!entry)
        return true;

    entry.close();
    return false;
}

String FileBrowser::createFolder(fs::FS &fs, const String &parentFolder, const String &baseName)
{

    int counter = 1;
    String path = parentFolder + "/" + baseName;

    while (fs.exists(path))
    {
        path = parentFolder + "/" + baseName + String(counter);
        counter++;
    }

    if (fs.mkdir(path))
        return path;
    return "";
}

bool FileBrowser::CopyFile(fs::FS &fs, const String &srcPath, const String &dstPath)
{
    File src = fs.open(srcPath, FILE_READ);
    if (!src)
        return false;

    File dst = fs.open(dstPath, FILE_WRITE);
    if (!dst)
    {
        src.close();
        return false;
    }

    uint8_t buf[512];
    size_t n;
    while ((n = src.read(buf, sizeof(buf))) > 0)
        dst.write(buf, n);

    src.close();
    dst.close();
    return true;
}
bool FileBrowser::CopyFolder(fs::FS &fs, const String &srcFolder, const String &dstFolder)
{
    fs.mkdir(dstFolder);

    File dir = fs.open(srcFolder);
    if (!dir)
        return false;

    File file = dir.openNextFile();
    while (file)
    {
        String nameOnly = file.name();
        int lastSlash = nameOnly.lastIndexOf('/');
        if (lastSlash >= 0)
            nameOnly = nameOnly.substring(lastSlash + 1);
        mainOS->ShowOnScreenMessege(String("Copy: " + nameOnly), 1);

        String fullSrc = srcFolder + "/" + nameOnly;
        String fullDst = dstFolder + "/" + nameOnly;

        if (file.isDirectory())
        {
            if (!CopyFolder(fs, fullSrc, fullDst))
                return false;
        }
        else
        {
            if (!CopyFile(fs, fullSrc, fullDst))
                return false;
        }

        file = dir.openNextFile();
    }

    dir.close();
    return true;
}

bool FileBrowser::MoveOrCopy(fs::FS &fs, const String &fileOrFolderPath, const String &dstFolderPath, bool Copy)
{
    if (!fs.exists(fileOrFolderPath))
    {
        mainOS->ShowOnScreenMessege("Source does not exist!", 2000);

        return false;
    }

    if (!fs.exists(dstFolderPath))
    {
        mainOS->ShowOnScreenMessege("Destination folder does not exist!", 2000);
        return false;
    }

    File f = fs.open(fileOrFolderPath);
    if (!f)
        return false;
    bool isDir = f.isDirectory();
    f.close();

    int slashIndex = fileOrFolderPath.lastIndexOf('/');
    String name = (slashIndex >= 0) ? fileOrFolderPath.substring(slashIndex + 1) : fileOrFolderPath;

    String dstPath = dstFolderPath + "/" + name;
    dstPath = MakeUniquePath(fs, dstPath); // Cheack if this line and function work

    if (Copy)
    {
        if (isDir)
        {
            return CopyFolder(fs, fileOrFolderPath, dstPath);
        }
        else
        {
            mainOS->ShowOnScreenMessege(String("Copy: " + mainOS->getFileNameFromPath(fileOrFolderPath)), 1);
            return CopyFile(fs, fileOrFolderPath, dstPath);
        }
    }
    else
    {
        return fs.rename(fileOrFolderPath, dstPath);
    }
}
String FileBrowser::MakeUniquePath(fs::FS &fs, const String &dstPath)
{
    if (!fs.exists(dstPath))
        return dstPath;

    int dot = dstPath.lastIndexOf('.');
    String base = (dot >= 0) ? dstPath.substring(0, dot) : dstPath;
    String ext = (dot >= 0) ? dstPath.substring(dot) : "";

    int index = 1;
    String newPath;
    do
    {
        newPath = base + "_" + String(index) + ext;
        index++;
    } while (fs.exists(newPath));

    // mainOS->ShowOnScreenMessege("File exist in destination folder", 1500);
    return newPath;
}
void FileBrowser::Show_BMP_picture(String fileName)
{
    File bmpFile = SD.open(fileName, FILE_READ);
    if (!bmpFile)
        return;

    uint8_t header[54];
    bmpFile.read(header, 54);

    int32_t width = *(int32_t *)&header[18];
    int32_t height = *(int32_t *)&header[22];
    uint16_t bpp = *(uint16_t *)&header[28];    // Bits per pixel
    uint32_t offset = *(uint32_t *)&header[10]; // Where the pixel data actually starts

    // Calculate bytes per pixel
    int bytesPerPixel = bpp / 8;

    // BMP row size must be a multiple of 4 bytes
    int rowSize = (width * bytesPerPixel + 3) & ~3;

    // Buffer for one full row
    uint8_t lineBuffer[width * bytesPerPixel];

    M5.Lcd.startWrite();
    for (int y = 0; y < abs(height); y++)
    {
        // Handle bottom-up vs top-down
        int pos = (height > 0) ? (height - 1 - y) : y;
        bmpFile.seek(offset + (pos * rowSize));
        bmpFile.read(lineBuffer, width * bytesPerPixel);

        for (int x = 0; x < width; x++)
        {
            uint8_t b = lineBuffer[x * bytesPerPixel];
            uint8_t g = lineBuffer[x * bytesPerPixel + 1];
            uint8_t r = lineBuffer[x * bytesPerPixel + 2];
            // If 32-bit, the 4th byte (alpha) is simply skipped/ignored

            M5.Lcd.drawPixel(x, y, M5.Lcd.color565(r, g, b));
        }
    }
    M5.Lcd.endWrite();
    bmpFile.close();
}
/*
void FileBrowser::Show_BMP_picture(String fileName) // only 24 bit
{
    File bmpFile = SD.open(fileName, FILE_READ);
    if (!bmpFile)
    {
        Serial.println("Error: Could not open file");
        return;
    }

    uint8_t header[54];
    bmpFile.read(header, 54);

    int32_t width = *(int32_t *)&header[18];
    int32_t height = *(int32_t *)&header[22];

    bool flip = true;
    if (height < 0)
    {
        height = -height;
        flip = false;
    }

    int rowSize = (width * 3 + 3) & ~3;
    int padding = rowSize - (width * 3);

    uint8_t lineBuffer[width * 3];

    M5.Lcd.startWrite();
    for (int y = 0; y < height; y++)
    {
        int pos = flip ? (height - 1 - y) : y;

        bmpFile.seek(54 + (pos * rowSize));
        bmpFile.read(lineBuffer, width * 3);

        for (int x = 0; x < width; x++)
        {
            uint8_t b = lineBuffer[x * 3];
            uint8_t g = lineBuffer[x * 3 + 1];
            uint8_t r = lineBuffer[x * 3 + 2];

            M5.Lcd.drawPixel(x, y, M5.Lcd.color565(r, g, b));
        }
    }

    M5.Lcd.endWrite();
    bmpFile.close();
} */
/*
void FileBrowser::EmulateSdCardAsUSB()
{
    // USB MSC
                CurrentImageOpenType = CurrentlyImageOpen::USB;

    M5Cardputer.Display.fillScreen(YELLOW);
    uint32_t secSize = SD.sectorSize();
    uint32_t numSectors = SD.numSectors();
    msc.vendorID("ESP32");
    msc.productID("USB_MSC");
    msc.productRevision("1.0");
    msc.onRead(usbReadCallback);
    msc.onWrite(usbWriteCallback);
    msc.onStartStop(usbStartStopCallback);
    msc.mediaPresent(true);
    msc.begin(numSectors, secSize);

    USB.onEvent([](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
                {
    if (event_base == ARDUINO_USB_EVENTS) {
      auto* data = reinterpret_cast<arduino_usb_event_data_t*>(event_data);
      switch (event_id) {
        case ARDUINO_USB_STARTED_EVENT:
          drawLedIndicator(true);
          break;
        case ARDUINO_USB_STOPPED_EVENT:
          drawLedIndicator(false);
          break;
        case ARDUINO_USB_SUSPEND_EVENT:
          displayMessage("USB suspend");
          break;
        case ARDUINO_USB_RESUME_EVENT:
          displayMessage("USB resume");
          break;
        default:
          break;
      }
    } });

    displayMessage("USB Stick is ready");
    drawUSBStickIcon();
    drawLedIndicator(false);
    USB.begin();
    ShowPicOnScreen = true;
} */

void FileBrowser::RefrashImage()
{
    M5Cardputer.Display.fillScreen(TFT_BLACK);

    switch (CurrentImageOpenType)
    {
    case CurrentlyImageOpen::PNG:
        DrawPNG(FileFound[mainOS->FileIndexSelected]);
        break;
    case CurrentlyImageOpen::GIF:
        /*         RenderFileOptionMenu = false;
                gif.begin(LITTLE_ENDIAN_PIXELS);

                GifPathToPlay = FileFound[mainOS->FileIndexSelected];
                mainOS->showTopBar = false;
                RenderGif = true;
                ShowPicOnScreen = true; */
        break;
    case CurrentlyImageOpen::JPG:
        drawJpeg(FileFound[mainOS->FileIndexSelected].c_str());

        break;
    case CurrentlyImageOpen::USB:
        ESP.restart();

        break;
    default:
        break;
    }
}
void FileBrowser::GoUpOneFolder()
{
    if (mainOS->currentPath == "/")
    {
        GlobalParentClass::BackToMainMenu();
        return;
    }
    mainOS->ShowOnScreenMessege("Loading...", 1);
    int lastSlash = mainOS->currentPath.lastIndexOf('/', mainOS->currentPath.length() - 1);

    if (lastSlash <= 0)
    {
        mainOS->currentPath = "/";
    }
    else

    {
        mainOS->currentPath = mainOS->currentPath.substring(0, lastSlash);
    }
    fileCount = 0;

    if (!mainOS->folderIndexHistory.empty())
    {
        mainOS->FileIndexSelected = mainOS->folderIndexHistory.back();
        mainOS->folderIndexHistory.pop_back();
    }
    else
    {
        mainOS->FileIndexSelected = 0;
    }

    ShowCurrentFilesInDir(GetActiveFS(BrowseLittleFS), mainOS->currentPath.c_str());
}

void FileBrowser::FileOfFolderSelected()
{
    FileOptionMenuItems.clear();
    String full_Path_Of_The_Current_Selected;
    if (mainOS->FileOpenFromMainMenu) // if selected file from main menu
    {
        full_Path_Of_The_Current_Selected = mainOS->FileSelectedInFS;
    }
    else // if select file from the file explorer
    {
        full_Path_Of_The_Current_Selected = FileFound[mainOS->FileIndexSelected];
    }

    File Actual_File_Or_folder = SD.open(full_Path_Of_The_Current_Selected);
    FileOptionMenuItems.clear();
    // FolderEmpty = false;
    if (Actual_File_Or_folder.isDirectory())
    {
        /*        if (isFolderEmpty(Actual_File_Or_folder) && fileCount <= 0) // if current folder focus is empty and no files are showen
               {
                   FolderEmpty = true;
               } */

        if (InFileMarkMode)
        {
            if (!mainOS->FileOpenFromMainMenu)

            {
                FileOptionMenuItems.push_back("Mark/Unmark");
            }
            FileOptionMenuItems.push_back("Open Folder");
            if (!mainOS->FileOpenFromMainMenu)

            {
                FileOptionMenuItems.push_back("Move Marked Here");
                FileOptionMenuItems.push_back("Copy Marked Here");
                FileOptionMenuItems.push_back("Delete Marked Files");
            }
        }
        else if (true)
        {
            FileOptionMenuItems.push_back("Open Folder");
            if (mainOS->FileOpenFromMainMenu)
            {

                FileOptionMenuItems.push_back("Remove From Main Menu");
            }
            else
            {
                FileOptionMenuItems.push_back("Add To Main Menu");
            }
        }
    }
    else // is file
    {
        String exte = mainOS->GetExtensionLower(full_Path_Of_The_Current_Selected.c_str());
        if (InFileMarkMode)
        {
            if (!mainOS->FileOpenFromMainMenu)

            {
                FileOptionMenuItems.push_back("Mark/Unmark");
                FileOptionMenuItems.push_back("Move Marked Here");
                FileOptionMenuItems.push_back("Copy Marked Here");
                FileOptionMenuItems.push_back("Delete Marked Files");
            }
        }
        else if (exte == "mp3")
        {
            FileOptionMenuItems.push_back("Play In Player");
        }

        else if (exte == "ard" || exte == "nes" || exte == "gb" || exte == "gbc")
        {
            FileOptionMenuItems.push_back("Play Game");
            if (mainOS->FileOpenFromMainMenu)
            {
                FileOptionMenuItems.push_back("Remove From Main Menu");
            }
            else
            {
                FileOptionMenuItems.push_back("Add To Main Menu");
            }
        }
        else if (exte == "wav")
        {
            FileOptionMenuItems.push_back("Play WAV");
        }
        else if (exte == "esprc")
        {
            FileOptionMenuItems.push_back("Open ESP Remote Control");
            FileOptionMenuItems.push_back("Open Text");
            FileOptionMenuItems.push_back("Edit Text");
        }
        else if (exte == "vlw")
        {
            FileOptionMenuItems.push_back("Set Font");
        }
        else if (exte == "obj")
        {
            FileOptionMenuItems.push_back("Open 3D model");
        }
        else if (exte == "mtl")
        {
            FileOptionMenuItems.push_back("Open 3D model");
        }
        else if (exte == "gif")
        {
            FileOptionMenuItems.push_back("Open GIF Image");
            FileOptionMenuItems.push_back("Open GIF Image In Painter");
        }
        else if (exte == "png")
        {
            FileOptionMenuItems.push_back("Open PNG Image");
            FileOptionMenuItems.push_back("Open PNG Image in Painter");
            /*             if (mainOS->FileOpenFromMainMenu)
                        {
                            FileOptionMenuItems.push_back("Remove From Main Menu");
                        }
                        else
                        {
                            FileOptionMenuItems.push_back("Add To Main Menu");
                        } */
        }
        else if (exte == "mc")
        {
            FileOptionMenuItems.push_back("Edit Music File");
            if (mainOS->FileOpenFromMainMenu)
            {
                FileOptionMenuItems.push_back("Remove From Main Menu");
            }
            else
            {
                FileOptionMenuItems.push_back("Add To Main Menu");
            }
        }
        else if (exte == "bmp")
        {
            FileOptionMenuItems.push_back("Open BMP Image");
            FileOptionMenuItems.push_back("Edit BMP Image");
            if (mainOS->FileOpenFromMainMenu)
            {
                FileOptionMenuItems.push_back("Remove From Main Menu");
            }
            else
            {
                FileOptionMenuItems.push_back("Add To Main Menu");
            }
        }
        else if (exte == "jpg" || exte == "jpeg")
        {
            FileOptionMenuItems.push_back("Open JPEG Image");
        }
        else if (exte == "ir")
        {
            FileOptionMenuItems.push_back("Open Infrared Remote Control");
            FileOptionMenuItems.push_back("Edit Infrared File");
            FileOptionMenuItems.push_back("Open Text");
            FileOptionMenuItems.push_back("Edit Text");
        }
        else if (exte == "thm")
        {
            FileOptionMenuItems.push_back("Set New theme");
            FileOptionMenuItems.push_back("Edit Theme");
        }
        else if (exte == "txt" || exte == "csv" || exte == "json" || exte == "conf" || exte == "sub" || exte == "py" || exte == "h" || exte == "hpp" || exte == "cpp")
        {
            FileOptionMenuItems.push_back("Open Text");
            FileOptionMenuItems.push_back("Edit Text");
            if (mainOS->FileOpenFromMainMenu)
            {
                FileOptionMenuItems.push_back("Remove From Main Menu");
            }
            else
            {
                FileOptionMenuItems.push_back("Add To Main Menu");
            }
        }
    }

    FileOptionMenuItems.push_back("Close");
    if (!FolderEmpty && !InFileMarkMode)
    {
        if (!mainOS->FileOpenFromMainMenu)

        {
            FileOptionMenuItems.push_back("Rename");
            FileOptionMenuItems.push_back("Delete");
            FileOptionMenuItems.push_back("Open HEX");
        }
    }

    if (FileOrFolderToMove.isEmpty() && !FolderEmpty && !InFileMarkMode) // no path to move to found
    {
        if (!mainOS->FileOpenFromMainMenu)

        {
            FileOptionMenuItems.push_back("Move Or Copy To..");
        }
    }
    else if (!FileOrFolderToMove.isEmpty() && !InFileMarkMode)
    {
        if (!mainOS->FileOpenFromMainMenu)

        {
            FileOptionMenuItems.push_back("Move here");
            FileOptionMenuItems.push_back("Copy here");
            FileOptionMenuItems.push_back("Don't Move Or Copy");
        }
    }
    if (!mainOS->FileOpenFromMainMenu)

    {
        FileOptionMenuItems.push_back("Create file");

        FileOptionMenuItems.push_back("Create Folder");
        FileOptionMenuItems.push_back("Toggle Mark File Mode");

        FileOptionMenuItems.push_back("Change Font Size");
    }
    // FileOptionMenuItems.push_back("Emulate SD card As Disk On key");
    if (mainOS->FileOpenFromMainMenu)
    {
        FileOptionMenuItems.push_back("Move UP");
        FileOptionMenuItems.push_back("Move DOWN");
    }
    OpenFileMenu();
}

void FileBrowser::SelectMenuItem()
{
    if (FileOptionMenuItems[InFileOptionMenuIndex] == "Close")
    {
        if (!mainOS->FileOpenFromMainMenu)

        {
            CloseFileMenu();
        }
        else
        {
            GlobalParentClass::BackToMainMenu();
            return;
        }
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Rename")
    {
        mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        //   mainOS->currentPath = FileFound[mainOS->FileIndexSelected];

        mainOS->ChangeMenu(new TextEditor(mainOS));
        // mainOS->currentApp

        TextEditor *child = static_cast<TextEditor *>(mainOS->currentApp);
        child->RenameMode = true;
        child->FileName = mainOS->getFileNameFromPath(mainOS->FileSelectedInFS);
        child->Begin();
    }

    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Open Infrared Remote Control")
    {
        mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        mainOS->ChangeMenu(new IRSenderApp(mainOS));
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Edit Infrared File")
    {
        mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        mainOS->ChangeMenu(new IR_Editor(mainOS));
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Edit BMP Image" || FileOptionMenuItems[InFileOptionMenuIndex] == "Open PNG Image in Painter" || FileOptionMenuItems[InFileOptionMenuIndex] == "Open GIF Image In Painter")
    {
        mainOS->ShowOnScreenMessege("Loading Image to paint", 10);

        if (mainOS->FileOpenFromMainMenu)
        {
            // mainOS->FileSelectedInFS=mainOS->file
        }
        else
        {
            mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        }
        mainOS->EditFromFile = true;
        mainOS->ChangeMenu(new Painter(mainOS));
        RenderFileOptionMenu = false;
        return;
    }

    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Edit Music File")
    {
        // mainOS->ShowOnScreenMessege("Loading Image to paint", 10);

        if (mainOS->FileOpenFromMainMenu)
        {
        }
        else
        {
            mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        }

        mainOS->EditFromFile = true;
        mainOS->ChangeMenu(new MusicCreator(mainOS));
        RenderFileOptionMenu = false;
        return;
    }

    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Play Game")
    {

        if (mainOS->FileOpenFromMainMenu) // if selected file from main menu
        {
            // full_Path_Of_The_Current_Selected = mainOS->FileSelectedInFS;
            // mainOS->RemoveFilePathFromArrayOfMAinMenu(mainOS->FileSelectedInFS);
        }
        else // if select file from the file explorer
        {
            // mainOS->RemoveFilePathFromArrayOfMAinMenu(FileFound[mainOS->FileIndexSelected]);
            mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        }

        mainOS->FastLoadGameFromFileExplorer = true;
        mainOS->saveSettings();

        mainOS->ChangeMenu(new GamesV2(mainOS));
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Add To Main Menu")
    {

        // mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        mainOS->AddFilePathToArrayOfMAinMenu(FileFound[mainOS->FileIndexSelected]);
    }

    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Remove From Main Menu")
    {
        if (mainOS->FileOpenFromMainMenu) // if selected file from main menu
        {
            // full_Path_Of_The_Current_Selected = mainOS->FileSelectedInFS;
            mainOS->RemoveFilePathFromArrayOfMAinMenu(mainOS->FileSelectedInFS);
        }
        else // if select file from the file explorer
        {
            mainOS->RemoveFilePathFromArrayOfMAinMenu(FileFound[mainOS->FileIndexSelected]);
        }
        // mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
    }

    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Open ESP Remote Control")
    {
#if defined(COMPILE_WIFI_THINGS)
        mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        mainOS->ChangeMenu(new ESP_RemoteControlFile(mainOS));
#endif
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Set Font")
    {
        if (SD.open(FileFound[mainOS->FileIndexSelected], FILE_READ, false))
        {
            M5Cardputer.Display.print(" vlw detected\n");
        }
        else
        {
            M5Cardputer.Display.print(" vlw not detected\n");
        }

        /*         SDFS::FS &fs_reference = SD;
                if (M5Cardputer.Display.loadFont(fs_reference,FileFound[mainOS->FileIndexSelected].c_str()))
                {
                }
                else
                {
                    M5Cardputer.Display.print(" vlw not loaded\n");
                } */
        delay(1000);
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Open Text")
    {
        mainOS->TextEditorGetTextFromFile = true;

        if (mainOS->FileOpenFromMainMenu)
        {
        }
        else
        {
            mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        }

        mainOS->ChangeMenu(new TextEditor(mainOS));
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Open HEX")
    {
        mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        mainOS->ChangeMenu(new HEX_Editor(mainOS));
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Open 3D model")
    {
        mainOS->DeleteMusicPlayer = true;
        mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        mainOS->ChangeMenu(new My3dRenderer(mainOS));
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Mark/Unmark")
    {
        if (InFileMarkMode)
        {
            Mark_UnmarkFile();
        }
        CloseFileMenu();
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Run BIN file")
    {
        return;
        // installTempBin(FileFound[mainOS->FileIndexSelected]);
        return;

        File bin = SD.open(FileFound[mainOS->FileIndexSelected]);
        if (!bin)
            return;

        const esp_partition_t *ota =
            esp_ota_get_next_update_partition(NULL);

        if (!Update.begin(bin.size(), U_FLASH, ota->address))
        {
            bin.close();
            return;
        }

        Update.writeStream(bin);
        bin.close();

        if (!Update.end(true))
            return;

        ESP.restart();
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Edit Text")
    {
        mainOS->TextEditorGetTextFromFile = true;

        if (mainOS->FileOpenFromMainMenu)
        {
        }
        else
        {
            mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];
        }

        mainOS->ChangeMenu(new TextEditor(mainOS));
        // mainOS->currentApp

        TextEditor *child = static_cast<TextEditor *>(mainOS->currentApp);
        child->EditMode = true;
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Edit Theme")
    {
        mainOS->inEditThemeFromFileExplorer = true;
        mainOS->FileSelectedInFS = FileFound[mainOS->FileIndexSelected];

        mainOS->LoadTheme(mainOS->FileSelectedInFS, false);

        mainOS->ChangeMenu(new Theme_Maneger(mainOS));
    }

    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Delete")
    {
        if (!os.AskSomthing("Are you sure You want to delete file?"))
        {
            CloseFileMenu();
            return;
        }
        if (!InFileMarkMode)

        {
            String full_Path_Of_The_Current_Selected = FileFound[mainOS->FileIndexSelected];
            mainOS->deleteFromSd(SD, full_Path_Of_The_Current_Selected);
            //   targetCameraY = 0;
            ShowCurrentFilesInDir(GetActiveFS(BrowseLittleFS), mainOS->currentPath.c_str());
        }

        CloseFileMenu();
    }

    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Move UP") // move up in main menu
    {

        MoveInVectorArray(mainOS->filesInMainMenu, mainOS->FileSelectedInFS, true);
        mainOS->saveSettings();
        GlobalParentClass::BackToMainMenu();
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Move DOWN") // move up in main menu
    {
        MoveInVectorArray(mainOS->filesInMainMenu, mainOS->FileSelectedInFS, false);
        mainOS->saveSettings();

        GlobalParentClass::BackToMainMenu();
    }

    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Create file")
    {
        String MyFileName = mainOS->AskFromUserForString("Choose Text File Name", true, false, false);
        MyFileName += ".txt";
        mainOS->createFile(SD, mainOS->currentPath, MyFileName);
        // ShowCurrentFilesInDir(mainOS->currentPath.c_str());
        mainOS->ChangeMenu(new FileBrowser(mainOS));

        RenderFileOptionMenu = false;
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Create Folder")
    {
        String MyFolderName = mainOS->AskFromUserForString("Choose Folder Name", true, false, false);
        createFolder(GetActiveFS(BrowseLittleFS), mainOS->currentPath, MyFolderName);
        mainOS->ChangeMenu(new FileBrowser(mainOS));
        RenderFileOptionMenu = false;
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Move Or Copy To..")
    {
        if (!InFileMarkMode)
        {
            FileOrFolderToMove = FileFound[mainOS->FileIndexSelected];
            RenderFileOptionMenu = false;
        }
        else
        {
            multipleFileOrFolderToMove = true;
            RenderFileOptionMenu = false;
        }
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Toggle Mark File Mode")
    {
        InFileMarkMode = !InFileMarkMode;
        if (!InFileMarkMode)
        {
            File_ID_Marked.clear();
        }
        CloseFileMenu();
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Move Marked Here")
    {
        if (InFileMarkMode)
        {
            for (size_t i = 0; i < File_ID_Marked.size(); i++)
            {
                MoveOrCopy(SD, File_ID_Marked[i], mainOS->currentPath, false);
                // FileOrFolderToMove = "";
                RenderFileOptionMenu = false;
            }
            InFileMarkMode = false;
            File_ID_Marked.clear();
            mainOS->ChangeMenu(new FileBrowser(mainOS));
        }
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Copy Marked Here")
    {
        if (InFileMarkMode)
        {
            for (size_t i = 0; i < File_ID_Marked.size(); i++)
            {
                MoveOrCopy(SD, File_ID_Marked[i], mainOS->currentPath, true);
                // FileOrFolderToMove = "";
                RenderFileOptionMenu = false;
            }
            InFileMarkMode = false;
            File_ID_Marked.clear();
            mainOS->ChangeMenu(new FileBrowser(mainOS));
        }
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Delete Marked Files")
    {
        if (!os.AskSomthing("Are you sure You want to delete Marked Files?"))
        {
            CloseFileMenu();
            return;
        }
        if (InFileMarkMode)
        {

            for (size_t i = 0; i < File_ID_Marked.size(); i++)
            {
                String full_Path_Of_The_Current_Selected = File_ID_Marked[i];
                mainOS->deleteFromSd(SD, full_Path_Of_The_Current_Selected);
            }
            InFileOptionMenuIndex = 0;
            ShowCurrentFilesInDir(GetActiveFS(BrowseLittleFS), mainOS->currentPath.c_str());
            InFileMarkMode = false;
            File_ID_Marked.clear();
            CloseFileMenu();
        }
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Move here")
    {
        if (!InFileMarkMode)

        {
            MoveOrCopy(SD, FileOrFolderToMove, mainOS->currentPath, false);
            FileOrFolderToMove = "";
            RenderFileOptionMenu = false;
            mainOS->ChangeMenu(new FileBrowser(mainOS));
        }
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Copy here")
    {
        if (!InFileMarkMode)

        {
            MoveOrCopy(SD, FileOrFolderToMove, mainOS->currentPath, true);
            FileOrFolderToMove = "";
            RenderFileOptionMenu = false;
            mainOS->ChangeMenu(new FileBrowser(mainOS));
        }
    }
    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Don't Move Or Copy")
    {
        if (!InFileMarkMode)

        {
            FileOrFolderToMove = "";
            RenderFileOptionMenu = false;
        }
        else
        {
            multipleFileOrFolderToMove = false;
        }
    }

    else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Change Font Size")
    {

        if (mainOS->TextSizeInMenuesSAVE == 1)
        {
            mainOS->TextSizeInMenuesSAVE = 2;
            mainOS->saveSettings();
        }
        else
        {
            mainOS->TextSizeInMenuesSAVE = 1;
            mainOS->saveSettings();
        }
    }
    /*     else if (FileOptionMenuItems[InFileOptionMenuIndex] == "Emulate SD card As Disk On key")
        {
            EmulateSdCardAsUSB();
        } */
    else
    {
        ExecuteFileOrFolder();
        CloseFileMenu();
    }
}

void FileBrowser::ExecuteFileOrFolder()
{
    FileOptionMenuItems.clear();

    String full_Path_Of_The_Current_Selected;

    // ← הוסף את זה:
    if (mainOS->FileOpenFromMainMenu)
        full_Path_Of_The_Current_Selected = mainOS->FileSelectedInFS;
    else
        full_Path_Of_The_Current_Selected = FileFound[mainOS->FileIndexSelected];

    File Actual_File_Or_folder = SD.open(full_Path_Of_The_Current_Selected);
    if (Actual_File_Or_folder.isDirectory())
    {
        // if (!fullPath.endsWith("/"))
        //     fullPath += "/";
        mainOS->ShowOnScreenMessege("Loading...", 1);
        mainOS->folderIndexHistory.push_back(mainOS->FileIndexSelected);
        if (mainOS->FileOpenFromMainMenu)
        {
            full_Path_Of_The_Current_Selected = mainOS->FileSelectedInFS;
            mainOS->currentPath = full_Path_Of_The_Current_Selected;
        }
        else
        {
            mainOS->currentPath = full_Path_Of_The_Current_Selected;
        }
        mainOS->FileIndexSelected = 0;
        fileCount = 0;
        //   targetCameraY = 0;
        ShowCurrentFilesInDir(GetActiveFS(BrowseLittleFS), full_Path_Of_The_Current_Selected.c_str());
        // FileOptionMenuItems.push_back("Open Folder");
        mainOS->FileOpenFromMainMenu = false;
    }
    else
    {
        String exte = mainOS->GetExtensionLower(full_Path_Of_The_Current_Selected.c_str());

        // String selectedFile = full_Path_Of_The_Current_Selected;
        if (exte == "mp3" || exte == "wav")
        {
            mainOS->ClearAllSong();
            mainOS->EnterMusicPlayer();

            mainOS->GetAllSoundFileFromFolderToPlaylist(mainOS->currentPath.c_str());
            mainOS->InCurrentSong = findStringInArrayStringReturnIndex(full_Path_Of_The_Current_Selected, mainOS->CurrentMusic->SongList, mainOS->SongsInTheList); // make the player play the current song selected
            mainOS->saveSettings();                                                                                                                                // save current song in index
            mainOS->SongReady = true;
            if (mainOS->CurrentMusic != nullptr) // if alredy play song register not play to apply the new song
            {
                mainOS->CurrentMusic->isPlaying = false;
            }
            mainOS->EnterMusicPlayer();

            // mainOS->PlaySingleSong(selectedFile.c_str());
            // FileOptionMenuItems.push_back("Play In Player");
        }

        else if (exte == "png")
        {
            CurrentImageOpenType = CurrentlyImageOpen::PNG;

            pic_zoom_out = 0;
            IMG_x_POS = 0;
            IMG_y_POS = 0;
            RenderFileOptionMenu = false;
            // DrawPNG(full_Path_Of_The_Current_Selected);
            if (mainOS->FileOpenFromMainMenu)
            {
                DrawPNGPartial(mainOS->FileSelectedInFS, 0, 0, SCREEN_W, SCREEN_H, 0, 0);
            }
            else
            {
                DrawPNGPartial(full_Path_Of_The_Current_Selected, 0, 0, SCREEN_W, SCREEN_H, 0, 0);
            }
            mainOS->showTopBar = false;
            ShowPicOnScreen = true;
            delay(500); // delay for input
        }
        else if (exte == "bmp")
        {

            M5Cardputer.Display.fillScreen(BLACK);
            //  SPI.end();
            //  M5Cardputer.Display.drawBmpFile(full_Path_Of_The_Current_Selected.c_str(), 0, 0);
            mainOS->showTopBar = false;
            if (mainOS->FileOpenFromMainMenu)
            {

                Show_BMP_picture(mainOS->FileSelectedInFS);
            }
            else
            {
                Show_BMP_picture(full_Path_Of_The_Current_Selected);
            }
            ShowPicOnScreen = true;
        }
        else if (exte == "gif")
        {

            CurrentImageOpenType = CurrentlyImageOpen::GIF;
            IMG_x_POS = 0,
            IMG_y_POS = 0;
            pic_zoom_out = 0; // 0 = normal, 1 = half, 2 = quarter...

            M5Cardputer.Display.fillScreen(TFT_BLACK);

            /*        pic_zoom_out = 0;
                   PNGxpos = 0;
                   PNGypos = 0; */
            RenderFileOptionMenu = false;
            gif.begin(LITTLE_ENDIAN_PIXELS);

            GifPathToPlay = full_Path_Of_The_Current_Selected;
            mainOS->showTopBar = false;
            RenderGif = true;
            ShowPicOnScreen = true;
            delay(500); // delay for input
        }
        else if (exte == "jpg" || exte == "jpeg")
        {
            CurrentImageOpenType = CurrentlyImageOpen::JPG;

            M5Cardputer.Display.fillScreen(TFT_BLACK);

            pic_zoom_out = 0;
            IMG_x_POS = 0;
            IMG_y_POS = 0;
            RenderFileOptionMenu = false;
            drawJpeg(full_Path_Of_The_Current_Selected.c_str());

            // M5Cardputer.Display.drawJpgFile(full_Path_Of_The_Current_Selected.c_str(), 0, 0, 500, 500, 0, 0);
            mainOS->showTopBar = false;
            ShowPicOnScreen = true;
            delay(500); // delay for input
        }
        else if (exte == "thm")
        {
            mainOS->LoadTheme(full_Path_Of_The_Current_Selected, true);
            RenderFileOptionMenu = false;

            // M5Cardputer.Display.drawJpgFile(full_Path_Of_The_Current_Selected.c_str(), 0, 0, 500, 500, 0, 0);
            delay(500); // delay for input
        }
    }
}

void FileBrowser::CloseFileMenu()
{
    RenderFileOptionMenu = false;
    InFileOptionMenuIndex = 0;
}

void FileBrowser::OpenFileMenu()
{
    RenderFileOptionMenu = true;
}
/* void FileBrowser::SortFiles()
{
    std::vector<String> folders;
    std::vector<String> files;

    // 1. הפרדת הרשימה לתיקיות וקבצים
    for (int i = 0; i < fileCount; i++)
    {
        // בדיקה אם הנתיב הוא תיקייה (צריך להשתמש בפונקציית FS המתאימה)
        File f = SD.open(FileFound[i].c_str());
        if (f && f.isDirectory())
        {
            folders.push_back(FileFound[i]);
        }
        else
        {
            files.push_back(FileFound[i]);
        }
        f.close();
    }

    // 2. מיון אלפביתי של שתי הרשימות
    std::sort(folders.begin(), folders.end());
    std::sort(files.begin(), files.end());

    // 3. החזרת הערכים למערך המקורי לפי הסדר הרצוי
    int index = 0;
    for (const String& f : folders) FileFound[index++] = f;
    for (const String& f : files) FileFound[index++] = f;
} */
