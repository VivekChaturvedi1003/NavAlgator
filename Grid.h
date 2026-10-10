#ifndef GRID_H
#define GRID_H

#include "raylib.h"

enum CellType {
    PATH = 0,
    WALL = 1,
    OBSTACLE = 2,
    EXIT = 3,
    SLOW = 4   // weighted tile - costs more to cross (used if you add Dijkstra later)
};

class Grid {
private:
    int** cells;      // 2D array - the core Data Structures concept for this class
    int rows, cols;
    int cellSize;      // pixel size of each cell, for rendering

public:
    Grid(int rows, int cols, int cellSize = 40);
    ~Grid();

    void loadHardcodedMaze();          // fills the grid with a fixed test layout
    bool isWalkable(int r, int c) const;
    int getCellType(int r, int c) const;
    void setCellType(int r, int c, int type);

    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int getCellSize() const { return cellSize; }

    void render() const;               // draws every cell using raylib
};

#endif
