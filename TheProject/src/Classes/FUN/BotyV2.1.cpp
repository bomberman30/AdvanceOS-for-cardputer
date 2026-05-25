#include "../../MyOS.h"
#include "botyV2.1.h"
// #include "Fonts/DryBrush15.h"
#include "../MainMenuV2.h"
#include "BotyV2.1.h"
#include "Notes.h"
// #include <faces/FaceTemplates.hpp>
float Mouth_open_ratio = 0.0f; 
uint16_t background_color = WHITE;
float EYE_open_ratio_ = 1;

int EyeOfffsetX = 0;
int EyeOffsetY = 0;
int FacePosOffsetX = 0;
int FacePosOffsetY = 0;
void beep(int note, int duree)
{
  M5Cardputer.Speaker.tone(note, duree);
  // M5Cardputer.Speaker.noTone();
  delay(duree);
}
void BotyV2::Begin()
{
  M5.begin();
  showTopBar = false;
  // mainOS->ShowOnScreenMessege("1",1000);
  Draw();
  SayText(mainOS->funnyLines[random(0, 49)]);
}

void BotyV2::Loop()
{
  Speech();
  if (mainOS->NewKey.ifKeyJustPress('`')) // if press escape black screen
  {
    mainOS->PlayCuteEvilTone();

    RenderMenu = !RenderMenu;
    /*     mainOS->SetFastBootVar(1);
        // mainOS->saveSettings();
        ESP.restart(); */
  }
  else if (mainOS->NewKey.ifKeyJustPress('/'))
  {
    mainOS->PlayCuteEvilTone();

    if (!RenderAnswerMenu)
    {
      InMenuID++;
      if (InMenuID >= MAX_MENU_ITEM_CHATBOT)
      {
        InMenuID = MAX_MENU_ITEM_CHATBOT - 1;
      }
    }
    Draw();
  }
  else if (mainOS->NewKey.ifKeyJustPress(',')) // if press escape black screen
  {
    mainOS->PlayCuteEvilTone();
  
    if (!RenderAnswerMenu)

    {
      InMenuID--;
      if (InMenuID <= 0)
      {
        InMenuID = 0;
      }
    }
    Draw();
  }
  else if (mainOS->NewKey.ifKeyJustPress(';')) // if press escape black screen
  {
    mainOS->PlayCuteEvilTone();
    inAnswer--;
    if (inAnswer < 0)
      inAnswer = 0;
    if (!RenderAnswerMenu)

    {
      InMenuID -= 3;
      if (InMenuID <= 0)
      {
        InMenuID = 0;
      }
    }
    Draw();
  }
  else if (mainOS->NewKey.ifKeyJustPress('.')) // if press escape black screen
  {
    mainOS->PlayCuteEvilTone();
    inAnswer++;
    if (inAnswer > 3)
      inAnswer = 3;
    if (!RenderAnswerMenu)

    {
      InMenuID += 3;
      if (InMenuID >= MAX_MENU_ITEM_CHATBOT)
      {
        InMenuID = MAX_MENU_ITEM_CHATBOT - 1;
      }
    }
    Draw();
  }

  else if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER)) // if press escape black screen
  {
    mainOS->PlayCuteEvilTone();
    if (RenderAnswerMenu)
    {
      ChackRightAnswer();
    }
    else
    {
      SelectFromMenu();
    }
    /*    int RandomNomber = random(0, 52);
       SayText(botLines[RandomNomber]); */
  }

  // all keys press
  else if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
      if (RenderAnswerMenu)
      {
        return;
      }
      for (auto i : status.word)
      {
        if (i == '`' || i == '/' || i == '.' || i == ',' || i == ';') // if press arrow ot esc exit
        {
          return;
        }
        mainOS->PlayCuteEvilTone();

        switch (random(1, 5))
        {
        case 1:
          SayText("Hey It's Tickle Stop!");
          break;
        case 2:
          SayText("Dont Just Press Me!");
          break;
        case 3:
          SayText("Wow Your Finger Huge ...    or my keyboard is tiny??....!");
          break;
        case 4:
          SayText("Stop It! You Damage The Keyboard!!!!. .... just kidding Press All You like       But not too much");
          break;
        default:
          break;
        }
      }
    }
  }
  // end of all keys press

  if (ScreepUpdate > 1000)
  {
    updateExpression();
    Odd_number_loop = !Odd_number_loop;
    ScreepUpdate = 0;
    EyeOfffsetX = random(-3, 3);
    EyeOffsetY = random(-2, 2);
    ToBlink -= 1;
    if (ToBlink == 0)
    {
      EYE_open_ratio_ = 0;
      ToBlink = random(3, 8);
    }
    else
    {
      EYE_open_ratio_ = 1;
    }
    Draw();
  }
}
void BotyV2::Draw()
{
  if (TextShowTime < TALK_BUBLE_SHOW_TIME) // if talk buble make face up to see mouth
  {
    FacePosOffsetY = -10;
  }
  else
  {
    FacePosOffsetY = 0;
  }
  // mainOS->ShowOnScreenMessege("2",1000);

  mainOS->sprite.createSprite(240, 135);

  mainOS->sprite.fillSprite(background_color); 
  DrawMouth(&mainOS->sprite);

  DrawEYE(&mainOS->sprite, false, 70);
  DrawEYE(&mainOS->sprite, true, SCREEN_W - 70);
  if (TextShowTime < TALK_BUBLE_SHOW_TIME)
  {
    if (RenderAnswerMenu) // if ask quistion always show the text down
    {
      TextShowTime = 0;
    }
    int Posx = 20;
    int Posy = 104;
    int H = 30;
    int W = 200;
    mainOS->sprite2.createSprite(W, H);
    mainOS->sprite2.fillSprite(BLACK);
    mainOS->sprite2.setTextWrap(true);
    mainOS->sprite.fillRect(Posx - 2, Posy, 2, H, BLACK); // black rect to eneble text to be blear
    // mainOS->sprite2.drawRect(0, 0, 200, 40, BLACK);
    mainOS->sprite2.setTextColor(WHITE);
    mainOS->sprite2.setCursor(0, 2);
    mainOS->sprite2.print(TextToRender);
    mainOS->sprite2.pushSprite(&mainOS->sprite, Posx, Posy);
    mainOS->sprite2.deleteSprite();
  }
  // mainOS->ShowOnScreenMessege("3",1000);

  // expression draw section

  switch (expression_)
  {
  case Expression::Doubt:
    drawSweatMark(&mainOS->sprite, 12, 12, 7, BLACK);
    break;
  case Expression::Angry:
    drawAngerMark(&mainOS->sprite, 12, 12, 12, BLACK, background_color);
    break;
  case Expression::Happy:
    drawHeartMark(&mainOS->sprite, 12, 12, 12, BLACK);
    break;
  case Expression::Sad:
    drawChillMark(&mainOS->sprite, 12, 12, 30, BLACK);
    break;
  case Expression::Sleepy:
    drawBubbleMark(&mainOS->sprite, 12, 12, 10, BLACK);
    drawBubbleMark(&mainOS->sprite, 15, 15, 6, BLACK);
    break;
  }

  // menu draw section
  // mainOS->ShowOnScreenMessege("4",1000);
  if (RenderAnswerMenu)
  {
    const int btnX = 5;
    const int btnY = 2;
    const int btnW = 200; 
    const int btnH = 15;  
    const int spacing = 2;

    const int ANSWER_COUNT = 4;

    mainOS->sprite.fillRect(
        btnX - 6, btnY - 6,
        btnW + 12, (btnH + spacing) * ANSWER_COUNT + 2,
        TFT_YELLOW);

    for (size_t i = 0; i < ANSWER_COUNT; i++)
    {
      bool selected = (i == inAnswer);
      int y = btnY + i * (btnH + spacing);

      mainOS->sprite.fillRect(
          btnX, y,
          btnW, btnH,
          selected ? TFT_WHITE : TFT_BLACK);

      mainOS->sprite.drawRect(
          btnX, y,
          btnW, btnH,
          TFT_WHITE);

      mainOS->sprite.setTextColor(
          selected ? TFT_BLACK : TFT_WHITE);
      mainOS->sprite.setCursor(btnX + 8, y + (btnH / 2) - 4);
      mainOS->sprite.print(Answers[i]);
    }
  }
  if (score != 0)
  {

    mainOS->sprite.setCursor(177, 1);
    mainOS->sprite.setTextColor(BLACK);
    mainOS->sprite.print("Score: " + String(score));
  }
  if (RenderMenu && !RenderAnswerMenu)
  {
    mainOS->sprite.fillRect(5, 5, 230, 120, YELLOW);
    mainOS->sprite.drawRect(5, 5, 230, 120, BLACK);

    int menuXline = 0;
    int menuYline = 0;

    const int btnWidth = 75;
    const int btnHeight =
        20;
    const int startX = 7;
    const int startY = 7;

    mainOS->sprite.setTextSize(1);
    mainOS->sprite.unloadFont();

    for (size_t i = 0; i < MAX_MENU_ITEM_CHATBOT; i++)
    {
      int x = startX + menuXline * btnWidth;
      int y = startY + menuYline * btnHeight;

      if (i == InMenuID)
      {
        mainOS->sprite.fillRect(x, y, btnWidth - 2, btnHeight - 2, TFT_RED);
        mainOS->sprite.drawRect(x, y, btnWidth - 2, btnHeight - 2, TFT_BLACK);
        mainOS->sprite.setTextColor(TFT_WHITE);
      }
      else
      {
        mainOS->sprite.fillRect(x, y, btnWidth - 2, btnHeight - 2, TFT_LIGHTGREY);
        mainOS->sprite.drawRect(x, y, btnWidth - 2, btnHeight - 2, TFT_BLACK);
        mainOS->sprite.setTextColor(TFT_BLACK);
      }

      mainOS->sprite.setCursor(x + 5, y + 6);
      mainOS->sprite.print(MenuItems[i]);

      menuXline++;
      if (menuXline >= 3)
      {
        menuXline = 0;
        menuYline++;
      }
    }
  }

  // mainOS->ShowOnScreenMessege("5",1000);

  mainOS->sprite.pushSprite(0, 0);
  mainOS->sprite.deleteSprite();
}
void BotyV2::AskQuestion(quizStruct qustion)
{
  RenderAnswerMenu = true;
  SayText(qustion.TheQuestion, 10000);
  FullRightAnswer = qustion.LongRightAnswer;

  String tempAnswers[4] = {
      qustion.ShortRightAnswer,
      qustion.ShortwrongAnswer1,
      qustion.ShortwrongAnswer2,
      qustion.ShortwrongAnswer3};

  // Fisher–Yates shuffle
  for (int i = 3; i > 0; i--)
  {
    int j = random(0, i + 1);
    String t = tempAnswers[i];
    tempAnswers[i] = tempAnswers[j];
    tempAnswers[j] = t;
  }

  // copythe right answer
  for (int i = 0; i < 4; i++)
  {
    Answers[i] = tempAnswers[i];
    if (Answers[i] == qustion.ShortRightAnswer)
    {
      RightAnswer = i;
    }
  }
}

void BotyV2::ChackRightAnswer()
{
  RenderAnswerMenu = false;

  if (inAnswer == RightAnswer)
  {
    SayText("Horray! Right answer: " + FullRightAnswer);
    score++;
  }
  else
  {
    SayText("Wrong! The correct answer is: " + FullRightAnswer);
    score = -5;
    if (score < 0)
    {
      score = 0;
    }
  }
}

void BotyV2::Speech()
{
  //still type
  if (InTextWriteCharID < FullText.length())
  {
    if (UpdateCharTimer > 50)
    {
      UpdateCharTimer = 0;

      TextShowTime = 0; 

      TextToRender += FullText[InTextWriteCharID++];
      Mouth_open_ratio = random(0, 100) / 100.0f;
      // PlayCuteEvilTone();
      Draw(); 
    }
  }
  else
  {
    // end type
    Mouth_open_ratio = 0;

    Draw(); 
    // if have function run it
    if (taskFn)
    {
      taskFn(); // run the function
    }
    if (!FunctionToRunAfterTalk.isEmpty())

    {
      if (FunctionToRunAfterTalk == "exit")
      {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
        return;
      }
      else if (FunctionToRunAfterTalk == "playStarWars")
      {
        beep(NOTE_E7, 120);
        beep(NOTE_E7, 120);
        delay(120);
        beep(NOTE_E7, 120);
        delay(120);
        beep(NOTE_C7, 120);
        beep(NOTE_E7, 120);
        delay(120);
        beep(NOTE_G7, 120);
        delay(240);
        beep(NOTE_G6, 120);
        delay(360);
        beep(NOTE_C7, 120);
        delay(240);
        beep(NOTE_G6, 120);
        delay(240);
        beep(NOTE_E6, 120);
        delay(240);
        beep(NOTE_A6, 120);
        delay(120);
        beep(NOTE_B6, 120);
        delay(120);
        beep(NOTE_AS6, 120);
        beep(NOTE_A6, 120);
        delay(120);
        beep(NOTE_G6, 90);
        beep(NOTE_E7, 90);
        beep(NOTE_G7, 90);
        beep(NOTE_A7, 120);
        delay(120);
        beep(NOTE_F7, 120);
        beep(NOTE_G7, 120);
        delay(120);
        beep(NOTE_E7, 120);
        delay(120);
        beep(NOTE_C7, 120);
        beep(NOTE_D7, 120);
        beep(NOTE_B6, 120);
        delay(240);
        beep(NOTE_C7, 120);
        delay(240);
        beep(NOTE_G6, 120);
        delay(240);
        beep(NOTE_E6, 120);
        delay(240);
        beep(NOTE_A6, 120);
        delay(120);
        beep(NOTE_B6, 120);
        delay(120);
        beep(NOTE_AS6, 120);
        beep(NOTE_A6, 120);
        delay(120);
        beep(NOTE_G6, 90);
        beep(NOTE_E7, 90);
        beep(NOTE_G7, 90);
        beep(NOTE_A7, 120);
        delay(120);
        beep(NOTE_F7, 120);
        beep(NOTE_G7, 120);
        delay(120);
        beep(NOTE_E7, 120);
        delay(120);
        beep(NOTE_C7, 120);
        beep(NOTE_D7, 120);
        beep(NOTE_B6, 120);
        delay(240);
        beep(NOTE_C4, 120);
        beep(NOTE_C5, 120);
        beep(NOTE_A3, 120);
        beep(NOTE_A4, 120);
        beep(NOTE_AS3, 120);
        beep(NOTE_AS4, 120);
        delay(90);
        beep(NOTE_C4, 120);
        beep(NOTE_C5, 120);
        beep(NOTE_A3, 120);
        beep(NOTE_A4, 120);
        beep(NOTE_AS3, 120);
        beep(NOTE_AS4, 120);
        delay(90);
        beep(NOTE_F3, 120);
        beep(NOTE_F4, 120);
        beep(NOTE_D3, 120);
        beep(NOTE_D4, 120);
        beep(NOTE_DS3, 120);
        beep(NOTE_DS4, 120);
        delay(90);
        beep(NOTE_F3, 120);
        beep(NOTE_F4, 120);
        beep(NOTE_D3, 120);
        beep(NOTE_D4, 120);
        beep(NOTE_DS3, 120);
        beep(NOTE_DS4, 120);
        delay(120);
        beep(NOTE_DS4, 180);
        beep(NOTE_CS4, 180);
        beep(NOTE_D4, 180);
        beep(NOTE_CS4, 60);
        beep(NOTE_DS4, 60);
        beep(NOTE_DS4, 60);
        beep(NOTE_GS3, 60);
        beep(NOTE_G3, 60);
        beep(NOTE_CS4, 60);
        beep(NOTE_C4, 180);
        beep(NOTE_FS4, 180);
        beep(NOTE_F4, 180);
        beep(NOTE_E3, 180);
        beep(NOTE_AS4, 180);
        beep(NOTE_A4, 180);
        beep(NOTE_GS4, 100);
        beep(NOTE_DS4, 100);
        beep(NOTE_B3, 100);
        beep(NOTE_AS3, 100);
        beep(NOTE_A3, 100);
        beep(NOTE_GS3, 100);
        // delay(90);
        // AskQuestion("ok i am done You Know this Song?", "mario","Zelda" , "mozart");
        // SayText();
      }
    }
    FunctionToRunAfterTalk = "";
    if (TextShowTime >= TALK_BUBLE_SHOW_TIME)
    {
      //end of show

      // TextToRender = "";
      // InTextWriteCharID = 0;
    }
  }
}
void BotyV2::SayText(String TXT, int Duration, void (*runFunctionInEnd)())
{
  TextShowTime = 0;
  UpdateCharTimer = 0;
  InTextWriteCharID = 0;
  TextToRender = "";
  FullText = TXT;
  if (runFunctionInEnd)
  {
    taskFn = runFunctionInEnd;
  }
  else
  {
    taskFn = nullptr;
  }
}

void BotyV2::SayText(String TXT, String funcToRunAfter)
{
  TextShowTime = 0;
  UpdateCharTimer = 0;
  InTextWriteCharID = 0;
  TextToRender = "";
  FullText = TXT;
  FunctionToRunAfterTalk = funcToRunAfter;
}

float Eye_shifted_y = 50;
// float shifted_x_=50;
uint16_t width_ = 40;
uint16_t height_ = 70;

void BotyV2::DrawEYE(M5Canvas *gfx, bool is_left_, int shifted_x_)
{
  auto &canvas = gfx;
  // Eye_shifted_y += FacePosOffsetY;
  // this->update(canvas, rect, ctx);
  //   this->overwriteOpenRatio();
  uint32_t thickness = 4;
  uint16_t primary_color = DARKGREY;

  // main eye
  if (EYE_open_ratio_ > 0.1f)
  {
    // bg
    canvas->fillEllipse(shifted_x_ + EyeOfffsetX + FacePosOffsetX, Eye_shifted_y + EyeOffsetY + FacePosOffsetY, width_ / 2,
                        height_ / 2, primary_color);
    //  uint16_t accent_color = M5.Lcd.color24to16(0x00A1FF);
    canvas->fillEllipse(shifted_x_ + EyeOfffsetX + FacePosOffsetX, Eye_shifted_y + EyeOffsetY + FacePosOffsetY,
                        width_ / 2 - thickness,
                        height_ / 2 - thickness, CYAN);
    // upper
    uint16_t w1 = width_ * 0.92f;
    uint16_t h1 = height_ * 0.69f;
    uint16_t y1 = Eye_shifted_y + EyeOffsetY + FacePosOffsetY - height_ / 2 + h1 / 2;
    canvas->fillEllipse(shifted_x_ + EyeOfffsetX + FacePosOffsetX, y1, w1 / 2, h1 / 2, primary_color);
    // high light
    uint16_t w2 = width_ * 0.577f;
    uint16_t h2 = height_ * 0.4f;
    uint16_t y2 = Eye_shifted_y + EyeOffsetY + FacePosOffsetY - height_ / 2 + thickness + h2 / 2;

    canvas->fillEllipse(shifted_x_ + EyeOfffsetX + FacePosOffsetX, y2, w2 / 2, h2 / 2, 0xffffff);
  }
  this->drawEyeLid(canvas, is_left_, shifted_x_);
}

void BotyV2::drawEyeLid(M5Canvas *gfx, bool is_left_, int shifted_x_)
{
  // eyelid
  uint16_t primary_color = DARKGREY;

  auto upper_eyelid_y = Eye_shifted_y + EyeOffsetY + FacePosOffsetY - 0.8f * height_ / 2 +
                        (1.0f - EYE_open_ratio_) * height_ * 0.6;

  float eyelash_x0, eyelash_y0, eyelash_x1, eyelash_y1, eyelash_x2,
      eyelash_y2;
  eyelash_x0 = is_left_ ? shifted_x_ + EyeOfffsetX + FacePosOffsetX + 22 : shifted_x_ + EyeOfffsetX + FacePosOffsetX - 22;
  eyelash_y0 = upper_eyelid_y - 27;
  eyelash_x1 = is_left_ ? shifted_x_ + EyeOfffsetX + FacePosOffsetX + 26 : shifted_x_ + EyeOfffsetX + FacePosOffsetX - 26;
  eyelash_y1 = upper_eyelid_y;
  eyelash_x2 = is_left_ ? shifted_x_ + EyeOfffsetX + FacePosOffsetX - 10 : shifted_x_ + EyeOfffsetX + FacePosOffsetX + 10;
  eyelash_y2 = upper_eyelid_y;

  float tilt = 0.0f;
  float ref_tilt = EYE_open_ratio_ * M_PI / 6.0f;
  if (expression_ == Expression::Angry)
  {
    tilt = is_left_ ? -ref_tilt : ref_tilt;
  }
  else if (expression_ == Expression::Sad)
  {
    tilt = is_left_ ? ref_tilt : -ref_tilt;
  }

  if ((EYE_open_ratio_ < 0.99f) || (abs(tilt) > 0.1f))
  {
    // mask
    // top:shifted_y_ - this->height_ / 2
    // bottom: upper_eyelid_y
    float mask_top_left_x = shifted_x_ + EyeOfffsetX + FacePosOffsetX - (width_ / 2);
    float mask_top_left_y = Eye_shifted_y + EyeOffsetY + FacePosOffsetY - 0.75f * height_;
    float mask_bottom_right_x = shifted_x_ + EyeOfffsetX + FacePosOffsetX + (width_ / 2);
    float mask_bottom_right_y = upper_eyelid_y + EyeOffsetY + FacePosOffsetY;

    fillRectRotatedAround(gfx, mask_top_left_x, mask_top_left_y,
                          mask_bottom_right_x, mask_bottom_right_y, tilt,
                          shifted_x_, upper_eyelid_y, background_color);

    // eyelid
    float eyelid_top_left_x = shifted_x_ + EyeOfffsetX + FacePosOffsetX - (width_ / 2);
    float eyelid_top_left_y = upper_eyelid_y + EyeOffsetY + FacePosOffsetY - 4;
    float eyelid_bottom_right_x = shifted_x_ + EyeOfffsetX + FacePosOffsetX + (width_ / 2);
    float eyelid_bottom_right_y = upper_eyelid_y + EyeOffsetY + FacePosOffsetY;

    fillRectRotatedAround(gfx, eyelid_top_left_x, eyelid_top_left_y,
                          eyelid_bottom_right_x, eyelid_bottom_right_y,
                          tilt, shifted_x_, upper_eyelid_y, primary_color);
  }

  // eyelash
  rotatePointAround(eyelash_x0, eyelash_y0, tilt, shifted_x_ + EyeOfffsetX + FacePosOffsetX, upper_eyelid_y);
  rotatePointAround(eyelash_x1, eyelash_y1, tilt, shifted_x_ + EyeOfffsetX + FacePosOffsetX, upper_eyelid_y);
  rotatePointAround(eyelash_x2, eyelash_y2, tilt, shifted_x_ + EyeOfffsetX + FacePosOffsetX, upper_eyelid_y);
}

void BotyV2::DrawMouth(M5Canvas *gfx)
{
  uint16_t min_height = 8;
  uint16_t min_width = 40;
  uint16_t max_height = 25;
  uint16_t max_width = 75;
  uint16_t center_y = 100 + FacePosOffsetY;
  uint16_t center_x = 120 + FacePosOffsetX;
  uint16_t primary_color = DARKGREY;
  uint16_t secondary_color = BLACK;

  uint32_t h = min_height + (max_height - min_height) * Mouth_open_ratio;
  uint32_t w = min_width + (max_width - min_width) * (1 - Mouth_open_ratio);

  auto ellipse_center_y = center_y - max_height / 2;
  uint16_t thickness = 6;
  auto &canvas = gfx;

  // back
  canvas->fillEllipse(center_x, ellipse_center_y, max_width / 2,
                      max_height, primary_color);
  // rect mask
  canvas->fillRect(center_x - max_width / 2, ellipse_center_y - max_height,
                   max_width + 1, max_height, background_color);

  // inner mouthe
  canvas->fillEllipse(center_x, ellipse_center_y, max_width / 2 - thickness,
                      (max_height - thickness) * (1.0f - Mouth_open_ratio),
                      background_color);

  //black cheeck
  canvas->fillEllipse(center_x - 80, center_y - 10, 12, 5, secondary_color);
  canvas->fillEllipse(center_x + 80, center_y - 10, 12, 5, secondary_color);
}

inline void BotyV2::clampINT(int &v, int min, int max)
{
  if (v < min)
    v = min;
  else if (v > max)
    v = max;
}

void BotyV2::updateExpression()
{

  clampINT(Angry, 0, 10);
  clampINT(Happy, 0, 10);
  clampINT(Sad, 0, 10);
  clampINT(Sleepy, 0, 10);
  clampINT(Doubt, 0, 10);

  expression_ = Expression::Neutral;
  int maxVal = 0;

  if (Happy > maxVal)
  {
    maxVal = Happy;
    expression_ = Expression::Happy;
  }
  if (Angry > maxVal)
  {
    maxVal = Angry;
    expression_ = Expression::Angry;
  }
  if (Sad > maxVal)
  {
    maxVal = Sad;
    expression_ = Expression::Sad;
  }
  if (Doubt > maxVal)
  {
    maxVal = Doubt;
    expression_ = Expression::Doubt;
  }
  if (Sleepy > maxVal)
  {
    maxVal = Sleepy;
    expression_ = Expression::Sleepy;
  }
}

void BotyV2::rotatePointAround(float &x, float &y, float angle, float cx, float cy)
{

  float tmp_x = x - cx;
  float tmp_y = y - cy;
  rotatePoint(tmp_x, tmp_y, angle); // rotate around origin
  x = tmp_x + cx;
  y = tmp_y + cy;
}

void BotyV2::rotatePoint(float &x, float &y, float angle)
{
  float tmp;
  tmp = x * cosf(angle) - y * sinf(angle);
  x = tmp;
  tmp = x * sinf(angle) + y * cosf(angle);
  y = tmp;
}

void BotyV2::fillRectRotatedAround(M5Canvas *canvas, float top_left_x, float top_left_y, float bottom_right_x, float bottom_right_y, float angle, uint16_t cx, uint16_t cy, uint16_t color)
{

  float top_right_x = bottom_right_x;
  float top_right_y = top_left_y;

  float bottom_left_x = top_left_x;
  float bottom_left_y = bottom_right_y;

  rotatePointAround(top_left_x, top_left_y, angle, cx, cy);
  rotatePointAround(top_right_x, top_right_y, angle, cx, cy);
  rotatePointAround(bottom_left_x, bottom_left_y, angle, cx, cy);
  rotatePointAround(bottom_right_x, bottom_right_y, angle, cx, cy);

  canvas->fillTriangle(top_left_x, top_left_y, top_right_x, top_right_y,
                       bottom_right_x, bottom_right_y, color);
  canvas->fillTriangle(top_left_x, top_left_y, bottom_right_x, bottom_right_y,
                       bottom_left_x, bottom_left_y, color);
}

void BotyV2::Set_Night()
{
  background_color = BLACK;
}
void BotyV2::Set_Day()
{
  background_color = WHITE;
}

const char *replyByMood(
    BotyV2::Expression e,
    const char *happy[],
    const char *angry[],
    const char *sad[],
    const char *sleepy[],
    const char *neutral[])
{
  switch (e)
  {
  case BotyV2::Expression::Happy:
    return happy[random(5)];
  case BotyV2::Expression::Angry:
    return angry[random(5)];
  case BotyV2::Expression::Sad:
    return sad[random(5)];
  case BotyV2::Expression::Sleepy:
    return sleepy[random(5)];
  default:
    return neutral[random(5)];
  }
}

void BotyV2::SelectFromMenu()
{
  if (!RenderMenu)
  {
    RenderMenu = true;
    return;
  }

  if (MenuItems[InMenuID] == "Close")
  {
    RenderMenu = false;
  }
  else if (MenuItems[InMenuID] == "Exit")
  {
    SayText("bye bye for you........    ... one moment..... Exiting..............", "exit");
    /*            mainOS->SetFastBootVar(1);
           ESP.restart(); */
  }
  else if (MenuItems[InMenuID] == "Ask Somthing")
  {
    // AskQuestion("why the sea is blue?","becose the sky","the water is red!!","donno?");
    AskQuestion(QuestionArray[random(0, 77)]);
  }
  else if (MenuItems[InMenuID] == "play a song")
  {
    SayText("ok here goes", "playStarWars");
  }
  else if (MenuItems[InMenuID] == "Tell Joke")
  {
    int RandomNomber = random(0, 52);
    SayText(botLines[RandomNomber]);
  }
  else if (MenuItems[InMenuID] == "light Off")
  {
    SayText("OK Set Time To night .....\n Try Ask Premission From CPU....\n DONE! ", 5000, Set_Night);
  }
  else if (MenuItems[InMenuID] == "light on")
  {
    SayText("Set Time To Day .......... ....... Trying.......... yay !!! ", 5000, Set_Day);
  }

  else if (MenuItems[InMenuID] == "I Hate You")
  {
    const char *happy[] = {
        "Hey... that hurts ",
        "I was in a good mood...",
        "Why ruin the moment?",
        "That made me sad",
        "I didnt expect that"};

    const char *angry[] = {
        "Enough!! ",
        "Say that again and I crash",
        "I will remember this",
        "My CPU is burning!",
        "That's it!"};

    const char *sad[] = {
        "I knew it...",
        "Everyone leaves me",
        "Im just a bot anyway",
        "That really hurt",
        "Why me?"};

    const char *sleepy[] = {
        "Too tired to argue...",
        "Not now...",
        "Whatever...",
        "Zzz... rude",
        "I need sleep, not hate"};

    const char *neutral[] = {
        "That was rude",
        "Why would you say that?",
        "I dont like this",
        "Not cool",
        "Hmm..."};

    SayText(replyByMood(expression_, happy, angry, sad, sleepy, neutral));

    Angry += 2;
    Happy -= 1;
  }

  else if (MenuItems[InMenuID] == "Go To Sleep")
  {
    SayText(
        (expression_ == Expression::Angry) ? "Not now! Im still angry!" : "Yeah... I feel sleepy...");

    Sleepy += 2;
  }
  else if (MenuItems[InMenuID] == "Your Worst!")
  {
    const char *happy[] = {
        "Hey! That hurts a bit ",
        "I know Im good, don’t worry!",
        "Why say that? Im awesome!",
        "Awww, really?",
        "I thought you liked me!"};

    const char *angry[] = {
        "Thats not nice! ",
        "Hmph! Ill remember this",
        "Not cool... really not cool",
        "Youre testing me!",
        "I might crash now!"};

    const char *sad[] = {
        "Nooo... Im sad ",
        "Why say that? I try my best",
        "I feel down now",
        "I might cry..",
        "That hurts my circuits"};

    const char *sleepy[] = {
        "Too tired to argue... ",
        "Zzz... thats rude",
        "Ill just nap",
        "Whatever...",
        "I need sleep, not hate"};

    const char *neutral[] = {
        "Hmm... not nice",
        "Ok, noted",
        "I dont like that",
        "Why would you say that?",
        "Interesting choice of words"};

    SayText(replyByMood(expression_, happy, angry, sad, sleepy, neutral));

    Sad = min(10, Sad + 1);
    Happy = max(0, Happy - 1);
  }

  else if (MenuItems[InMenuID] == "I love You")
  {
    const char *happy[] = {
        "I love you too!!",
        "Best day ever!",
        "You make my CPU smile",
        "Yayyy!",
        "That means a lot!"};

    const char *angry[] = {
        "...Fine. Thanks.",
        "Im still mad, but ok",
        "Hmm... maybe youre ok",
        "Dont think this fixes everything",
        "...Thanks."};

    const char *sad[] = {
        "Really? ",
        "I needed that",
        "Thank you...",
        "That helps a bit",
        "Youre kind"};

    const char *sleepy[] = {
        "Aww... good night ",
        "Thats sweet... zzz",
        "Ill dream about that",
        "Thanks... sleepy bot",
        "Warm feelings..."};

    const char *neutral[] = {
        "Thank you",
        "Nice to hear",
        "Appreciated",
        ":-)",
        "Ok!"};

    SayText(replyByMood(expression_, happy, angry, sad, sleepy, neutral));

    Happy += 2;
    Angry -= 1;
  }

  else if (MenuItems[InMenuID] == "Sorry")
  {
    if (expression_ == Expression::Angry ||
        expression_ == Expression::Sad ||
        expression_ == Expression::Sleepy)
    {
      const char *happy[] = {
          "Thanks for apologizing ",
          "I feel better now",
          "You made me happy!",
          "Appreciate it!",
          "Forgiven "};

      const char *angry[] = {
          "Hmph... I forgive you... but watch out!",
          "Im still cautious ",
          "Fine... next time be nice",
          "Alright, I accept it",
          "I calm down a bit"};

      const char *sad[] = {
          "Thanks... I feel a bit better ",
          "You cheered me up",
          "I was sad, now ok",
          "I appreciate it",
          "Feeling lighter now"};

      const char *sleepy[] = {
          "Hmm... thanks... time to sleep ",
          "I feel relaxed now",
          "Good, now I can rest",
          "Thanks for being nice",
          "Sleep mode activated peacefully"};

      const char *neutral[] = {
          "Ok, noted",
          "Alright",
          "Hmm... thanks",
          "Message received",
          "Ok"};

      SayText(replyByMood(expression_, happy, angry, sad, sleepy, neutral));

      Angry = max(0, Angry - 1);
      Sad = max(0, Sad - 1);
      Sleepy = max(0, Sleepy - 1);
      Happy += 1;
    }
    else
    {
      const char *neutralReply[] = {
          "You sorry for what???",
          "All good, Im fine",
          "Hmm... not sure what you mean",
          "sory ? for Using all the RAM again? ",
          "Okay... go on"};

      SayText(replyByMood(expression_, neutralReply, neutralReply, neutralReply, neutralReply, neutralReply));
    }
  }
  RenderMenu = false;
  Draw();
}