#include "GameEngine.h"
#include "Difficulty.h"
#include "Theme.h"
#include "raylib.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>

static const float PLAYER_STEP_TIME = 0.13f;  
static const int   HUD_HEIGHT       = 70;
static const int   FOOTER_HEIGHT    = 36;
static const int   MENU_ITEMS       = 3;
static const int   START_WINDOW_W   = 1100;
static const int   START_WINDOW_H   = 740;
static const int   MIN_WINDOW_W     = 760;
static const int   MIN_WINDOW_H     = 620;

static int largestDistance(int** dist, int rows, int cols) {
    int best = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (dist[i][j] > best) best = dist[i][j];
        }
    }
    return best;
}


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

GameEngine::GameEngine(int cellSize, const char* databaseFile)
    : grid(getDifficulty(0).rows, getDifficulty(0).cols, cellSize),
      player(1, 1, cellSize, PLAYER_STEP_TIME),
      enemy(1, 1, cellSize, getDifficulty(0).enemyStepTime),
      bomb(cellSize, getDifficulty(0).bombInterval),
      scoreboard(),
      database(databaseFile)
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
    enteringName = false;
    nameLength = 0;
    nameBuffer[0] = '\0';
    nameEntryDelay = 0.0f;
    pendingScore = 0;
    savedCount = 0;

    srand((unsigned int)time(nullptr));
    loadSavedScores();
}

void GameEngine::loadSavedScores() {
    DbScore rows[50];
    int count = database.topScores(50, rows);
    for (int i = 0; i < count; i++) {
        scoreboard.addScore(rows[i].name + " (" + rows[i].difficulty + ")", rows[i].score);
    }
    savedCount = database.countScores();
}

void GameEngine::startGame() {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    grid.resize(d.rows, d.cols);      
    enemy.setStepTime(d.enemyStepTime);  
    roundTimeLimit = d.timeLimit;
    player.resetLives();
    setupRound();
}

void GameEngine::setupRound() {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    int rows = grid.getRows();
    int cols = grid.getCols();
    grid.generateMaze(d.extraOpeningsPercent);
    startRow = 1;
    startCol = 1;

    grid.placeObstacles(d.obstacleCount, startRow, startCol);
    grid.placeMud(d.mudPercent, startRow, startCol);
    grid.placePowerUps(d.powerUpCount, startRow, startCol);
    int** dist = grid.distancesFrom(startRow, startCol);
    int maxDist = largestDistance(dist, rows, cols);
    int threshold = (maxDist * 85) / 100;
    if (threshold < 1) threshold = 1;

    int exitRow = rows - 2;      
    int exitCol = cols - 2;
    pickRandomCell(grid, dist, threshold, exitRow, exitCol);
    grid.setCellType(exitRow, exitCol, EXIT);
    Grid::freeDistances(dist, rows);
    player.reset(startRow, startCol);
    placeEnemyFarFrom(startRow, startCol);
    graceTimer = d.graceTime;
    enemy.setFrozen(true);
    bomb.setInterval(d.bombInterval);
    bomb.relocate(grid, startRow, startCol);

    roundTimer = roundTimeLimit;
    endMessage = "";
    state = PLAYING;
}

void GameEngine::placeEnemyFarFrom(int fromRow, int fromCol) {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    int rows = grid.getRows();
    int cols = grid.getCols();

    int** dist = grid.distancesFrom(fromRow, fromCol);
    int maxDist = largestDistance(dist, rows, cols);

    int minDist = maxDist / 2;                      
    if (minDist < d.minSpawnDistance) minDist = d.minSpawnDistance;
    if (minDist > maxDist) minDist = maxDist;        
    if (minDist < 1) minDist = 1;

    int enemyRow = fromRow;
    int enemyCol = fromCol;
    if (!pickRandomCell(grid, dist, minDist, enemyRow, enemyCol)) {
        pickRandomCell(grid, dist, 1, enemyRow, enemyCol);  
    }
    Grid::freeDistances(dist, rows);

    enemy.reset(enemyRow, enemyCol);
}

void GameEngine::respawnAfterHit() {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    player.reset(startRow, startCol);
    placeEnemyFarFrom(startRow, startCol);
    bomb.relocate(grid, startRow, startCol);
    graceTimer = d.graceTime;
    enemy.setFrozen(true);
}

void GameEngine::checkWinLose(bool swappedPlaces) {
    const DifficultySettings& d = getDifficulty(difficultyIndex);

    if (grid.getCellType(player.getRow(), player.getCol()) == EXIT) {
        state = WON;
        endMessage = "YOU ESCAPED!";
        pendingScore = ((int)(roundTimer * 10) + player.getLives() * 100) * d.scoreMultiplier;
        enteringName = true;          
        nameLength = 0;
        nameBuffer[0] = '\0';
        nameEntryDelay = 0.4f;
        return;
    }


    bool sameCell = player.getRow() == enemy.getRow() && player.getCol() == enemy.getCol();
    bool caughtByEnemy = graceTimer <= 0.0f && (sameCell || swappedPlaces);
    bool hitByBomb = bomb.isArmed() && bomb.getRow() == player.getRow() && bomb.getCol() == player.getCol();

    if ((caughtByEnemy || hitByBomb) && !player.isImmune()) {
        player.loseLife();
        if (player.getLives() <= 0) {
            state = LOST;
            endMessage = (hitByBomb && !caughtByEnemy) ? "BOOM!" : "CAUGHT!";
        } else {
            respawnAfterHit();
        }
        return;
    }

  
    if (roundTimer <= 0.0f) {
        state = LOST;
        endMessage = "TIME'S UP!";
    }
}
void GameEngine::collectPowerUps() {
    int r = player.getRow();
    int c = player.getCol();
    int type = grid.getCellType(r, c);

    if (type == POWER_SPEED) {
        player.activateSpeed();
        grid.setCellType(r, c, PATH);         
    } else if (type == POWER_SHIELD) {
        player.activateShield();
        grid.setCellType(r, c, PATH);
    }
}

void GameEngine::updateNameEntry(float deltaTime) {
    int typed = GetCharPressed();

    if (nameEntryDelay > 0.0f) {                             
        nameEntryDelay -= deltaTime;
        while (typed > 0) typed = GetCharPressed();
        return;
    }

    while (typed > 0) {
        if (typed >= 32 && typed <= 126 && nameLength < MAX_NAME_LENGTH) {
            nameBuffer[nameLength] = (char)typed;
            nameLength++;
            nameBuffer[nameLength] = '\0';
        }
        typed = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && nameLength > 0) {
        nameLength--;
        nameBuffer[nameLength] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER))  finishNameEntry(true);
    if (IsKeyPressed(KEY_ESCAPE)) finishNameEntry(false);     
}

void GameEngine::finishNameEntry(bool save) {
    enteringName = false;
    if (!save) return;

    const DifficultySettings& d = getDifficulty(difficultyIndex);
    std::string name = (nameLength > 0) ? std::string(nameBuffer) : std::string("Player");

    database.saveScore(name, pendingScore, d.name);                    
    scoreboard.addScore(name + " (" + d.name + ")", pendingScore);      
    savedCount = database.countScores();
}

void GameEngine::updateMenu() {
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) menuSelection = (menuSelection + 1) % MENU_ITEMS;
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) menuSelection = (menuSelection + MENU_ITEMS - 1) % MENU_ITEMS;

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

    if (state == WON && enteringName) {                       
        updateNameEntry(deltaTime);
        return;
    }

    if (state != PLAYING) {                               
        if (IsKeyPressed(KEY_R)) startGame();                 
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_M)) state = MENU;
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        state = MENU;
        return;
    }

    if (graceTimer > 0.0f) {
        graceTimer -= deltaTime;
        if (graceTimer <= 0.0f) enemy.setFrozen(false);
    }

    roundTimer -= deltaTime;

    int oldPlayerRow = player.getRow(), oldPlayerCol = player.getCol();
    int oldEnemyRow  = enemy.getRow(),  oldEnemyCol  = enemy.getCol();

    player.handleInput(grid, deltaTime);
    collectPowerUps();
    bomb.update(grid, player.getRow(), player.getCol(), deltaTime);
    enemy.update(grid, player, deltaTime);

    bool swapped = player.getRow() == oldEnemyRow && player.getCol() == oldEnemyCol &&
                   enemy.getRow() == oldPlayerRow && enemy.getCol() == oldPlayerCol;

    checkWinLose(swapped);
}

void GameEngine::drawHUD(int screenW) const {
    const DifficultySettings& d = getDifficulty(difficultyIndex);
    float t = (float)GetTime();

    DrawRectangle(0, 0, screenW, HUD_HEIGHT, Theme::Panel);
    DrawRectangle(0, HUD_HEIGHT - 3, screenW, 3, Theme::hsv(t * 40.0f, 0.6f, 1.0f));  

    DrawText("NAVAIGATOR", 20, 10, 30, Theme::Gold);
    DrawText(TextFormat("%s mode", d.name), 22, 44, 16, Theme::TextDim);

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

    if (player.isSpeedBoosted()) {
        DrawText(TextFormat("SPEED  %.1f", player.speedTimeLeft()), 352, 14, 18, Theme::PowerSpeed);
    }
    if (player.isImmune()) {
        DrawText(TextFormat("SHIELD  %.1f", player.shieldTimeLeft()), 352, 40, 18, Theme::PowerShield);
    }

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

    if (state == WON && enteringName) {
        const char* scoreText = TextFormat("Score: %d", pendingScore);
        DrawText(scoreText, px + (panelW - MeasureText(scoreText, 28)) / 2, py + 80, 28, Theme::Gold);
        DrawText("Enter your name:", px + 60, py + 135, 20, Theme::TextDim);

        int boxX = px + 60;
        int boxY = py + 165;
        int boxW = panelW - 120;
        int boxH = 46;
        DrawRectangle(boxX, boxY, boxW, boxH, Fade(WHITE, 0.10f));
        DrawRectangleLines(boxX, boxY, boxW, boxH, Theme::Gold);
        DrawText(nameBuffer, boxX + 12, boxY + 8, 28, Theme::TextMain);
        if (((int)(GetTime() * 2.0)) % 2 == 0) {                    
            DrawText("_", boxX + 12 + MeasureText(nameBuffer, 28), boxY + 8, 28, Theme::Gold);
        }

        const char* nameHint = "ENTER: save score     ESC: skip";
        DrawText(nameHint, px + (panelW - MeasureText(nameHint, 18)) / 2, py + panelH - 38, 18, Theme::TextMain);
        return;
    }

    scoreboard.render(px + 40, py + 90);

    const char* hint = "R: play again     ESC: menu";
    int hintW = MeasureText(hint, 18);
    DrawText(hint, px + (panelW - hintW) / 2, py + panelH - 38, 18, Theme::TextMain);
}

void GameEngine::drawGame(int screenW, int screenH) const {
    float worldW = (float)(grid.getCols() * grid.getCellSize());
    float worldH = (float)(grid.getRows() * grid.getCellSize());
    float areaW  = (float)screenW;
    float areaH  = (float)(screenH - HUD_HEIGHT - FOOTER_HEIGHT);

    float zoomX = areaW / worldW;
    float zoomY = areaH / worldH;
    float zoom = zoomX < zoomY ? zoomX : zoomY;

    Camera2D camera = {
        { (areaW - worldW * zoom) / 2.0f, HUD_HEIGHT + (areaH - worldH * zoom) / 2.0f },  
        { 0.0f, 0.0f },                                                                  
        0.0f,                                                                            
        zoom                                                                             
    };

    BeginMode2D(camera);
    grid.render();
    bomb.render();
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

    const char* title = "NAVAIGATOR";
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

    int blocks = 24;
    int blockSize = 18;
    int stripW = blocks * (blockSize + 4);
    int stripX = (screenW - stripW) / 2;
    int stripY = titleY + titleSize + 50;
    for (int k = 0; k < blocks; k++) {
        DrawRectangle(stripX + k * (blockSize + 4), stripY, blockSize, blockSize,
                      Theme::hsv(k * 15.0f + t * 40.0f, 0.55f, 0.9f));
    }


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


    const char* info = TextFormat("Maze %d x %d    |    Enemy speed: %s    |    Time: %.0f s",
                                  d.cols, d.rows, d.enemySpeedLabel, d.timeLimit);
    int infoY = itemY + MENU_ITEMS * itemGap + 6;
    DrawText(info, (screenW - MeasureText(info, 20)) / 2, infoY, 20, Theme::TextDim);

    // scoreboard on the right when there is room
    if (screenW >= 1100) {
        int boardY = (int)(screenH * 0.46f);
        scoreboard.render(screenW - 310, boardY);
        DrawText(TextFormat("%d scores saved in database", savedCount), screenW - 310, boardY + 160, 16, Theme::TextDim);
    }

    const char* legend = "S = speed boost     I = immunity     brown = mud (slow)     bomb = lose a life     crates block the way";
    DrawText(legend, (screenW - MeasureText(legend, 16)) / 2, screenH - 64, 16, Theme::TextMain);

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

void GameEngine::run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);       
    InitWindow(START_WINDOW_W, START_WINDOW_H, "NavAIgator - AI Maze Escape");
    SetWindowMinSize(MIN_WINDOW_W, MIN_WINDOW_H);
    SetExitKey(KEY_NULL);                        
    SetTargetFPS(60);

    while (!quitRequested && !WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        if (deltaTime > 0.05f) deltaTime = 0.05f;    
        update(deltaTime);
        render();
    }

    CloseWindow();
}
