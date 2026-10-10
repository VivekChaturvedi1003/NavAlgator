#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "Grid.h"
#include "Player.h"

class Enemy {
private:
    int row, col;
    int cellSize;
    float moveTimer;
    float moveInterval;   // controls difficulty: lower = enemy moves more often

public:
    Enemy(int startRow, int startCol, int cellSize, float moveInterval = 0.3f);

    // Recalculates the shortest path to the player using BFS, then takes ONE
    // step along it. Called every frame from GameEngine - this is what makes
    // the enemy re-target continuously instead of following a stale path.
    void update(const Grid& grid, const Player& player, float deltaTime);

    int getRow() const { return row; }
    int getCol() const { return col; }

    void reset(int startRow, int startCol);
    void render() const;
};

#endif
