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
    float moveInterval;   
    bool frozen;          

public:
    Enemy(int startRow, int startCol, int cellSize, float moveInterval = 0.3f);

 
    void update(const Grid& grid, const Player& player, float deltaTime);

    int getRow() const { return row; }
    int getCol() const { return col; }

    void setFrozen(bool value) { frozen = value; }
    void setStepTime(float seconds) { moveInterval = seconds; }

    void reset(int startRow, int startCol);
    void render() const;
};

#endif
