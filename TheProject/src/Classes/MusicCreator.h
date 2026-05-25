#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <vector>

struct Note
{
    int x; // מיקום על ציר הזמן (Grid Step)
    int y; // גובה הצליל (Pitch)
};

class MusicCreator : public GlobalParentClass
{
public:
    MusicCreator(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop() override;
    void Draw() override;
    String currentProjectPath;

private:
    unsigned long channelStartTime[8] = {0};
    bool channelActive[8] = {false};
    // הגדרות הגריד
    const int cellWidth = 10;
    const int cellHeight = 10;

    const int CamXOffset = 100;
    const int CamYOffset = 60;
    int stepDuration = 100;

    int oldVolume;
    int cameraX = 0;
    int cameraY = 0;
    int cursorX = 0;
    int cursorY = 59;

    // משתני נגינה
    bool isPlaying = false;
    int currentPlayStep = -1;
    unsigned long lastStepMillis = 0;
    int bpm = 200;
    std::vector<Note> notes; // רשימת התווים שהמשתמש הניח

    std::vector<float> currentChord;
    int arpIndex = 0;
    unsigned long lastArp = 0;
    int arpSpeed = 30;
    std::vector<String> menuItem = {"Exit", "Save", "Save As New File", "Set BPM", "Clear All", "Export As WAV", "Open Music Creator Folder"};
    void handleInput();
    // void PlayNotes();
    void TriggerStepAudio(int step);
    void PlaySingleNote(Note theNote, int channel);
    bool SaveProject(bool SaveAsNew);
    bool LoadProject(String filePath);
    void drawGrid();
    void drawNotes();
    void drawCursor();
    void MenuRender();
    int InMenuSelect = 0;
    bool RenderMenu = false;
    bool PlayFromCurrentScreen = false;
    void MoveAllNoteFromRightOfTheCourser(int x, int y);

    // פונקציית עזר לכתיבת ה-Header של קובץ ה-WAV
    void writeWavHeader(File &file, uint32_t dataSize, uint32_t sampleRate);
    void removeDuplicateNotes(std::vector<Note> &notes);
    void ExportAsWav();
    // void PlaySingleNote(Note theNote);
};