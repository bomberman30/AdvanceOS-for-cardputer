#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include "m5gfx.h" // Ensure sprite methods are available

// Grid Configuration for Snake Game
#define CELL_SIZE 12      
#define GRID_W   (240 / CELL_SIZE) 
#define GRID_H   (135 / CELL_SIZE) 

class Snake : public GlobalParentClass
{
public:
    Snake(MyOS *os) : GlobalParentClass(os) {}

    void drawRectOnSprite(int x, int y, uint16_t color);

    void Begin() override;
    void Loop() override;
    void Draw() override;

    void handleInput();

    void updateGameLogic();

    void spawnApple();

private:
    // Game State Variables
    struct Point { int x, y; };
    
    // Snake body (max length 100 is safe for ESP32)
    Point snakeBody[100]; 
    int   snakeLength = 4;
    int   currentX, currentY; // Head position
    
    Point applePos;
    int   direction;          // Current moving direction
    int   score = 0;
    bool  gameOver = false;

    // Timing variables to control speed (FPS throttling)
    unsigned long lastMoveTime = 0;
    const unsigned long MOVE_INTERVAL = 150; // ms between moves (adjust for difficulty)

    // Direction Constants
    enum Dir { UP, DOWN, LEFT, RIGHT };
};
