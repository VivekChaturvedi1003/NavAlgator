#include "GameEngine.h"
#include "DataStructures.h"
#include "raylib.h"
#include <cstdlib>
#include <ctime>

GameEngine::GameEngine(int rows, int cols, int cellSize, float roundTimeLimit)
    : grid(rows, cols, cellSize),
      player(1, 1, cellSize),
      enemy(1, 1, cellSize, 0.3f),
      scoreboard()
{
    this->roundTimeLimit = roundTimeLimit;
    roundTimer = roundTimeLimit;
    state = PLAYING;
    startRow = 1; startCol = 1;
    enemyStartRow = 1; enemyStartCol = 1;

    grid.loadHardcodedMaze();
    srand((unsigned int)time(nullptr));
    placeEntitiesWithMinDistance(4);   // fair spawning - see below
}

// ---------------------------------------------------------------
// Distance-validated spawning: reuses BFS (as a distance calculator)
// to make sure the enemy doesn't spawn right next to the player.
// ---------------------------------------------------------------
static int bfsDistance(const Grid& grid, int r1, int c1, int r2, int c2) {
    int rows = grid.getRows();
    int cols = grid.getCols();

    bool** visited = new bool*[rows];
    int** dist = new int*[rows];
    for (int i = 0; i < rows; i++) {
        visited[i] = new bool[cols];
        dist[i] = new int[cols];
        for (int j = 0; j < cols; j++) {
            visited[i][j] = false;
            dist[i][j] = -1;
        }
    }

    ManualQueue<Position> queue;
    queue.push({r1, c1});
    visited[r1][c1] = true;
    dist[r1][c1] = 0;

    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};
    int result = -1;

    while (!queue.isEmpty()) {
        Position cur = queue.pop();
        if (cur.row == r2 && cur.col == c2) {
            result = dist[cur.row][cur.col];
            break;
        }
        for (int d = 0; d < 4; d++) {
            int nr = cur.row + dr[d];
            int nc = cur.col + dc[d];
            if (grid.isWalkable(nr, nc) && !visited[nr][nc]) {
                visited[nr][nc] = true;
                dist[nr][nc] = dist[cur.row][cur.col] + 1;
                queue.push({nr, nc});
            }
        }
    }

    for (int i = 0; i < rows; i++) { delete[] visited[i]; delete[] dist[i]; }
    delete[] visited;
    delete[] dist;
    return result;
}

void GameEngine::placeEntitiesWithMinDistance(int minDistance) {
    int rows = grid.getRows();
    int cols = grid.getCols();

    // Player always starts at a fixed known-walkable cell for simplicity.
    startRow = 1; startCol = 1;
    while (!grid.isWalkable(startRow, startCol)) startRow++;

    // Try random walkable cells for the enemy until one is far enough away.
    int attempts = 0;
    do {
        enemyStartRow = rand() % rows;
        enemyStartCol = rand() % cols;
        attempts++;
    } while ((!grid.isWalkable(enemyStartRow, enemyStartCol) ||
              bfsDistance(grid, startRow, startCol, enemyStartRow, enemyStartCol) < minDistance)
             && attempts < 200);

    player.reset(startRow, startCol);
    enemy.reset(enemyStartRow, enemyStartCol);
}

void GameEngine::resetRound() {
    state = PLAYING;
    roundTimer = roundTimeLimit;
    placeEntitiesWithMinDistance(4);
}

void GameEngine::checkWinLose() {
    // Win: player reached the exit cell
    if (grid.getCellType(player.getRow(), player.getCol()) == EXIT) {
        state = WON;
        int scoreEarned = (int)(roundTimer * 10);   // faster escape = higher score
        scoreboard.addScore("Player", scoreEarned);
    }

    // Lose: enemy caught the player
    if (player.getRow() == enemy.getRow() && player.getCol() == enemy.getCol()) {
        player.loseLife();
        if (player.getLives() <= 0) {
            state = LOST;
            scoreboard.addScore("Player", 0);
        } else {
            placeEntitiesWithMinDistance(4);   // survived a hit - respawn fairly
        }
    }

    // Timeout
    if (roundTimer <= 0.0f && state == PLAYING) {
        state = LOST;
        scoreboard.addScore("Player", 0);
    }
}

void GameEngine::update(float deltaTime) {
    if (state != PLAYING) {
        if (IsKeyPressed(KEY_R)) resetRound();
        return;
    }

    roundTimer -= deltaTime;
    player.handleInput(grid);
    enemy.update(grid, player, deltaTime);
    checkWinLose();
}

void GameEngine::render() const {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    grid.render();
    player.render();
    enemy.render();

    DrawText(TextFormat("Lives: %d", player.getLives()), 10, grid.getRows() * grid.getCellSize() + 10, 20, BLACK);
    DrawText(TextFormat("Time: %.1f", roundTimer > 0 ? roundTimer : 0.0f), 200, grid.getRows() * grid.getCellSize() + 10, 20, BLACK);

    if (state == WON) {
        DrawText("YOU ESCAPED! Press R to play again", 10, grid.getRows() * grid.getCellSize() + 40, 20, DARKGREEN);
        scoreboard.render(10, grid.getRows() * grid.getCellSize() + 70);
    } else if (state == LOST) {
        DrawText("CAUGHT! Press R to try again", 10, grid.getRows() * grid.getCellSize() + 40, 20, MAROON);
        scoreboard.render(10, grid.getRows() * grid.getCellSize() + 70);
    } else {
        DrawText("WASD/Arrows: move   U: undo", 10, grid.getRows() * grid.getCellSize() + 40, 16, GRAY);
    }

    EndDrawing();
}

void GameEngine::run() {
    int screenWidth = grid.getCols() * grid.getCellSize();
    int screenHeight = grid.getRows() * grid.getCellSize() + 150; // extra space for UI text

    InitWindow(screenWidth, screenHeight, "NavAlgator - AI Maze Escape");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        update(deltaTime);
        render();
    }

    CloseWindow();
}
