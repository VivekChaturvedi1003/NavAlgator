#include "Player.h"
#include "Theme.h"

Player::Player(int startRow, int startCol, int cellSize, float stepTime) {
    row = startRow;
    col = startCol;
    lives = MAX_LIVES;
    this->cellSize = cellSize;
    this->stepTime = stepTime;
    moveTimer = 0.0f;
    speedTimer = 0.0f;
    shieldTimer = 0.0f;
}

void Player::handleInput(const Grid& grid, float deltaTime) {
    moveTimer -= deltaTime;
    if (moveTimer < 0.0f) moveTimer = 0.0f;

    
    if (speedTimer > 0.0f)  speedTimer -= deltaTime;
    if (shieldTimer > 0.0f) shieldTimer -= deltaTime;

    int dRow = 0;
    int dCol = 0;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))         dRow = -1;
    else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  dRow = 1;
    else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dCol = -1;
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dCol = 1;

    if ((dRow != 0 || dCol != 0) && moveTimer <= 0.0f) {
        int newRow = row + dRow;
        int newCol = col + dCol;

        float wait = isSpeedBoosted() ? stepTime * 0.5f : stepTime;

        
        if (grid.isWalkable(newRow, newCol)) {
            moveHistory.push({row, col});   
            row = newRow;
            col = newCol;
            wait *= grid.moveCost(row, col);   
        }
        moveTimer = wait;
    }

    if (IsKeyPressed(KEY_U)) {
        undo();
        moveTimer = stepTime;
    }
}

void Player::undo() {
    if (!moveHistory.isEmpty()) {
        Position last = moveHistory.pop();
        row = last.row;
        col = last.col;
    }
}

void Player::reset(int startRow, int startCol) {
    row = startRow;
    col = startCol;
    moveTimer = 0.0f;
    speedTimer = 0.0f;
    shieldTimer = 0.0f;
    moveHistory.clear();
}

void Player::render() const {
    float cx = col * cellSize + cellSize / 2.0f;
    float cy = row * cellSize + cellSize / 2.0f;
    float r  = cellSize * 0.36f;

    
    if (isImmune()) {
        DrawCircleV(Vector2{cx, cy}, r + 7.0f, Fade(Theme::PowerShield, 0.30f));
        DrawCircleLines((int)cx, (int)cy, r + 7.0f, Theme::PowerShield);
    }
    if (isSpeedBoosted()) {
        DrawCircleLines((int)cx, (int)cy, r + 4.0f, Theme::PowerSpeed);
    }

    DrawCircleV(Vector2{cx + 2.0f, cy + 4.0f}, r, Fade(BLACK, 0.35f));       
    DrawCircleV(Vector2{cx, cy}, r, Theme::PlayerBody);                       
    DrawCircleV(Vector2{cx - r * 0.30f, cy - r * 0.30f}, r * 0.30f, Fade(WHITE, 0.65f));  
    DrawCircleLines((int)cx, (int)cy, r, WHITE);                              
}
