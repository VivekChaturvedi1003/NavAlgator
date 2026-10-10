#include "Grid.h"

Grid::Grid(int rows, int cols, int cellSize) {
    this->rows = rows;
    this->cols = cols;
    this->cellSize = cellSize;

    // Manually allocate the 2D array (not std::vector<vector<int>>)
    cells = new int*[rows];
    for (int i = 0; i < rows; i++) {
        cells[i] = new int[cols];
        for (int j = 0; j < cols; j++) {
            cells[i][j] = PATH;
        }
    }
}

Grid::~Grid() {
    for (int i = 0; i < rows; i++) {
        delete[] cells[i];
    }
    delete[] cells;
}

void Grid::loadHardcodedMaze() {
    // 1 = wall, 0 = path. Edit this layout or replace with random generation later.
    // Must match the rows x cols you constructed the Grid with.
    int layout[10][10] = {
        {1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,3,1},
        {1,0,1,1,0,1,1,0,1,1},
        {1,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,0,1,0,1,1,1},
        {1,0,0,0,0,1,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,1}
    };

    for (int i = 0; i < rows && i < 10; i++) {
        for (int j = 0; j < cols && j < 10; j++) {
            cells[i][j] = layout[i][j];
        }
    }
}

bool Grid::isWalkable(int r, int c) const {
    if (r < 0 || r >= rows || c < 0 || c >= cols) return false;
    return cells[r][c] != WALL && cells[r][c] != OBSTACLE;
}

int Grid::getCellType(int r, int c) const {
    if (r < 0 || r >= rows || c < 0 || c >= cols) return WALL;
    return cells[r][c];
}

void Grid::setCellType(int r, int c, int type) {
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;
    cells[r][c] = type;
}

void Grid::render() const {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Color color;
            switch (cells[i][j]) {
                case WALL:     color = DARKGRAY; break;
                case OBSTACLE: color = MAROON;   break;
                case EXIT:     color = GREEN;    break;
                case SLOW:     color = BROWN;    break;
                default:       color = RAYWHITE; break; // PATH
            }
            DrawRectangle(j * cellSize, i * cellSize, cellSize - 2, cellSize - 2, color);
        }
    }
}
