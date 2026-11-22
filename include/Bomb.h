#ifndef BOMB_H
#define BOMB_H

#include "raylib.h"
#include "Grid.h"

class Bomb {
private:
    int row, col;
    int cellSize;
    float interval;      
    float timer;         
    float armTimer;      

public:
    static constexpr float ARM_DELAY = 0.8f;       
    static constexpr int MIN_DISTANCE = 5;         

    Bomb(int cellSize, float interval = 4.0f);

    void setInterval(float seconds) { interval = seconds; }


    void relocate(const Grid& grid, int avoidRow, int avoidCol);

    void update(const Grid& grid, int playerRow, int playerCol, float deltaTime);

    bool isArmed() const { return armTimer <= 0.0f; }
    int getRow() const { return row; }
    int getCol() const { return col; }

    void render() const;
};

#endif
