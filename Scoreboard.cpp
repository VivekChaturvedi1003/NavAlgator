#include "Scoreboard.h"
#include "raylib.h"

Scoreboard::Scoreboard(int initialCapacity) {
    capacity = initialCapacity;
    entries = new ScoreEntry[capacity];
    count = 0;
}

Scoreboard::~Scoreboard() {
    delete[] entries;
}

void Scoreboard::addScore(const std::string& name, int score) {
    if (count == capacity) {
        // grow the array manually
        int newCapacity = capacity * 2;
        ScoreEntry* newEntries = new ScoreEntry[newCapacity];
        for (int i = 0; i < count; i++) newEntries[i] = entries[i];
        delete[] entries;
        entries = newEntries;
        capacity = newCapacity;
    }
    entries[count] = {name, score};
    count++;
    sortDescending();
}

// Manual insertion sort - highest score first.
// Chosen over std::sort because the point of this feature is to
// demonstrate a sorting algorithm, not call a library function.
void Scoreboard::sortDescending() {
    for (int i = 1; i < count; i++) {
        ScoreEntry key = entries[i];
        int j = i - 1;
        while (j >= 0 && entries[j].score < key.score) {
            entries[j + 1] = entries[j];
            j--;
        }
        entries[j + 1] = key;
    }
}

void Scoreboard::render(int x, int y) const {
    DrawText("SCOREBOARD", x, y, 20, BLACK);
    int shown = count < 5 ? count : 5;   // show top 5
    for (int i = 0; i < shown; i++) {
        std::string line = std::to_string(i + 1) + ". " + entries[i].name + " - " + std::to_string(entries[i].score);
        DrawText(line.c_str(), x, y + 25 + i * 22, 18, DARKGRAY);
    }
}
