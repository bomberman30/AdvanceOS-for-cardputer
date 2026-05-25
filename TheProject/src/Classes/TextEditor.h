#pragma once
#include "./GlobalParentClass.h"
#include <M5Cardputer.h>
#include <elapsedMillis.h>
#include <FS.h>
#include <SD.h>

class MyOS;
class TextEditor : public GlobalParentClass
{
private:
    bool showMode2 = false;
    elapsedMillis ChangeTextView;
    // int TextSize = 2;
    void SetTypePos();
    int typePosX = 0;
    int typePosY = 0;

public:
    void ResetView();
    bool RenameMode = false;
    String FileName = "";
    bool RenameFileSafe(fs::FS &fs, const String &oldPath, const String &newPath);
    String FileRenameError = "";

    bool CameFromNotes = false;

    // M5Canvas sprite;
    int cameraY = 0;
  //  int cameraX = 0;

    int TargetCameraY = 0;
   // int TargetCameraX = 0;

    TextEditor(MyOS *os) : GlobalParentClass(os) {}
    ~TextEditor() {}
    void Begin() override;

    void Loop() override;
    void Draw() override;
    bool EditMode = false;
    String TextFileText = "";

    int EditAt = 0; // if 0 edit at the end
    void insertFromEnd(String &s, int fromEnd, const String &text);
    void removeFromEnd(String &s, int fromEnd, int count);

    String ShowBefore(int beforeIndex, const String &text);
    String ShowAfter(int afterIndex, const String &text);

    bool showMenu = false;

    void SaveCurrentEdit(bool exit);
    bool SaveStringToFile(const String &path, const String &data, fs::FS &fs);

    std::vector<String> MenuItems;
    void MenuSelected();
    int inMenuIndex = 0;







    std::vector<String> FileTextRow;
void splitTextToRows(String input, std::vector<String>& outputVector);

};
