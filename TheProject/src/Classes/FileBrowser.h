#pragma once
#include <Arduino.h>
#include "GlobalParentClass.h"
#include <M5Cardputer.h>
#include <SPI.h>
#include <FS.h>
#include <Wire.h>
#include <SD.h>
#include <elapsedMillis.h>
#include <PNGdec.h>

#define MAX_FILES 100
class FileBrowser : public GlobalParentClass
{
public:
    //  M5Canvas sprite;

    // std::vector<String> FileFounds;
    String FileOrFolderToMove = "";
    String FileFound[MAX_FILES];
    // FileBrowser(MyOS *os);
    FileBrowser(MyOS *os) : GlobalParentClass(os) {}
    ~FileBrowser()
    {
    }
    void Begin() override;
    void Loop() override;
    void Draw() override;
    bool hasExtension(const char *filename, const char *ext);
    bool fileExists(const String &path);
bool FlashBinFile(const char *binPath);



    void ShowCurrentFilesInDir(fs::FS &fs,const char *dirname);
    // void GetAllSoundFileFromFolderToPlaylist(const char *dirname);
bool BrowseLittleFS=false;
fs::FS& GetActiveFS(bool browseLittleFS);


    // void listFiles(fs::FS &fs, const char *dirname, uint8_t levels);
    bool isFolderEmpty(File dir);
    // String createFile(fs::FS &fs, const String &folderPath = "/", const String &baseName = "NewFile.txt");
    String createFolder(fs::FS &fs, const String &parentFolder = "/", const String &baseName = "newFolder");

    bool CopyFile(fs::FS &fs, const String &srcPath, const String &dstPath);
    bool CopyFolder(fs::FS &fs, const String &srcFolder, const String &dstFolder);
    bool MoveOrCopy(fs::FS &fs, const String &fileOrFolderPath, const String &dstFolderPath, bool Copy);
String MakeUniquePath(fs::FS &fs, const String &dstPath);
void Show_BMP_picture(String fileName);


private:
//void EmulateSdCardAsUSB();
void RefrashImage();

struct FileEntry {
    String path;
    bool isDir;
};
    enum class CurrentlyImageOpen
    {
        GIF,
        JPG,
        PNG,
        BMP,
        USB,
    };
    CurrentlyImageOpen CurrentImageOpenType;


    FileEntry FileFoundE[MAX_FILES];





std::vector<String> File_ID_Marked;
bool InFileMarkMode=false;
bool multipleFileOrFolderToMove=false;
void Mark_UnmarkFile();
bool IsFileMarked(const String& id);


    void DrawFileOptionMenu();
    bool RenderFileOptionMenu = false;
    int InFileOptionMenuIndex = 0;
    // int FirsTimePressTimer = 0;
    bool FolderEmpty = true;
    //  String entries[128];
    int fileCount = 0;
    // String selectedFile = "";

    void GoUpOneFolder();
    void FileOfFolderSelected();
    void SelectMenuItem();
    void ExecuteFileOrFolder();
    int FileListYoffset = -40;
    float cameraYpos = 0;
    float cameraYposMenu = 0;

    // float targetCameraY = 0;
    std::vector<String> FileOptionMenuItems;
    void CloseFileMenu();
    void OpenFileMenu();

    //void SortFiles();


    // image sector
    bool ShowPicOnScreen = false;


    void UndrawPic();
};
