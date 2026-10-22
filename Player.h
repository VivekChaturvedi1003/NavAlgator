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
    float stepTime;      // seconds between steps while a key is held
    float moveTimer;     // counts down; the player can step when it reaches 0
    ManualStack<Position> moveHistory;   // Stack-based Undo feature

public:
    static constexpr int MAX_LIVES = 3;

    Player(int startRow, int startCol, int cellSize, float stepTime = 0.13f);

    // Reads the keyboard. Holding a key keeps moving, one cell every stepTime seconds.
    void handleInput(const Grid& grid, float deltaTime);
    void undo();                         // pops last position off the stack

    int getRow() const { return row; }
    int getCol() const { return col; }
    int getLives() const { return lives; }

    void loseLife() { lives--; }
    void resetLives() { lives = MAX_LIVES; }
    void reset(int startRow, int startCol);   // new position, undo history cleared

    void render() const;
};

#endif
