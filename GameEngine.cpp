#include "GameEngine.h"
#include "Difficulty.h"
#include "Theme.h"
#include "raylib.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>

// ---------------------------------------------------------------
// Fixed settings (per-difficulty numbers live in Difficulty.h)
// ---------------------------------------------------------------
static const float PLAYER_STEP_TIME = 0.13f;   // seconds per player step while a key is held
static const int   HUD_HEIGHT       = 70;
static const int   FOOTER_HEIGHT    = 36;
static const int   MENU_ITEMS       = 3;
static const int   START_WINDOW_W   = 1100;
static const int   START_WINDOW_H   = 740;
static const int   MIN_WINDOW_W     = 760;
static const int   MIN_WINDOW_H     = 620;

// ---------------------------------------------------------------
// Small helpers that work on a BFS distance table (see Grid::distancesFrom)
// ---------------------------------------------------------------
static int largestDistance(int** dist, int rows, int cols) {
    int best = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (dist[i][j] > best) best = dist[i][j];
        }
    }
    return best;
}

// Picks a random PATH cell whose BFS distance is at least minDist.
static bool pickRandomCell(const Grid& grid, int** dist, int minDist, int& outRow, int& outCol) {
    int rows = grid.getRows();
    int cols = grid.getCols();

    int candidates = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (grid.getCellType(i, j) == PATH && dist[i][j] >= minDist) candidates++;
        }
    }
    if (candidates == 0) return false;

    int pick = rand() % candidates;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (grid.getCellType(i, j) == PATH && dist[i][j] >= minDist) {
                if (pick == 0) {
                    outRow = i;
                    outCol = j;
                    return true;
                }
                pick--;
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------
// Construction
// ---------------------------------------------------------------
GameEngine::GameEngine(int cellSize)
    : grid(getDifficulty(0).rows, getDifficulty(0).cols, cellSize),
      player(1, 1, cellSize, PLAYER_STEP_TIME),
      enemy(1, 1, cellSize, getDifficulty(0).enemyStepTime),
      scoreboard()
{
    this->cellSize = cellSize;
    state = MENU;
    difficultyIndex = 0;
    menuSelection = 0;
    quitRequested = false;
    roundTimeLimit = getDifficulty(0).timeLimit;
    roundTimer = roundTimeLimit;
    graceTimer = 0.0f;
    startRow = 1;
    startCol = 1;
    endMessage = "";

    srand((unsigned int)time(nullptr));
}

// ---------------------------------------------------------------
// Round setup
// ---------------------------------------------------------------
void GameEngine::startGame() {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    grid.resize(d.rows, d.cols);          // maze size depends on difficulty
    enemy.setStepTime(d.enemyStepTime);   // so does the enemy's speed
    roundTimeLimit = d.timeLimit;
    player.resetLives();
    setupRound();
}

void GameEngine::setupRound() {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    int rows = grid.getRows();
    int cols = grid.getCols();

    // 1. brand new random maze - always fully connected, so no blocked paths
    grid.generateMaze(d.extraOpeningsPercent);
    startRow = 1;
    startCol = 1;

    // 2. exit: a random cell among the farthest 15% from the player's start
    int** dist = grid.distancesFrom(startRow, startCol);
    int maxDist = largestDistance(dist, rows, cols);
    int threshold = (maxDist * 85) / 100;
    if (threshold < 1) threshold = 1;

    int exitRow = rows - 2;       // fallback, only used if nothing qualifies
    int exitCol = cols - 2;
    pickRandomCell(grid, dist, threshold, exitRow, exitCol);
    grid.setCellType(exitRow, exitCol, EXIT);
    Grid::freeDistances(dist, rows);

    // 3. player at the start, enemy far away and frozen for a few seconds
    player.reset(startRow, startCol);
    placeEnemyFarFrom(startRow, startCol);
    graceTimer = d.graceTime;
    enemy.setFrozen(true);

    roundTimer = roundTimeLimit;
    endMessage = "";
    state = PLAYING;
}

// ---------------------------------------------------------------
// Fair spawning: uses BFS distances (not straight-line distance) so the
// enemy is genuinely far away along the maze corridors.
// ---------------------------------------------------------------
void GameEngine::placeEnemyFarFrom(int fromRow, int fromCol) {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    int rows = grid.getRows();
    int cols = grid.getCols();

    int** dist = grid.distancesFrom(fromRow, fromCol);
    int maxDist = largestDistance(dist, rows, cols);

    int minDist = maxDist / 2;                       // at least half of the longest route away
    if (minDist < d.minSpawnDistance) minDist = d.minSpawnDistance;
    if (minDist > maxDist) minDist = maxDist;        // tiny mazes
    if (minDist < 1) minDist = 1;

    int enemyRow = fromRow;
    int enemyCol = fromCol;
    if (!pickRandomCell(grid, dist, minDist, enemyRow, enemyCol)) {
        pickRandomCell(grid, dist, 1, enemyRow, enemyCol);   // last resort
    }
    Grid::freeDistances(dist, rows);

    enemy.reset(enemyRow, enemyCol);
}

void GameEngine::respawnAfterHit() {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    player.reset(startRow, startCol);
    placeEnemyFarFrom(startRow, startCol);
    graceTimer = d.graceTime;
    enemy.setFrozen(true);
}

// ---------------------------------------------------------------
// Win / lose checks
// ---------------------------------------------------------------
void GameEngine::checkWinLose(bool swappedPlaces) {
    const DifficultySettings& d = getDifficulty(difficultyIndex);

    // Win: player reached the exit
    if (grid.getCellType(player.getRow(), player.getCol()) == EXIT) {
        state = WON;
        endMessage = "YOU ESCAPED!";
        int score = ((int)(roundTimer * 10) + player.getLives() * 100) * d.scoreMultiplier;
        std::string name = std::string("Player (") + d.name + ")";
        scoreboard.addScore(name, score);
        return;
    }

    // Caught: same cell, or the two walked straight through each other
    bool sameCell = player.getRow() == enemy.getRow() && player.getCol() == enemy.getCol();
    if (graceTimer <= 0.0f && (sameCell || swappedPlaces)) {
        player.loseLife();
        if (player.getLives() <= 0) {
            state = LOST;
            endMessage = "CAUGHT!";
        } else {
            respawnAfterHit();
        }
        return;
    }

    // Timeout
    if (roundTimer <= 0.0f) {
        state = LOST;
        endMessage = "TIME'S UP!";
    }
}

// ---------------------------------------------------------------
// Updating
// ---------------------------------------------------------------
void GameEngine::updateMenu() {
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) menuSelection = (menuSelection + 1) % MENU_ITEMS;
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) menuSelection = (menuSelection + MENU_ITEMS - 1) % MENU_ITEMS;

    // difficulty can be changed with left/right on its row, or with 1/2/3 anywhere
    if (menuSelection == 1) {
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) difficultyIndex = (difficultyIndex + 1) % 3;
        if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) difficultyIndex = (difficultyIndex + 2) % 3;
    }
    if (IsKeyPressed(KEY_ONE))   difficultyIndex = 0;
    if (IsKeyPressed(KEY_TWO))   difficultyIndex = 1;
    if (IsKeyPressed(KEY_THREE)) difficultyIndex = 2;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (menuSelection == 0)      startGame();
        else if (menuSelection == 1) difficultyIndex = (difficultyIndex + 1) % 3;
        else                         quitRequested = true;
    }

    if (IsKeyPressed(KEY_ESCAPE)) quitRequested = true;
}

void GameEngine::update(float deltaTime) {
    if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

    if (state == MENU) {
        updateMenu();
        return;
    }

    if (state != PLAYING) {                                   // WON or LOST screen
        if (IsKeyPressed(KEY_R)) startGame();                 // same difficulty, new maze
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_M)) state = MENU;
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        state = MENU;
        return;
    }

    // enemy wakes up when the grace period ends
    if (graceTimer > 0.0f) {
        graceTimer -= deltaTime;
        if (graceTimer <= 0.0f) enemy.setFrozen(false);
    }

    roundTimer -= deltaTime;

    int oldPlayerRow = player.getRow(), oldPlayerCol = player.getCol();
    int oldEnemyRow  = enemy.getRow(),  oldEnemyCol  = enemy.getCol();

    player.handleInput(grid, deltaTime);
    enemy.update(grid, player, deltaTime);

    // true if the two swapped cells in this frame (they would pass through each other)
    bool swapped = player.getRow() == oldEnemyRow && player.getCol() == oldEnemyCol &&
                   enemy.getRow() == oldPlayerRow && enemy.getCol() == oldPlayerCol;

    checkWinLose(swapped);
}

// ---------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------
void GameEngine::drawHUD(int screenW) const {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    float t = (float)GetTime();

    DrawRectangle(0, 0, screenW, HUD_HEIGHT, Theme::Panel);
    DrawRectangle(0, HUD_HEIGHT - 3, screenW, 3, Theme::hsv(t * 40.0f, 0.6f, 1.0f));   // colourful accent line

    DrawText("NAVALGATOR", 20, 10, 30, Theme::Gold);
    DrawText(TextFormat("%s mode", d.name), 22, 44, 16, Theme::TextDim);

    // lives: filled circles = lives left, outlines = lives lost
    DrawText("LIVES", 238, 12, 16, Theme::TextDim);
    for (int i = 0; i < Player::MAX_LIVES; i++) {
        int cx = 250 + i * 34;
        if (i < player.getLives()) {
            DrawCircle(cx, 42, 11.0f, Theme::Danger);
            DrawCircleLines(cx, 42, 11.0f, WHITE);
        } else {
            DrawCircleLines(cx, 42, 11.0f, Theme::TextDim);
        }
    }

    // timer bar (turns red in the last quarter)
    float remaining = roundTimer > 0.0f ? roundTimer : 0.0f;
    float fraction = remaining / roundTimeLimit;
    int barW = 240, barH = 16;
    int barX = screenW - barW - 24;
    int barY = 40;
    Color barColor = fraction > 0.25f ? Theme::Exit : Theme::Danger;

    DrawText(TextFormat("TIME  %.0f s", remaining), barX, 12, 20, Theme::TextMain);
    DrawRectangle(barX, barY, barW, barH, Fade(WHITE, 0.12f));
    DrawRectangle(barX, barY, (int)(barW * fraction), barH, barColor);
    DrawRectangleLines(barX, barY, barW, barH, Theme::TextDim);
}

void GameEngine::drawFooter(int screenW, int screenH) const {
    DrawRectangle(0, screenH - FOOTER_HEIGHT, screenW, FOOTER_HEIGHT, Theme::Panel);
    DrawText("WASD / Arrows: move (hold to keep moving)    U: undo    ESC: menu    F11: fullscreen",
             20, screenH - FOOTER_HEIGHT + 10, 16, Theme::TextDim);
}

void GameEngine::drawEndOverlay(int screenW, int screenH) const {
    DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.65f));

    int panelW = 460;
    int panelH = 340;
    int px = (screenW - panelW) / 2;
    int py = (screenH - panelH) / 2;
    Color accent = (state == WON) ? Theme::Exit : Theme::Danger;

    DrawRectangle(px, py, panelW, panelH, Theme::Panel);
    DrawRectangleLines(px, py, panelW, panelH, accent);

    int titleW = MeasureText(endMessage, 38);
    DrawText(endMessage, px + (panelW - titleW) / 2, py + 22, 38, accent);

    scoreboard.render(px + 40, py + 90);

    const char* hint = "R: play again     ESC: menu";
    int hintW = MeasureText(hint, 18);
    DrawText(hint, px + (panelW - hintW) / 2, py + panelH - 38, 18, Theme::TextMain);
}

void GameEngine::drawGame(int screenW, int screenH) const {
    // The maze is drawn in "world" pixels and scaled to fit the window, so it
    // always fills the available space - resize or maximise the window freely.
    float worldW = (float)(grid.getCols() * grid.getCellSize());
    float worldH = (float)(grid.getRows() * grid.getCellSize());
    float areaW  = (float)screenW;
    float areaH  = (float)(screenH - HUD_HEIGHT - FOOTER_HEIGHT);

    float zoomX = areaW / worldW;
    float zoomY = areaH / worldH;
    float zoom = zoomX < zoomY ? zoomX : zoomY;

    Camera2D camera = {
        { (areaW - worldW * zoom) / 2.0f, HUD_HEIGHT + (areaH - worldH * zoom) / 2.0f },  // offset
        { 0.0f, 0.0f },                                                                  // target
        0.0f,                                                                            // rotation
        zoom                                                                             // zoom
    };

    BeginMode2D(camera);
    grid.render();
    enemy.render();
    player.render();
    EndMode2D();

    drawHUD(screenW);
    drawFooter(screenW, screenH);

    if (state == PLAYING && graceTimer > 0.0f) {
        const char* message = TextFormat("GET READY!  Enemy wakes in %.1f", graceTimer);
        int w = MeasureText(message, 22);
        DrawRectangle((screenW - w) / 2 - 14, HUD_HEIGHT + 8, w + 28, 34, Fade(BLACK, 0.65f));
        DrawText(message, (screenW - w) / 2, HUD_HEIGHT + 14, 22, Theme::Gold);
    }

    if (state == WON || state == LOST) {
        drawEndOverlay(screenW, screenH);
    }
}

void GameEngine::drawMenu(int screenW, int screenH) const {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    float t = (float)GetTime();

    // title - every letter has its own moving colour
    const char* title = "NAVALGATOR";
    int titleSize = 68;
    int titleY = (int)(screenH * 0.10f);
    char letter[2] = {0, 0};

    int totalW = 0;
    for (int k = 0; title[k] != '\0'; k++) {
        letter[0] = title[k];
        totalW += MeasureText(letter, titleSize) + 3;
    }
    int x = (screenW - totalW) / 2;
    for (int k = 0; title[k] != '\0'; k++) {
        letter[0] = title[k];
        DrawText(letter, x + 3, titleY + 3, titleSize, Fade(BLACK, 0.5f));
        DrawText(letter, x, titleY, titleSize, Theme::hsv(k * 36.0f + t * 40.0f, 0.65f, 1.0f));
        x += MeasureText(letter, titleSize) + 3;
    }

    const char* subtitle = "AI Maze Escape";
    DrawText(subtitle, (screenW - MeasureText(subtitle, 24)) / 2, titleY + titleSize + 8, 24, Theme::TextDim);

    // a strip of coloured blocks as decoration
    int blocks = 24;
    int blockSize = 18;
    int stripW = blocks * (blockSize + 4);
    int stripX = (screenW - stripW) / 2;
    int stripY = titleY + titleSize + 50;
    for (int k = 0; k < blocks; k++) {
        DrawRectangle(stripX + k * (blockSize + 4), stripY, blockSize, blockSize,
                      Theme::hsv(k * 15.0f + t * 40.0f, 0.55f, 0.9f));
    }

    // menu items
    int itemY = (int)(screenH * 0.46f);
    int itemGap = 58;
    const char* labels[MENU_ITEMS];
    labels[0] = "Play";
    labels[1] = TextFormat("Difficulty:   <  %s  >", d.name);
    labels[2] = "Quit";

    for (int i = 0; i < MENU_ITEMS; i++) {
        int fontSize = 34;
        int w = MeasureText(labels[i], fontSize);
        int itemX = (screenW - w) / 2;
        int y = itemY + i * itemGap;

        if (i == menuSelection) {
            DrawRectangle(screenW / 2 - 230, y - 8, 460, fontSize + 16, Fade(WHITE, 0.08f));
            DrawRectangleLines(screenW / 2 - 230, y - 8, 460, fontSize + 16, Theme::Gold);
            DrawText(labels[i], itemX, y, fontSize, Theme::Gold);
        } else {
            DrawText(labels[i], itemX, y, fontSize, Theme::TextMain);
        }
    }

    // details of the selected difficulty
    const char* info = TextFormat("Maze %d x %d    |    Enemy speed: %s    |    Time: %.0f s",
                                  d.cols, d.rows, d.enemySpeedLabel, d.timeLimit);
    int infoY = itemY + MENU_ITEMS * itemGap + 6;
    DrawText(info, (screenW - MeasureText(info, 20)) / 2, infoY, 20, Theme::TextDim);

    // scoreboard on the right when there is room
    if (screenW >= 1100) {
        scoreboard.render(screenW - 310, (int)(screenH * 0.46f));
    }

    const char* hint = "UP / DOWN: select      LEFT / RIGHT or 1-2-3: difficulty      ENTER: confirm      F11: fullscreen";
    DrawText(hint, (screenW - MeasureText(hint, 16)) / 2, screenH - 36, 16, Theme::TextDim);
}

void GameEngine::render() const {
    BeginDrawing();
    ClearBackground(Theme::Background);

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    if (state == MENU) drawMenu(screenW, screenH);
    else               drawGame(screenW, screenH);

    EndDrawing();
}

// ---------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------
void GameEngine::run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);       // gives the window a maximise button and resizable edges
    InitWindow(START_WINDOW_W, START_WINDOW_H, "NavAlgator - AI Maze Escape");
    SetWindowMinSize(MIN_WINDOW_W, MIN_WINDOW_H);
    SetExitKey(KEY_NULL);                        // ESC now goes back to the menu instead of closing the game
    SetTargetFPS(60);

    while (!quitRequested && !WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        if (deltaTime > 0.05f) deltaTime = 0.05f;    // avoids a huge jump after dragging/resizing the window
        update(deltaTime);
        render();
    }

    CloseWindow();
}
