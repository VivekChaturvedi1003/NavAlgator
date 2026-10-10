#include "Player.h"

Player::Player(int startRow, int startCol, int cellSize) {
    row = startRow;
    col = startCol;
    lives = 3;
    this->cellSize = cellSize;
}

void Player::handleInput(const Grid& grid) {
    int newRow = row;
    int newCol = col;

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))    newRow--;
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))  newRow++;
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))  newCol--;
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) newCol++;

    if ((newRow != row || newCol != col) && grid.isWalkable(newRow, newCol)) {
        moveHistory.push({row, col});   // save current position before moving, for Undo
        row = newRow;
        col = newCol;
    }

    if (IsKeyPressed(KEY_U)) {
        undo();
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
}

void Player::render() const {
    int x = col * cellSize + cellSize / 2;
    int y = row * cellSize + cellSize / 2;
    DrawCircle(x, y, cellSize / 2.5f, BLUE);
}
