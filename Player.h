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
    ManualStack<Position> moveHistory;   // Stack-based Undo feature

public:
    Player(int startRow, int startCol, int cellSize);

    void handleInput(const Grid& grid);  // reads keyboard, moves if the target cell is walkable
    void undo();                         // pops last position off the stack

    int getRow() const { return row; }
    int getCol() const { return col; }
    int getLives() const { return lives; }

    void loseLife() { lives--; }
    void reset(int startRow, int startCol);

    void render() const;
};

#endif
