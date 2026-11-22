#include "Database.h"
#include <cstdio>

Database::Database(const char* filename) {
    db = nullptr;
    ok = false;

    if (sqlite3_open(filename, &db) != SQLITE_OK) {
        printf("Database: could not open '%s': %s\n", filename, db ? sqlite3_errmsg(db) : "out of memory");
        if (db) sqlite3_close(db);
        db = nullptr;
        return;
    }

    const char* createTable =
        "CREATE TABLE IF NOT EXISTS scores ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  player     TEXT    NOT NULL,"
        "  score      INTEGER NOT NULL,"
        "  difficulty TEXT    NOT NULL,"
        "  played_at  TEXT    NOT NULL DEFAULT (datetime('now', 'localtime'))"
        ");";

    char* error = nullptr;
    if (sqlite3_exec(db, createTable, nullptr, nullptr, &error) != SQLITE_OK) {
        printf("Database: could not create table: %s\n", error ? error : "unknown error");
        sqlite3_free(error);
        sqlite3_close(db);
        db = nullptr;
        return;
    }
    ok = true;
}

Database::~Database() {
    if (db) sqlite3_close(db);
}

bool Database::saveScore(const std::string& player, int score, const char* difficulty) {
    if (!ok) return false;
    const char* sql = "INSERT INTO scores (player, score, difficulty) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, player.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, score);
    sqlite3_bind_text(stmt, 3, difficulty, -1, SQLITE_TRANSIENT);

    bool saved = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return saved;
}

int Database::topScores(int limit, DbScore* out) const {
    if (!ok || limit <= 0) return 0;

    const char* sql = "SELECT player, score, difficulty FROM scores "
                      "ORDER BY score DESC, id ASC LIMIT ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, limit);

    int count = 0;
    while (count < limit && sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* name = sqlite3_column_text(stmt, 0);
        const unsigned char* diff = sqlite3_column_text(stmt, 2);
        out[count].name = name ? (const char*)name : "";
        out[count].score = sqlite3_column_int(stmt, 1);
        out[count].difficulty = diff ? (const char*)diff : "";
        count++;
    }
    sqlite3_finalize(stmt);
    return count;
}

int Database::countScores() const {
    if (!ok) return 0;

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM scores;", -1, &stmt, nullptr) != SQLITE_OK) return 0;

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
}
