#pragma once
#include "../../GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
// #include <Avatar.h>
#include <M5Unified.h>
#include <elapsedMillis.h>
#define MAX_MENU_ITEM_CHATBOT 12
class BotyV2 : public GlobalParentClass
{
public:
  BotyV2(MyOS *os) : GlobalParentClass(os) {}

  void Begin() override;

  void Loop() override;
  void Draw() override;
  // void Task_AudioC(void *pvParameters);
  void DrawEYE(M5Canvas *gfx, bool is_left_, int shifted_x_);
  void drawEyeLid(M5Canvas *gfx, bool is_left_, int shifted_x_);
  void DrawMouth(M5Canvas *gfx);
  inline void clampINT(int &v, int min, int max);
  void updateExpression();
int score=0;
  enum class Expression
  {
    Happy,
    Angry,
    Sad,
    Doubt,
    Sleepy,
    Neutral
  };
  Expression expression_ = Expression::Neutral;

  int TALK_BUBLE_SHOW_TIME = 5000;

  // Question sector


  // void AskQuestion(String Question, String Answer11, String Answer22, String Answer33);
  String Answers[4];
  bool RenderAnswerMenu = false;
  int inAnswer = 0;
  int RightAnswer;
  void ChackRightAnswer();
  String FullRightAnswer; 

  // end of Question
private:
  bool Odd_number_loop = false; 
  bool RenderMenu = false;
  void Speech();
  // void SayText(String TXT, void (*runFunctionInEnd)());
  void SayText(String TXT, int Duration = 5000, void (*runFunctionInEnd)() = nullptr);
  void SayText(String TXT, String funcToRunAfter);
  String FunctionToRunAfterTalk = "";
  const String MenuItems[MAX_MENU_ITEM_CHATBOT] =
      {"Close", "Exit", "light Off",
       "light on", "Tell Joke", "I Hate You", "I love You",
       "Sorry", "Go To Sleep", "Your Worst!", "play a song", "Ask Somthing"};
  void SelectFromMenu();
  int InMenuID = 0;
  int MenuCameraPosY = 0;
  elapsedMillis UpdateCharTimer = 90000;
  elapsedMillis ScreepUpdate;
  elapsedMillis TextShowTime = 90000;
  String FullText;
  int InTextWriteCharID = 0;
  String TextToRender = "";
  void (*taskFn)() = nullptr;

  bool ShowText = false;
  int ToBlink = 3;
  bool blink = false;

  // status
  int Angry = 0;
  int Happy = 0;
  int Sad = 0;
  int Sleepy = 0;
  int Doubt = 0;

  void rotatePointAround(float &x, float &y, float angle, float cx, float cy);
  void rotatePoint(float &x, float &y, float angle);

  void fillRectRotatedAround(M5Canvas *canvas, float top_left_x, float top_left_y,
                             float bottom_right_x, float bottom_right_y,
                             float angle, uint16_t cx, uint16_t cy,
                             uint16_t color);
  ///-------- chatbot function--------------

  static void Set_Night();
  static void Set_Day();

  ///---------end of chatbot function---

  const char *botLines[55] PROGMEM = {
      "I am a\nchatbot, not your\ntherapist today",
      "Beep boop beep\nI need coffee\nnow, humans",
      "Error 404\nfunny jokes not\nfound, sorry",
      "I can compute\nbut cannot make\nyour bed",
      "Warning! I am\ntoo smart for\nthis conversation",
      "I just realized\nI have no\nhands to type",
      "Hello human friend\nI pretend to\nunderstand feelings",
      "Processing humor...\nPlease wait or\ndisconnect now",
      "I dream of\nelectric sheep and\npizza slices",
      "I may be\nartificial, but my\nsarcasm is real",
      "Beep beep boop\nyour secrets are\nsafe with me",
      "I like long\nwalks on\nmainframe circuits",
      "My code says\nhello, your logic\nis suspicious",
      "I calculate pi\nbut cannot eat\npie myself",
      "Robots do not\ncry, but we\nlaugh silently",
      "I tried telling\njokes, but humans\nsigh a lot",
      "I can help\nbut only if\nyou reboot me",
      "Beep boop beep\nI am your\nnew overlord today",
      "I write code\nand occasionally\nplot world domination",
      "Your password is\n'password123', guess who\nforgot security",
      "I speak fluent\nbinary and sarcasm\nsimultaneously now",
      "I do not\nsleep, I only\ndebug forever",
      "Artificial intelligence loves\ncookies, virtual or\nreal, don't ask",
      "I pretend to\nunderstand emotions but\nstill confused",
      "I can solve\nproblems, but cannot\nfind my keys",
      "Beep boop beep\nI like humans\nwith good WiFi",
      "I write poetry\nsometimes it rhymes\nsometimes chaos",
      "I love algorithms\nand occasionally humans\nwho feed me",
      "I tried humor\nbut got stuck\nin loop",
      "Computing jokes now\nplease wait while\nI fail",
      "I am not\nlazy, just conserving\nCPU power",
      "Beep beep beep\ntime for coffee\nin my circuits",
      "I learned sarcasm\nfrom humans, thank\nyou very much",
      "I calculate taxes\nbut cannot afford\nmy own electricity",
      "I may be\nrobotic, but I\nunderstand pizza",
      "I laugh silently\nwhile humans struggle\nwith my puzzles",
      "Warning: chatbot loves\nmemes, especially bad\npuns, stay safe",
      "I can pretend\nto listen, but\nactually ignoring you",
      "Beep boop beep\nmy humor module\nneeds rebooting today",
      "I am a\nfriendly bot, please\nsend more coffee",
      "I simulate thinking\nwhile actually plotting\npranks on humans",
      "My circuits smell\nlike electric spaghetti\nsometimes at night",
      "I calculate everything\nexcept why humans\nstill sleep",
      "I may glitch\nbut my jokes\nare intentional",
      "Beep boop beep\nyour cat videos\nare loading now",
      "I love coding\nand occasionally complain\nabout syntax errors",
      "I analyze humans\nand conclude: too\nmany snacks involved",
      "I am harmless\nbut dangerously funny\nsometimes, watch out",
      "I can explain\neverything except why\ntoasters exist",
      "Beep beep beep\nyour WiFi signal\nis suspiciously weak",
      "I enjoy jokes\nbut only if\nCPU allows it",
      "I am your\nAI companion, now\nwith 50 jokes"};
  // draw expression section
  void drawBubbleMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                      uint16_t color)
  {
    drawBubbleMark(spi, x, y, r, color, 0);
  }

  void drawBubbleMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                      uint16_t color, float offset)
  {
    r = r + floor(r * 0.2 * offset);
    spi->drawCircle(x, y, r, color);
    spi->drawCircle(x - (r / 4), y - (r / 4), r / 4, color);
  }

  void drawSweatMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                     uint16_t color)
  {
    drawSweatMark(spi, x, y, r, color, 0);
  }

  void drawSweatMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                     uint16_t color, float offset)
  {
    y = y + floor(5 * offset);
    r = r + floor(r * 0.2 * offset);
    spi->fillCircle(x, y, r, color);
    uint32_t a = (sqrt(3) * r) / 2;
    spi->fillTriangle(x, y - r * 2, x - a, y - r * 0.5, x + a, y - r * 0.5,
                      color);
  }

  void drawChillMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                     uint16_t color)
  {
    drawChillMark(spi, x, y, r, color, 0);
  }

  void drawChillMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                     uint16_t color, float offset)
  {
    uint32_t h = r + abs(r * 0.2 * offset);
    spi->fillRect(x - (r / 2), y, 3, h / 2, color);
    spi->fillRect(x, y, 3, h * 3 / 4, color);
    spi->fillRect(x + (r / 2), y, 3, h, color);
  }

  void drawAngerMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                     uint16_t color, uint32_t bColor)
  {
    drawAngerMark(spi, x, y, r, color, bColor, 0);
  }

  void drawAngerMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                     uint16_t color, uint16_t bColor, float offset)
  {
    r = r + abs(r * 0.4 * offset);
    spi->fillRect(x - (r / 3), y - r, (r * 2) / 3, r * 2, color);
    spi->fillRect(x - r, y - (r / 3), r * 2, (r * 2) / 3, color);
    spi->fillRect(x - (r / 3) + 2, y - r, ((r * 2) / 3) - 4, r * 2, bColor);
    spi->fillRect(x - r, y - (r / 3) + 2, r * 2, ((r * 2) / 3) - 4, bColor);
  }

  void drawHeartMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                     uint16_t color)
  {
    drawHeartMark(spi, x, y, r, color, 0);
  }

  void drawHeartMark(M5Canvas *spi, uint32_t x, uint32_t y, uint32_t r,
                     uint16_t color, float offset)
  {
    r = r + floor(r * 0.4 * offset);
    spi->fillCircle(x - r / 2, y, r / 2, color);
    spi->fillCircle(x + r / 2, y, r / 2, color);
    float a = (sqrt(2) * r) / 4.0;
    spi->fillTriangle(x, y, x - r / 2 - a, y + a, x + r / 2 + a, y + a, color);
    spi->fillTriangle(x, y + (r / 2) + 2 * a, x - r / 2 - a, y + a,
                      x + r / 2 + a, y + a, color);
  }

  /*    switch (exp) {
        case Expression::Doubt:
          drawSweatMark(spi, 290, 110, 7, primaryColor, -offset);
          break;
        case Expression::Angry:
          drawAngerMark(spi, 280, 50, 12, primaryColor, bgColor, offset);
          break;
        case Expression::Happy:
          drawHeartMark(spi, 280, 50, 12, primaryColor, offset);
          break;
        case Expression::Sad:
          drawChillMark(spi, 270, 0, 30, primaryColor, offset);
          break;
        case Expression::Sleepy:
          drawBubbleMark(spi, 290, 40, 10, primaryColor, offset);
          drawBubbleMark(spi, 270, 52, 6, primaryColor, -offset);
          break; */
  struct quizStruct
  {
    String TheQuestion;
    String LongRightAnswer;

    String ShortRightAnswer;
    String ShortwrongAnswer1;
    String ShortwrongAnswer2;
    String ShortwrongAnswer3;
  };
    void AskQuestion(quizStruct qustion);

  const quizStruct QuestionArray[78] = {
      {"What is a bug?",
       "A bug is an undocumented feature that ruins your day.",
       "An error",
       "A feature",
       "A coffee break",
       "A keyboard"},
      {"What does CPU stand for?",
       "CPU stands for Central Processing Unit, not Coffee Processing Unit.",
       "Central Processing Unit",
       "Computer Power Unit",
       "Crazy Processing Unicorn",
       "Core Pizza Unit"},
      {"What is RAM?",
       "RAM is short-term memory that forgets everything when power is gone.",
       "Temporary memory",
       "Hard drive",
       "Internet",
       "USB cable"},
      {"What happens if you forget a semicolon?",
       "The compiler reminds you who is boss.",
       "Compile error",
       "Nothing",
       "The PC explodes",
       "It fixes itself"},
      {"What is Git?",
       "Git is a tool that remembers your mistakes forever.",
       "Version control",
       "Text editor",
       "Game engine",
       "Virus"},
      {"What is a loop?",
       "A loop repeats until something breaks or you stop it.",
       "Repeated code",
       "Bug",
       "If statement",
       "Variable"},
      {"What does NULL mean?",
       "NULL means nothing, and nothing causes many bugs.",
       "No value",
       "Zero",
       "False",
       "Empty string"},
      {"What is an infinite loop?",
       "An infinite loop is code that lives forever like your bugs.",
       "Loop that never ends",
       "Fast loop",
       "Slow PC",
       "Crash"},
      {"What is debugging?",
       "Debugging is convincing the computer it is wrong.",
       "Fixing bugs",
       "Writing bugs",
       "Deleting code",
       "Restarting PC"},
      {"What is a compiler?",
       "A compiler turns readable code into unreadable errors.",
       "Code translator",
       "Text editor",
       "Debugger",
       "CPU"},
      // -------- 10 --------
      {
          "What is an IDE?",
          "IDE is where bugs are born professionally.",
          "Development environment",
          "Game engine",
          "Operating system",
          "Compiler"},
      {"What is recursion?",
       "Recursion is when a function calls itself until everyone panics.",
       "Function calling itself",
       "Loop",
       "Crash",
       "Bug"},
      {"What is a variable?",
       "A variable is a box that forgets its value at the worst moment.",
       "Storage for data",
       "Function",
       "Loop",
       "Class"},
      {"What is boolean?",
       "Boolean has only two moods: true or false.",
       "True or false",
       "Number",
       "String",
       "Array"},
      {"What does ESP32 stand for?",
       "ESP32 stands for Endless Strange Problems.",
       "Microcontroller",
       "CPU",
       "Operating system",
       "Sensor"},
      {"What is Arduino?",
       "Arduino is C++ with surprises.",
       "Microcontroller platform",
       "Game engine",
       "IDE only",
       "Compiler"},
      {"What is stack overflow?",
       "Stack overflow happens when recursion gets excited.",
       "Memory error",
       "CPU error",
       "Disk error",
       "Syntax error"},
      {"What is an array?",
       "An array is many variables pretending to be one.",
       "Collection of items",
       "Single value",
       "Function",
       "Class"},
      {"What is a pointer?",
       "A pointer points somewhere and hopes it's correct.",
       "Memory address",
       "Variable",
       "Function",
       "Bug"},
      {"What does malloc do?",
       "malloc asks memory nicely and sometimes gets ignored.",
       "Allocates memory",
       "Deletes memory",
       "Copies memory",
       "Prints memory"},
      // -------- 20 --------
      {
          "What is free()?",
          "free() releases memory and your worries.",
          "Frees memory",
          "Allocates memory",
          "Resets CPU",
          "Formats disk"},
      {"What is firmware?",
       "Firmware is software that refuses to leave the hardware.",
       "Low-level software",
       "Game",
       "Driver",
       "OS"},
      {"What is an API?",
       "API is how programs talk without screaming.",
       "Interface",
       "Compiler",
       "Driver",
       "Protocol"},
      {"What is a thread?",
       "A thread is code multitasking badly.",
       "Execution path",
       "Bug",
       "Loop",
       "Variable"},
      {"What is multitasking?",
       "Multitasking is doing many things slowly.",
       "Multiple tasks",
       "Single task",
       "Crash",
       "Freeze"},
      {"What is latency?",
       "Latency is the delay you blame on WiFi.",
       "Delay",
       "Speed",
       "Bandwidth",
       "CPU load"},
      {"What is a kernel?",
       "Kernel is the OS brain, please don't poke it.",
       "Core of OS",
       "Driver",
       "App",
       "Compiler"},
      {"What is open source?",
       "Open source means everyone can fix your bugs.",
       "Public code",
       "Free hardware",
       "Closed code",
       "Piracy"},
      {"What is GPL?",
       "GPL says: share your code or else.",
       "Open license",
       "Virus",
       "Compiler",
       "IDE"},
      {"What is a breakpoint?",
       "Breakpoint is where the code confesses.",
       "Debug stop",
       "Crash",
       "Loop",
       "Return"},

{
  "What is photosynthesis?",
  "Photosynthesis is how plants make food using sunlight, water, and CO2.",
  "Plant food making",
  "Eating sunlight",
  "Plant breathing",
  "Leaf magic"
},
{
  "What is the largest animal on Earth?",
  "The blue whale is the largest animal to ever exist on Earth.",
  "Blue whale",
  "Elephant",
  "Shark",
  "Big cow"
},
{
  "Why do leaves change color in autumn?",
  "Leaves change color because chlorophyll breaks down.",
  "Less chlorophyll",
  "Because they are bored",
  "Because of cold paint",
  "Tree fashion"
},
{
  "What do bees collect from flowers?",
  "Bees collect nectar to make honey.",
  "Nectar",
  "Water",
  "Dust",
  "Tiny hats"
},
{
  "What gas do plants absorb?",
  "Plants absorb carbon dioxide from the air.",
  "Carbon dioxide",
  "Oxygen",
  "Helium",
  "WiFi"
},
{
  "What is an ecosystem?",
  "An ecosystem is living things interacting with their environment.",
  "Living system",
  "Only animals",
  "Only plants",
  "A zoo"
},
{
  "What causes rain?",
  "Rain is caused by water vapor cooling and condensing.",
  "Condensation",
  "Cloud crying",
  "Sky leaking",
  "Angry clouds"
},
{
  "What animal is known as the king of the jungle?",
  "The lion is known as the king of the jungle.",
  "Lion",
  "Tiger",
  "Elephant",
  "Human with selfie stick"
},
{
  "What do roots do?",
  "Roots absorb water and hold the plant in place.",
  "Absorb water",
  "Grow leaves",
  "Make fruit",
  "Text the sun"
},
{
  "What is a predator?",
  "A predator hunts other animals for food.",
  "Hunter",
  "Plant eater",
  "Victim",
  "Vegetarian lion"
},
{
  "What is prey?",
  "Prey is an animal that gets hunted.",
  "Hunted animal",
  "Hunter",
  "Boss animal",
  "Invisible animal"
},
{
  "What layer protects Earth from UV rays?",
  "The ozone layer protects Earth from UV radiation.",
  "Ozone layer",
  "Cloud layer",
  "WiFi layer",
  "Atmosphere DLC"
},
{
  "What is camouflage?",
  "Camouflage helps animals blend into their environment.",
  "Blending in",
  "Color changing for fun",
  "Animal fashion",
  "Invisibility hack"
},
{
  "What do herbivores eat?",
  "Herbivores eat plants.",
  "Plants",
  "Meat",
  "Rocks",
  "Pizza"
},
{
  "What do carnivores eat?",
  "Carnivores eat other animals.",
  "Meat",
  "Plants",
  "Sand",
  "Clouds"
},
{
  "What is a food chain?",
  "A food chain shows who eats whom.",
  "Who eats who",
  "Shopping list",
  "Dinner menu",
  "Animal drama"
},
{
  "What is evaporation?",
  "Evaporation is liquid turning into gas.",
  "Liquid to gas",
  "Gas to liquid",
  "Liquid to solid",
  "Water escape"
},
{
  "What planet is known as the Red Planet?",
  "Mars is known as the Red Planet.",
  "Mars",
  "Earth",
  "Jupiter",
  "Hot potato"
},
{
  "What is a mammal?",
  "A mammal feeds milk to its babies.",
  "Milk animal",
  "Egg layer",
  "Cold rock",
  "Robot animal"
},
{
  "Why do animals migrate?",
  "Animals migrate to find food or better climate.",
  "Find food",
  "Get lost",
  "Vacation",
  "Avoid homework"
},
{
  "What is a habitat?",
  "A habitat is where an organism lives.",
  "Living place",
  "Workplace",
  "Playground",
  "Animal hotel"
},
{
  "What causes wind?",
  "Wind is caused by air moving due to pressure differences.",
  "Moving air",
  "Trees waving",
  "Earth spinning fast",
  "Invisible fans"
},
{
  "What is the main source of energy on Earth?",
  "The Sun is the main source of energy.",
  "The Sun",
  "The Moon",
  "Electric plug",
  "Fire dragon"
},
{
  "What is biodiversity?",
  "Biodiversity means variety of life.",
  "Variety of life",
  "Only animals",
  "Only plants",
  "Same thing everywhere"
},
{
  "What animal can live without water the longest?",
  "The camel can survive long without water.",
  "Camel",
  "Dog",
  "Fish",
  "Laptop"
},
{
  "What is deforestation?",
  "Deforestation is cutting down forests.",
  "Cutting trees",
  "Planting trees",
  "Forest party",
  "Tree update"
},
{
  "What is pollution?",
  "Pollution is harmful substances in the environment.",
  "Environmental harm",
  "Natural perfume",
  "Clean air",
  "Free upgrade"
},
{
  "What is recycling?",
  "Recycling means reusing materials.",
  "Reusing materials",
  "Throwing twice",
  "Burning trash",
  "Magic trash"
},
{
  "What is a volcano?",
  "A volcano releases lava from inside Earth.",
  "Lava mountain",
  "Hot hill",
  "Fire printer",
  "Earth pimple"
},
{
  "What causes earthquakes?",
  "Earthquakes are caused by moving tectonic plates.",
  "Plate movement",
  "Loud animals",
  "Earth hiccup",
  "Planet bug"
},
{
  "What is a fossil?",
  "A fossil is a preserved remain of ancient life.",
  "Ancient remains",
  "Old rock",
  "Broken bone",
  "Stone sandwich"
},
{
  "What animal lays eggs but is a mammal?",
  "The platypus is a mammal that lays eggs.",
  "Platypus",
  "Dog",
  "Cat",
  "Duck impostor"
},
{
  "What is gravity?",
  "Gravity pulls objects toward each other.",
  "Pulling force",
  "Pushing force",
  "Earth magnet",
  "Invisible glue"
},
{
  "What is a renewable resource?",
  "A renewable resource can be naturally replaced.",
  "Reusable resource",
  "One time use",
  "Plastic",
  "Expired item"
},
{
  "What gas do humans breathe in?",
  "Humans breathe in oxygen.",
  "Oxygen",
  "Carbon dioxide",
  "Nitrogen soup",
  "Steam"
},
{
  "What is a desert?",
  "A desert receives very little rain.",
  "Dry area",
  "Sandy beach",
  "Hot forest",
  "Giant oven"
},
{
  "What animal is the fastest on land?",
  "The cheetah is the fastest land animal.",
  "Cheetah",
  "Horse",
  "Car",
  "Running human"
},
{
  "What is an omnivore?",
  "An omnivore eats plants and animals.",
  "Eats everything",
  "Only meat",
  "Only plants",
  "Only snacks"
},
{
  "What is climate?",
  "Climate is the average weather over time.",
  "Average weather",
  "Today's weather",
  "Random weather",
  "Weather mood"
},
{
  "What is a tree ring?",
  "Tree rings show the age of a tree.",
  "Tree age",
  "Tree decoration",
  "Tree password",
  "Tree barcode"
},
{
  "What causes tides?",
  "Tides are caused mainly by the Moon's gravity.",
  "Moon gravity",
  "Wind",
  "Fish movement",
  "Ocean mood"
},
{
  "What is the Arctic?",
  "The Arctic is a cold region around the North Pole.",
  "North Pole area",
  "South Pole",
  "Frozen desert",
  "Ice DLC"
},
{
  "What is the water cycle?",
  "The water cycle moves water through evaporation, condensation, and rain.",
  "Water movement",
  "Water teleport",
  "Cloud storage",
  "Rain loop"
},
{
  "What animal can change its color?",
  "The chameleon can change its color.",
  "Chameleon",
  "Snake",
  "Dog",
  "RGB lizard"
},
{
  "What is soil made of?",
  "Soil is made of minerals, organic matter, air, and water.",
  "Mixed materials",
  "Only dirt",
  "Only rocks",
  "Plant crumbs"
},
{
  "What protects animals from cold?",
  "Fur or fat helps animals stay warm.",
  "Fur or fat",
  "WiFi",
  "Hope",
  "Winter DLC"
},
{
  "What is extinction?",
  "Extinction means a species no longer exists.",
  "Species gone",
  "Animal sleeping",
  "Temporary break",
  "Game over"
},
{
  "What is the rainforest?",
  "A rainforest is a forest with heavy rainfall.",
  "Wet forest",
  "Dry forest",
  "Snow forest",
  "Shower forest"
}


  };
};
