#ifndef DIFFICULTY_H
#define DIFFICULTY_H

struct DifficultySettings {
    const char* name;
    const char* enemySpeedLabel;
    int   rows;                  // must be odd (the maze generator carves on odd cells)
    int   cols;                  // must be odd
    float enemyStepTime;         // seconds per enemy move (smaller = faster enemy)
    float graceTime;             // seconds the enemy stays frozen at the start of a round
    int   minSpawnDistance;      // minimum BFS steps between enemy spawn and player
    int   extraOpeningsPercent;  // % of removable walls opened up to create loops
    float timeLimit;             // round time in seconds
    int   scoreMultiplier;
};

inline const DifficultySettings& getDifficulty(int index) {
    static const DifficultySettings table[3] = {
        //  name      speed     rows cols  enemy  grace  spawn  loops  time   score
        { "Easy",   "Slow",     15,  21,   0.25f, 2.0f,  14,    22,   60.0f,  1 },
        { "Medium", "Normal",   19,  29,   0.20f, 1.5f,  18,    16,   75.0f,  2 },
        { "Hard",   "Fast",     23,  35,   0.16f, 1.0f,  22,    10,   90.0f,  3 }
    };
    return table[index];
}

#endif
