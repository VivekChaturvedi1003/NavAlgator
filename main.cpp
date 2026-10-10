#include "GameEngine.h"

int main() {
    // 10 rows x 10 cols, 40px per cell, 60 second round timer
    GameEngine game(10, 10, 40, 60.0f);
    game.run();
    return 0;
}
