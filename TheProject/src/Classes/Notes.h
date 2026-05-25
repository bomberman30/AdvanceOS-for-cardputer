#pragma once
#include "./GlobalParentClass.h"
#include <Arduino.h>
//#include <vector>
#include <M5Cardputer.h>

class Notes : public GlobalParentClass
{
private:
int cameraY=0;
int InNoteIDselect=0;

public:
    M5Canvas NoteSprite;

    Notes(MyOS *os) : GlobalParentClass(os), NoteSprite(&M5Cardputer.Display) {}
    ~Notes() {}
    struct NoteData
    {
        String NotePath;
        String TextFileContain;
    };

    std::vector<NoteData> NodesInSD;
     int NoteHight = 60;

    int destanceBetweenNotes = NoteHight+3;

    void Begin() override;

    void Loop() override;
    void DrawGrid();
    void DrawStatusBar();
    void DrawNoteCard(int index, int yPos, bool selected);
    void DrawCmdPanel();
    void Draw() override;

    void SetNotesVariable();
};
