#ifndef GRID_H
#define GRID_H

#include "raylib.h"

enum CellType {
    PATH = 0,
    WALL = 1,
    OBSTACLE = 2,
    EXIT = 3,
    SLOW = 4,          
    POWER_SPEED = 5,   
    POWER_SHIELD = 6   
};

class Grid {
private:
    int** cells;      
    int rows, cols;
    int cellSize;     
        void allocate();  
    void release();   
    bool isFullyConnected(int fromRow, int fromCol) const;   
public:
    static constexpr int MUD_COST = 3;   

    Grid(int rows, int cols, int cellSize = 40);
    ~Grid();

    void resize(int newRows, int newCols);

    void loadHardcodedMaze();                       
    void generateMaze(int extraOpeningsPercent);

    
    void placeObstacles(int count, int safeRow, int safeCol);   
    void placeMud(int percentOfOpenCells, int safeRow, int safeCol);
    void placePowerUps(int count, int safeRow, int safeCol);

    
    int moveCost(int r, int c) const;

    
    int** distancesFrom(int startRow, int startCol) const;
    static void freeDistances(int** distances, int rows);

    bool isWalkable(int r, int c) const;
    int getCellType(int r, int c) const;
    void setCellType(int r, int c, int type);

    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int getCellSize() const { return cellSize; }

    void render() const;                           
};

#endif
