#include "TextEditor.h"
#include "./MyOS.h"
#include "FileBrowser.h"
#include "MainMenuV2.h"
#include "Notes.h"
#include "Function.h"

void TextEditor::Begin()
{
    // Serial.print("text editor");
    if (RenameMode)
    {
        TextFileText = FileName;
        EditMode = true;
        // change etit at before the extension (nit fully work on folder)
        EditAt = mainOS->GetExtensionLower(FileName.c_str()).length() + 1; // TODO fix if keybord to 0 Currently is 1
    }
    else if (mainOS->TextEditorGetTextFromFile)
    {
        TextFileText = mainOS->GetFileText(mainOS->FileSelectedInFS); // get the text inside the file and put it in TextFileText variable
    }
}

void TextEditor::MenuSelected()
{
    if (MenuItems[inMenuIndex] == "Normal Mode")
    {

        EditMode = false;
        inMenuIndex = 0;
        showMenu = false;
    }
    else if (MenuItems[inMenuIndex] == "Edit Mode")
    {

        EditMode = true;
        inMenuIndex = 0;
        showMenu = false;
    }
    else if (MenuItems[inMenuIndex] == "Change Text Size")
    {
        if (mainOS->TextSizeIntextEditorSAVE == 1)
        {
            mainOS->TextSizeIntextEditorSAVE = 2;
            mainOS->saveSettings();
        }
        else
        {
            mainOS->TextSizeIntextEditorSAVE = 1;
            mainOS->saveSettings();
        }
        showMenu = false;
    }
    else if (MenuItems[inMenuIndex] == "close Menu")
    {
        inMenuIndex = 0;
        showMenu = false;
    }
    else if (MenuItems[inMenuIndex] == "Quit")
    {
        inMenuIndex = 0;
        if (CameFromNotes)
        {
            mainOS->ChangeMenu(new Notes(mainOS));
        }
        else if (mainOS->TextEditorGetTextFromFile)
        {
            mainOS->ChangeMenu(new FileBrowser(mainOS));
        }
        else
        {
            mainOS->ChangeMenu(new MainMenuV2(mainOS));
        }
    }
    else if (MenuItems[inMenuIndex] == "Save")
    {
        if (RenameMode)
        {
            SaveCurrentEdit(true);
        }
        else
        {
            SaveCurrentEdit(false);
        }
        SaveCurrentEdit(false);
        showMenu = false;
        inMenuIndex = 0;
    }
    else if (MenuItems[inMenuIndex] == "Save and Quit")
    {
        SaveCurrentEdit(true);
        inMenuIndex = 0;
        showMenu = false;
    }
    else if (MenuItems[inMenuIndex] == "Quit without Save")
    {
        inMenuIndex = 0;
        if (CameFromNotes)
        {
            mainOS->ChangeMenu(new Notes(mainOS));
        }
        else if (mainOS->TextEditorGetTextFromFile)
        {
            mainOS->ChangeMenu(new FileBrowser(mainOS));
        }
        else
        {

            mainOS->ChangeMenu(new MainMenuV2(mainOS));
        }
    }
}

void TextEditor::splitTextToRows(String input, std::vector<String> &outputVector) // in the end not need this function!! i use text warp
{
    int startIndex = 0;
    int endIndex = input.indexOf('\n');

    while (endIndex != -1)
    {

        startIndex = endIndex + 1;
        endIndex = input.indexOf('\n', startIndex);
    }

    if (startIndex < input.length())
    {
        outputVector.push_back(input.substring(startIndex));
    }
}

void TextEditor::Loop()
{
    if (mainOS->screenOff)
    {
        return;
    }

    if (mainOS->NewKey.ifKeyJustPress('`')) // esc
    {
        mainOS->PlayCuteEvilTone();

        if (CameFromNotes)
        {
            MenuItems.clear();

            MenuItems = {"Change Text Size", "Save and Quit", "Quit without Save", "close Menu"};
        }

        else if (RenameMode)
        {
            MenuItems.clear();

            MenuItems = {"Change Text Size", "close Menu", "Save", "Quit"};
        }
        else if (EditMode && !RenameMode)
        {
            MenuItems.clear();
            MenuItems = {"Change Text Size", "close Menu", "Normal Mode", "Save", "Save and Quit", "Quit without Save"};
        }
        else if (!EditMode && !RenameMode)
        {
            MenuItems.clear();

            MenuItems = {"Change Text Size", "close Menu", "Edit Mode", "Quit"};
        }

        if (showMenu)
        {
            inMenuIndex = 0;
            if (CameFromNotes)
            {
                mainOS->ChangeMenu(new Notes(mainOS));
            }
            else if (mainOS->TextEditorGetTextFromFile || RenameMode)
            {
                mainOS->ChangeMenu(new FileBrowser(mainOS));
            }
            else
            {

                mainOS->ChangeMenu(new MainMenuV2(mainOS));
            }
        }
        else
        {
            showMenu = true;
        }
    }

    if (EditMode && !showMenu)
    {

        bool FNpressed;
        if (M5Cardputer.Keyboard.isKeyPressed(KEY_FN))
        {

            FNpressed = true;
        }
        else
        {
            FNpressed = false;
        }
        /*         if (M5Cardputer.Keyboard.isChange())
                { */
        //  if (M5Cardputer.Keyboard.isPressed())
        if (mainOS->NewKey.Any_Key_Press_1_Click_And_After_Few_MS_RepeatClick(700, 20))

        {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            for (auto i : status.word)
            {

                if (i == '/') // arow right
                {
                    if (FNpressed)
                    {
                        insertFromEnd(TextFileText, EditAt, String('/'));
                        ResetView();
                    }
                    else
                    {
                        EditAt -= 1;
                        if (EditAt <= 0)
                        {
                            EditAt = 0;
                        }
                    }
                }
                else if (i == ',') // arrow left
                {
                    if (FNpressed)
                    {
                        insertFromEnd(TextFileText, EditAt, String(','));
                        ResetView();
                    }
                    else
                    {
                        EditAt += 1;
                        if (EditAt >= TextFileText.length())
                        {
                            EditAt = TextFileText.length();
                        }
                    }
                }
                else if (i == ';') // arrow up
                {
                    if (FNpressed)
                    {
                        insertFromEnd(TextFileText, EditAt, String(';'));
                        ResetView();
                    }
                    else
                    {
                        EditAt += 15;
                        if (EditAt >= TextFileText.length())
                        {
                            EditAt = TextFileText.length();
                        }
                    }
                }
                else if (i == '.') // arrow down
                {
                    if (FNpressed)
                    {
                        insertFromEnd(TextFileText, EditAt, String('.'));
                        ResetView();
                    }
                    else
                    {
                        EditAt -= 15;
                        if (EditAt <= 0)
                        {
                            EditAt = 0;
                        }
                    }
                }
                else
                {
                    insertFromEnd(TextFileText, EditAt, String(i));
                    ResetView();
                }
            }
            if (status.del)
            {

                removeFromEnd(TextFileText, EditAt + 1, 1);
                ResetView();
            }

            if (status.enter)
            {

                if (!RenameMode)
                {
                    insertFromEnd(TextFileText, EditAt, "\n");
                    ResetView();
                }
            }

            // auto status = M5Cardputer.Keyboard.keysState();
        }
        // }
    }
    // if in bormal veiw arrow scroll the text
    else if (!EditMode && !showMenu)
    {

        if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick(';', 700, 30))
        {
            mainOS->PlayCuteEvilTone();

            TargetCameraY += 10;
            if (TargetCameraY >= 0)
            {
                TargetCameraY = 0;
            }
        }
        if (mainOS->NewKey.Key_Press_1_Click_And_After_Few_MS_RepeatClick('.', 700, 30))
        {
            mainOS->PlayCuteEvilTone();

            TargetCameraY -= 10;
        }
    }

    else if (showMenu)
    {
        if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
        {
            mainOS->PlayCuteEvilTone();

            MenuSelected();
        }
        if (mainOS->NewKey.ifKeyJustPress(';'))
        {
            mainOS->PlayCuteEvilTone();

            inMenuIndex--;
            if (inMenuIndex < 0)
            {
                inMenuIndex = MenuItems.size() - 1;
            }
        }
        if (mainOS->NewKey.ifKeyJustPress('.'))
        {
            mainOS->PlayCuteEvilTone();

            inMenuIndex++;
            if (inMenuIndex >= MenuItems.size())
            {
                inMenuIndex = 0;
            }
        }
    }
    cameraY = lerpFloat(cameraY, TargetCameraY, 0.15);
    //  cameraX = lerpFloat(cameraX, TargetCameraX, 0.15);

    if (!mainOS->screenOff)
    {
        Draw();
    }
}
void TextEditor::Draw()
{
    auto sprite = &mainOS->sprite; // Get reference to Cardputer's display object

    sprite->createSprite(SCREEN_W, SCREEN_H - TopOffset); 
    sprite->setTextWrap(true);
    if (EditMode)
    {
        mainOS->SetFontForSprite(mainOS->sprite);
        sprite->fillSprite(WHITE);
        sprite->setTextSize(3);
        sprite->setTextColor(LIGHTGREY, WHITE);
        if (RenameMode)
        {
            sprite->setCursor(30, 64);

            sprite->print("RENAME FILE");

            sprite->setTextSize(1);

            /*           sprite->setCursor(1, 95);
                      sprite->print("Press FN And Arrow Keys To Scroll And Change Cursor"); */
        }
        else
        {
            sprite->setCursor(75, 64);

            sprite->print("EDIT MODE");

            sprite->setTextSize(1);
            /*           sprite->setCursor(1, 95);
                      sprite->print("Press FN And Arrow Keys To Scroll And Change Cursor"); */
        }
        sprite->setCursor(0, cameraY);
        sprite->setTextSize(mainOS->TextSizeIntextEditorSAVE); // show text size edit
        sprite->setTextColor(BLACK);

        if (ChangeTextView > 200)
        {
            ChangeTextView = 0;
            showMode2 = !showMode2;
        }
        if (showMode2) // if edit text before the end hide the end and show
        {
            String FirstPart = ShowBefore(EditAt, TextFileText);
            String LastPart = ShowAfter(EditAt, TextFileText);
            // s+="|";
            sprite->print(FirstPart);
            SetTypePos(); // get typing position for indicator
            ResetView();
            sprite->print(LastPart);

            sprite->fillRect(typePosX, typePosY, 2, sprite->fontHeight(), RED); // edit location indicator
        }
        else
        {
            sprite->print(TextFileText);
            // SetTypePos();
        }
    }
    else // normal mode
    {
        mainOS->SetFontForSprite(mainOS->sprite);

        sprite->fillSprite(BLACK);
        sprite->setCursor(0, cameraY);
        sprite->setTextColor(WHITE);
        sprite->setTextSize(mainOS->TextSizeIntextEditorSAVE); // show text size edit

        // sprite->setTextSize(2);

        sprite->print(TextFileText);
    }

    if (showMenu)
    {
        int CurrentY = 5;
        sprite->unloadFont();
        sprite->fillRect(2, 2, 150, 100, BLACK);
        sprite->drawRect(2, 2, 150, 100, YELLOW);
        sprite->setTextSize(1);
        for (size_t i = 0; i < MenuItems.size(); i++)
        {
            if (inMenuIndex == i)
            {
                sprite->setTextColor(RED);
            }
            else
            {
                sprite->setTextColor(YELLOW);
            }
            sprite->setCursor(10, CurrentY);
            sprite->print(MenuItems[i]);
            CurrentY += 10;
        }
    }

    sprite->pushSprite(0, 0);
    sprite->deleteSprite();
}

void TextEditor::insertFromEnd(String &s, int fromEnd, const String &text)
{
    int pos = s.length() - fromEnd;
    if (pos < 0)
        pos = 0;
    if (pos > (int)s.length())
        pos = s.length();

    s = s.substring(0, pos) + text + s.substring(pos);
}

String TextEditor::ShowBefore(int beforeIndex, const String &text)
{
    int cutPos = text.length() - beforeIndex;

    if (beforeIndex <= 0)
        return text;
    if (cutPos <= 0)
        return "";
    return text.substring(0, cutPos);
}

String TextEditor::ShowAfter(int afterIndex, const String &text)
{
    if (afterIndex <= 0)
        return "";

    if (afterIndex >= text.length())
        return text;

    int startPos = text.length() - afterIndex;
    return text.substring(startPos);
}

void TextEditor::removeFromEnd(String &s, int fromEnd, int count)
{

    if (s.length() == 0)
        return;

    int pos = s.length() - fromEnd;

    if (pos < 0)
        pos = 0;
    if (pos >= (int)s.length())
        return;

    s.remove(pos, count);
}

void TextEditor::SaveCurrentEdit(bool exit)
{
    if (RenameMode)
    {
        String FilePath = "";
        FilePath += mainOS->currentPath;
        if (mainOS->currentPath == "/") // if path is root not need to add /
        {
        }
        else // if path not root folder need to add / to get right path
        {
            FilePath += "/";
        }
        FilePath += TextFileText;
        if (RenameFileSafe(SD, mainOS->FileSelectedInFS, FilePath))
        {
            // succes
        }
        else
        {
           // Serial2.print(FileRenameError);
            mainOS->sprite2.createSprite(200, 100);
            mainOS->sprite2.setCursor(1, 1);
            mainOS->sprite2.println("Rename Error");
            // mainOS->sprite4.println(FilePath);

            mainOS->sprite2.println(FileRenameError);

            mainOS->sprite2.pushSprite(0, 0);
            delay(5000);
            mainOS->sprite2.deleteSprite();
        }
        EditMode = false;
        RenameMode = false;
        mainOS->ChangeMenu(new FileBrowser(mainOS));
    }
    if (EditMode && !RenameMode)
    {
        SaveStringToFile(mainOS->FileSelectedInFS, TextFileText, SD);
        if (!exit)
        {
            return;
        }
        EditMode = false;
        RenameMode = false;
        // mainOS->TextEditorGetTextFromFile = false;
        if (mainOS->TextEditorGetTextFromFile)
        {
            mainOS->TextEditorGetTextFromFile = false;
            if (CameFromNotes)
            {
                mainOS->ChangeMenu(new Notes(mainOS));
            }
            else
            {
                mainOS->ChangeMenu(new FileBrowser(mainOS));
            }
        }
        else
        {
            mainOS->TextEditorGetTextFromFile = false;

            mainOS->ChangeMenu(new MainMenuV2(mainOS));
        }
    }
}

bool TextEditor::SaveStringToFile(const String &path, const String &data, fs::FS &fs)
{

    File file = fs.open(path, FILE_WRITE);
    if (!file)
        return false;

    file.print(data);
    file.close();
    return true;
}

void TextEditor::SetTypePos()
{
    auto sprite = &mainOS->sprite; // Get reference to Cardputer's display object

    typePosX = sprite->getCursorX();
    typePosY = sprite->getCursorY();
}
const int TOP_MARGIN = 10;
const int BOTTOM_MARGIN = 20;
void TextEditor::ResetView()
{
    //  return;
    TargetCameraY = -(typePosY - cameraY);
    TargetCameraY += 60;
    if (TargetCameraY > 0)
    {
        TargetCameraY = 0;
    }
    /*     TargetCameraX = -(typePosX - cameraX);
        TargetCameraX += 60; */
    /*     if (TargetCameraX > 0)
        {
            TargetCameraX = 0;
        } */
}

bool TextEditor::RenameFileSafe(fs::FS &fs, const String &oldPath, const String &newPath)
{
    const char *illegal = "<>:\"\\|?*";

    int slashIndex = newPath.lastIndexOf('/');
    String fileName = (slashIndex >= 0) ? newPath.substring(slashIndex + 1) : newPath;

    // file name empty
    if (fileName.length() == 0)
    {
        Serial.println("Empty filename!");
        FileRenameError += " Empty filename!";
        return false;
    }

    // cheack for illigel letters
    for (size_t i = 0; i < strlen(illegal); i++)
    {
        if (fileName.indexOf(illegal[i]) != -1)
        {
            Serial.printf("Illegal char found: %c\n", illegal[i]);
            FileRenameError += " Illegal char found: ";
            FileRenameError += illegal[i];
            return false;
        }
    }

    if (!fs.exists(oldPath))
    {
        Serial.println("Old file does not exist!");
        FileRenameError += " Old file does not exist!";
        return false;
    }

    if (fs.exists(newPath))
    {
        Serial.println("Target name already exists, error!!");
        FileRenameError += " file in the same name alredy exist!";
        return false;
    }

    bool ok = fs.rename(oldPath, newPath);
    if (!ok)
    {
        Serial.println("Rename failed!");
        FileRenameError += " Rename failed!";
    }

    return ok;
}