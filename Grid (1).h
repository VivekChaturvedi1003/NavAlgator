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
    int cellSize;     // pixel size of each cell, for rendering

    void allocate();  // creates the 2D array for the current rows x cols
    void release();   // frees the 2D array

public:
    Grid(int rows, int cols, int cellSize = 40);
    ~Grid();

    // Changes the maze size (used when a different difficulty is chosen).
    void resize(int newRows, int newCols);

    void loadHardcodedMaze();                       // fixed test layout (10x10)

    // Builds a new random maze. Every open cell is guaranteed to be connected.
    // extraOpeningsPercent: how many extra walls to knock down so the maze has
    // loops (higher = easier to run around the enemy).
    void generateMaze(int extraOpeningsPercent);

    // BFS from (startRow, startCol): returns a rows x cols table of step counts.
    // -1 means "not reachable". Free it with freeDistances().
    int** distancesFrom(int startRow, int startCol) const;
    static void freeDistances(int** distances, int rows);

    bool isWalkable(int r, int c) const;
    int getCellType(int r, int c) const;
    void setCellType(int r, int c, int type);

    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int getCellSize() const { return cellSize; }

    void render() const;                            // draws every cell using raylib
};

#endif
