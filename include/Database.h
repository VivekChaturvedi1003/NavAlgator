#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include "sqlite3.h"

struct DbScore {
    std::string name;
    int score;
    std::string difficulty;
};

class Database {
private:
    sqlite3* db;
    bool ok;

public:
    Database(const char* filename = "navalgator.db");
    ~Database();

    bool isOpen() const { return ok; }
    bool saveScore(const std::string& player, int score, const char* difficulty);


    int topScores(int limit, DbScore* out) const;


    int countScores() const;
};

#endif
