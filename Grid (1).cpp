#include "Grid.h"
#include "Theme.h"
#include "DataStructures.h"
#include <cstdlib>
#include <cmath>


struct MazeCell {
    int r, c;
};


void Grid::allocate() {
    
    cells = new int*[rows];
    for (int i = 0; i < rows; i++) {
        cells[i] = new int[cols];
        for (int j = 0; j < cols; j++) {
            cells[i][j] = PATH;
        }
    }
}

void Grid::release() {
    for (int i = 0; i < rows; i++) {
        delete[] cells[i];
    }
    delete[] cells;
    cells = nullptr;
}

Grid::Grid(int rows, int cols, int cellSize) {
    this->rows = rows;
    this->cols = cols;
    this->cellSize = cellSize;
    allocate();
}

Grid::~Grid() {
    release();
}

void Grid::resize(int newRows, int newCols) {
    release();
    rows = newRows;
    cols = newCols;
    allocate();
}


void Grid::loadHardcodedMaze() {
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


void Grid::generateMaze(int extraOpeningsPercent) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            cells[i][j] = WALL;
        }
    }

    ManualStack<MazeCell> stack;         
    cells[1][1] = PATH;
    stack.push({1, 1});

    int dr[4] = {-2, 2, 0, 0};
    int dc[4] = {0, 0, -2, 2};

    while (!stack.isEmpty()) {
        MazeCell cur = stack.pop();

       
        int options[4];
        int optionCount = 0;
        for (int d = 0; d < 4; d++) {
            int nr = cur.r + dr[d];
            int nc = cur.c + dc[d];
            if (nr >= 1 && nr <= rows - 2 && nc >= 1 && nc <= cols - 2 && cells[nr][nc] == WALL) {
                options[optionCount++] = d;
            }
        }

        if (optionCount > 0) {
            stack.push(cur);                                  
            int d = options[rand() % optionCount];
            int nr = cur.r + dr[d];
            int nc = cur.c + dc[d];
            cells[cur.r + dr[d] / 2][cur.c + dc[d] / 2] = PATH;   
            cells[nr][nc] = PATH;
            stack.push({nr, nc});
        }
       
    }

   
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            if (cells[i][j] != WALL) continue;

            bool wallBetweenUpAndDown = cells[i - 1][j] == PATH && cells[i + 1][j] == PATH &&
                                        cells[i][j - 1] == WALL && cells[i][j + 1] == WALL;
            bool wallBetweenLeftAndRight = cells[i][j - 1] == PATH && cells[i][j + 1] == PATH &&
                                           cells[i - 1][j] == WALL && cells[i + 1][j] == WALL;

            if ((wallBetweenUpAndDown || wallBetweenLeftAndRight) && rand() % 100 < extraOpeningsPercent) {
                cells[i][j] = PATH;
            }
        }
    }
}


int** Grid::distancesFrom(int startRow, int startCol) const {
    int** dist = new int*[rows];
    for (int i = 0; i < rows; i++) {
        dist[i] = new int[cols];
        for (int j = 0; j < cols; j++) {
            dist[i][j] = -1;
        }
    }

    if (!isWalkable(startRow, startCol)) return dist;

    ManualQueue<MazeCell> queue;         
    queue.push({startRow, startCol});
    dist[startRow][startCol] = 0;

    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};

    while (!queue.isEmpty()) {
        MazeCell cur = queue.pop();
        for (int d = 0; d < 4; d++) {
            int nr = cur.r + dr[d];
            int nc = cur.c + dc[d];
            if (isWalkable(nr, nc) && dist[nr][nc] == -1) {
                dist[nr][nc] = dist[cur.r][cur.c] + 1;
                queue.push({nr, nc});
            }
        }
    }
    return dist;
}

void Grid::freeDistances(int** distances, int rows) {
    for (int i = 0; i < rows; i++) {
        delete[] distances[i];
    }
    delete[] distances;
}

// ---------------------------------------------------------------
// Queries
// ---------------------------------------------------------------
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
   
    DrawRectangle(0, 0, cols * cellSize, rows * cellSize, Theme::FloorB);

    float t = (float)GetTime();
    float pulse = 0.5f + 0.5f * std::sin(t * 3.0f);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int x = j * cellSize;
            int y = i * cellSize;
            int type = cells[i][j];

            if (type == WALL) {
                
                float hue = i * 9.0f + j * 7.0f + t * 12.0f;
                Color face = Theme::hsv(hue, 0.55f, 0.85f);
                Color top  = Theme::hsv(hue, 0.30f, 1.00f);
                Color side = Theme::hsv(hue, 0.70f, 0.55f);

                DrawRectangle(x, y, cellSize, cellSize, side);          // dark base (shadow)
                DrawRectangle(x, y, cellSize, cellSize - 6, face);      // main face
                DrawRectangle(x, y, cellSize, 4, top);                  // lit top edge
                DrawRectangleLines(x, y, cellSize, cellSize, Fade(BLACK, 0.25f));
                continue;
            }

            Color floorColor = ((i + j) % 2 == 0) ? Theme::FloorA : Theme::FloorB;
            DrawRectangle(x, y, cellSize, cellSize, floorColor);

            if (type == EXIT) {
                DrawRectangle(x + 3, y + 3, cellSize - 6, cellSize - 6, Fade(Theme::Exit, 0.30f + 0.35f * pulse));
                DrawRectangle(x + 10, y + 10, cellSize - 20, cellSize - 20, Theme::Exit);
                DrawRectangleLines(x + 3, y + 3, cellSize - 6, cellSize - 6, Theme::Exit);
            } else if (type == OBSTACLE) {
                DrawRectangle(x + 5, y + 5, cellSize - 10, cellSize - 10, Theme::Crate);
                DrawLine(x + 5, y + 5, x + cellSize - 5, y + cellSize - 5, Theme::CrateEdge);
                DrawLine(x + cellSize - 5, y + 5, x + 5, y + cellSize - 5, Theme::CrateEdge);
                DrawRectangleLines(x + 5, y + 5, cellSize - 10, cellSize - 10, Theme::CrateEdge);
            } else if (type == SLOW) {
                DrawRectangle(x, y, cellSize, cellSize, Fade(Theme::Slow, 0.55f));
            }
        }
    }
}
