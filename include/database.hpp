#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <mutex>

class Database {
private:
    std::unique_ptr<SQLite::Database> db;
    std::mutex mtx;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
public:
    Database() {
        db = std::make_unique<SQLite::Database>("messenger.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        // запись в отдельный лог, а чтение работает параллельно
        db->exec("PRAGMA journal_mode=WAL");
        db->exec("PRAGMA foreign_keys=ON");

        create_tables();
    }
    ~Database() = default;

    static Database& getInstance() {
        static Database inst;
        return inst;
    }

    SQLite::Database& get_db() {
        return *db;
    }

    std::mutex& get_mutex() {
        return mtx;
    }

    void create_tables() {
        db->exec(R"(
            CREATE TABLE IF NOT EXISTS users(
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                login TEXT UNIQUE NOT NULL,
                salt TEXT NOT NULL,
                password TEXT NOT NULL
            );
        )");

        db->exec(R"(
            CREATE TABLE IF NOT EXISTS friendships(
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user1 INTEGER NOT NULL,
                user2 INTEGER NOT NULL,
                status TEXT NOT NULL DEFAULT 'pending',
                initiator INTEGER NOT NULL,
                FOREIGN KEY(user1) REFERENCES users(id),
                FOREIGN KEY(user2) REFERENCES users(id)
            );
        )");

        db->exec(R"(
            CREATE TABLE IF NOT EXISTS direct_messages(
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                sender INTEGER NOT NULL,
                recipient INTEGER NOT NULL,
                create_at INTEGER NOT NULL,
                delivered INTEGER NOT NULL,
                FOREIGN KEY(sender) REFERENCES users(id),
                FOREIGN KEY(recipient) REFERENCES users(id)
            );
        )");

        db->exec(R"(
            CREATE INDEX IF NOT EXISTS idx_undelivered ON direct_messages (recipient, delivered);
        )");
    }
};