#include "GameEngine.h"

int main() {
    // 40 px per cell is the base size; the window scales the maze to fit.
    // Maze size, enemy speed and time limit come from the difficulty chosen in the menu.
    GameEngine game(40);
    game.run();
    return 0;
}
