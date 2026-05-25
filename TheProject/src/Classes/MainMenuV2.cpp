#include "MainMenuV2.h"
#include "Pic/MainMenu2Pic.cpp"
#include "Pic/MainMenuPic.c"
#include "FileBrowser.h"
#include "MusicCreator.h"
#include "FUN/UnitAudioPlayerController.h"
#include "Setting.h"
#include "Painter.h"
#include "tools/Calculator.h"
#include "tools/IRedit.h"
#include "PasswordVault.h"
#include "GamesV2.h"
#include "wifiSpectrum.h"
#include "Timer.h"
#include "AlarmClock.h"
#include "LoanCalculator.h"
#include "RecordV2.h"
#include "tools/ColorCode.h"
#include "partition_maneger.h"
#include "FUN/BotyV2.1.h"
#include "Function.h"
#include "Notes.h"
#include "Theme_Maneger.h"
#include "Classes/EspProjectRC/EspProjectRemoteControl.h"

#include "Cardputer_Remote.h"
#include "MassStorage.h"
#include "GameKeyRemapper.h"
bool ShowInfoWindow = false;

extern const unsigned short mainMenuBackground[32400];
#define APP_COLOE NAVYBLUE
// ══════════════════════════════════════════════════════════════
//  Begin — הגדרת כל האפליקציות
// ══════════════════════════════════════════════════════════════
void MainMenuV2::Begin()
{
    mainOS->FileOpenFromMainMenu = false;

    // ── הגדרת כל האפליקציות במקום אחד ───────────────────────
    //    כל item מקבל אינדקס קבוע בתוך mainOS->allApps
    //
    // ── CATEGORY items ראשונים (indices 0,1,2) ──────────────
    // ── CATEGORY items (indices 0–5) ─────────────────────────
    mainOS->allApps = {
        // 0 – Sound
        {"Sound", APP_COLOE, Speaker32, 32, 32, CATEGORY, 0, nullptr},
        // 1 – Fun Stuff
        {"Fun Stuff", APP_COLOE, BallonsPA, 32, 32, CATEGORY, 1, nullptr},
        // 2 – Tools
        {"Tools", APP_COLOE, toolPic, 32, 32, CATEGORY, 2, nullptr},
        // 3 – placeholder
        {"Category 4", APP_COLOE, nullptr, 0, 0, CATEGORY, 3, nullptr},
        // 4 – placeholder
        {"Category 5", APP_COLOE, nullptr, 0, 0, CATEGORY, 4, nullptr},
        // 5 – placeholder
        {"Category 6", APP_COLOE, nullptr, 0, 0, CATEGORY, 5, nullptr},
        // ── Apps (indices 6–25) ──────────────────────────────
        // 6  – Music Player
        {"Music\nPlayer", 0x07FF, Headphone, 32, 32, APP, -1, []()
         { os.EnterMusicPlayer(); }, "Music player\n Press + or - to change Volume\n Press > or < to change song\n press [ or ] to\n fast forward or backwords"},
        // 7  – Music Composer
        {"Music\nComposer", 0xF81F, composer, 32, 32, APP, -1, []()
         { os.ChangeMenu(new MusicCreator(&os)); }, "Composer, Create Your Music \n Press \\ To Add Or Delete Note\n Press ENTER to Play\n Press SPACE To play From Courser\n Press D F G T To move note\n to any direction\n S to save A To Load From File"},
        // 8  – Unit AudioPlayer
        {"Unit\nAudioPlayer\nN9301", 0xFFE0, MusicPic, 32, 32, APP, -1, []()
         { os.ChangeMenu(new UnitAudioPlayerController(&os)); }, "Unit Audio Player\n Play Audio From\n Connected Unit Module"},
        // 9  – Record To SD
        {"Record\nTo SD", 0xFFE0, MicPA, 32, 32, APP, -1, []()
         { os.ChangeMenu(new RecordV2(&os)); }, "Record Audio To SD Card\n Press ENTER to Start\n Press ENTER again to Stop\n Files saved as WAV"},
        // 10 – Games
        {"Games", 0x07E0, ChessPA, 32, 32, APP, -1, []()
         { os.ShowOnScreenMessege("Loading...",1); os.ChangeMenu(new GamesV2(&os)); }, "Games\n Arduboy Games\n Keys: Arrows and Z and X\n Press [ or ] to change volume\n Press 1 to change color palette"},
        // 11 – Mr.BOT
        {"Mr.BOT", 0x001F, botPA, 32, 32, APP, -1, []()
         { os.ChangeMenu(new BotyV2(&os)); }, "Enjoy To Talk To Our Chatbot"},
        // 12 – Painter
        {"Painter", 0x07E0, PAinterPA, 32, 32, APP, -1, []()
         { os.ChangeMenu(new Painter(&os)); }, "Painter !!\n ESC Open Menu\n See shortcut in Menu\n Hold FN to Move Slower\n Press 2 or 3 To Zoom In Or Out"},
        // 13 – Calculator
        {"Calculator", 0x001F, PACalc, 32, 32, APP, -1, []()
         { os.ChangeMenu(new Calculator(&os)); }, "Calculator\n Basic Math Operations\n Press Numbers and Operators"},
        // 14 – Loan Calculator
        {"Loan\nCalculator", 0x001F, PACalc, 32, 32, APP, -1, []()
         { os.ChangeMenu(new LoanCalculator(&os)); }, "Loan Calculator\n Calculate Monthly Payments\n Enter Principal Amount\n Enter Interest Rate\n Enter Loan Duration"},
        // 15 – Create IR File
        {"Create\nIR File", 0xFD20, irRemote, 32, 32, APP, -1, []()
         { os.ChangeMenu(new IR_Editor(&os)); }, "IR Remote Editor\n Create And Edit\n Infrared Signal Files\n Send IR Commands\n Save To SD Card"},
        // 16 – Password Vault
        {"Password\nVault", 0xF800, KEYPA, 32, 32, APP, -1, []()
         { os.ChangeMenu(new PasswordVault(&os)); }, "Password Vault\n Securely Store Passwords\n Press + to Add Entry\n Press DEL to Delete\n Data Encrypted On SD"},
        // 17 – Wifi Spectrum
        {"Wifi\nSpectrum", 0xF800, WIFI32, 32, 32, APP, -1, []()
         { os.ChangeMenu(new wifiSpectrum(&os)); }, "WiFi Spectrum Analyzer\n Scan Nearby Networks\n View Signal Strength\n Channel Distribution"},
        // 18 – Timer
        {"Timer", 0xF800, timerPA, 32, 32, APP, -1, []()
         { os.ChangeMenu(new Timer(&os)); }, "Timer\n Countdown and Stopwatch\n Press ENTER to Start or Stop\n Press ESC to Reset"},
        // 19 – Alarm Clock
        {"Alarm\nClock", 0xF800, aLArmPic, 32, 32, APP, -1, []()
         { os.ChangeMenu(new AlarmClock(&os)); }, "Alarm Clock\n Set time and the cardputer will go to deep sleep, do not turn off the switch"},
        // 20 – Color Code
        {"Color\nCode", 0xF800, ColorCodePic, 32, 32, APP, -1, []()
         { os.ChangeMenu(new ColorPicker(&os)); }, "Color Code Picker\n Browse And Pick Colors\n View HEX and RGB Values\n Press Arrows to Navigate\n Press ENTER to Select"},
        // 21 – Partition Viewer
        {"Partition\nViewer", 0xF800, PicPartition, 32, 32, APP, -1, []()
         { os.ChangeMenu(new partition_maneger(&os)); }, "Partition Viewer of the ESP32 flash"},
        // 22 – Cardputer INFO
        {"Cardputer", APP_COLOE, PCIcon, 32, 32, APP, -1, []()
         { ShowInfoWindow = true; }, "Cardputer Info\n View Device Information\n CPU and Memory Stats\n Firmware Version\n Hardware Details"},
        // 23 – File Browser
        {"Files", APP_COLOE, folderPIC, 32, 32, APP, -1, []()
         { os.ShowOnScreenMessege("Loading...",1);
   os.ChangeMenu(new FileBrowser(&os)); }, "File Explorer\n Move Copy Delete\n Multiple Files\n Text Edit\n Image View"},
        // 24 – Settings
        {"Settings", APP_COLOE, GearPic, 32, 32, APP, -1, []()
         { os.ChangeMenu(new Setting(&os)); }, "Settings\n The OS Setting"},
        // 25 – Notes
        {"Notes", APP_COLOE, WriteNotesPA, 32, 32, APP, -1, []()
         { os.ChangeMenu(new Notes(&os)); }, "Notes\n Save And Edit\n Multiple Notes"},
        // 26
        {"ESP Project\nRemote Controll", APP_COLOE, esp32PIC, 32, 32, APP, -1, []()
         { os.ChangeMenu(new EspProjectRemoteControl(&os)); }, "Remote Controll on your esp project by ESP NOW"},
        // 27
        {"Theme Maneger", APP_COLOE, themePA, 32, 32, APP, -1, []()
         { os.ChangeMenu(new Theme_Maneger(&os)); }, "Edit The OS colors"},
        // 28
        {"Quick IR \nRemote Signals", APP_COLOE, nullptr, 32, 32, APP, -1, []()
         {delay(500); os.ChangeMenu(new Cardputer_Remote(&os)); }, "Beta"},
        // 29
        {"Game Key\nRemapper", APP_COLOE, nullptr, 32, 32, APP, -1, []()
         { os.ChangeMenu(new GameKeyRemapper(&os)); }, "Beta"},
 /*                         {"Mass Storage", APP_COLOE, nullptr, 32, 32, APP, -1, []()
                { os.ChangeMenu(new MassStorage(&os)); }, "Connect To Your PC as Disk On Key"},  */
    };
    // ── סאב-מנואים — indices של allApps ──────────────────────
    // ── סאב-מנואים ───────────────────────────────────────────
    subMenus.resize(6); // 3 קיימים + 3 חדשים

    // subMenu 0: Sound Stuff
    subMenus[0].title = "Sound Stuff";
    // subMenus[0].indices = {3, 4, 5, 6};
    subMenus[0].indices = {6, 7, 8, 9}; // Sound

    // subMenu 1: Fun Stuff
    subMenus[1].title = "Fun Stuff";
    subMenus[1].indices = {10, 11,29}; // Fun Stuff
    // subMenu 2: Tools
    subMenus[2].title = "Tools";
    subMenus[2].indices = {12, 13, 14, 15, 16,                  // Tools
                           17, 18, 19, 20, 21, 25, 26, 27, 28}; // subMenu 3: placeholder
    subMenus[3].title = "Category 4";
    subMenus[3].indices = {};
    // subMenu 4: placeholder
    subMenus[4].title = "Category 5";
    subMenus[4].indices = {};
    // subMenu 5: placeholder
    subMenus[5].title = "Category 6";
    subMenus[5].indices = {};

    if (mainOS->mainScreenIndices.empty())
    {
        mainOS->mainScreenIndices = {22, 0, 23, 1, 2, 24, 25, 12, 10};
    }
    //   ^    ^   ^    ^  ^   ^    ^    ^   ^
    // Cardp Sound File Fun Tools Set Notes Paint Games
    currentIndices = &mainOS->mainScreenIndices;
    inSubMenu = false;

    // ── שחזור מצב שמור ───────────────────────────────────────
    if (mainOS->savedMainMenu.valid)
    {
        mainOS->selectedRow = mainOS->savedMainMenu.selectedRow;
        mainOS->selectedCol = mainOS->savedMainMenu.selectedCol;
        inSubMenu = mainOS->savedMainMenu.inSubMenu;
        parentIndex = mainOS->savedMainMenu.parentIndex;
        camX = mainOS->savedMainMenu.camX;
        targetCamX = mainOS->savedMainMenu.targetCamX;
        camY = mainOS->savedMainMenu.camY;
        targetCamY = mainOS->savedMainMenu.targetCamY;
        inFileRow = mainOS->savedMainMenu.inFileRow;
        selectedFileIndex = mainOS->savedMainMenu.selectedFileIndex;
        if (inSubMenu)
        {
            if (!mainOS->CurrentThemeSelectedPath.isEmpty())
            {
                CurrentThemePath = mainOS->FromFilePathToFolderPath(mainOS->CurrentThemeSelectedPath);
            }
            DrawOnceIfNoAnimation();

            int subId = mainOS->allApps[mainOS->mainScreenIndices[parentIndex]].subMenuId;
            currentIndices = &subMenus[subId].indices;
        }
    }
    else
    {
        mainOS->selectedRow = 0;
        mainOS->selectedCol = 0;
        camX = targetCamX = 0;
    }

    if (!mainOS->CurrentThemeSelectedPath.isEmpty())
    {
        CurrentThemePath = mainOS->FromFilePathToFolderPath(mainOS->CurrentThemeSelectedPath);
    }
    Draw();
}
// ══════════════════════════════════════════════════════════════
//  עוזרי גריד
// ══════════════════════════════════════════════════════════════

// אינדקס ה-item מ-row,col
static inline int gridIndex(int row, int col)
{
    return row * GRID_COLS + col;
}

// ══════════════════════════════════════════════════════════════
//  עוזרי סאב-מנו
// ══════════════════════════════════════════════════════════════

/* void MainMenuV2::moveSubMenuTo(int col)
{
    col = constrain(col, 0, currentItemCount - 1);
    mainOS->selectedCol = col;
    targetCamX = mainOS->selectedCol * ICON_STRIDE - (SCREEN_W / 2) + (SUB_ICON_SIZE / 2) + 150;
}
 */
void MainMenuV2::moveSubMenuTo(int col)
{
    col = constrain(col, 0, (int)currentIndices->size() - 1);
    mainOS->selectedCol = col;
    int step = SUB_ICON_SIZE + ICON_GAP_HORIZON;
    targetCamX = mainOS->selectedCol * step;
}
void MainMenuV2::updateCamera()
{
    if (noAnimation)
    {
        camX = targetCamX;
        camY = targetCamY;
        return;
    }
    camX += (targetCamX - camX) * ANIM_SPEED;
    if (fabs(camX - targetCamX) < 0.3f)
        camX = targetCamX;
    camY += (targetCamY - camY) * ANIM_SPEED;
    if (fabs(camY - targetCamY) < 0.3f)
        camY = targetCamY;
}
void MainMenuV2::openSubMenu(int subMenuId)
{
    inSubMenu = true;
    parentIndex = gridIndex(mainOS->selectedRow, mainOS->selectedCol);
    currentIndices = &subMenus[subMenuId].indices;
    mainOS->selectedCol = 0;
    camX = targetCamX = 0;
    moveSubMenuTo(0);
    camX = targetCamX;
}

void MainMenuV2::closeSubMenu()
{
    inSubMenu = false;
    if (noAnimation)
    {
        DrawFirstFrame = true;
    }
    currentIndices = &mainOS->mainScreenIndices;
    mainOS->selectedRow = parentIndex / GRID_COLS;
    mainOS->selectedCol = parentIndex % GRID_COLS;
}
void MainMenuV2::Loop()
{
    // ── אם המסך כבוי — לא עושים כלום ─────────────────────────
    if (mainOS->screenOff)
        return;

    // ══════════════════════════════════════════════════════════
    //  קריאת מקשים
    // ══════════════════════════════════════════════════════════
    bool curLeft = mainOS->NewKey.ifKeyJustPress(',');
    bool curRight = mainOS->NewKey.ifKeyJustPress('/');
    bool curUp = mainOS->NewKey.ifKeyJustPress(';');
    bool curDown = mainOS->NewKey.ifKeyJustPress('.');
    bool curEnter = mainOS->NewKey.ifKeyJustPress(KEY_ENTER);
    bool curBack = mainOS->NewKey.ifKeyJustPress('`');

    // ── הצגת מידע על אפליקציה בזמן לחיצה על 'i' ──────────────
    Show_APP_INFO = M5Cardputer.Keyboard.isKeyPressed('i');

    // ── צליל לכל ניווט / פעולה ─────────────────────────────────
    if (curLeft || curRight || curUp || curDown || curEnter || curBack)
    {
        mainOS->PlayCuteEvilTone();
    }

    // ══════════════════════════════════════════════════════════
    //  מצב: תפריט ראשי (לא בסאב-מנו)
    // ══════════════════════════════════════════════════════════
    if (!inSubMenu)
    {
        // ── ESC סוגר את חלון INFO ─────────────────────────────
        if (curBack)
        {
            if (ShowInfoWindow)
            {
                ShowInfoWindow = false;
                DrawFirstFrame = true;
            }
        }

        // ── אם חלון INFO פתוח — רק מצייר ויוצא ─────────────
        if (ShowInfoWindow)
        {
            if (noAnimation)
            {
                bool anyKey = curLeft || curRight || curUp || curDown || curEnter || curBack;
                if (anyKey)
                    Draw();
            }
            else
            {
                Draw();
            }
            return;
        }

        // ══════════════════════════════════════════════════════
        //  ניווט בשורת הקבצים (inFileRow) — חייב להיות ראשון
        //  כדי שלחצני UP/DOWN לא ייתפסו בניווט הגריד הכללי
        // ══════════════════════════════════════════════════════
        if (inFileRow)
        {
            // פונקציה עזר לחישוב Y של שורת קבצים
            // totalRows = כמה שורות יש בגריד הרגיל
            int totalGridRows = ((int)mainOS->mainScreenIndices.size() + GRID_COLS - 1) / GRID_COLS;

            auto updateFileCamY = [&]()
            {
                // שורה נוכחית בתוך גריד הקבצים
                int fileRow = selectedFileIndex / GRID_COLS;

                // Y של השורה הזו = כל שורות הגריד הרגיל + שורות הקבצים שלפניה
                targetCamY = (totalGridRows + fileRow) * (ICON_SIZE + GRID_GAP_Y);
            };

            if (curRight && !prevRight)
            {
                selectedFileIndex = min(selectedFileIndex + 1,
                                        (int)mainOS->filesInMainMenu.size() - 1);
                updateFileCamY();
            }

            if (curLeft && !prevLeft)
            {
                selectedFileIndex = max(selectedFileIndex - 1, 0);
                updateFileCamY();
            }

            if (curDown && !prevDown)
            {
                int next = selectedFileIndex + GRID_COLS;
                if (next < (int)mainOS->filesInMainMenu.size())
                {
                    selectedFileIndex = next;
                    updateFileCamY(); // ← מצלמה עוקבת
                }
            }

            if (curUp && !prevUp)
            {
                int prev = selectedFileIndex - GRID_COLS;
                if (prev >= 0)
                {
                    selectedFileIndex = prev;
                    updateFileCamY(); // ← מצלמה עוקבת
                }
                else
                {
                    // חזרה לגריד — המצלמה חוזרת לשורה האחרונה של הגריד
                    inFileRow = false;
                    selectedFileIndex = -1;
                    targetCamY = mainOS->selectedRow * (ICON_SIZE + GRID_GAP_Y);
                }
            }

            // ENTER — פתח קובץ
            if (curEnter && !prevEnter)
            {
                saveState();
                String path = mainOS->filesInMainMenu[selectedFileIndex];
                mainOS->FileOpenFromMainMenu = true;
                mainOS->FileSelectedInFS = path;
                os.ChangeMenu(new FileBrowser(&os));
                return;
            }

            prevLeft = curLeft;
            prevRight = curRight;
            prevUp = curUp;
            prevDown = curDown;
            prevEnter = curEnter;
            prevBack = curBack;
            updateCamera();
            if (noAnimation)
            {
                bool anyKey = curLeft || curRight || curUp || curDown || curEnter || curBack;
                if (anyKey)
                {
                    if (!inSubMenu)
                    {
                        DrawFirstFrame = true;
                    }
                    Draw();
                }
            }
            else
            {
                Draw();
            }
            return;
        }

        // ══════════════════════════════════════════════════════
        //  ניווט בגריד הראשי (רק כשלא inFileRow)
        // ══════════════════════════════════════════════════════

        // ימינה — אינדקס +1 (זזים שמאל-ימין ואוטומטית שורה)
        if (curRight && !prevRight)
        {
            int idx = gridIndex(mainOS->selectedRow, mainOS->selectedCol) + 1;
            if (idx < (int)mainOS->mainScreenIndices.size())
            {
                mainOS->selectedRow = idx / GRID_COLS;
                mainOS->selectedCol = idx % GRID_COLS;
            }
        }

        // שמאלה — אינדקס -1
        if (curLeft && !prevLeft)
        {
            int idx = gridIndex(mainOS->selectedRow, mainOS->selectedCol) - 1;
            if (idx >= 0)
            {
                mainOS->selectedRow = idx / GRID_COLS;
                mainOS->selectedCol = idx % GRID_COLS;
            }
        }

        // למטה — שורה +1, ואם אין עוד שורות עוברים לשורת הקבצים
        if (curDown && !prevDown)
        {
            int newRow = mainOS->selectedRow + 1;
            int idx = gridIndex(newRow, mainOS->selectedCol);

            if (idx < (int)mainOS->mainScreenIndices.size())
            {
                // יש עוד שורה בגריד — זזים אליה
                mainOS->selectedRow = newRow;
                targetCamY = mainOS->selectedRow * (ICON_SIZE + GRID_GAP_Y);
                inFileRow = false;
            }
            else if (!mainOS->filesInMainMenu.empty())
            {
                // אין שורה — יורדים לשורת הקבצים
                inFileRow = true;
                selectedFileIndex = 0;
            }
        }

        // למעלה — שורה -1, ואם כבר בשורה 0 לא עושים כלום
        if (curUp && !prevUp)
        {
            if (mainOS->selectedRow > 0)
            {
                mainOS->selectedRow--;
                targetCamY = mainOS->selectedRow * (ICON_SIZE + GRID_GAP_Y);
            }
            // (inFileRow כבר טופל למעלה בבלוק הנפרד)
        }

        // ENTER — פתח סאב-מנו או הפעל אפליקציה
        if (curEnter && !prevEnter)
        {
            int idx = gridIndex(mainOS->selectedRow, mainOS->selectedCol);
            MenuItem &item = currentItem(idx);

            if (item.type == CATEGORY)
            {
                // פתיחת סאב-מנו של קטגוריה
                openSubMenu(item.subMenuId);
            }
            else if (item.onLaunch)
            {
                // שמירת מצב והפעלת האפליקציה
                saveState();
                item.onLaunch();
                return;
            }
        }
    }
    // ══════════════════════════════════════════════════════════
    //  מצב: סאב-מנו (תפריט קטגוריה אופקי)
    // ══════════════════════════════════════════════════════════
    else
    {
        // ימינה / שמאלה — זזים בין אייקוני הסאב-מנו
        if (curRight && !prevRight)
            moveSubMenuTo(mainOS->selectedCol + 1);

        if (curLeft && !prevLeft)
            moveSubMenuTo(mainOS->selectedCol - 1);

        // ENTER — הפעל את האפליקציה הנבחרת
        if (curEnter && !prevEnter)
        {
            MenuItem &item = currentItem(mainOS->selectedCol);
            if (item.onLaunch)
            {
                saveState();
                item.onLaunch();
                return;
            }
        }

        // ESC / ` — חזרה לתפריט הראשי
        if (curBack && !prevBack)
            closeSubMenu();
    }

    // ══════════════════════════════════════════════════════════
    //  עדכון prev-flags (רק אם לא יצאנו בreturn למעלה)
    // ══════════════════════════════════════════════════════════
    prevLeft = curLeft;
    prevRight = curRight;
    prevUp = curUp;
    prevDown = curDown;
    prevEnter = curEnter;
    prevBack = curBack;

    updateCamera();

    if (noAnimation)
    {
        // צייר רק אם הייתה פעולה (מקש נלחץ)
        bool anyKey = curLeft || curRight || curUp || curDown || curEnter || curBack;
        if (anyKey)
        {
            if (!inSubMenu)
            {
                DrawFirstFrame = true;
            }
            Draw();
        }
    }
    else
    {
        Draw();
    }
}
void MainMenuV2::drawFileRow()
{
    if (mainOS->filesInMainMenu.empty())
        return;

    int totalRows = ((int)mainOS->mainScreenIndices.size() + GRID_COLS - 1) / GRID_COLS;
    int separatorY = GRID_PAD_Y + totalRows * (ICON_SIZE + GRID_GAP_Y) - GRID_GAP_Y / 2 - (int)camY;

    auto &target = noAnimation
                       ? (LovyanGFX &)M5Cardputer.Display
                       : (LovyanGFX &)mainOS->sprite;

    target.drawFastHLine(0, separatorY, SCREEN_W, 0x4A49);
    target.drawString("Files", 20, separatorY - 2);

    // ── שלב 1: ציור כל ריבועי הרקע של שורת הקבצים ──
    /*    if (noAnimation)
       {
           for (int row = 0; row < totalRows; row++)
           {
               for (int col = 0; col < GRID_COLS; col++)
               {
                   int sx = GRID_PAD_X + col * (ICON_SIZE + GRID_GAP_X);
                   int sy = GRID_PAD_Y + (totalRows + row) * (ICON_SIZE + GRID_GAP_Y) - (int)camY;

                   if (sy + ICON_SIZE < 0 || sy > SCREEN_H)
                       continue;

                   int iconDrawSize = ICON_SIZE - 10;
                   int ox = sx + (ICON_SIZE - iconDrawSize) / 2;
                   int oy = sy + (ICON_SIZE - iconDrawSize) / 2 - 6;

                   int bgX = ox - 4;
                   int bgY = oy - 4;
                   int bgW = iconDrawSize + 15;
                   int bgH = (ICON_SIZE - (oy - sy)) + 12;

                   M5Cardputer.Display.fillRoundRect(bgX, bgY, bgW, bgH, 5, mainOS->BACKGROUND_COLOR);
               }
           }
       } */
    int selectedI = -1, selectedSx = 0, selectedSy = 0;
    MenuItem selectedItem;

    for (int i = 0; i < (int)mainOS->filesInMainMenu.size(); i++)
    {
        int col = i % GRID_COLS;
        int row = i / GRID_COLS;

        int sx = GRID_PAD_X + col * (ICON_SIZE + GRID_GAP_X);
        int sy = GRID_PAD_Y + (totalRows + row) * (ICON_SIZE + GRID_GAP_Y) - (int)camY;

        if (sy + ICON_SIZE < 0 || sy > SCREEN_H)
            continue;

        bool selected = (inFileRow && i == selectedFileIndex);

        String name = mainOS->filesInMainMenu[i];
        int slash = name.lastIndexOf('/');
        if (slash >= 0)
            name = name.substring(slash + 1);

        MenuItem fileItem;
        fileItem.name = name;
        fileItem.color = 0x4A69;

        String ext = mainOS->GetExtensionLower(name.c_str());
        if (ext == "ard" || ext == "gb" || ext == "gbc")
            fileItem.image = GameFileIcon;
        else if (ext == "nes")
            fileItem.image = NesController;
        else if (ext == "bmp" || ext == "png")
            fileItem.image = PicIcon;
        else if (ext == "txt")
            fileItem.image = TextFileIcon;
        else if (ext == "mc")
            fileItem.image = composer;
        else if (ext == "none")
            fileItem.image = folderPIC;
        else
            fileItem.image = nullptr;

        fileItem.imageW = 0;
        fileItem.imageH = 0;
        fileItem.type = FILE_ITEM;
        fileItem.subMenuId = -1;

        if (selected)
        {
            selectedI = i;
            selectedSx = sx;
            selectedSy = sy;
            selectedItem = fileItem;
            continue;
        }

        drawIcon(fileItem, sx, sy, ICON_SIZE, ICON_SIZE, false);
    }

    if (selectedI >= 0)
        drawIcon(selectedItem, selectedSx, selectedSy, ICON_SIZE, ICON_SIZE, true);
}
// ══════════════════════════════════════════════════════════════
//  ציור אייקון (משמש גם גריד וגם סאב-מנו)
// ══════════════════════════════════════════════════════════════
void MainMenuV2::drawIcon(MenuItem &item, int sx, int sy, int cellW, int cellH, bool selected)
{
    int iconDrawSize = min(cellW, cellH) - 10;
    int ox = sx + (cellW - iconDrawSize) / 2;
    int oy = sy + (cellH - iconDrawSize) / 2 - 6;

    // ===== רקע שחור כשאין אנימציה =====
    /*     if (noAnimation)
        {
            int bgX = ox - 4;
            int bgY = oy - 4;
            int bgW = iconDrawSize + 15;
            int bgH = (cellH - oy + sy) + 12; // גובה עד סוף הטקסט + 2px
            M5Cardputer.Display.fillRoundRect(bgX, bgY, bgW, bgH, 5, mainOS->BACKGROUND_COLOR);
            // M5Cardputer.Display.fillRect(bgX, bgY, bgW, bgH, TFT_BLACK);
        }
     */
    // helper — מצביע על היעד (מסך ישיר או ספרייט)
    auto drawPixelTarget = [&](int x, int y, uint16_t c)
    {
        if (noAnimation)
            M5Cardputer.Display.drawPixel(x, y, c);
        else
            mainOS->sprite.drawPixel(x, y, c);
    };

    String AppImagePath = CurrentThemePath + "/" + item.name + ".png";
    AppImagePath.replace("\n", " ");
    if (noAnimation && SD.exists(AppImagePath.c_str()))
    {
        // os.ShowOnScreenMessege("ddddd", 1000);fff
        IMG_x_POS = ox;
        IMG_y_POS = oy;
        DrawPNG(AppImagePath);

        if (selected)
        {
            // צייר שוב מעל התמונה
            M5Cardputer.Display.drawRoundRect(ox - 2, oy - 2, iconDrawSize + 4, iconDrawSize + 4, 8, TFT_WHITE);
            M5Cardputer.Display.drawRoundRect(ox - 1, oy - 1, iconDrawSize + 2, iconDrawSize + 2, 7, TFT_WHITE); // קו כפול לעובי
        }
    }
    else if (item.image != nullptr)
    {
        if (item.type == ItemType::FILE_ITEM)
        {
            drawImageTransparent(ox, oy, 32, 32, item.image, drawPixelTarget, 0xF800);

            if (selected)
            {
                if (noAnimation)
                    M5Cardputer.Display.drawRoundRect(ox - 2, oy - 2, iconDrawSize + 4, iconDrawSize + 4, 8, TFT_WHITE);
                else
                    mainOS->sprite.drawRoundRect(ox - 2, oy - 2, iconDrawSize + 4, iconDrawSize + 4, 8, TFT_WHITE);
            }
        }
        else
        {
            int imgX = sx + (cellW - item.imageW) / 2;
            int imgY = sy + (cellH - item.imageH) / 2 - 6;

            drawImageTransparent(imgX, imgY, item.imageW, item.imageH, item.image, drawPixelTarget, 0xF800);

            if (selected)
            {
                if (noAnimation)
                    M5Cardputer.Display.drawRoundRect(ox - 2, oy - 2, iconDrawSize + 4, iconDrawSize + 4, 8, TFT_WHITE);
                else
                    mainOS->sprite.drawRoundRect(ox - 2, oy - 2, iconDrawSize + 4, iconDrawSize + 4, 8, TFT_WHITE);
            }
        }
    }
    else // no uimage from sd and no from sofftware
    {
        uint16_t color = item.color;
        if (!selected)
        {
            uint8_t r = ((color >> 11) & 0x1F) >> 1;
            uint8_t g = ((color >> 5) & 0x3F) >> 1;
            uint8_t b = (color & 0x1F) >> 1;
            color = (r << 11) | (g << 5) | b;
        }

        if (noAnimation)
        {
            M5Cardputer.Display.fillRoundRect(ox, oy, iconDrawSize, iconDrawSize, 6, color);
            if (selected)
                M5Cardputer.Display.drawRoundRect(ox - 2, oy - 2, iconDrawSize + 4, iconDrawSize + 4, 8, TFT_WHITE);
        }
        else
        {
            mainOS->sprite.fillRoundRect(ox, oy, iconDrawSize, iconDrawSize, 6, color);
            if (selected)
                mainOS->sprite.drawRoundRect(ox - 2, oy - 2, iconDrawSize + 4, iconDrawSize + 4, 8, TFT_WHITE);
        }
    }

    // ===== טקסט (תמיכה ב-2 שורות) =====
    String name = item.name;
    int splitIndex = name.indexOf('\n');
    int centerX = sx + cellW / 2;
    int baseY = sy + cellH;

    // בחר יעד ציור טקסט
    auto &target = noAnimation
                       ? (LovyanGFX &)M5Cardputer.Display
                       : (LovyanGFX &)mainOS->sprite;

    target.setTextDatum(MC_DATUM);
    target.setTextSize(1);

    // shadow (רק במסך ראשי)
    if (!inSubMenu)
    {
        target.setTextColor(BLACK);
        if (splitIndex != -1)
        {
            target.drawString(name.substring(0, splitIndex), centerX + 1, baseY - 6 + 2);
            target.drawString(name.substring(splitIndex + 1), centerX + 1, baseY + 4);
        }
        else
        {
            target.drawString(name, centerX + 1, baseY + 1);
        }
    }

    target.setTextColor(selected ? BROWN : LIGHTGREY);

    if (splitIndex != -1)
    {
        target.drawString(name.substring(0, splitIndex), centerX, baseY - 6 + 1);
        target.drawString(name.substring(splitIndex + 1), centerX, baseY + 3);
    }
    else
    {
        target.drawString(name, centerX, baseY);
    }
    target.setTextDatum(TL_DATUM);
}

// ══════════════════════════════════════════════════════════════
//  ציור גריד ראשי (ללא אנימציה)
// ══════════════════════════════════════════════════════════════
void MainMenuV2::drawMainGrid()
{
    int total = (int)mainOS->mainScreenIndices.size();
    int totalRows = (total + GRID_COLS - 1) / GRID_COLS;

    // ── שלב 1: ציור כל ריבועי הרקע (תמיד GRID_COLS עמודות) ──
    /*     if (noAnimation)
        {
            for (int row = 0; row < totalRows; row++)
            {
                for (int col = 0; col < GRID_COLS; col++)
                {
                    int sx = GRID_PAD_X + col * (ICON_SIZE + GRID_GAP_X);
                    int sy = GRID_PAD_Y + row * (ICON_SIZE + GRID_GAP_Y) - (int)camY;

                    if (sy + ICON_SIZE < 0 || sy > SCREEN_H)
                        continue;

                    int ox = sx + (ICON_SIZE - (ICON_SIZE - 10)) / 2;
                    int oy = sy + (ICON_SIZE - (ICON_SIZE - 10)) / 2 - 6;
                    int iconDrawSize = ICON_SIZE - 10;

                    int bgX = ox - 4;
                    int bgY = oy - 4;
                    int bgW = iconDrawSize + 15;
                    int bgH = (ICON_SIZE - (oy - sy)) + 12;
                    if (walpaperFromSD_exist_And_Show)
            {DrawPNGPartial(WallpaperPath.c_str(), bgX+4, bgY+4, bgW, bgH, bgX, bgY);}

                   // M5Cardputer.Display.fillRoundRect(bgX, bgY, bgW, bgH, 5, mainOS->BACKGROUND_COLOR);
                }
            }
        } */
    for (int i = 0; i < total; i++)
    {
        int row = i / GRID_COLS;
        int col = i % GRID_COLS;

        int sx = GRID_PAD_X + col * (ICON_SIZE + GRID_GAP_X);
        int sy = GRID_PAD_Y + row * (ICON_SIZE + GRID_GAP_Y) - (int)camY;

        if (sy + ICON_SIZE < 0 || sy > SCREEN_H)
            continue;

        // ← אם אנחנו בשורת הקבצים, אף אייקון בגריד לא selected
        bool selected = !inFileRow && (row == mainOS->selectedRow && col == mainOS->selectedCol);

        MenuItem &item = mainOS->allApps[mainOS->mainScreenIndices[i]];
        drawIcon(item, sx, sy, ICON_SIZE, ICON_SIZE, selected);
    }
}
// ══════════════════════════════════════════════════════════════
//  ציור סאב-מנו אופקי עם אנימציה
// ══════════════════════════════════════════════════════════════
void MainMenuV2::drawSubMenuHorizontal()
{
    auto &tgt = noAnimation
                    ? (LovyanGFX &)M5Cardputer.Display
                    : (LovyanGFX &)mainOS->sprite;

    const int wx = 0, wy = 5;
    const int ww = SCREEN_W;
    const int wh = SCREEN_H - 33;
    const int radius = 10;

    tgt.fillRoundRect(wx + 3, wy + 3, ww, wh, radius, TFT_BLACK);
    tgt.fillRoundRect(wx, wy, ww, wh, radius, TFT_WHITE);
    tgt.drawRoundRect(wx, wy, ww, wh, radius, BLACK);

    tgt.fillRoundRect(wx, wy, ww, 18, radius, TFT_BLUE);
    tgt.fillRect(wx, wy + 10, ww, 8, TFT_BLUE);

    tgt.setTextColor(TFT_WHITE, TFT_BLUE);
    tgt.setCursor(wx + 6, wy + 4);

    tgt.fillCircle(wx + ww - 10, wy + 9, 4, TFT_RED);
    tgt.fillCircle(wx + ww - 22, wy + 9, 4, TFT_YELLOW);

    int contentY = wy + 22;
    int contentH = wh - 30;

    tgt.fillRoundRect(wx + 2, contentY, ww - 4, contentH, 6, 0xEF5D);

    int step = SUB_ICON_SIZE + ICON_GAP_HORIZON;

    for (int i = 0; i < (int)currentIndices->size(); i++)
    {
        int worldX = i * step;
        int sx = (SCREEN_W / 2) + (worldX - (int)camX) - (SUB_ICON_SIZE / 2);
        int sy = contentY + (contentH / 2) - (SUB_ICON_SIZE / 2);

        if (sx + SUB_ICON_SIZE < wx || sx > wx + ww)
            continue;

        bool selected = (i == mainOS->selectedCol);
        drawIcon(mainOS->allApps[(*currentIndices)[i]], sx, sy, SUB_ICON_SIZE, SUB_ICON_SIZE, selected);
    }

    // Scrollbar
    int barW = ww - 40;
    int barX = wx + 20;
    int barY = wy + wh - 10;

    tgt.fillRoundRect(barX, barY, barW, 4, 2, TFT_LIGHTGREY);

    if ((int)currentIndices->size() > 0)
    {
        int visibleRatio = (SUB_ICON_SIZE * 3);
        int totalWidth = (int)currentIndices->size() * step;

        int scrollW = max(10, (barW * visibleRatio) / totalWidth);
        int scrollX = barX + (barW * camX) / totalWidth;

        tgt.fillRoundRect(scrollX, barY, scrollW, 4, 2, TFT_BLUE);
    }

    drawBreadcrumb();

    tgt.setTextColor(0x8410);
    tgt.setTextDatum(ML_DATUM);
    tgt.drawString("ESC -> back", wx + 6, wy + wh - 4);
}

void MainMenuV2::drawBreadcrumb()
{
    if (!inSubMenu)
        return;
    int subId = mainOS->allApps[mainOS->mainScreenIndices[parentIndex]].subMenuId; // ← דרך הוקטור
    mainOS->sprite.setTextColor(TFT_BLACK);
    mainOS->sprite.setTextDatum(MC_DATUM);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.drawString(subMenus[subId].title, SCREEN_W / 2, 10);
}
void MainMenuV2::DrawINFO_Window()
{
    auto &tgt = noAnimation
                    ? (LovyanGFX &)M5Cardputer.Display
                    : (LovyanGFX &)mainOS->sprite;

    const int wx = 20, wy = 20;
    const int ww = 180, wh = 110;
    const int radius = 8;

    tgt.fillRoundRect(wx + 3, wy + 3, ww, wh, radius, TFT_DARKGREY);
    tgt.fillRoundRect(wx, wy, ww, wh, radius, TFT_WHITE);
    tgt.drawRoundRect(wx, wy, ww, wh, radius, TFT_SILVER);

    tgt.fillRoundRect(wx, wy, ww, 16, radius, TFT_BLUE);
    tgt.fillRect(wx, wy + 8, ww, 8, TFT_BLUE);

    tgt.setTextColor(TFT_WHITE, TFT_BLUE);
    tgt.setTextSize(1);
    tgt.setCursor(wx + 6, wy + 4);
    tgt.print("INFO");

    tgt.fillCircle(wx + ww - 10, wy + 8, 4, TFT_RED);
    tgt.fillCircle(wx + ww - 22, wy + 8, 4, TFT_YELLOW);

    tgt.setTextColor(TFT_BLACK, TFT_WHITE);
    int cy = wy + 22;

    uint64_t sdSize = SD.totalBytes();
    uint64_t sdUsed = SD.usedBytes();

    float sdSizeGB = sdSize / 1073741824.0f;
    float sdUsedGB = sdUsed / 1073741824.0f;

    tgt.setCursor(wx + 6, cy);
    tgt.print("Storage");
    cy += 10;

    tgt.setCursor(wx + 6, cy);
    tgt.printf("%.1f / %.1f GB", sdUsedGB, sdSizeGB);
    cy += 10;

    int barX = wx + 6;
    int barW = ww - 12;
    int barH = 8;

    tgt.fillRoundRect(barX, cy, barW, barH, 4, TFT_LIGHTGREY);

    int fill = (sdSize > 0) ? (int)((sdUsed * barW) / sdSize) : 0;
    tgt.fillRoundRect(barX, cy, fill, barH, 4, TFT_GREEN);
    tgt.fillRoundRect(barX, cy, fill, barH / 2, 4, TFT_DARKGREEN);

    cy += 14;

    uint32_t cpuMhz = getCpuFrequencyMhz();
    tgt.setCursor(wx + 6, cy);
    tgt.print("CPU Speed");
    tgt.setCursor(wx + 100, cy);
    tgt.printf("%u MHz", cpuMhz);

    cy += 12;
    tgt.drawFastHLine(wx + 6, cy, ww - 12, TFT_LIGHTGREY);

    cy += 6;
    int bx = wx + (ww / 2) - 30;

    tgt.fillRoundRect(bx, cy, 60, 16, 6, TFT_BLUE);
    tgt.drawRoundRect(bx, cy, 60, 16, 6, TFT_DARKGREY);

    tgt.setTextColor(TFT_WHITE, TFT_BLUE);
    tgt.setCursor(bx + 18, cy + 4);
    tgt.print("OK");
}
// ══════════════════════════════════════════════════════════════
//  Draw — מנתב בין גריד לסאב-מנו
// ══════════════════════════════════════════════════════════════
void MainMenuV2::DrawOnceIfNoAnimation()
{

    WallpaperPath = CurrentThemePath + "/Wallpaper.png";
    if (!mainOS->ShowWallpaperInMainMenu)
    {
        // mainOS->ShowOnScreenMessege("1", 1000);
        noAnimation = false;
        walpaperFromSD_exist_And_Show = false;
        M5Cardputer.Display.fillScreen(mainOS->BACKGROUND_COLOR);
    }

    else if (SD.exists(WallpaperPath.c_str()) && mainOS->ShowWallpaperInMainMenu)
    {
        // mainOS->ShowOnScreenMessege("2", 1000);
        walpaperFromSD_exist_And_Show = true;
        noAnimation = true;
        IMG_x_POS = 0;
        IMG_y_POS = 0;
        DrawPNG(WallpaperPath.c_str());
        // DrawPNGPartial(WallpaperPath.c_str(), 0, 0, SCREEN_W, SCREEN_W, 0, 0);

        // DrawPNGPartial(WallpaperPath.c_str(), 50, 50, 50, 50, 80, 80);
    }

    else if (mainOS->ShowWallpaperInMainMenu)
    {
        walpaperFromSD_exist_And_Show = false;

        // mainOS->ShowOnScreenMessege("3", 1000);

        noAnimation = false;

        mainOS->sprite.pushImage(0, 0, 240, 135, mainMenuBackground);
    }
}

void MainMenuV2::Draw()
{
    if (mainOS->screenOff)
    {
        return;
    }
    if (!noAnimation)

    {
        mainOS->sprite.createSprite(SCREEN_W, SCREEN_H - TopOffset);
        mainOS->sprite.setSwapBytes(true);
    }

    if (!inSubMenu)
    {
        if (DrawFirstFrame)
        {
            DrawOnceIfNoAnimation();
        }
        if (noAnimation)
        // תפריט ראשי: רקע תמונה + גריד סטטי
        {
        }
        else
        {

            if (mainOS->ShowWallpaperInMainMenu)
            {
                mainOS->sprite.pushImage(0, 0, 240, 135, mainMenuBackground);
            }
            else
            {
                mainOS->sprite.fillSprite(mainOS->BACKGROUND_COLOR);
            }
            drawMainGrid();
            drawFileRow();
        }
    }
    else
    {
        // סאב-מנו: רקע לבן + גלילה אופקית מונפשת
        drawSubMenuHorizontal();
    }
    if (ShowInfoWindow)
    {
        DrawINFO_Window();
    }
    if (Show_APP_INFO)
    {
        DrawAppINFO();
    }
    if (DrawFirstFrame)
    {
        // mainOS->sprite.pushSprite(0, 0);
        DrawFirstFrame = false;
    }
    else if (!noAnimation)
    {
        mainOS->sprite.pushSprite(0, 0);
    }
    mainOS->sprite.setTextDatum(TL_DATUM);
    if (noAnimation && !inSubMenu)
    {
        drawMainGrid();
        drawFileRow();
    }
    mainOS->sprite.setSwapBytes(false);
    mainOS->sprite.deleteSprite();
}
void MainMenuV2::DrawAppINFO()
{
    // קבל את ה-item הנוכחי
    int idx = inSubMenu
                  ? (*currentIndices)[mainOS->selectedCol]
                  : mainOS->mainScreenIndices[gridIndex(mainOS->selectedRow, mainOS->selectedCol)];

    MenuItem &item = mainOS->allApps[idx];

    if (item.HelpText.isEmpty())
        return; // אם אין טקסט, אל תציג

    int W = 200;
    int H = 80;
    mainOS->sprite2.createSprite(W, H);
    mainOS->sprite2.setTextColor(BLACK);
    mainOS->sprite2.setTextSize(1);
    mainOS->sprite2.unloadFont();

    mainOS->sprite2.fillRect(0, 0, W, H, TFT_WHITE);
    mainOS->sprite2.drawRect(0, 0, W, H, BLACK);
    mainOS->sprite2.setCursor(4, 4);
    mainOS->sprite2.print(item.HelpText); // ← הטקסט מה-MenuItem
    mainOS->sprite2.pushSprite(&mainOS->sprite, 10, 10);
    mainOS->sprite2.deleteSprite();
}

void MainMenuV2::saveState()
{
    mainOS->savedMainMenu.valid = true;
    mainOS->savedMainMenu.selectedRow = mainOS->selectedRow;
    mainOS->savedMainMenu.selectedCol = mainOS->selectedCol;
    mainOS->savedMainMenu.inSubMenu = inSubMenu;
    mainOS->savedMainMenu.parentIndex = parentIndex;
    mainOS->savedMainMenu.camX = camX;
    mainOS->savedMainMenu.targetCamX = targetCamX;
    mainOS->savedMainMenu.camY = camY;
    mainOS->savedMainMenu.targetCamY = targetCamY;
    mainOS->savedMainMenu.inFileRow = inFileRow;
    mainOS->savedMainMenu.selectedFileIndex = selectedFileIndex;
}