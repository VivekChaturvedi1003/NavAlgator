#include "Bomb.h"
#include "Theme.h"
#include <cstdlib>
#include <cmath>

Bomb::Bomb(int cellSize, float interval) {
    row = 1;
    col = 1;
    this->cellSize = cellSize;
    this->interval = interval;
    timer = 0.0f;
    armTimer = 0.0f;
}

void Bomb::relocate(const Grid& grid, int avoidRow, int avoidCol) {
    int rows = grid.getRows();
    int cols = grid.getCols();

    for (int attempt = 0; attempt < 300; attempt++) {
        int r = 1 + rand() % (rows - 2);
        int c = 1 + rand() % (cols - 2);

        if (grid.getCellType(r, c) != PATH) continue;                
        int distance = std::abs(r - avoidRow) + std::abs(c - avoidCol);
        if (distance < MIN_DISTANCE) continue;                        

        row = r;
        col = c;
        timer = 0.0f;
        armTimer = ARM_DELAY;                                       
        return;
    }
}

void Bomb::update(const Grid& grid, int playerRow, int playerCol, float deltaTime) {
    if (armTimer > 0.0f) armTimer -= deltaTime;

    timer += deltaTime;
    if (timer >= interval) {
        relocate(grid, playerRow, playerCol);
        timer = 0.0f;
    }
}

void Bomb::render() const {
    float cx = col * cellSize + cellSize / 2.0f;
    float cy = row * cellSize + cellSize / 2.0f;
    float t = (float)GetTime();

    if (!isArmed()) {
    
        float k = armTimer / ARM_DELAY;                           
        float ringR = cellSize * (0.45f + 0.5f * k);
        DrawCircleLines((int)cx, (int)cy, ringR, Theme::Danger);
        DrawCircleLines((int)cx, (int)cy, ringR - 2.0f, Fade(Theme::Danger, 0.6f));
        DrawCircleV(Vector2{cx, cy}, cellSize * 0.18f, Fade(Theme::BombBody, 0.6f));
        return;
    }

    float r = cellSize * 0.30f;
    DrawCircleV(Vector2{cx + 2.0f, cy + 4.0f}, r, Fade(BLACK, 0.35f));            
    DrawCircleV(Vector2{cx, cy}, r, Theme::BombBody);                               
    DrawCircleV(Vector2{cx - r * 0.35f, cy - r * 0.35f}, r * 0.25f, Fade(WHITE, 0.5f));
    DrawCircleLines((int)cx, (int)cy, r, Theme::Danger);

    
    DrawLine((int)(cx + r * 0.3f), (int)(cy - r * 0.8f), (int)(cx + r * 0.8f), (int)(cy - r * 1.4f), Theme::TextDim);
    float spark = 0.5f + 0.5f * std::sin(t * 14.0f);
    DrawCircleV(Vector2{cx + r * 0.85f, cy - r * 1.45f}, cellSize * (0.07f + 0.05f * spark), Theme::BombSpark);
}
