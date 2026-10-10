#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <string>

struct ScoreEntry {
    std::string name;
    int score;
};

class Scoreboard {
private:
    ScoreEntry* entries;
    int capacity;
    int count;

public:
    Scoreboard(int initialCapacity = 10);
    ~Scoreboard();

    void addScore(const std::string& name, int score);
    void sortDescending();     // manual insertion sort, not std::sort
    void render(int x, int y) const;  // draws top entries on screen via raylib
};

#endif
