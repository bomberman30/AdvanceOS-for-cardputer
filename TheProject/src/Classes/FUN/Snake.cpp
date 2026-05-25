#include "Snake.h"
#include <M5Cardputer.h>
#include "m5gfx.h"
#include "MyOS.h"
// Helper to draw a block at grid coordinates onto the sprite
void Snake::drawRectOnSprite(int x, int y, uint16_t color)
{
    // Convert Grid Coords to Pixel Coords
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;

    // Draw rounded rect on the sprite buffer
    // Padding ensures blocks don't touch perfectly for a cleaner look
    mainOS->sprite.fillRoundRect(px + 1, py + 1, CELL_SIZE - 2, CELL_SIZE - 2, 3, color);
}

void Snake::Begin()
{
    showTopBar=false;
    // Initialize Game State
    score = 0;
    gameOver = false;
    
    // Start in the middle of the screen
    currentX = GRID_W / 2;
    currentY = GRID_H / 2;
    snakeLength = 3;

    // Initialize Body (Snake starts at head position)
    for(int i=0; i<snakeLength; i++) {
        snakeBody[i].x = currentX - i;
        snakeBody[i].y = currentY;
    }

    direction = RIGHT; // Default moving right
    
    spawnApple();
    
    lastMoveTime = millis();
}

void Snake::Loop()
{
    if (gameOver) {
        // Check for restart trigger while on Game Over screen
        // Using a simple debounce or key check logic
        if(M5Cardputer.Keyboard.isKeyPressed('/')) { 
            Begin(); 
        }
        return; 
    }

    // Handle Input
    handleInput();

    // Speed Throttle: Only update game logic every MOVE_INTERVAL ms
    unsigned long currentTime = millis();
    if (currentTime - lastMoveTime >= MOVE_INTERVAL) {
        lastMoveTime = currentTime;
        updateGameLogic();
    }
    Draw();
}

void Snake::Draw()
{
    // 1. Clear the Sprite Buffer
    mainOS->sprite.createSprite(240,135);
    mainOS->sprite.fillScreen(TFT_BLACK); 
    
    // 2. Draw Game Elements onto the Sprite
    
    // Draw Apple (Red)
    drawRectOnSprite(applePos.x, applePos.y, TFT_RED);

    // Draw Snake Body
    for(int i=0; i<snakeLength; i++) {
        if(i == 0) 
            drawRectOnSprite(snakeBody[i].x, snakeBody[i].y, TFT_GREEN); // Head Green
        else 
            drawRectOnSprite(snakeBody[i].x, snakeBody[i].y, LIMEGREEN);  // Body Light Green
    }

    // Draw Score UI (Top Left)
    mainOS->sprite.setTextColor(TFT_WHITE);
    mainOS->sprite.setTextSize(2);
    String scoreText = "Score: ";
    scoreText += score;
    
    // Calculate position for text to ensure it stays within sprite bounds
    int textWidth = mainOS->sprite.textWidth(scoreText);
    if (textWidth < 240) {
        mainOS->sprite.drawString(scoreText, (240 - textWidth) / 2, 5);
    } else {
         mainOS->sprite.drawString(scoreText, 10, 5);
    }

    // Draw Game Over Text on Sprite if active
    if (gameOver) {
        mainOS->sprite.setTextColor(TFT_RED);
        mainOS->sprite.setTextSize(3);
        String msg = "GAME OVER";
        
        int w = mainOS->sprite.textWidth(msg);
        int x_pos = (240 - w) / 2;
        int y_pos = 60;
        
        // Draw on sprite buffer
        mainOS->sprite.drawString(msg, x_pos, y_pos);

        // Restart instruction
        mainOS->sprite.setTextColor(TFT_WHITE);
        String subMsg = "Press '/' to Retry";
        int sw = mainOS->sprite.textWidth(subMsg);
        mainOS->sprite.drawString(subMsg, (240 - sw) / 2, 95);
    }

    // 3. Push Sprite to Display for Smooth Rendering
    // This effectively does double-buffering: draw in memory -> push to screen
    mainOS->sprite.pushSprite(0, 0);
}

void Snake::handleInput()
{
    // Prevent reversing direction directly (e.g., cannot go Left if moving Right)
    
    // Up: ;
    if(M5Cardputer.Keyboard.isKeyPressed(';')) {
        if(direction != DOWN) direction = UP;
    }
    // Down: .
    else if(M5Cardputer.Keyboard.isKeyPressed('.')) {
        if(direction != UP) direction = DOWN;
    }
    // Left: ,
    else if(M5Cardputer.Keyboard.isKeyPressed(',')) {
        if(direction != RIGHT) direction = LEFT;
    }
    // Right: /
    else if(M5Cardputer.Keyboard.isKeyPressed('/')) {
        if(direction != LEFT) direction = RIGHT;
    }
}

void Snake::updateGameLogic()
{
    // Calculate new head position based on current direction
    int nextX = currentX;
    int nextY = currentY;

    switch (direction) {
        case UP:    nextY--; break;
        case DOWN:  nextY++; break;
        case LEFT:  nextX--; break;
        case RIGHT: nextX++; break;
    }

    // Check Wall Collision
    if (nextX < 0 || nextX >= GRID_W || nextY < 0 || nextY >= GRID_H) {
        gameOver = true;
        return;
    }

    // Check Self Collision
    for(int i=0; i<snakeLength; i++) {
        if(snakeBody[i].x == nextX && snakeBody[i].y == nextY) {
            gameOver = true;
            return;
        }
    }

    // Move Snake: Shift body array (Shift back by 1 index)
    for(int i=snakeLength-1; i>0; i--) {
        snakeBody[i] = snakeBody[i-1];
    }

    currentX = nextX;
    currentY = nextY;
    snakeBody[0].x = currentX;
    snakeBody[0].y = currentY;

    // Check Apple Collision
    if (currentX == applePos.x && currentY == applePos.y) {
        score += 10;
        if(snakeLength < 100) snakeLength++; // Grow snake
        spawnApple();
    }
}

void Snake::spawnApple()
{
    bool valid = false;
    while(!valid) {
        applePos.x = random(0, GRID_W);
        applePos.y = random(0, GRID_H);
        
        // Ensure apple doesn't spawn on snake body
        valid = true;
        for(int i=0; i<snakeLength; i++) {
            if(snakeBody[i].x == applePos.x && snakeBody[i].y == applePos.y) {
                valid = false;
                break;
            }
        }
    }
}
