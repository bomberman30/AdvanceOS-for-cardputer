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
    showTopBar = false;

    // If opened via "Edit from File", load the selected project and mark it as file-backed
    if (mainOS->EditFromFile)
    {
        LoadedFromFile = true;
        mainOS->EditFromFile = false;
        currentProjectPath = mainOS->FileSelectedInFS;
        LoadProject(currentProjectPath);
    }

    // Initialize all 8 audio channels as inactive
    for (int i = 0; i < 8; i++)
    {
        channelActive[i] = false;
        channelStartTime[i] = 0;
    }
}

void MusicCreator::handleInput()
{
    // Toggle the context menu
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        RenderMenu = !RenderMenu;
    }

    // Move cursor right
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('/', 700, 20))
    {
        cursorX++;
    }

    // Move cursor left (clamped to 0)
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(',', 700, 20))
    {
        cursorX--;
        if (cursorX < 0)
            cursorX = 0;
    }

    // Move cursor up / navigate menu up
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(';', 700, 20))
    {
        if (RenderMenu)
        {
            // Wrap-around upward selection in menu
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

    // Move cursor down / navigate menu down
    if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('.', 700, 20))
    {
        if (RenderMenu)
        {
            // Wrap-around downward selection in menu
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

    // Toggle note at cursor position: remove if exists, add if not
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
            // Add new note and play a preview at slightly lower volume
            Note newNote = {cursorX, cursorY};
            notes.push_back(newNote);
            oldVolume = M5Cardputer.Speaker.getVolume();
            M5Cardputer.Speaker.setVolume(M5Cardputer.Speaker.getVolume() - 50);
            PlaySingleNote(newNote, -1);
            delay(stepDuration);
            M5Cardputer.Speaker.setVolume(oldVolume);
        }
    }

    // Save project
    if (mainOS->NewKey.ifKeyJustPress('s'))
    {
        SaveProject(!LoadedFromFile);
    }

    // Reload project from file (only if it was originally loaded from one)
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

    // Shift all notes to the right of cursor: down by 1
    if (mainOS->NewKey.ifKeyJustPress('f'))
        MoveAllNoteFromRightOfTheCourser(0, 1);

    // Shift all notes to the right of cursor: up by 1
    if (mainOS->NewKey.ifKeyJustPress('t'))
        MoveAllNoteFromRightOfTheCourser(0, -1);

    // Shift all notes to the right of cursor: left by 1 step
    if (mainOS->NewKey.ifKeyJustPress('d'))
        MoveAllNoteFromRightOfTheCourser(-1, 0);

    // Shift all notes to the right of cursor: right by 1 step
    if (mainOS->NewKey.ifKeyJustPress('g'))
        MoveAllNoteFromRightOfTheCourser(1, 0);

    // Play / Stop playback from cursor position (Space bar)
    if (mainOS->NewKey.ifKeyJustPress(' '))
    {
        PlayFromCurrentScreen = true;
        isPlaying = !isPlaying;

        if (isPlaying)
        {
            // Start playback from cursor X position
            if (PlayFromCurrentScreen)
            {
                currentPlayStep = cursorX;
                if (currentPlayStep < 0)
                    currentPlayStep = 0;
            }

            lastStepMillis = millis();
        }
    }

    // Enter key: confirm menu selection or toggle full playback from step 0
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
            // Play from the beginning (step 0)
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

    // --- Camera scrolling: smoothly follow the cursor ---
    cameraX = lerpFloat(cameraX, cursorX * cellWidth - 100, 0.05);
    cameraY = lerpFloat(cameraY, cursorY * cellHeight - 60, 0.05);

    if (isPlaying)
    {
        // Find the last step that has any note placed on it
        int maxStep = 0;
        for (const auto &n : notes)
        {
            if (n.x > maxStep)
                maxStep = n.x;
        }

        // Stop playback once we've gone past the last note
        if (currentPlayStep > maxStep + 1)
        {
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
                // Advance to the next step and trigger its notes
                lastStepMillis = millis();
                TriggerStepAudio(currentPlayStep);
                currentPlayStep++;
            }
            else
            {
                // --- Per-channel fade-in (attack) envelope ---
                int attackTime = 40; // milliseconds for volume to ramp up
                int masterVol = M5Cardputer.Speaker.getVolume();

                for (int i = 0; i < 8; i++)
                {
                    if (!channelActive[i])
                        continue;

                    unsigned long t = millis() - channelStartTime[i];

                    // Deactivate channel once its step duration has elapsed
                    if (t >= stepDuration)
                    {
                        channelActive[i] = false;
                        continue;
                    }

                    int vol;
                    if (t < attackTime)
                    {
                        // Ramp volume from 0 to master during attack window
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
    // Play all notes that fall on this step, each on its own channel
    int channelIndex = 0;
    for (const auto &note : notes)
    {
        if (note.x == step)
        {
            // Assign each simultaneous note a separate channel to avoid overwriting
            PlaySingleNote(note, channelIndex);
            channelIndex++;

            // Hardware limit: M5Cardputer speaker supports up to 8 channels
            if (channelIndex >= 8)
                break;
        }
    }
}

void MusicCreator::PlaySingleNote(Note theNote, int channel)
{
    // Chromatic scale base frequencies (C4 = 261.63 Hz)
    float baseFrequencies[] = {261.63, 277.18, 293.66, 311.13, 329.63, 349.23,
                               369.99, 392.00, 415.30, 440.00, 466.16, 493.88};

    // Convert grid Y position to note index and octave
    // Grid rows go top-to-bottom, so higher Y = lower pitch
    int noteIndex = 11 - (theNote.y % 12);
    int octave = 8 - (theNote.y / 12);

    if (noteIndex < 0)
        noteIndex = 0;
    if (noteIndex > 11)
        noteIndex = 11;

    // Transpose frequency to the correct octave using equal temperament
    float freq = baseFrequencies[noteIndex] * pow(2, octave - 4);

    // Record channel start time for envelope tracking
    channelStartTime[channel] = millis();
    channelActive[channel] = true;

    // Start channel at volume 0; the Loop() fade-in will ramp it up
    M5Cardputer.Speaker.setChannelVolume(channel, 0);
    M5Cardputer.Speaker.tone(freq, stepDuration, channel);
}

void MusicCreator::Draw()
{
    // Clamp camera position to valid bounds
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

    // --- Layout dimensions ---
    int leftSidebarWidth = 25;  // Wide enough for note names like "C#"
    int rightSidebarWidth = 10; // Wide enough for octave numbers
    int topBarHeight = 15;      // Timeline bar height

    // Compute grid area boundaries
    int gridAreaWidth = 240 - leftSidebarWidth - rightSidebarWidth;
    int rightSidebarX = 240 - rightSidebarWidth;
    int gridBottomY = 135; // Bottom edge of the screen

    const char *noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    // Determine which step and note row are at the top-left of the visible area
    int startStep = (int)cameraX / cellWidth;
    int startNote = (int)cameraY / cellHeight;

    // --- 1. Draw row backgrounds (black keys = dark grey, white keys = default) ---
    for (int i = startNote; i < startNote + (135 / cellHeight) + 2; i++)
    {
        int yPos = (i * cellHeight) - (int)cameraY + topBarHeight;
        if (yPos >= topBarHeight - 20 && yPos <= 135 + 20) // extra 20px buffer to avoid clipping artifacts
        {
            int noteIndex = 11 - (i % 12);
            if (noteIndex < 0)
                noteIndex += 12;

            // Sharp (black key) notes: C#, D#, F#, G#, A#
            bool isSharp = (noteIndex == 1 || noteIndex == 3 || noteIndex == 6 || noteIndex == 8 || noteIndex == 10);

            if (isSharp)
            {
                // Shade only within the grid area, not the sidebars
                mainOS->sprite.fillRect(leftSidebarWidth, yPos, gridAreaWidth, cellHeight, DARKGREY);
            }
        }
    }

    // --- 2. Draw grid lines ---
    // Vertical lines (time steps) — span only the grid area
    for (int i = startStep; i < startStep + (gridAreaWidth / cellWidth) + 2; i++)
    {
        int xPos = (i * cellWidth) - (int)cameraX + leftSidebarWidth;
        if (xPos >= leftSidebarWidth && xPos <= rightSidebarX)
            mainOS->sprite.drawFastVLine(xPos, topBarHeight, 135 - topBarHeight, ILI9341_NAVY);
    }
    // Horizontal lines (note rows) — span only the grid area
    for (int i = startNote; i < startNote + (135 / cellHeight) + 2; i++)
    {
        int yPos = (i * cellHeight) - (int)cameraY + topBarHeight;
        if (yPos >= topBarHeight && yPos <= 135)
            mainOS->sprite.drawFastHLine(leftSidebarWidth, yPos, gridAreaWidth, ILI9341_NAVY);
    }

    // --- 3. Draw placed notes as yellow rounded rectangles ---
    for (const auto &note : notes)
    {
        int sx = (note.x * cellWidth) - (int)cameraX + leftSidebarWidth;
        int sy = (note.y * cellHeight) - (int)cameraY + topBarHeight;

        // Only draw notes within (or just outside) the visible grid bounds
        if (sx >= (leftSidebarWidth - 20) && sx < (rightSidebarX + 20) &&
            sy >= (topBarHeight - 20) && sy < (135 + 20))
        {
            mainOS->sprite.fillRoundRect(sx + 1, sy + 1, cellWidth - 2, cellHeight - 2, 2, YELLOW);
        }
    }

    // --- 4. Draw sidebars ---

    // Left sidebar: note name labels (navy background)
    mainOS->sprite.fillRect(0, topBarHeight, leftSidebarWidth, 135 - topBarHeight, ILI9341_NAVY);

    // Right sidebar: octave number labels (dark grey background)
    mainOS->sprite.fillRect(rightSidebarX, topBarHeight, rightSidebarWidth, 135 - topBarHeight, DARKGREY);

    mainOS->sprite.setTextColor(WHITE);

    // Populate both sidebars for each visible row
    for (int i = startNote; i < startNote + (135 / cellHeight) + 2; i++)
    {
        int yPos = (i * cellHeight) - (int)cameraY + topBarHeight;
        if (yPos >= topBarHeight && yPos < 135)
        {
            int noteIndex = 11 - (i % 12);
            if (noteIndex < 0)
                noteIndex += 12;

            int octave = 8 - (i / 12);

            // Draw note name on the left sidebar for every row
            mainOS->sprite.drawString(String(noteNames[noteIndex]), 2, yPos + 2);

            // Draw octave number on the right sidebar only on "C" rows (start of each octave)
            if (noteIndex == 0)
            {
                mainOS->sprite.setTextColor(YELLOW); // Yellow makes octave markers stand out
                mainOS->sprite.drawString(String(octave), rightSidebarX + 4, yPos + 2);
                mainOS->sprite.setTextColor(WHITE);
            }
        }
    }

    // Sidebar border lines separating them from the grid
    mainOS->sprite.drawFastVLine(leftSidebarWidth, topBarHeight, 135 - topBarHeight, ILI9341_NAVY);
    mainOS->sprite.drawFastVLine(rightSidebarX, topBarHeight, 135 - topBarHeight, DARKGREY);

    // --- 5. Draw timeline bar (top strip) ---
    // Spans from left sidebar to right sidebar
    mainOS->sprite.fillRect(leftSidebarWidth, 0, gridAreaWidth, topBarHeight, BLUE);

    // Draw step numbers every 5 steps
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

    // --- Draw cursor (red outline rectangle at current grid position) ---
    int csx = (cursorX * cellWidth) - (int)cameraX + leftSidebarWidth;
    int csy = (cursorY * cellHeight) - (int)cameraY + topBarHeight;

    // Only draw cursor if it's within the visible grid area
    if (csx >= leftSidebarWidth && csx < rightSidebarX && csy >= topBarHeight && csy < 135)
    {
        mainOS->sprite.drawRoundRect(csx, csy, cellWidth, cellHeight, 2, TFT_RED);
    }

    // --- Draw playhead (vertical white line at the current playback step) ---
    if (isPlaying)
    {
        int playheadX = (currentPlayStep * cellWidth) - (int)cameraX + leftSidebarWidth;
        if (playheadX >= leftSidebarWidth && playheadX < rightSidebarX)
        {
            mainOS->sprite.drawFastVLine(playheadX, topBarHeight, 135 - topBarHeight, TFT_WHITE);
        }
    }

    // --- Draw off-screen note indicators on the grid border ---
    // Notes outside the visible area are shown as small yellow markers on the nearest edge.
    // Corners are handled separately to avoid double-drawing.
    for (const auto &note : notes)
    {
        int sx = (note.x * cellWidth) - (int)cameraX + leftSidebarWidth;
        int sy = (note.y * cellHeight) - (int)cameraY + topBarHeight;

        const int thickness = 3;
        const uint16_t indicatorColor = YELLOW;

        bool offLeft   = (sx + cellWidth < leftSidebarWidth);
        bool offRight  = (sx >= rightSidebarX);
        bool offTop    = (sy + cellHeight < topBarHeight);
        bool offBottom = (sy >= gridBottomY);

        // --- Corner cases: note is off in two directions simultaneously ---
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

        // --- Edge indicators (single direction off-screen) ---

        // Off to the left
        if (offLeft)
        {
            int yStart = max(sy, topBarHeight);
            int yEnd = min(sy + cellHeight, gridBottomY);
            if (yStart < yEnd)
                mainOS->sprite.fillRect(leftSidebarWidth, yStart, thickness, yEnd - yStart, indicatorColor);
        }

        // Off to the right
        if (offRight)
        {
            int yStart = max(sy, topBarHeight);
            int yEnd = min(sy + cellHeight, gridBottomY);
            if (yStart < yEnd)
                mainOS->sprite.fillRect(rightSidebarX - thickness, yStart, thickness, yEnd - yStart, indicatorColor);
        }

        // Off above
        if (offTop)
        {
            int xStart = max(sx, leftSidebarWidth);
            int xEnd = min(sx + cellWidth, rightSidebarX);
            if (xStart < xEnd)
                mainOS->sprite.fillRect(xStart, topBarHeight, xEnd - xStart, thickness, indicatorColor);
        }

        // Off below
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
        // Prompt user for a file name and build the full path
        String NameToSave = mainOS->AskFromUserForString("Choose File Name", true);
        FullfileName = String(MUSIC_SAVE_PATH) + "/" + NameToSave + ".mc";

        // Ask before overwriting an existing file
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
        // Overwrite the currently open project file
        FullfileName = currentProjectPath;
    }

    File file = SD.open(FullfileName, FILE_WRITE);
    if (!file)
    {
        os.ShowOnScreenMessege("file open error");
        return false;
    }

    // Serialize project data to JSON
    JsonDocument doc;

    doc["bpm"] = bpm;

    JsonArray notesArray = doc.createNestedArray("notes");

    for (const auto &n : notes)
    {
        JsonObject obj = notesArray.createNestedObject();
        obj["x"] = n.x;
        obj["y"] = n.y;
    }

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

    // Clear any existing notes before loading
    notes.clear();

    // Read BPM (default to 120 if missing)
    bpm = doc["bpm"] | 120;

    // Read all note positions from the JSON array
    JsonArray notesArray = doc["notes"];

    for (JsonObject obj : notesArray)
    {
        Note n;
        n.x = obj["x"];
        n.y = obj["y"];
        notes.push_back(n);
    }

    currentProjectPath = filePath;

    return true;
}

void MusicCreator::MenuRender()
{
    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextSize(1);

    // Draw menu background panel
    mainOS->sprite.fillRect(10, 10, 200, 120, YELLOW);
    mainOS->sprite.drawRect(10, 10, 200, 120, BLACK);

    int DistanceBetweenMenu = 10;

    for (size_t i = 0; i < menuItem.size(); i++)
    {
        mainOS->sprite.setCursor(15, i * DistanceBetweenMenu + 12);

        // Highlight selected menu item in red, others in black
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
    // Shift every note at or to the right of the cursor by (x, y)
    for (int i = 0; i < notes.size(); i++)
    {
        if (notes[i].x >= cursorX)
        {
            notes[i].x += x;
            notes[i].y += y;
        }
    }

    // Remove any notes that now overlap after the shift
    removeDuplicateNotes(notes);
}

void MusicCreator::writeWavHeader(File &file, uint32_t dataSize, uint32_t sampleRate)
{
    uint32_t totalFileSize = dataSize + 36;
    uint32_t byteRate = sampleRate * 1 * 2; // SampleRate * NumChannels * (BitsPerSample / 8)

    // RIFF chunk descriptor
    file.write((const uint8_t *)"RIFF", 4);
    file.write((const uint8_t *)&totalFileSize, 4);
    file.write((const uint8_t *)"WAVE", 4);

    // fmt sub-chunk (PCM format descriptor)
    file.write((const uint8_t *)"fmt ", 4);

    uint32_t fmtChunkSize = 16;
    file.write((const uint8_t *)&fmtChunkSize, 4);

    uint16_t audioFormat = 1; // PCM = 1 (no compression)
    file.write((const uint8_t *)&audioFormat, 2);

    uint16_t numChannels = 1; // Mono output
    file.write((const uint8_t *)&numChannels, 2);

    file.write((const uint8_t *)&sampleRate, 4);
    file.write((const uint8_t *)&byteRate, 4);

    uint16_t blockAlign = 2; // NumChannels * BitsPerSample / 8
    file.write((const uint8_t *)&blockAlign, 2);

    uint16_t bitsPerSample = 16;
    file.write((const uint8_t *)&bitsPerSample, 2);

    // data sub-chunk header
    file.write((const uint8_t *)"data", 4);
    file.write((const uint8_t *)&dataSize, 4);
}

void MusicCreator::removeDuplicateNotes(std::vector<Note> &notes)
{
    // Step 1: Sort so that identical notes are adjacent (required for std::unique)
    std::sort(notes.begin(), notes.end(), [](const Note &a, const Note &b)
              {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y; });

    // Step 2: Move duplicates to the end using std::unique
    auto last = std::unique(notes.begin(), notes.end(), [](const Note &a, const Note &b)
                            { return a.x == b.x && a.y == b.y; });

    // Step 3: Erase the duplicate tail from memory
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

    // Find the last step to determine total audio length
    int maxStep = 0;
    for (const auto &n : notes)
        if (n.x > maxStep)
            maxStep = n.x;

    int stepMs = 60000 / (bpm * 2);
    uint32_t samplesPerStep = (sampleRate * stepMs) / 1000;
    uint32_t totalSamples = (maxStep + 1) * samplesPerStep;
    uint32_t dataSize = totalSamples * sizeof(int16_t);

    writeWavHeader(file, dataSize, sampleRate);

    // --- Envelope & volume settings ---
    const int attackTimeMs  = 20;        // Short attack for a punchy sound
    const int releaseTimeMs = 30;        // Release to avoid clicks at note end
    const float masterVolume = 0.4;      // Keep below 0.5 to avoid clipping when mixing
    const float gainBoost    = 1.8;      // Aggressive pre-limiter boost for presence

    uint32_t attackSamples  = (sampleRate * attackTimeMs)  / 1000;
    uint32_t releaseSamples = (sampleRate * releaseTimeMs) / 1000;

    for (int s = 0; s <= maxStep; s++)
    {
        // Collect all frequencies that play on this step
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

                // Generate square wave for each frequency and mix them
                // Square wave: output +1 when sine is positive, -1 when negative
                for (float f : stepFreqs)
                {
                    float sVal = sin(2.0 * PI * f * currentTime);
                    mixed += (sVal >= 0) ? 1.0f : -1.0f;
                }

                // Normalize by a power-of-size divisor to reduce inter-note clipping
                float divisor = pow((float)stepFreqs.size(), 0.8f);
                float value = (mixed / divisor) * gainBoost;

                // Hard limiter: clamp to [-1, 1] to prevent digital distortion
                if (value > 1.0f)  value = 1.0f;
                if (value < -1.0f) value = -1.0f;

                // Apply amplitude envelope (attack and release)
                if (i < attackSamples)
                    value *= (float)i / attackSamples;
                else if (i >= samplesPerStep - releaseSamples)
                    value *= (float)(samplesPerStep - i) / releaseSamples;

                sample = (int16_t)(value * 32767.0f * masterVolume);
            }
            file.write((const uint8_t *)&sample, 2);
        }

        // Log export progress every 10 steps
        if (s % 10 == 0)
            Serial.printf("Exporting %d/%d\n", s, maxStep);
    }

    file.close();
    os.ShowOnScreenMessege("Done!", 2000);
}