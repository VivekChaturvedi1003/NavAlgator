#include "Enemy.h"
#include "DataStructures.h"

Enemy::Enemy(int startRow, int startCol, int cellSize, float moveInterval) {
    row = startRow;
    col = startCol;
    this->cellSize = cellSize;
    this->moveInterval = moveInterval;
    moveTimer = 0.0f;
}

void Enemy::reset(int startRow, int startCol) {
    row = startRow;
    col = startCol;
    moveTimer = 0.0f;
}

// ---------------------------------------------------------------
// BFS Module
// ---------------------------------------------------------------
// Returns the very next step the enemy should take to move one cell
// closer to the player, using BFS on the grid (unweighted, so BFS
// guarantees the shortest path). Recalculated fresh every call -
// this is what lets the enemy react to the player moving.
// ---------------------------------------------------------------
static bool bfsNextStep(const Grid& grid, int startRow, int startCol,
                         int targetRow, int targetCol, int& outRow, int& outCol) {
    int rows = grid.getRows();
    int cols = grid.getCols();

    bool** visited = new bool*[rows];
    int** parentRow = new int*[rows];
    int** parentCol = new int*[rows];
    for (int i = 0; i < rows; i++) {
        visited[i] = new bool[cols];
        parentRow[i] = new int[cols];
        parentCol[i] = new int[cols];
        for (int j = 0; j < cols; j++) {
            visited[i][j] = false;
            parentRow[i][j] = -1;
            parentCol[i][j] = -1;
        }
    }

    ManualQueue<Position> queue;
    queue.push({startRow, startCol});
    visited[startRow][startCol] = true;

    bool found = false;
    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};

    while (!queue.isEmpty() && !found) {
        Position current = queue.pop();

        if (current.row == targetRow && current.col == targetCol) {
            found = true;
            break;
        }

        for (int d = 0; d < 4; d++) {
            int nr = current.row + dr[d];
            int nc = current.col + dc[d];

            if (grid.isWalkable(nr, nc) && !visited[nr][nc]) {
                visited[nr][nc] = true;
                parentRow[nr][nc] = current.row;
                parentCol[nr][nc] = current.col;
                queue.push({nr, nc});
            }
        }
    }

    bool result = false;
    if (found) {
        // Walk backward from target to start to find the first step taken.
        int r = targetRow, c = targetCol;
        int prevR = r, prevC = c;
        while (!(r == startRow && c == startCol)) {
            prevR = r;
            prevC = c;
            int pr = parentRow[r][c];
            int pc = parentCol[r][c];
            r = pr;
            c = pc;
        }
        outRow = prevR;
        outCol = prevC;
        result = true;
    }
    // If !found: no path exists (e.g. player fully walled off).
    // Caller must handle this - the enemy simply won't move this tick.

    for (int i = 0; i < rows; i++) {
        delete[] visited[i];
        delete[] parentRow[i];
        delete[] parentCol[i];
    }
    delete[] visited;
    delete[] parentRow;
    delete[] parentCol;

    return result;
}

void Enemy::update(const Grid& grid, const Player& player, float deltaTime) {
    moveTimer += deltaTime;
    if (moveTimer < moveInterval) return;   // difficulty control: don't recalc every single frame
    moveTimer = 0.0f;

    int nextRow, nextCol;
    bool pathFound = bfsNextStep(grid, row, col, player.getRow(), player.getCol(), nextRow, nextCol);

    if (pathFound) {
        row = nextRow;
        col = nextCol;
    }
    // if no path found, enemy holds its position this tick - handled gracefully, no crash
}

void Enemy::render() const {
    int x = col * cellSize + cellSize / 2;
    int y = row * cellSize + cellSize / 2;
    DrawCircle(x, y, cellSize / 2.5f, RED);
}
