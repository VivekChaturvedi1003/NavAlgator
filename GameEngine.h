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
    float graceTimer;       
    int startRow, startCol;
    const char* endMessage;

public:
    GameEngine(int cellSize = 40);

    void run();  
private:
    // game flow
    void update(float deltaTime);
    void updateMenu();
    void startGame();                 
    void setupRound();               
    void respawnAfterHit();           
    void placeEnemyFarFrom(int row, int col);
    void checkWinLose(bool swappedPlaces);

    
    void render() const;
    void drawMenu(int screenW, int screenH) const;
    void drawGame(int screenW, int screenH) const;
    void drawHUD(int screenW) const;
    void drawFooter(int screenW, int screenH) const;
    void drawEndOverlay(int screenW, int screenH) const;
};

#endif
