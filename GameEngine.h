#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "Grid.h"
#include "Player.h"
#include "Enemy.h"
#include "Scoreboard.h"

enum GameState {
    MENU,
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
    int cellSize;
    int difficultyIndex;      // 0 = Easy, 1 = Medium, 2 = Hard
    int menuSelection;        // 0 = Play, 1 = Difficulty, 2 = Quit
    bool quitRequested;

    float roundTimer;
    float roundTimeLimit;
    float graceTimer;         // enemy stays frozen until this reaches 0
    int startRow, startCol;
    const char* endMessage;

public:
    GameEngine(int cellSize = 40);

    void run();   // opens the (resizable) window and runs the menu + game loop

private:
    // game flow
    void update(float deltaTime);
    void updateMenu();
    void startGame();                 // applies the chosen difficulty, builds a new maze
    void setupRound();                // new maze, exit, player and enemy placement
    void respawnAfterHit();           // keep the maze, reposition both characters
    void placeEnemyFarFrom(int row, int col);
    void checkWinLose(bool swappedPlaces);

    // drawing
    void render() const;
    void drawMenu(int screenW, int screenH) const;
    void drawGame(int screenW, int screenH) const;
    void drawHUD(int screenW) const;
    void drawFooter(int screenW, int screenH) const;
    void drawEndOverlay(int screenW, int screenH) const;
};

#endif
