#include "MusicCreator.h"
#include "MyOS.h"
#include <ArduinoJson.h>
#include "MainMenuV2.h"
#define MUSIC_SAVE_PATH "/AdvanceOS/RetroMusic"
#include "Function.h"
#include "FileBrowser.h"
#define MAX_CAMERA_Y 96 * cellHeight

    bool LoadedFromFile = false;

void MusicCreator::Begin()
{
    M5Cardputer.Speaker.setVolume(200);
    // mainOS->ExitMusicPlayer();
    showTopBar = false;
    if (mainOS->EditFromFile)
    {
        LoadedFromFile = true;
        mainOS->EditFromFile = false;
        currentProjectPath = mainOS->FileSelectedInFS;
        LoadProject(currentProjectPath);
        // LoadBMP_To_PixelsArray_var(mainOS->FileSelectedInFS);
        // 107*cellHeight
    }
    for (int i = 0; i < 8; i++)
    {
        channelActive[i] = false;
        channelStartTime[i] = 0;
    }
}
void MusicCreator::handleInput()
{
    // פתיחת תפריט
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        RenderMenu = !RenderMenu;
    }

    // ימינה
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('/', 700, 20))
    {
        cursorX++;
    }

    // שמאלה
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(',', 700, 20))
    {
        cursorX--;
        if (cursorX < 0)
            cursorX = 0;
    }

    // למעלה
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(';', 700, 20))
    {
        if (RenderMenu)
        {
            InMenuSelect--;
            if (InMenuSelect < 0)
                InMenuSelect = menuItem.size() - 1;
        }
        else
        {
            cursorY--;
            if (cursorY < 0)
                cursorY = 0;
        }
    }

    // למטה
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('.', 700, 20))
    {
        if (RenderMenu)
        {
            InMenuSelect++;
            if (InMenuSelect >= menuItem.size())
                InMenuSelect = 0;
        }
        else
        {
            cursorY++;
            if (cursorY > 107)
                cursorY = 107;
        }
    }

    // הוספה/מחיקה של תו
    if (mainOS->NewKey.ifKeyJustPress('\\'))
    {
        bool found = false;

        for (int i = 0; i < notes.size(); i++)
        {
            if (notes[i].x == cursorX && notes[i].y == cursorY)
            {
                notes.erase(notes.begin() + i);
                found = true;
                break;
            }
        }

        if (!found)
        {
            Note newNote = {cursorX, cursorY};
            notes.push_back(newNote);
            oldVolume = M5Cardputer.Speaker.getVolume();
            M5Cardputer.Speaker.setVolume(M5Cardputer.Speaker.getVolume() - 50);
            PlaySingleNote(newNote, -1);
            delay(stepDuration);
            M5Cardputer.Speaker.setVolume(oldVolume);
        }
    }

    // הזזת תווים
    if (mainOS->NewKey.ifKeyJustPress('s'))
    {
        SaveProject(!LoadedFromFile);
    }

    if (mainOS->NewKey.ifKeyJustPress('a'))
    {

        if (LoadedFromFile)
        {
            LoadProject(currentProjectPath);
            mainOS->ShowOnScreenMessege("Project Reloaded From File", 1000);
        }
        else
        {
            mainOS->ShowOnScreenMessege("The Music Project Not Save In Any File", 1000);
        }
    }

    if (mainOS->NewKey.ifKeyJustPress('f'))
        MoveAllNoteFromRightOfTheCourser(0, 1);

    if (mainOS->NewKey.ifKeyJustPress('t'))
        MoveAllNoteFromRightOfTheCourser(0, -1);
    if (mainOS->NewKey.ifKeyJustPress('d'))
        MoveAllNoteFromRightOfTheCourser(-1, 0);

    if (mainOS->NewKey.ifKeyJustPress('g'))
        MoveAllNoteFromRightOfTheCourser(1, 0);

    // Play / Stop (space)
    if (mainOS->NewKey.ifKeyJustPress(' '))
    {
        PlayFromCurrentScreen = true;
        isPlaying = !isPlaying;

        if (isPlaying)
        {
            if (PlayFromCurrentScreen)
            {
                currentPlayStep = cursorX;
                if (currentPlayStep < 0)
                    currentPlayStep = 0;
            }

            lastStepMillis = millis();
        }
    }

    // ENTER
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        if (RenderMenu)
        {
            if (menuItem[InMenuSelect] == "Exit")
            {
                mainOS->ChangeMenu(new MainMenuV2(mainOS));
                return;
            }
            else if (menuItem[InMenuSelect] == "Save")
            {
                SaveProject(!LoadedFromFile);
                RenderMenu = false;
            }
            else if (menuItem[InMenuSelect] == "Save As New File")
            {
                SaveProject(true);
                RenderMenu = false;
            }
            else if (menuItem[InMenuSelect] == "Export As WAV")
            {
                ExportAsWav();
                RenderMenu = false;
            }
            else if (menuItem[InMenuSelect] == "Set BPM")
            {
                int newBPM = mainOS
                                 ->AskFromUserForString("choose BMP (100-300 Recomended)", true, false, true)
                                 .toInt();

                bpm = newBPM;
                RenderMenu = false;
            }
            else if (menuItem[InMenuSelect] == "Open Music Creator Folder")
            {
                mainOS->currentPath = MUSIC_SAVE_PATH;
                mainOS->ChangeMenu(new FileBrowser(mainOS));
            }
            else if (menuItem[InMenuSelect] == "Clear All")
            {
                notes.clear();
                RenderMenu = false;
            }
        }
        else
        {
            PlayFromCurrentScreen = false;
            isPlaying = !isPlaying;

            if (isPlaying)
            {
                currentPlayStep = 0;
                lastStepMillis = millis();
            }
        }
    }
}

void MusicCreator::Loop()
{

    if (mainOS->screenOff)
    {
        return;
    }
    handleInput();

    // --- מעקב מצלמה (Scrolling) ---
    // ציר X
    cameraX = lerpFloat(cameraX, cursorX * cellWidth - 100, 0.05);
    cameraY = lerpFloat(cameraY, cursorY * cellHeight - 60, 0.05);

    if (isPlaying)
    {
        // 1. חישוב הצעד המקסימלי (כדי לדעת מתי לעצור)
        int maxStep = 0;
        for (const auto &n : notes)
        {
            if (n.x > maxStep)
                maxStep = n.x;
        }

        // 2. בדיקה: האם עברנו את הצעד האחרון?
        // if (currentPlayStep >= maxStep)
        if (currentPlayStep > maxStep + 1)
        {
            // עצירה ואיפוס
            isPlaying = false;
            currentPlayStep = 0;
            M5Cardputer.Speaker.stop();
        }
        else
        {
            stepDuration = 60000 / (bpm * 2);
            unsigned long timeSinceStepStart = millis() - lastStepMillis;

            if (timeSinceStepStart >= stepDuration)
            {
                lastStepMillis = millis();
                TriggerStepAudio(currentPlayStep);
                currentPlayStep++;
            }
            else
            {
                // --- מנגנון הריכוך (Fade-in) ---
                int attackTime = 40;
                int masterVol = M5Cardputer.Speaker.getVolume();

                for (int i = 0; i < 8; i++)
                {
                    if (!channelActive[i])
                        continue;

                    unsigned long t = millis() - channelStartTime[i];

                    if (t >= stepDuration)
                    {
                        channelActive[i] = false;
                        continue;
                    }

                    int vol;
                    if (t < attackTime)
                    {
                        vol = map(t, 0, attackTime, 0, masterVol);
                    }
                    else
                    {
                        vol = masterVol;
                    }

                    M5Cardputer.Speaker.setChannelVolume(i, vol);
                }
            }
        }
    }
    Draw();
}

void MusicCreator::TriggerStepAudio(int step)
{
    int channelIndex = 0;
    for (const auto &note : notes)
    {
        if (note.x == step)
        {
            // שולחים את התו יחד עם אינדקס ערוץ כדי שלא ידרסו אחד את השני
            PlaySingleNote(note, channelIndex);
            channelIndex++;

            // הגבלה למספר הערוצים המקסימלי של ה-Speaker (בדרך כלל 8)
            if (channelIndex >= 8)
                break;
        }
    }
}
void MusicCreator::PlaySingleNote(Note theNote, int channel)
{
    float baseFrequencies[] = {261.63, 277.18, 293.66, 311.13, 329.63, 349.23,
                               369.99, 392.00, 415.30, 440.00, 466.16, 493.88};

    int noteIndex = 11 - (theNote.y % 12);
    int octave = 8 - (theNote.y / 12);

    if (noteIndex < 0)
        noteIndex = 0;
    if (noteIndex > 11)
        noteIndex = 11;

    float freq = baseFrequencies[noteIndex] * pow(2, octave - 4);

    // שמירת זמן התחלה לערוץ הזה
    channelStartTime[channel] = millis();
    channelActive[channel] = true;

    // התחלת הצליל בווליום 0
    M5Cardputer.Speaker.setChannelVolume(channel, 0);
    M5Cardputer.Speaker.tone(freq, stepDuration, channel);
}

void MusicCreator::Draw()
{
    if (cameraX < 0)
        cameraX = 0;
    if (cameraY < 0)
        cameraY = 0;
    if (cameraY > MAX_CAMERA_Y)
        cameraY = MAX_CAMERA_Y;

    mainOS->sprite.createSprite(240, 135);
    mainOS->sprite.fillSprite(0xb596);
    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextSize(1);

    // --- הגדרות מימדים ---
    int leftSidebarWidth = 25;  // מספיק לטקסט כמו "C#"
    int rightSidebarWidth = 10; // מספיק לטקסט כמו "4"
    int topBarHeight = 15;

    // חישוב השטח הנותר לגריד
    int gridAreaWidth = 240 - leftSidebarWidth - rightSidebarWidth;
    int rightSidebarX = 240 - rightSidebarWidth;
    int gridBottomY = 135; // קצה תחתון של המסך

    const char *noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    int startStep = (int)cameraX / cellWidth;
    int startNote = (int)cameraY / cellHeight;

    // --- 1. ציור רקע לשורות (קלידים שחורים/לבנים) בתוך אזור הגריד ---
    for (int i = startNote; i < startNote + (135 / cellHeight) + 2; i++)
    {
        int yPos = (i * cellHeight) - (int)cameraY + topBarHeight;
        if (yPos >= topBarHeight - 20 && yPos <= 135 + 20) // add 20 to limit clipping
        {
            int noteIndex = 11 - (i % 12);
            if (noteIndex < 0)
                noteIndex += 12;

            bool isSharp = (noteIndex == 1 || noteIndex == 3 || noteIndex == 6 || noteIndex == 8 || noteIndex == 10);

            if (isSharp)
            {
                // צביעת הרקע רק בתחום הגריד (בין הסרגלים)
                mainOS->sprite.fillRect(leftSidebarWidth, yPos, gridAreaWidth, cellHeight, DARKGREY); // 0xb596
            }
        }
    }

    // --- 2. ציור הגריד ---
    // אנכי - מתחיל מ-leftSidebarWidth ומסתיים לפני rightSidebarX
    for (int i = startStep; i < startStep + (gridAreaWidth / cellWidth) + 2; i++)
    {
        int xPos = (i * cellWidth) - (int)cameraX + leftSidebarWidth;
        if (xPos >= leftSidebarWidth && xPos <= rightSidebarX)
            mainOS->sprite.drawFastVLine(xPos, topBarHeight, 135 - topBarHeight, ILI9341_NAVY);
    }
    // אופקי - נמתח לרוחב אזור הגריד בלבד
    for (int i = startNote; i < startNote + (135 / cellHeight) + 2; i++)
    {
        int yPos = (i * cellHeight) - (int)cameraY + topBarHeight;
        if (yPos >= topBarHeight && yPos <= 135)
            mainOS->sprite.drawFastHLine(leftSidebarWidth, yPos, gridAreaWidth, ILI9341_NAVY);
    }

    // --- 3. ציור התווים (Yellow Blocks) ---
    for (const auto &note : notes)
    {
        int sx = (note.x * cellWidth) - (int)cameraX + leftSidebarWidth;
        int sy = (note.y * cellHeight) - (int)cameraY + topBarHeight;

        // בדיקה שהתו בתוך גבולות הגריד הראויים לצפייה
        if (sx >= (leftSidebarWidth - 20) && sx < (rightSidebarX + 20) &&
            sy >= (topBarHeight - 20) && sy < (135 + 20))
        {
            mainOS->sprite.fillRoundRect(sx + 1, sy + 1, cellWidth - 2, cellHeight - 2, 2, YELLOW);
        }
    }

    // --- 4. סרגלי צד (Sidebars) ---

    // א. סרגל תוים שמאלי (BLACK background)
    mainOS->sprite.fillRect(0, topBarHeight, leftSidebarWidth, 135 - topBarHeight, ILI9341_NAVY);

    // ב. סרגל אוקטבות ימני (BLACK background)
    mainOS->sprite.fillRect(rightSidebarX, topBarHeight, rightSidebarWidth, 135 - topBarHeight, DARKGREY);

    mainOS->sprite.setTextColor(WHITE);

    // side bar for octeve

    for (int i = startNote; i < startNote + (135 / cellHeight) + 2; i++)
    {
        int yPos = (i * cellHeight) - (int)cameraY + topBarHeight;
        if (yPos >= topBarHeight && yPos < 135)
        {
            int noteIndex = 11 - (i % 12);
            if (noteIndex < 0)
                noteIndex += 12;

            int octave = 8 - (i / 12);
            /*           bool isSharp = (noteIndex == 1 || noteIndex == 3 || noteIndex == 6 || noteIndex == 8 || noteIndex == 10);

                      // --- רקע לשורות שחורות ---
                      if (isSharp)
                      {
                          uint16_t sharpBgColor = 0x2104;
                          mainOS->sprite.fillRect(0, yPos, leftSidebarWidth, cellHeight, sharpBgColor);
                          mainOS->sprite.fillRect(rightSidebarX, yPos, rightSidebarWidth, cellHeight, sharpBgColor);
                      } */

            // --- ציור שם התו בשמאל (נשאר לכל שורה) ---
            mainOS->sprite.drawString(String(noteNames[noteIndex]), 2, yPos + 2);

            // --- שינוי כאן: ציור האוקטבה רק בשורת התו C ---
            if (noteIndex == 0) // 0 מייצג את "C" במערך noteNames שלך
            {
                mainOS->sprite.setTextColor(YELLOW); // אפשר להשתמש בצהוב כדי שיבלוט שזו אוקטבה חדשה
                mainOS->sprite.drawString(String(octave), rightSidebarX + 4, yPos + 2);
                mainOS->sprite.setTextColor(WHITE); // מחזיר ללבן לשאר הטקסט
            }

            // --- קווים מפרידים ---
            // mainOS->sprite.drawFastHLine(0, yPos, leftSidebarWidth, DARKGREY);
            // mainOS->sprite.drawFastHLine(rightSidebarX, yPos, rightSidebarWidth, DARKGREY);
        }
    }
    // קווים אנכיים המפרידים את הסרגלים מהגריד
    mainOS->sprite.drawFastVLine(leftSidebarWidth, topBarHeight, 135 - topBarHeight, ILI9341_NAVY);
    mainOS->sprite.drawFastVLine(rightSidebarX, topBarHeight, 135 - topBarHeight, DARKGREY);

    // --- 5. ציר זמן (למעלה) ---
    // נמתח מרוחב הסרגל השמאלי ועד הימני
    mainOS->sprite.fillRect(leftSidebarWidth, 0, gridAreaWidth, topBarHeight, BLUE);

    for (int i = startStep; i < startStep + (gridAreaWidth / cellWidth) + 2; i++)
    {
        if (i % 5 != 0)
            continue;
        int xPos = (i * cellWidth) - (int)cameraX + leftSidebarWidth;
        if (xPos >= leftSidebarWidth && xPos < rightSidebarX)
        {
            mainOS->sprite.setTextColor(WHITE);
            mainOS->sprite.drawString(String(i), xPos + 2, 2);
            mainOS->sprite.drawFastVLine(xPos, 0, topBarHeight, TFT_WHITE);
        }
    }

    // --- סמן הבחירה (Cursor) ---
    int csx = (cursorX * cellWidth) - (int)cameraX + leftSidebarWidth;
    int csy = (cursorY * cellHeight) - (int)cameraY + topBarHeight;

    // וודא שהסמן מצויר רק בתוך אזור הגריד הנראה
    if (csx >= leftSidebarWidth && csx < rightSidebarX && csy >= topBarHeight && csy < 135)
    {
        mainOS->sprite.drawRoundRect(csx, csy, cellWidth, cellHeight, 2, TFT_RED);
    }

    // --- קו נגינה (Playhead) ---
    if (isPlaying)
    {
        int playheadX = (currentPlayStep * cellWidth) - (int)cameraX + leftSidebarWidth;
        if (playheadX >= leftSidebarWidth && playheadX < rightSidebarX)
        {
            mainOS->sprite.drawFastVLine(playheadX, topBarHeight, 135 - topBarHeight, TFT_WHITE);
        }
    }

    // --- 3ב. ציור אינדיקטורים לתווים מחוץ למסך (FIX כולל פינות) ---
    for (const auto &note : notes)
    {
        int sx = (note.x * cellWidth) - (int)cameraX + leftSidebarWidth;
        int sy = (note.y * cellHeight) - (int)cameraY + topBarHeight;

        const int thickness = 3;
        const uint16_t indicatorColor = YELLOW;

        bool offLeft = (sx + cellWidth < leftSidebarWidth);
        bool offRight = (sx >= rightSidebarX);
        bool offTop = (sy + cellHeight < topBarHeight);
        bool offBottom = (sy >= gridBottomY);

        // --- פינות ---
        if (offRight && offBottom)
        {
            mainOS->sprite.fillRect(rightSidebarX - thickness, gridBottomY - thickness, thickness, thickness, indicatorColor);
            continue;
        }
        if (offRight && offTop)
        {
            mainOS->sprite.fillRect(rightSidebarX - thickness, topBarHeight, thickness, thickness, indicatorColor);
            continue;
        }
        if (offLeft && offBottom)
        {
            mainOS->sprite.fillRect(leftSidebarWidth, gridBottomY - thickness, thickness, thickness, indicatorColor);
            continue;
        }
        if (offLeft && offTop)
        {
            mainOS->sprite.fillRect(leftSidebarWidth, topBarHeight, thickness, thickness, indicatorColor);
            continue;
        }

        // --- צדדים רגילים ---

        // שמאל
        if (offLeft)
        {
            int yStart = max(sy, topBarHeight);
            int yEnd = min(sy + cellHeight, gridBottomY);
            if (yStart < yEnd)
                mainOS->sprite.fillRect(leftSidebarWidth, yStart, thickness, yEnd - yStart, indicatorColor);
        }

        // ימין
        if (offRight)
        {
            int yStart = max(sy, topBarHeight);
            int yEnd = min(sy + cellHeight, gridBottomY);
            if (yStart < yEnd)
                mainOS->sprite.fillRect(rightSidebarX - thickness, yStart, thickness, yEnd - yStart, indicatorColor);
        }

        // למעלה
        if (offTop)
        {
            int xStart = max(sx, leftSidebarWidth);
            int xEnd = min(sx + cellWidth, rightSidebarX);
            if (xStart < xEnd)
                mainOS->sprite.fillRect(xStart, topBarHeight, xEnd - xStart, thickness, indicatorColor);
        }

        // למטה
        if (offBottom)
        {
            int xStart = max(sx, leftSidebarWidth);
            int xEnd = min(sx + cellWidth, rightSidebarX);
            if (xStart < xEnd)
                mainOS->sprite.fillRect(xStart, gridBottomY - thickness, xEnd - xStart, thickness, indicatorColor);
        }
    }

    if (RenderMenu)
        MenuRender();

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

bool MusicCreator::SaveProject(bool SaveAsNew)
{
    if (!SD.begin())
    {
        os.ShowOnScreenMessege("no sd");
        return false;
    }

    SD.mkdir(MUSIC_SAVE_PATH);

    String FullfileName;

    if (SaveAsNew)
    {
        String NameToSave = mainOS->AskFromUserForString("Choose File Name", true);
        FullfileName = String(MUSIC_SAVE_PATH) + "/" + NameToSave + ".mc";

        if (SD.exists(FullfileName.c_str()))
        {
            if (!mainOS->AskSomthing("File already exists overwrite?"))
            {
                return false;
            }
        }
        LoadedFromFile = true;
    }
    else
    {
        FullfileName = currentProjectPath; // אם יש משתנה כזה בפרויקט
    }

    File file = SD.open(FullfileName, FILE_WRITE);
    if (!file)
    {
        os.ShowOnScreenMessege("file open error");
        return false;
    }

    // יצירת JSON
    JsonDocument doc;

    doc["bpm"] = bpm;
    /*     doc["cellWidth"] = cellWidth;
        doc["cellHeight"] = cellHeight; */

    JsonArray notesArray = doc.createNestedArray("notes");

    for (const auto &n : notes)
    {
        JsonObject obj = notesArray.createNestedObject();
        obj["x"] = n.x;
        obj["y"] = n.y;
    }

    // כתיבה לקובץ
    serializeJson(doc, file);

    file.close();

    currentProjectPath = FullfileName;

    os.ShowOnScreenMessege("saved in AdvanceOS/RetroMusic", 1000);

    return true;
}
bool MusicCreator::LoadProject(String filePath)
{
    if (!SD.begin())
    {
        os.ShowOnScreenMessege("no sd");
        return false;
    }

    if (!SD.exists(filePath))
    {
        os.ShowOnScreenMessege("file not found");
        return false;
    }

    File file = SD.open(filePath, FILE_READ);
    if (!file)
    {
        os.ShowOnScreenMessege("open error");
        return false;
    }

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        os.ShowOnScreenMessege("json error");
        return false;
    }

    // ניקוי התווים הישנים
    notes.clear();

    // קריאת הגדרות
    bpm = doc["bpm"] | 120;
    /*     cellWidth = doc["cellWidth"] | cellWidth;
        cellHeight = doc["cellHeight"] | cellHeight; */

    // קריאת התווים
    JsonArray notesArray = doc["notes"];

    for (JsonObject obj : notesArray)
    {
        Note n;
        n.x = obj["x"];
        n.y = obj["y"];
        notes.push_back(n);
    }

    currentProjectPath = filePath;

    // os.ShowOnScreenMessege("loaded", 1000);

    return true;
}

void MusicCreator::MenuRender()
{
    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.fillRect(10, 10, 200, 120, YELLOW);
    mainOS->sprite.drawRect(10, 10, 200, 120, BLACK);
    int DistanceBetweenMenu = 10;
    for (size_t i = 0; i < menuItem.size(); i++)
    {
        mainOS->sprite.setCursor(15, i * DistanceBetweenMenu + 12);
        if (InMenuSelect == i)
        {
            mainOS->sprite.setTextColor(RED);
        }
        else
        {
            mainOS->sprite.setTextColor(BLACK);
        }
        mainOS->sprite.print(menuItem[i]);
    }
}

void MusicCreator::MoveAllNoteFromRightOfTheCourser(int x, int y)
{
    for (int i = 0; i < notes.size(); i++)
    {
        if (notes[i].x >= cursorX)
        {
            // 1. נבצע את ההזזה
            notes[i].x += x;
            notes[i].y += y;
        }
    }
    removeDuplicateNotes(notes);
}

void MusicCreator::writeWavHeader(File &file, uint32_t dataSize, uint32_t sampleRate)
{
    uint32_t totalFileSize = dataSize + 36;
    uint32_t byteRate = sampleRate * 1 * 2; // SampleRate * NumChannels * BitsPerSample/8

    // שימוש ב- (const uint8_t*) כדי לפתור את שגיאת הקומפילציה
    file.write((const uint8_t *)"RIFF", 4);
    file.write((const uint8_t *)&totalFileSize, 4);
    file.write((const uint8_t *)"WAVE", 4);
    file.write((const uint8_t *)"fmt ", 4);

    uint32_t fmtChunkSize = 16;
    file.write((const uint8_t *)&fmtChunkSize, 4);

    uint16_t audioFormat = 1; // PCM
    file.write((const uint8_t *)&audioFormat, 2);

    uint16_t numChannels = 1; // Mono
    file.write((const uint8_t *)&numChannels, 2);

    file.write((const uint8_t *)&sampleRate, 4);
    file.write((const uint8_t *)&byteRate, 4);

    uint16_t blockAlign = 2; // NumChannels * BitsPerSample/8
    file.write((const uint8_t *)&blockAlign, 2);

    uint16_t bitsPerSample = 16;
    file.write((const uint8_t *)&bitsPerSample, 2);

    file.write((const uint8_t *)"data", 4);
    file.write((const uint8_t *)&dataSize, 4);
}
void MusicCreator::removeDuplicateNotes(std::vector<Note> &notes)
{
    // 1. מיון הוקטור - חובה כדי שכל האיברים הזהים יהיו אחד ליד השני
    std::sort(notes.begin(), notes.end(), [](const Note &a, const Note &b)
              {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y; });

    // 2. העברת הכפילויות לסוף הוקטור ומחיקתן
    auto last = std::unique(notes.begin(), notes.end(), [](const Note &a, const Note &b)
                            { return a.x == b.x && a.y == b.y; });

    // 3. מחיקה פיזית של ה"זנב" המיותר מהזיכרון
    notes.erase(last, notes.end());
}

void MusicCreator::ExportAsWav()
{
    if (!SD.begin())
    {
        os.ShowOnScreenMessege("No SD Card!");
        return;
    }

    String fileName = mainOS->AskFromUserForString("WAV File Name", true) + ".wav";
    String path = String(MUSIC_SAVE_PATH) + "/" + fileName;

    File file = SD.open(path, FILE_WRITE);
    if (!file)
    {
        os.ShowOnScreenMessege("File Error!");
        return;
    }

    os.ShowOnScreenMessege("Exporting...", 1);

    const uint32_t sampleRate = 22050;
    float baseFrequencies[] = {261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88};

    int maxStep = 0;
    for (const auto &n : notes)
        if (n.x > maxStep)
            maxStep = n.x;

    int stepMs = 60000 / (bpm * 2);
    uint32_t samplesPerStep = (sampleRate * stepMs) / 1000;
    uint32_t totalSamples = (maxStep + 1) * samplesPerStep;
    uint32_t dataSize = totalSamples * sizeof(int16_t);

    writeWavHeader(file, dataSize, sampleRate);

    // הגדרות עוצמה משופרות
    const int attackTimeMs = 20; // קיצרתי מעט כדי לתת יותר "קיק"
    const int releaseTimeMs = 30;
    const float masterVolume = 0.4; // less then half
    const float gainBoost = 1.8;    // הגברה אגרסיבית לפני הנרמול

    uint32_t attackSamples = (sampleRate * attackTimeMs) / 1000;
    uint32_t releaseSamples = (sampleRate * releaseTimeMs) / 1000;

    for (int s = 0; s <= maxStep; s++)
    {
        std::vector<float> stepFreqs;
        for (const auto &n : notes)
        {
            if (n.x == s)
            {
                int noteIndex = 11 - (n.y % 12);
                int octave = 8 - (n.y / 12);
                float freq = baseFrequencies[noteIndex] * pow(2, octave - 4);
                stepFreqs.push_back(freq);
            }
        }

        for (uint32_t i = 0; i < samplesPerStep; i++)
        {
            int16_t sample = 0;
            if (!stepFreqs.empty())
            {
                float mixed = 0;
                float currentTime = (float)(s * samplesPerStep + i) / sampleRate;

                for (float f : stepFreqs)
                {
                    // --- שינוי לגל ריבועי (Square Wave) ---
                    // במקום sin, אנחנו בודקים אם הסינוס חיובי או שלילי
                    float sVal = sin(2.0 * PI * f * currentTime);
                    mixed += (sVal >= 0) ? 1.0f : -1.0f;
                }

                // נרמול עוצמה עם Gain Boost
                float divisor = pow((float)stepFreqs.size(), 0.8f);
                float value = (mixed / divisor) * gainBoost;

                // Limiter קשיח למניעת רעשים
                if (value > 1.0f)
                    value = 1.0f;
                if (value < -1.0f)
                    value = -1.0f;

                // Envelope
                if (i < attackSamples)
                    value *= (float)i / attackSamples;
                else if (i >= samplesPerStep - releaseSamples)
                    value *= (float)(samplesPerStep - i) / releaseSamples;

                sample = (int16_t)(value * 32767.0f * masterVolume);
            }
            file.write((const uint8_t *)&sample, 2);
        }

        if (s % 10 == 0)
            Serial.printf("Exporting %d/%d\n", s, maxStep);
    }

    file.close();
    os.ShowOnScreenMessege("Done!", 2000);
}

/*  void MusicCreator::ExportAsWav()
{
    if (!SD.begin())
    {
        os.ShowOnScreenMessege("No SD Card!");
        return;
    }

    String fileName = mainOS->AskFromUserForString("WAV File Name", true) + ".wav";
    String path = String(MUSIC_SAVE_PATH) + "/" + fileName;

    File file = SD.open(path, FILE_WRITE);
    if (!file)
    {
        os.ShowOnScreenMessege("File Error!");
        return;
    }

    os.ShowOnScreenMessege("Exporting... Please wait", 1);

    const uint32_t sampleRate = 22050;
    float baseFrequencies[] = {261.63, 277.18, 293.66, 311.13, 329.63, 349.23,
                               369.99, 392.00, 415.30, 440.00, 466.16, 493.88};

    // מציאת הצעד האחרון
    int maxStep = 0;
    for (const auto &n : notes)
        if (n.x > maxStep)
            maxStep = n.x;

    int stepMs = 60000 / (bpm * 2);
    uint32_t samplesPerStep = (sampleRate * stepMs) / 1000;
    uint32_t totalSamples = (maxStep + 1) * samplesPerStep;
    uint32_t dataSize = totalSamples * sizeof(int16_t);

    writeWavHeader(file, dataSize, sampleRate);

    // פרמטרים ל-fade
    const int attackTimeMs = 40;   // Fade-in
    const int releaseTimeMs = 40;  // Fade-out
    const float masterVolume = 1.1; // 70% עוצמה
    uint32_t attackSamples = (sampleRate * attackTimeMs) / 1000;
    uint32_t releaseSamples = (sampleRate * releaseTimeMs) / 1000;

    for (int s = 0; s <= maxStep; s++)
    {
        // אוסף התווים של הצעד הנוכחי
        std::vector<float> stepFreqs;
        for (const auto &n : notes)
        {
            if (n.x == s)
            {
                int noteIndex = 11 - (n.y % 12);
                int octave = 8 - (n.y / 12);

                if (noteIndex < 0) noteIndex = 0;
                if (noteIndex > 11) noteIndex = 11;

                float freq = baseFrequencies[noteIndex] * pow(2, octave - 4);
                stepFreqs.push_back(freq);
            }
        }

        for (uint32_t i = 0; i < samplesPerStep; i++)
        {
            int16_t sample = 0;
            if (!stepFreqs.empty())
            {
                float mixed = 0;
                float currentTime = (float)(s * samplesPerStep + i) / sampleRate;

                // ערבוב כל התווים
                for (float f : stepFreqs)
                    mixed += sin(2.0 * PI * f * currentTime);

                float value = mixed / stepFreqs.size(); // מניעת clipping

                // --- Fade-in ---
                if (i < attackSamples)
                    value *= (float)i / attackSamples;

                // --- Fade-out ---
                else if (i >= samplesPerStep - releaseSamples)
                    value *= (float)(samplesPerStep - i) / releaseSamples;

                // עוצמה כוללת 70%
                sample = (int16_t)(value * 32767 * masterVolume);
            }

            file.write((const uint8_t *)&sample, 2);
        }

        if (s % 10 == 0)
            Serial.printf("Exporting step %d/%d\n", s, maxStep);
    }

    file.close();
    os.ShowOnScreenMessege("Export Complete!", 2000);
}  */