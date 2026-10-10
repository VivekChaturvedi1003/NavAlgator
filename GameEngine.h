#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "Grid.h"
#include "Player.h"
#include "Enemy.h"
#include "Scoreboard.h"

enum GameState {
    PLAYING,
    WON,
    LOST
};

class GameEngine {
private:
    Grid grid;
    Player player;
    Enemy enemy;
    Scoreboard scoreboard;

    GameState state;
    float roundTimer;
    float roundTimeLimit;
    int startRow, startCol;
    int enemyStartRow, enemyStartCol;

public:
    GameEngine(int rows, int cols, int cellSize, float roundTimeLimit = 60.0f);

    void run();   // top-level loop: init window, update+render every frame, close window

private:
    void update(float deltaTime);
    void render() const;
    void checkWinLose();
    void resetRound();
    void placeEntitiesWithMinDistance(int minDistance); // random, distance-validated spawning
};

#endif
