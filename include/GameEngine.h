#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "Grid.h"
#include "Player.h"
#include "Enemy.h"
#include "Bomb.h"
#include "Scoreboard.h"
#include "Database.h"

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
    Bomb bomb;                
    Scoreboard scoreboard;    
    Database database;        

    GameState state;
    int cellSize;
    int difficultyIndex;      
    int menuSelection;        
    bool quitRequested;

    float roundTimer;
    float roundTimeLimit;
    float graceTimer;         
    int startRow, startCol;
    const char* endMessage;

    static constexpr int MAX_NAME_LENGTH = 12;
    bool enteringName;
    char nameBuffer[MAX_NAME_LENGTH + 1];
    int nameLength;
    float nameEntryDelay;     
    int pendingScore;
    int savedCount;           

public:
    GameEngine(int cellSize = 40, const char* databaseFile = "navalgator.db");

    void run();   

private:
    void update(float deltaTime);
    void updateMenu();
    void updateNameEntry(float deltaTime);
    void finishNameEntry(bool save);
    void loadSavedScores();           
    void startGame();                 
    void setupRound();                
    void respawnAfterHit();           
    void placeEnemyFarFrom(int row, int col);
    void collectPowerUps();           
    void checkWinLose(bool swappedPlaces);
    void render() const;
    void drawMenu(int screenW, int screenH) const;
    void drawGame(int screenW, int screenH) const;
    void drawHUD(int screenW) const;
    void drawFooter(int screenW, int screenH) const;
    void drawEndOverlay(int screenW, int screenH) const;
};

#endif
