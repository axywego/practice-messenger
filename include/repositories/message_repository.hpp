#pragma once

#include "../database.hpp"
#include "../codes.hpp"
#include "../structs/stored_message.hpp"
#include "../structs/direct_message.hpp"
#include <mutex>
#include <expected>
#include <chrono>
#include <vector>
#include <format>

class MessageRepository {
private:
    std::mutex mtx;

    MessageRepository() = default;
    ~MessageRepository() = default;
    MessageRepository(const MessageRepository&) = delete;
    MessageRepository& operator=(const MessageRepository&) = delete;
public:
    
    static MessageRepository& getInstance() {
        static MessageRepository instance;
        return instance;
    }

    int64_t saveMessage(
        int64_t sender, int64_t recipient, const std::string& message
    ) {
        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();

        const int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); 

        SQLite::Statement ins(db, R"(
            INSERT INTO direct_messages (sender, recipient, create_at, message, delivered) VALUES (?, ?, ?, ?, 0)
        )");
        ins.bind(1, sender);
        ins.bind(2, recipient);
        ins.bind(3, now);
        ins.bind(4, message);
        ins.exec();

        return db.getLastInsertRowid();
    }

    std::vector<StoredMessage> getUndelivered(int64_t recipient) {
        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();

        SQLite::Statement stmt(db, R"(
            SELECT id, sender, create_at, message FROM direct_messages WHERE recipient = ? AND delivered = 0 ORDER BY create_at ASC    
        )");
        stmt.bind(1, recipient);
        
        std::vector<StoredMessage> messages;
        while(stmt.executeStep()) {
            StoredMessage msg;
            msg.id = stmt.getColumn(0).getInt64();
            msg.sender = stmt.getColumn(1).getInt64();
            msg.timestamp = stmt.getColumn(2).getInt64();
            msg.message = stmt.getColumn(3).getText();
            messages.push_back(msg);
        }
        return messages;
    }

    void markDelivered(const std::vector<int64_t>& ids) {
        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();

        std::string placeholders;
        for (size_t i = 0; i < ids.size(); ++i) {
            if (i > 0) placeholders += ',';
            placeholders += '?';
        }

        SQLite::Statement upd(db, std::format("UPDATE direct_messages SET delivered = 1 WHERE id IN ({})", placeholders));
        for(int i = 0; i < ids.size(); ++i) {
            upd.bind(i + 1, ids[i]);
        }
        upd.exec();
    }

    std::vector<DirectMessage> getHistory(int64_t user1, int64_t user2, int64_t limit) {
        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();

        SQLite::Statement stmt(db, R"(
            SELECT id, sender, recipient, create_at, message, delivered FROM direct_messages
            WHERE (sender = ? AND recipient = ?) OR (sender = ? AND recipient = ?) ORDER BY create_at ASC LIMIT ?
        )");

        stmt.bind(1, user1);
        stmt.bind(2, user2);
        stmt.bind(3, user2);
        stmt.bind(4, user1);
        stmt.bind(5, limit);

        std::vector<DirectMessage> history;
        while(stmt.executeStep()) {
            DirectMessage msg;
            msg.id = stmt.getColumn(0).getInt64();
            msg.sender = stmt.getColumn(1).getInt64();
            msg.recipient = stmt.getColumn(2).getInt64();
            msg.timestamp = stmt.getColumn(3).getInt64();
            msg.message = stmt.getColumn(4).getText();
            msg.delivered = stmt.getColumn(5).getInt64() == 0 ? false : true;
            history.push_back(msg);
        }
        return history;
    }
};