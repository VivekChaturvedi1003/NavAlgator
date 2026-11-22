#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "Grid.h"
#include "DataStructures.h"

struct Position {
    int row, col;
};

class Player {
private:
    int row, col;
    int lives;
    int cellSize;
    float stepTime;      
    float moveTimer;     
    float speedTimer;    
    float shieldTimer;   
    ManualStack<Position> moveHistory;  

public:
    static constexpr int MAX_LIVES = 3;
    static constexpr float SPEED_BOOST_DURATION = 5.0f;
    static constexpr float SHIELD_DURATION = 6.0f;

    Player(int startRow, int startCol, int cellSize, float stepTime = 0.13f);

    
    void handleInput(const Grid& grid, float deltaTime);
    void undo();                         

    int getRow() const { return row; }
    int getCol() const { return col; }
    int getLives() const { return lives; }

    void loseLife() { lives--; }
    void resetLives() { lives = MAX_LIVES; }
    void reset(int startRow, int startCol);  

    
    void activateSpeed() { speedTimer = SPEED_BOOST_DURATION; }
    void activateShield() { shieldTimer = SHIELD_DURATION; }
    bool isSpeedBoosted() const { return speedTimer > 0.0f; }
    bool isImmune() const { return shieldTimer > 0.0f; }
    float speedTimeLeft() const { return speedTimer; }
    float shieldTimeLeft() const { return shieldTimer; }

    void render() const;
};

#endif
