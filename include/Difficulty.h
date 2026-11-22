#ifndef DIFFICULTY_H
#define DIFFICULTY_H

struct DifficultySettings {
    const char* name;
    const char* enemySpeedLabel;
    int   rows;                  
    int   cols;                  
    float enemyStepTime;         
    float graceTime;             
    int   minSpawnDistance;      
    int   extraOpeningsPercent;  
    float timeLimit;            
    int   scoreMultiplier;
    int   obstacleCount;         
    int   mudPercent;           
    int   powerUpCount;          
    float bombInterval;          
};

inline const DifficultySettings& getDifficulty(int index) {
    static const DifficultySettings table[3] = {
        { "Easy",   "Slow",     15,  21,   0.25f, 2.0f,  14,    22,   60.0f,  1,     4,     4,   6,      5.0f },
        { "Medium", "Normal",   19,  29,   0.20f, 1.5f,  18,    16,   75.0f,  2,     8,     6,   5,      4.0f },
        { "Hard",   "Fast",     23,  35,   0.16f, 1.0f,  22,    10,   90.0f,  3,     12,    8,   4,      3.0f }
    };
    return table[index];
}

#endif
