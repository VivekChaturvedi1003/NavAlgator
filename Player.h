#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "Grid.h"
#include "DataStructures.h"

struct Position {
    int row, col;
};

class Player {
private:
    int row, col;
    int lives;
    int cellSize;
    float stepTime;    
    float moveTimer;    
    ManualStack<Position> moveHistory;   

public:
    static constexpr int MAX_LIVES = 3;

    Player(int startRow, int startCol, int cellSize, float stepTime = 0.13f);

  
    void handleInput(const Grid& grid, float deltaTime);
    void undo();                         

    int getRow() const { return row; }
    int getCol() const { return col; }
    int getLives() const { return lives; }

    void loseLife() { lives--; }
    void resetLives() { lives = MAX_LIVES; }
    void reset(int startRow, int startCol);   

    void render() const;
};

#endif
