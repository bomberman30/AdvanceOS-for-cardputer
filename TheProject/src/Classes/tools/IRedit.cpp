#include "IRedit.h"
// #include <ArduinoJson.h>
#include "../../MyOS.h"
#include "../FileBrowser.h"
#include "../../Function.h"
#define MICROS_PER_TICKS 2L

void IR_Editor::Begin()
{
    ir.begin();
    //os.ShowOnScreenMessege("1", 1000);
    CommandFromFile.clear();
    if (mainOS->FileSelectedInFS == "") // no file selected Entering New file Mode
    {
       // os.ShowOnScreenMessege("2", 1000);

        // create new file
        StartNewFile();
       // os.ShowOnScreenMessege("6", 1000);
    }

    else
    {
        String ext = mainOS->GetExtensionLower(mainOS->FileSelectedInFS.c_str());
        if (ext != "ir")
        {
            StartNewFile();
            return;
        }
        CommandFromFile = getAllCommandNamesFromFile(mainOS->FileSelectedInFS);

        DeviceID = mainOS->getFileNameFromPath(mainOS->FileSelectedInFS);
       // os.ShowOnScreenMessege("24", 1000);
    }
}

void IR_Editor::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`')) // esc
    {
        if (inMenu)
        {
            inMenu = false;
        }
        else
        {
            mainOS->ChangeMenu(new FileBrowser(mainOS));
        }
    }
    if (mainOS->NewKey.ifKeyJustPress(';')) // up
    {
        if (inMenu)
        {
            menuSelection--;
            if (menuSelection < 0)
            {
                menuSelection = FirstmenuItems.size() - 1;
            }
        }
        else
        {
            InCorrentCommand--;
        }
    }
    if (mainOS->NewKey.ifKeyJustPress('.')) // down
    {
        if (inMenu)
        {
            menuSelection++;
            if (menuSelection > FirstmenuItems.size() - 1)
            {
                menuSelection = 0;
            }
        }
        else
        {
            InCorrentCommand++;
        }
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER)) // down
    {
        if (inMenu)
        {
            if (FirstmenuItems[menuSelection] == "Exit")
            {
                mainOS->ChangeMenu(new FileBrowser(mainOS));
            }
            else if (FirstmenuItems[menuSelection] == "Emit IR signal")
            {
                if (CommandFromFile.size() == 0)
                {
                    mainOS->ShowOnScreenMessege("No signal found", 1000);
                    return;
                }
                loadAndSendFromFile(CommandFromFile[InCorrentCommand], mainOS->FileSelectedInFS);
                mainOS->ShowOnScreenMessege("\"" + CommandFromFile[InCorrentCommand] + "\" Emitted!", 1000);
                inMenu = false;
            }
            else if (FirstmenuItems[menuSelection] == "Create New Signal")
            {
                String NewSignalName = mainOS->AskFromUserForString("Choose New Name", true, false, false);

                if (NewSignalName == "")
                {
                    inMenu = false;
                    return;
                }

                // בדיקה אם השם כבר קיים
                for (const String &existing : CommandFromFile)
                {
                    if (existing == NewSignalName)
                    {
                        mainOS->ShowOnScreenMessege("Name already exists!", 1000);
                        inMenu = false;
                        return;
                    }
                }

                // קליטת אות IR
                irRecive.enableIRIn();
                mainOS->ShowOnScreenMessege("Waiting for IR... (10s)", 0);

                bool signalReceived = false;
                unsigned long startTime = millis();
                decode_results results;

                while (!signalReceived && (millis() - startTime < 10000))
                {
                    if (irRecive.decode(&results))
                    {
                        if (results.rawlen > 0)
                        {
                            uint32_t NEW_FREQ = 38000; // need to change by the recived signal
                                                       // TODO make NEW_FREQ as the signal recived
                            switch (results.decode_type)
                            {
                            case NEC:
                            case SAMSUNG:
                            case LG:
                            case PANASONIC:
                                NEW_FREQ = 38000;
                                break;
                            case SONY:
                                NEW_FREQ = 40000;
                                break;
                            case RC5:
                            case RC6:
                                NEW_FREQ = 36000;
                                break;
                            case SHARP:
                                NEW_FREQ = 38000;
                                break;
                            case JVC:
                                NEW_FREQ = 38000;
                                break;
                            default:
                                NEW_FREQ = 38000; // רוב המכשירים
                                break;
                            }

                            String rawDataString = "";
                            for (int i = 1; i < results.rawlen; i++)
                            {
                                uint32_t duration = results.rawbuf[i] * MICROS_PER_TICKS;
                                rawDataString += String(duration);
                                if (i < results.rawlen - 1)
                                    rawDataString += " ";
                            }

                            // שמירה כסיגנל חדש בקובץ
                            File f = SD.open(mainOS->FileSelectedInFS, "a"); // append
                            if (f)
                            {
                                f.println("name: " + NewSignalName);
                                f.println("frequency: " + String(NEW_FREQ));
                                f.println("data: " + rawDataString);
                                f.println();
                                f.close();

                                // רענון הרשימה בזיכרון
                                CommandFromFile = getAllCommandNamesFromFile(mainOS->FileSelectedInFS);
                                InCorrentCommand = CommandFromFile.size() - 1; // קפיצה לסיגנל החדש

                                mainOS->ShowOnScreenMessege("\"" + NewSignalName + "\" Saved!", 1000);
                            }
                            else
                            {
                                mainOS->ShowOnScreenMessege("Failed to save!", 1000);
                            }

                            signalReceived = true;
                        }
                        if (!signalReceived)
                            irRecive.resume();
                    }
                    yield();
                }

                if (!signalReceived)
                    mainOS->ShowOnScreenMessege("No Signal Found", 1000);

                inMenu = false;
            }

            else if (FirstmenuItems[menuSelection] == "Edit Signal Name")
            {
                if (CommandFromFile.size() == 0)
                {
                    mainOS->ShowOnScreenMessege("No signal found", 1000);
                    return;
                }
                String NewName = mainOS->AskFromUserForString("Choose New Name", true, false, false);
                if (NewName != "")
                {
                    if (renameSignalInFile(CommandFromFile[InCorrentCommand], NewName, mainOS->FileSelectedInFS))
                    {
                        CommandFromFile[InCorrentCommand] = NewName; // עדכון הרשימה בזיכרון
                        mainOS->ShowOnScreenMessege("Renamed to " + NewName, 1000);
                    }
                }
                inMenu = false;
            }
            else if (FirstmenuItems[menuSelection] == "Delete Signal")
            {
                if (CommandFromFile.size() == 0)
                {
                    mainOS->ShowOnScreenMessege("No signal found", 1000);
                    return;
                }
                if (deleteSignalFromFile(CommandFromFile[InCorrentCommand], mainOS->FileSelectedInFS))
                {
                    mainOS->ShowOnScreenMessege("Deleted!", 1000);
                    // רענון הרשימה מהקובץ
                    CommandFromFile = getAllCommandNamesFromFile(mainOS->FileSelectedInFS);
                    if (InCorrentCommand >= CommandFromFile.size())
                        InCorrentCommand = 0;
                }
                inMenu = false;
            }
            else if (FirstmenuItems[menuSelection] == "Change This IR Signal")
            {
                if (CommandFromFile.size() == 0)
                {
                    mainOS->ShowOnScreenMessege("No signal found", 1000);
                    return;
                }
                // 1. נבדוק מה התדר הקיים בקובץ כדי לא לדרוס אותו בטעות
                uint32_t NEW_FREQ;
                irRecive.enableIRIn();
                mainOS->ShowOnScreenMessege("Waiting for IR... (10s)", 0);

                bool Signalrecived = false;
                unsigned long startTime = millis();
                decode_results results;

                while (!Signalrecived && (millis() - startTime < 10000))
                {
                    if (irRecive.decode(&results))
                    {
                        uint32_t NEW_FREQ = 38000; // need to change by the recived signal
                                                   // TODO make NEW_FREQ as the signal recived
                        switch (results.decode_type)
                        {
                        case NEC:
                        case SAMSUNG:
                        case LG:
                        case PANASONIC:
                            NEW_FREQ = 38000;
                            break;
                        case SONY:
                            NEW_FREQ = 40000;
                            break;
                        case RC5:
                        case RC6:
                            NEW_FREQ = 36000;
                            break;
                        case SHARP:
                            NEW_FREQ = 38000;
                            break;
                        case JVC:
                            NEW_FREQ = 38000;
                            break;
                        default:
                            NEW_FREQ = 38000; // רוב המכשירים
                            break;
                        }

                        if (results.rawlen > 0)
                        {
                            String rawDataString = "";

                            for (int i = 1; i < results.rawlen; i++)
                            {
                                uint32_t duration = results.rawbuf[i] * MICROS_PER_TICKS;
                                rawDataString += String(duration);
                                if (i < results.rawlen - 1)
                                    rawDataString += " ";
                            }
                            // 2. עדכון הקובץ עם התדר הנכון
                            if (updateSignalDataInFile(CommandFromFile[InCorrentCommand], rawDataString, NEW_FREQ, mainOS->FileSelectedInFS))
                            {
                                mainOS->ShowOnScreenMessege("Captured! Saving...", 500);

                                // 3. "שישלח לפי התדר" - בדיקה מיידית של מה שנקלט
                            }
                            Signalrecived = true;
                        }
                        if (!Signalrecived)
                        {
                            irRecive.resume();
                        }
                    }
                    yield();
                }
                if (!Signalrecived)
                {
                    mainOS->ShowOnScreenMessege("No Signal Found", 1000);
                }
                inMenu = false;
            }
        }
        else
        {
            inMenu = true;
        }

        /*     loadAndSendFromFile(CommandFromFile[InCorrentCommand], mainOS->FileSelectedInFS);
            mainOS->ShowOnScreenMessege("\"" + CommandFromFile[InCorrentCommand] + "\" Emitted!", 1000);
     */
    }
    if (InCorrentCommand < 0)
        InCorrentCommand = CommandFromFile.size() - 1;

    if (InCorrentCommand >= CommandFromFile.size())
        InCorrentCommand = 0;
    // CameraY = InCorrentCommand * distancebetweenMenuLines;
    CameraY = lerpFloat(CameraY, InCorrentCommand * distancebetweenMenuLines, 0.05f);
    Draw();
}
#define BG_COLOR 0x3000
#define CARD_COLOR 0x4208
#define CARD_SELECTED 0xF800
#define CARD_BORDER 0xFFFF
#define TEXT_NORMAL 0xFFFF
#define TEXT_SELECTED 0xFFFF

void IR_Editor::Draw()
{
    mainOS->sprite.createSprite(240, 135 - TopOffset);
    mainOS->sprite.fillSprite(BG_COLOR);
    mainOS->sprite.setTextWrap(false);
    mainOS->sprite.unloadFont();

    int cardH = 22;
    // int startY = 30 - CameraY;

    mainOS->sprite.setTextSize(1);

    for (int i = 0; i < CommandFromFile.size(); i++)
    {
        int y = i * distancebetweenMenuLines + 60 - CameraY; // * (cardH + distancebetweenMenuLines);

        bool selected = (i == InCorrentCommand);

        uint16_t bg = selected ? CARD_SELECTED : CARD_COLOR;

        mainOS->sprite.fillRoundRect(14, y + 2, 210, cardH, 6, TFT_BLACK);

        mainOS->sprite.fillRoundRect(12, y, 210, cardH, 6, bg);

        if (selected)
            mainOS->sprite.drawRoundRect(12, y, 210, cardH, 6, CARD_BORDER);

        if (selected)
            mainOS->sprite.fillRect(8, y, 4, cardH, TFT_YELLOW);

        mainOS->sprite.setTextColor(TEXT_SELECTED);
        mainOS->sprite.setCursor(20, y + 6);
        mainOS->sprite.print(CommandFromFile[i]);
    }
    mainOS->sprite.fillRect(0, 0, 240, 24, 0x1800);
    mainOS->sprite.setTextColor(WHITE);
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(6, 8);
    mainOS->sprite.print("IR Device: " + DeviceID);
    if (inMenu)
    {
        DrawFirstMenu();
    }
    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

void IR_Editor::DrawFirstMenu()
{

    mainOS->sprite.fillRect(10, 10, 200, 120, YELLOW);
    mainOS->sprite.setTextSize(1);

    mainOS->sprite.unloadFont();
    mainOS->sprite.drawRect(10, 10, 200, 120, BLACK);
    int DistanceBetweenMenu = 10;
    for (size_t i = 0; i < FirstmenuItems.size(); i++)
    {
        mainOS->sprite.setCursor(15, i * DistanceBetweenMenu + 12);
        if (menuSelection == i)
        {
            mainOS->sprite.setTextColor(RED);
        }
        else
        {
            mainOS->sprite.setTextColor(BLACK);
        }
        mainOS->sprite.print(FirstmenuItems[i]);
    }
}

void IR_Editor::OnExit()
{
}

bool IR_Editor::SendCommand(const String &commandName, const String &filePathh)
{
    return loadAndSendFromFile(commandName, filePathh);
}

std::vector<String> IR_Editor::getAllCommandNamesFromFile(const String &filePath)
{

    std::vector<String> names;

    File f = SD.open(filePath, "r");
    if (!f)
    {
        mainOS->ShowOnScreenMessege("Failed to open file: " + filePath);
        return names;
    }

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.startsWith("name: "))
        {
            names.push_back(line.substring(6));
        }
    }

    f.close();
    return names;
}

bool IR_Editor::loadAndSendFromFile(const String &name, const String &filePath)
{
    File f = SD.open(filePath, "r");
    if (!f)
        return false;

    bool found = false;
    // uint32_t freq = 38000; // דיפולט
    uint32_t freq = getFrequencyFromFile(CommandFromFile[InCorrentCommand], mainOS->FileSelectedInFS);

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();

        if (line == "name: " + name)
        {
            found = true;
        }
        else if (found && line.startsWith("frequency: "))
        {
            freq = line.substring(11).toInt();
        }
        else if (found && line.startsWith("data: "))
        {
            line.remove(0, 6);
            uint16_t raw[400];
            int count = 0;
            char buf[line.length() + 1];
            line.toCharArray(buf, sizeof(buf));
            char *tok = strtok(buf, " ");
            while (tok && count < 400)
            {
                raw[count++] = (uint16_t)atoi(tok);
                tok = strtok(nullptr, " ");
            }

            // השליחה האמיתית עם התדר מהקובץ!
            ir.sendRaw(raw, count, freq);
            f.close();
            return true;
        }
    }
    f.close();
    return false;
}
bool IR_Editor::renameSignalInFile(const String &oldName, const String &newName, const String &filePath)
{
    File f = SD.open(filePath, "r");
    File temp = SD.open("/temp.ir", "w");
    if (!f || !temp)
        return false;

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        String trimmedLine = line;
        trimmedLine.trim();

        if (trimmedLine == "name: " + oldName)
        {
            temp.println("name: " + newName);
        }
        else
        {
            temp.println(line); // שומר על המבנה המקורי כולל ירידות שורה
        }
    }

    f.close();
    temp.close();
    SD.remove(filePath);
    SD.rename("/temp.ir", filePath);
    return true;
}
bool IR_Editor::deleteSignalFromFile(const String &name, const String &filePath)
{
    File f = SD.open(filePath, "r");
    File temp = SD.open("/temp.ir", "w");
    if (!f || !temp)
        return false;

    bool skipNextData = false;

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        String trimmedLine = line;
        trimmedLine.trim();

        if (trimmedLine == "name: " + name)
        {
            skipNextData = true; // מצאנו את השם, נדלג עליו ועל ה-data הבא
            continue;
        }

        if (skipNextData && trimmedLine.startsWith("data: "))
        {
            skipNextData = false; // דילגנו על הנתונים, ממשיכים כרגיל
            continue;
        }

        temp.println(line);
    }

    f.close();
    temp.close();
    SD.remove(filePath);
    SD.rename("/temp.ir", filePath);
    return true;
}
bool IR_Editor::updateSignalDataInFile(const String &name, const String &newData, uint32_t frequency, const String &filePath)
{
    // mainOS->ShowOnScreenMessege("--- Starting updateSignalDataInFile ---");
    // mainOS->ShowOnScreenMessege("Looking for: " + name);
    // mainOS->ShowOnScreenMessege("In file: " + filePath);

    File f = SD.open(filePath, "r");
    File temp = SD.open("/temp.ir", "w");

    if (!f)
    {
        // mainOS->ShowOnScreenMessege("ERROR: Could not open source file!");
        if (temp)
            temp.close();
        return false;
    }
    if (!temp)
    {
        // mainOS->ShowOnScreenMessege("ERROR: Could not create temp file!");
        f.close();
        return false;
    }

    bool inTargetBlock = false;
    bool foundAndUpdated = false;
    int lineCount = 0;

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        lineCount++;
        String trimmedLine = line;
        trimmedLine.trim();

        // דיבאג: הדפסת השם שאנחנו מחפשים מול השורה שנקראה
        if (trimmedLine == "name: " + name)
        {
            // mainOS->ShowOnScreenMessege("MATCH FOUND! Line " + String(lineCount) + ": " + trimmedLine);
            inTargetBlock = true;
            temp.println(line);
            continue;
        }

        if (inTargetBlock)
        {
            if (trimmedLine.startsWith("frequency: "))
            {
                // mainOS->ShowOnScreenMessege("Updating frequency to: " + String(frequency));
                temp.println("frequency: " + String(frequency));
                continue;
            }
            else if (trimmedLine.startsWith("data: "))
            {
                // mainOS->ShowOnScreenMessege("Updating data block...");
                temp.println("data: " + newData);
                inTargetBlock = false;
                foundAndUpdated = true;
                continue;
            }
            else if (trimmedLine.startsWith("name: "))
            {
                // אם הגענו לשם אחר לפני שמצאנו data, משהו בפורמט לא תקין
                // mainOS->ShowOnScreenMessege("Warning: Found another 'name:' before finishing update for " + name);
                inTargetBlock = false;
            }
        }

        temp.println(line);
    }

    f.close();
    temp.close();

    if (foundAndUpdated)
    {
        // mainOS->ShowOnScreenMessege("Success! Deleting old file and renaming temp.");
        SD.remove(filePath);
        if (SD.rename("/temp.ir", filePath))
        {
            // mainOS->ShowOnScreenMessege("File renamed successfully.");
            return true;
        }
        else
        {
            // mainOS->ShowOnScreenMessege("ERROR: Rename failed!");
            return false;
        }
    }
    else
    {
        // mainOS->ShowOnScreenMessege("FAILED: Target signal '" + name + "' was not found in the file.");
        SD.remove("/temp.ir");
        return false;
    }
}

uint32_t IR_Editor::getFrequencyFromFile(const String &name, const String &filePath)
{

    File f = SD.open(filePath, "r");
    if (!f)
    {
        return 38000;
    }

    bool found = false;
    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line == "name: " + name)
        {
            found = true;
        }
        else if (found && line.startsWith("frequency: "))
        {
            uint32_t frq = line.substring(11).toInt();
            // uint32_t frq = line.substring(String("frequency: ").length()).toInt();
            // uint32_t frq = line.substring(6).toInt();
            f.close();

            return (frq > 0) ? frq : 38000;
        }
        else if (found && line.startsWith("name: "))
        { // התחלנו בלוק חדש בלי למצוא freq
            break;
        }
    }

    f.close();
    return 38000;
}
void IR_Editor::StartNewFile()
{
    //os.ShowOnScreenMessege("3", 1000);

    String NewFileName = mainOS->AskFromUserForString("Choose File Name", true);
    if (NewFileName == "")
    {
     //   os.ShowOnScreenMessege("4", 1000);

        mainOS->ChangeMenu(new FileBrowser(mainOS));
        return;
    }
  //  os.ShowOnScreenMessege("8", 1000);

    NewFileName += ".ir";
    String FolderName = "/AdvanceOS/InfraRed";
    SD.mkdir(FolderName.c_str());
    String FullFileName = FolderName + "/" + NewFileName;

    // בדיקה אם קובץ כבר קיים
    if (SD.exists(FullFileName))
    {
        mainOS->ShowOnScreenMessege("File already exists!", 1000);
        mainOS->ChangeMenu(new FileBrowser(mainOS));
        return;
    }

    // יצירת הקובץ עם header
    File f = SD.open(FullFileName, "w");
    if (f)
    {
        f.println("Filetype: AdvanceOS IR File");
        f.println("Version: 1");
        f.close();
    }
    else
    {
        mainOS->ShowOnScreenMessege("Failed to create file!", 1000);
        mainOS->ChangeMenu(new FileBrowser(mainOS));
        return;
    }
    mainOS->ShowOnScreenMessege("File Created! File Location In AdvanceOS/InfraRed Folder");

    mainOS->FileSelectedInFS = FullFileName;
    DeviceID = mainOS->getFileNameFromPath(mainOS->FileSelectedInFS);
}
