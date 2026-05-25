#include "Notes.h"
#include <M5Cardputer.h>
#include "./MyOS.h"
#include "./Function.h"
#include "TextEditor.h"
#include "Fonts/DryBrush15.h"
#include "MainMenuV2.h"

void Notes::Begin()
{
    showTopBar=false;
    SD.mkdir("/AdvanceOS");
    SD.mkdir("/AdvanceOS/Notes");
    SetNotesVariable();
}

void Notes::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
        return;
    }

    if (mainOS->NewKey.ifKeyJustPress('.'))
    {
        mainOS->PlayCuteEvilTone();

        InNoteIDselect++;
        if (InNoteIDselect >= NodesInSD.size())
        {
            InNoteIDselect = 0;
        }
    }
    if (mainOS->NewKey.ifKeyJustPress(';'))
    {
        mainOS->PlayCuteEvilTone();

        InNoteIDselect--;
        if (InNoteIDselect < 0)
        {
            InNoteIDselect = NodesInSD.size() - 1;
        }
    }
    if (mainOS->NewKey.ifKeyJustPress('1')) // create new note
    {
        mainOS->PlayCuteEvilTone();

        delay(500); // delay for the nomber 1 not be presed inside the text editor
        String fileName = "Note";
        fileName += String(NodesInSD.size());
        fileName += ".txt";
        //   += String(NodesInSD.size()) += ".txt";
        String CreatedFilePath;
        CreatedFilePath = mainOS->createFile(SD, "/AdvanceOS/Notes", fileName);

        mainOS->TextEditorGetTextFromFile = true;

        mainOS->FileSelectedInFS = CreatedFilePath;
        mainOS->ChangeMenu(new TextEditor(mainOS));
        // mainOS->currentApp

        TextEditor *child = static_cast<TextEditor *>(mainOS->currentApp);
        child->EditMode = true;
        child->CameFromNotes = true;
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_BACKSPACE)) // delete current note
    {
        mainOS->PlayCuteEvilTone();
        if (mainOS->AskSomthing("Are you sure you want to delete this note?"))
        {
            mainOS->deleteFromSd(SD, NodesInSD[InNoteIDselect].NotePath);
            SetNotesVariable();
        }
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER)) // edit current note

    {
        mainOS->PlayCuteEvilTone();

        delay(500); // delay for the ENTER KEY not be presed inside the text editor

        mainOS->TextEditorGetTextFromFile = true;
        mainOS->FileSelectedInFS = NodesInSD[InNoteIDselect].NotePath;
        mainOS->ChangeMenu(new TextEditor(mainOS));
        // mainOS->currentApp

        TextEditor *child = static_cast<TextEditor *>(mainOS->currentApp);
        child->EditMode = true;
        child->CameFromNotes = true;
    }
    cameraY = lerpFloat(cameraY, InNoteIDselect * destanceBetweenNotes, 0.15);

    Draw();
}

 void Notes::Draw()
{

    mainOS->sprite.createSprite(240, 135 - TopOffset);
    mainOS->sprite.fillScreen(rgb565(0, 194, 218));
    mainOS->sprite.setTextWrap(false);

    int CurrentPos = 0;
    int downOffset = 10;

    uint16_t colorNormal = rgb565(255, 220, 0);
    uint16_t colorSelected = rgb565(255, 220, 0);
    uint16_t borderColor = BLACK;
    NoteSprite.setTextColor(BLACK);

    for (int i = 0; i < NodesInSD.size(); i++)
    {
        int w = 140;

        NoteSprite.createSprite(w, NoteHight);
        // NoteSprite.loadFont(DryBrush15);
        NoteSprite.fillSprite(RED); // for transperent
        NoteSprite.fillRoundRect(0, 0, w, NoteHight, 4, rgb565(60, 60, 100));

        uint16_t bg = (InNoteIDselect == i ? colorSelected : colorNormal);
        NoteSprite.fillRoundRect(1, 1, w - 2, NoteHight - 2, 6, bg);

        for (size_t i = 0; i < 8; i++)
        {
            NoteSprite.drawRect(3, i * 8 + 11, 130, 1, ORANGE); // lines
        }
        NoteSprite.drawRoundRect(0, 0, w, NoteHight, 6, borderColor);

        NoteSprite.fillCircle(w - 3, 3, 5, MAROON); // pin circle

        NoteSprite.setCursor(6, 3);
        NoteSprite.print(NodesInSD[i].TextFileContain);

        NoteSprite.pushSprite(&mainOS->sprite, 8, CurrentPos - cameraY + downOffset, RED);
        NoteSprite.deleteSprite();

        destanceBetweenNotes = NoteHight + 4;
        CurrentPos += destanceBetweenNotes;
    }

    NoteSprite.createSprite(74, 50);
    NoteSprite.fillRoundRect(0, 0, 74, 50, 2, YELLOW);
    NoteSprite.drawRoundRect(0, 0, 74, 50, 2, BLACK);

    // NoteSprite.setTextColor(WHITE);
    NoteSprite.unloadFont();
    NoteSprite.setCursor(1, 4);
    NoteSprite.setTextSize(1);
    NoteSprite.print("1: New Note\n\n ENTER: Edit\n\n DEL: Delete");

    NoteSprite.pushSprite(&mainOS->sprite, 164, 5);
    NoteSprite.deleteSprite();

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
} 







#define COL_BG       rgb565(3, 7, 24)      // deep space black
#define COL_CYAN     rgb565(0, 212, 255)   // hologram cyan
#define COL_CYAN_DIM rgb565(0, 80, 120)    // dim grid / borders
#define COL_GREEN    rgb565(0, 255, 136)   // online / idle nodes
#define COL_PURPLE   rgb565(102, 68, 255)  // accent nodes
#define COL_RED_PIN  rgb565(255, 68, 102)  // pin indicator
#define COL_TEXT     rgb565(160, 196, 255) // main text
#define COL_AMBER    rgb565(255, 170, 0)   // warnings
#define NOTE_H       54
#define NOTE_W       196
#define NOTE_GAP     4
#define NOTE_X       6
#define CMD_X        206
#define CMD_W        30   // right panel width (240-CMD_X-2)

void Notes::DrawGrid() {
    // subtle background grid — 14px spacing
    for (int y = 0; y < 135; y += 14)
        mainOS->sprite.drawFastHLine(0, y, 240, COL_CYAN_DIM);
    for (int x = 0; x < 240; x += 14)
        mainOS->sprite.drawFastVLine(x, 0, 135, COL_CYAN_DIM);
}

void Notes::DrawStatusBar() {
    // top status bar
    mainOS->sprite.fillRect(0, 0, 240, 11, rgb565(10, 15, 46));
    mainOS->sprite.drawFastHLine(0, 11, 240, COL_CYAN_DIM);
    mainOS->sprite.setTextColor(COL_CYAN);
    mainOS->sprite.setCursor(3, 2);
    mainOS->sprite.print("NOTES_SYS v2.1");
    mainOS->sprite.setTextColor(COL_GREEN);
    mainOS->sprite.setCursor(150, 2);
    mainOS->sprite.print("*ONLINE");
}

void Notes::DrawNoteCard(int index, int yPos, bool selected) {
    int y = yPos - cameraY + TopOffset + 14;
    if (y + NOTE_H < 0 || y > 135) return; // off screen

    uint16_t accentCol = selected ? COL_CYAN
                       : (index % 2 == 0 ? COL_GREEN : COL_PURPLE);

    NoteSprite.createSprite(NOTE_W, NOTE_H);
    NoteSprite.fillSprite(RED);                           // transparent key
    NoteSprite.fillRoundRect(0, 0, NOTE_W, NOTE_H, 2,
                              rgb565(0, 8, 32));            // dark fill

    // border — bright if selected, dim otherwise
    uint16_t borderCol = selected ? COL_CYAN : COL_CYAN_DIM;
    NoteSprite.drawRoundRect(0, 0, NOTE_W, NOTE_H, 2, borderCol);

    // left accent bar (3px)
    NoteSprite.fillRect(0, 0, 3, NOTE_H, accentCol);

    // pin circle (top-right)
    NoteSprite.fillCircle(NOTE_W - 4, 4, 3, COL_RED_PIN);

    // scan lines
    for (int sy = 14; sy < NOTE_H - 2; sy += 7)
        NoteSprite.drawFastHLine(5, sy, NOTE_W - 10,
            selected ? COL_CYAN_DIM : rgb565(0, 30, 50));

    // node ID header
    NoteSprite.setTextColor(accentCol);
    NoteSprite.setCursor(6, 3);
    char header[20];
    sprintf(header, "MEM://NODE_%03d", index + 1);
    NoteSprite.print(header);

    // note text preview
    NoteSprite.setTextColor(COL_TEXT);
    NoteSprite.setCursor(6, 16);
    NoteSprite.print(NodesInSD[index].TextFileContain);

    NoteSprite.pushSprite(&mainOS->sprite, NOTE_X, y, RED);
    NoteSprite.deleteSprite();
}

void Notes::DrawCmdPanel() {
    NoteSprite.createSprite(CMD_W, 74);
    NoteSprite.fillSprite(RED);
    NoteSprite.fillRoundRect(0, 0, CMD_W, 74, 2, rgb565(0, 8, 32));
    NoteSprite.drawRoundRect(0, 0, CMD_W, 74, 2, COL_CYAN_DIM);

    // corner tick marks (sci-fi frame)
    NoteSprite.drawFastHLine(0, 4, 4, COL_CYAN);
    NoteSprite.drawFastVLine(4, 0, 4, COL_CYAN);
    NoteSprite.drawFastHLine(CMD_W - 4, 4, 4, COL_CYAN);
    NoteSprite.drawFastVLine(CMD_W - 4, 0, 4, COL_CYAN);

    NoteSprite.setTextColor(COL_CYAN_DIM);
    NoteSprite.setCursor(3, 2);  NoteSprite.print("CMD");
    NoteSprite.drawFastHLine(0, 11, CMD_W, COL_CYAN_DIM);

    struct { const char* label; uint16_t col; } cmds[] = {
        { "[1] NEW",  COL_GREEN  },
      /*   { "[CR] EDT", COL_CYAN   }, */
        { "[D] DEL",  COL_RED_PIN},
       /*  { "[^v] NAV", COL_AMBER  }, */
    };
    for (int i = 0; i < 4; i++) {
        NoteSprite.setTextColor(cmds[i].col);
        NoteSprite.setCursor(3, 14 + i * 14);
        NoteSprite.print(cmds[i].label);
    }
    NoteSprite.pushSprite(&mainOS->sprite, CMD_X, 14, RED);
    NoteSprite.deleteSprite();
}
/* 
void Notes::Draw() {
    mainOS->sprite.createSprite(240, 135);
    mainOS->sprite.fillScreen(COL_BG);
    mainOS->sprite.setTextWrap(false);

    DrawGrid();
    DrawStatusBar();

    int yPos = 0;
    for (int i = 0; i < NodesInSD.size(); i++) {
        DrawNoteCard(i, yPos, InNoteIDselect == i);
        yPos += NOTE_H + NOTE_GAP;
    }
    destanceBetweenNotes = NOTE_H + NOTE_GAP;

    DrawCmdPanel();

    mainOS->sprite.pushSprite(0,0);
    mainOS->sprite.deleteSprite();
}
 */


void Notes::SetNotesVariable()
{
    NodesInSD.clear();
    mainOS->GetFilesToList("txt", "/AdvanceOS/Notes");
    for (int i = 0; i < mainOS->FileList.size(); i++)
    {
        NoteData n;
        n.NotePath = mainOS->FileList[i];
        n.TextFileContain = mainOS->GetFileText(mainOS->FileList[i]);
        NodesInSD.push_back(n);

        // mainOS->FileList[i]
    }
}
