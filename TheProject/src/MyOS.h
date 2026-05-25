#pragma once
#include <Arduino.h>
#include "GlobalParentClass.h"
// #include "ChildClass.h"
#include <M5Cardputer.h>
// #include "Classes/MainMenu.h"
#include <elapsedMillis.h>
#include <newKeyboardHandle.h>
#include <SPI.h>
#include <FS.h>
#include <Wire.h>
#include <SD.h>
#include "Classes/MusicPlayerV2.h"
#include <ESP32Time.h>
// #include <esp_attr.h>

// extern RTC_DATA_ATTR int FastBootVar;
#define SCREEN_W 240
#define SCREEN_H 135
String getHeapInfoKB();

enum ItemType
{
    APP,
    CATEGORY,
    FILE_ITEM
};

struct MenuItem
{
    String name;
    uint16_t color;
    const uint16_t *image;
    int imageW;
    int imageH;
    ItemType type;
    int subMenuId;
    std::function<void()> onLaunch;
    String HelpText;
};

struct SubMenu
{
    String title;
    std::vector<int> indices;
};

class MyOS
{
public:
    M5Canvas TopBarSprite;
    ESP32Time rtc;

    M5Canvas sprite;
    M5Canvas sprite2;
    M5Canvas sprite3;
    //  M5Canvas sprite4;
    GlobalParentClass *currentApp = nullptr;
    GlobalParentClass *appToDelete = nullptr;
    bool saveScreenshot(String fileName);
    bool DeleteMusicPlayer = false;
    bool EditFromFile = false;
    char AskForAnyKey(String prompt);
    bool AskSomthing(String Question);
    String AskFromUserForString(String Question, bool NoSpecialChar = true, bool PasswordMode = false, bool OnlyDigit = false);
    int AskForColor(String Question, uint16_t DefaultColor = WHITE);
    elapsedSeconds TurnOffTimer;
    
    u_int TimeToTurnOff=1800;// 30 min
    // main menu
    int selectedRow = 0;
    int selectedCol = 0;
    struct MainMenuState
    {
        bool valid = false; // האם יש מצב שמור?
        int selectedRow = 0;
        int selectedCol = 0;
        bool inSubMenu = false;
        int parentIndex = 0;
        float camX = 0;
        float targetCamX = 0;
        float camY = 0;
        float targetCamY = 0;
        bool inFileRow;
        int selectedFileIndex;
    };
    MainMenuState savedMainMenu;
    // בתוך class MyOS:
    std::vector<MenuItem> allApps;
    std::vector<int> mainScreenIndices;
    std::vector<String> filesInMainMenu;
    void AddFilePathToArrayOfMAinMenu(String Path);
    void RemoveFilePathFromArrayOfMAinMenu(String Path);
    bool FileOpenFromMainMenu = false;

    int InExtraMenuID = 0;
    MyOS() : TopBarSprite(&M5Cardputer.Display), sprite(&M5Cardputer.Display), sprite2(&M5Cardputer.Display), sprite3(&M5Cardputer.Display) /* , sprite4(&M5Cardputer.Display) */ {}
    ~MyOS() {}
    void begin();
    void loop();
    void Draw();
    void ChangeMenu(GlobalParentClass *classReff);

    bool ChangePartitionLayoutToAdvanceOSinLauncher = false;
    MusicPlayerV2 *CurrentMusic = nullptr;
    void EnterMusicPlayer(bool Focus = true);
    void ExitMusicPlayer();
    int volume = 50;
    bool UI_SOUND = true;
    int FontSelected = 0;
    // app and game load
    String LastAppInstalledPath = "";
    bool TryToRunApp = false;
    bool RunAppSucsses = false;

    // theme_variable
    // int TOP_BAR_COLOR_1= 0x5800;
    // int TOP_BAR_COLOR_2= 0x7800;

    int BAR_COLOR_1 = 0x1d3f;
    int BAR_COLOR_2 = 0x1b18;
    int BAR_TEXT_COLOR = 0xFFFF;
    int BACKGROUND_COLOR = BLACK;
    bool ShowWallpaperInMainMenu = true;

    /*

    int MENU_1_MUSIC_COLOR_1= 0xFE00;
    int MENU_1_MUSIC_COLOR_2= 0x0000;

    int MENU_2_FILES_COLOR_1= 0x0000;
    int MENU_2_FILES_COLOR_2= 0xFE00;

    int MENU_3_BOT_COLOR_1= 0xFFFF;
    int MENU_3_BOT_COLOR_2= 0x012C;

    int MENU_4_NOTES_COLOR_1= 0x7006;
    int MENU_4_NOTES_COLOR_2= 0xE10C;


    int MENU_4_PAINT_COLOR_1= 0xFE00;
    int MENU_4_PAINT_COLOR_2= 0x012C;

    int MENU_4_COMPOSER_COLOR_1= 0xff5a;
    int MENU_4_COMPOSER_COLOR_2= 0xfc43;

    int MENU_5_EXTRA_COLOR_1= 0x0002;
    int MENU_5_EXTRA_COLOR_2= 0xFE80;

    int MENU_6_GAMES_COLOR_1= 0x2E9B;
    int MENU_6_GAMES_COLOR_2= 0x006B;

    int MENU_7_SETTINGS_COLOR_1= 0x8122;
    int MENU_7_SETTINGS_COLOR_2= 0x1820; */
    void getRGB_from_HEX(uint16_t hex565, uint8_t &r, uint8_t &g, uint8_t &b);
    void LoadTheme(String path, bool SetAsDefault = false);
    bool SaveCurrentTheme(String Custom_path = "");
    void ResetToDefaultTheme();
    String CurrentThemeSelectedPath = "";

    bool inEditThemeFromFileExplorer = false;
    // end of theme_variable

    void SetFontForSprite(M5Canvas &spriteReff);

    bool isInBruceLauncherNormalLayout();

    void PlayCuteEvilTone();

    enum class ClassesOfAdvanceOS
    {
        MAIN_MENU,
        MUSIC_PLAYER,
        NOTES,
        FILE_EXPLORER,

    };

    ClassesOfAdvanceOS classToLoad;
    bool fucisOnPlayer = false;
    // bool SampleGyro = false;
    NewKeyboardHandle NewKey;
    void SampleGyroscope();
    // main menu vars
    int currentMenuIndex = 0;
    bool WifiConnected = false;
    // top bar var
    bool showTopBar = true;
    bool TopBarInBottom = true;
    void UpdateTopBar();
    const int TopBarYsize = 18;

    // audio player Sector
    int SongsInTheList = 0;
    int InCurrentSong = 0;
    bool SongReady = false;
    const char *SongToPlay;
    //  bool StopPlay = true;

    void AddSongToTheList(const char *path);
    void ClearAllSong();
    void GetAllSoundFileFromFolderToPlaylist(const char *dirname); // to replace with GetFilesToList
    // String SongList[500];                                           // to replace with FileList

    // file handeling var
    void GetFilesToList(String FileExtension, String dirname);
    std::vector<String> FileList;

    bool FilePathHasExtension(const char *filename, const char *ext);

    String GetFileText(String path);

    bool deleteFromSd(FS fs, String path);

    String createFile(fs::FS &fs, const String &folderPath = "/", const String &baseName = "NewFile.txt");

    // setting sector
    void setBrightness(int brt);
    // float volume = 10;
    bool screenOff = false;
    bool haveSDcard = true;

    // float CameraPosX = 0;
    // float CameraPosY = 0;

    // void ChangeMenu(GlobalParentClass* newApp);

    // motion Sensor sector
    float CardputerRotationZ = 0.0;
    float CardputerRotationX = 0.0;
    float CardputerRotationY = 0.0;

    unsigned long lastTimeRotationUpdate = 0;

    // save and load

    String LastFolderPlaySongInSave = "";
    String CustomFontPathToLoadInSave = "";
    int ScreenDimInTimeSecond = 20;
    int TextSizeInMenuesSAVE = 1;
    int TextSizeIntextEditorSAVE = 1;
    bool showMemoryOnScreen = false;
    void saveSettings();
    void loadSettings();

    // file maneger Var
    String currentPath = "/";
    int FileIndexSelected = 0;
    std::vector<int> folderIndexHistory;
    String getFileNameFromPath(const String &path);

    int brightness = 100;

    bool TextEditorGetTextFromFile = false;
    String FileSelectedInFS = "";
    String GetExtensionLower(const char *filename);
    String FromFilePathToFolderPath(String FilePath);
    void ShowOnScreenMessege(String MSG, int TimeInMs = 4000);
    // step counter sector
    bool StepCounterWork = false;
    void StepCounterLoop();
    int steps = 0;
    // Audio audio;

    // timer Sector
    bool timerRunning = false;
    unsigned long lastTick = 0;
    void TimerStart(int Second);
    void TimerLoop();
    int remainingSeconds = 0;
    bool FastLoadGameFromFileExplorer = false;

    int loadFastBootVar();
    void SetFastBootVar(int nom);

    // int FastBoot = 0;//0 normal boot 1 fast boot 2 to record
    const String funnyLines[50] PROGMEM = {
        "Booting system, please pretend this is fast",
        "Loading critical components, mostly hope and luck",
        "System starting, expectations set extremely low",
        "Initializing hardware, crossing fingers quietly",
        "Thinking very hard with extremely limited resources",
        "Calculating something important, results questionable",
        "System ready, probably, maybe, do not ask",
        "Low memory detected, optimism also very low",
        "Heap is shrinking faster than my patience",
        "Stack overflow avoided, this time only",
        "Watchdog is watching, act natural please",
        "CPU running hot thoughts and bad decisions",
        "System encountered error, pretending nothing happened",
        "Unexpected error occurred, acting surprised now",
        "Error is feature, documentation coming someday",
        "Working as intended, please stop questioning it",
        "Nothing is broken, everything is slightly wrong",
        "User input detected, preparing to blame user",
        "Human mistake confirmed, system feels validated",
        "Button pressed, consequences will follow shortly",
        "Please wait patiently, system is doing its best",
        "Doing important stuff, do not unplug now",
        "Almost finished loading, just kidding, still loading",
        "System frozen, please enjoy the moment",
        "Rebooting fixes everything, including user confidence",
        "Debug mode enabled, confidence level disabled",
        "Random behavior detected, calling it a feature",
        "System confused, attempting confident response",
        "This should not happen, yet here we are",
        "Magic smoke still inside, operation continues",
        "Running on hope, caffeine, and interrupts",
        "Time critical task delayed, nobody noticed",
        "Memory leak suspected, ignoring it professionally",
        "Multitasking enabled, none of tasks happy",
        "Core one working, core two pretending",
        "Everything fine, just ignore the warning messages",
        "Warning ignored successfully, proceeding with confidence",
        "System busy, thinking about life choices",
        "Loading complete, starting new loading phase",
        "Operation successful, errors were purely cosmetic",
        "Please do not panic, system already panicked",
        "Waiting for something, unsure what exactly",
        "Task started, forgot why immediately",
        "System awake, motivation still sleeping",
        "Sensors initialized, trusting them blindly",
        "Network connected, internet mood unstable",
        "Processing data, making it worse slightly",
        "All systems nominal, definition of nominal unclear",
        "Shutdown requested, system pretending not to hear"};

private:
    int batteryPercent = 0;
    elapsedSeconds DimTimer;
    elapsedSeconds BatteryUpdateTimer = 21;

    // TaskHandle_t taskCore2Handle = NULL;

    // static void LoopCore2(void *pvParameters);
    //  unsigned long lastInputTime = 0;
};

inline MyOS os;