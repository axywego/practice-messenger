#pragma once

#include "../database.hpp"
#include "../codes.hpp"
#include <expected>
#include <vector>

inline std::pair<int64_t, int64_t> normalize(int64_t a, int64_t b) {
    return a < b ? std::make_pair(a, b) : std::make_pair(b, a);
}

class FriendRepository {
private:
    std::mutex mtx;
    
    FriendRepository() = default;
    ~FriendRepository() = default;
    FriendRepository(const FriendRepository&) = delete;
    FriendRepository& operator=(const FriendRepository&) = delete;
public:
    
    static FriendRepository& getInstance() {
        static FriendRepository instance;
        return instance;
    }

    ErrorCode sendRequest(int64_t from, int64_t to) {
        if(from == to) return Error::Friends::RequestToYourself;

        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();

        const auto& [u1, u2] = normalize(from, to);

        SQLite::Statement check(db, "SELECT status FROM friendships WHERE user1 = ? AND user2 = ?");
        check.bind(1, u1);
        check.bind(2, u2);

        if(check.executeStep()) {
            std::string status = check.getColumn(0).getText();
            if(status == "pending") return Error::Friends::AlreadySent;
            else if(status == "accepted") return Error::Friends::AlreadyFriends;
        }

        SQLite::Statement ins(db, "INSERT INTO friendships (user1, user2, status, initiator) VALUES (?, ?, ?, ?)");
        ins.bind(1, u1);
        ins.bind(2, u2);
        ins.bind(3, "pending");
        ins.bind(4, from);
        ins.exec();

        return Error::Base::OK;
    }

    ErrorCode acceptRequest(int64_t acceptor, int64_t requester) {
        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();

        const auto& [u1, u2] = normalize(acceptor, requester);

        SQLite::Statement check(db, "SELECT initiator FROM friendships WHERE user1 = ? AND user2 = ? AND status = 'pending'");
        check.bind(1, u1);
        check.bind(2, u2);

        if(!check.executeStep()) {
            return Error::Friends::AcceptNonExistentRequest;
        }
        int64_t initiator = check.getColumn(0).getInt64();
        if(initiator == acceptor) return Error::Friends::AcceptRequestToYourself;

        SQLite::Statement upd(db, "UPDATE friendships SET status = 'accepted' WHERE user1 = ? AND user2 = ?");
        upd.bind(1, u1);
        upd.bind(2, u2);
        upd.exec();

        return Error::Base::OK;
    }

    ErrorCode rejectRequest(int64_t rejecter, int64_t requester) {
        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();

        const auto& [u1, u2] = normalize(rejecter, requester);

        SQLite::Statement check(db, "SELECT initiator FROM friendships WHERE user1 = ? AND user2 = ? AND status = 'pending'");
        check.bind(1, u1);
        check.bind(2, u2);

        if(!check.executeStep()) {
            return Error::Friends::RejectNonExistentRequest;
        }
        int64_t initiator = check.getColumn(0).getInt64();
        if(initiator == rejecter) return Error::Friends::RejectRequestToYourself;

        SQLite::Statement del(db, "DELETE FROM friendships WHERE user1 = ? AND user2 = ?");
        del.bind(1, u1);
        del.bind(2, u2);
        del.exec();

        return Error::Base::OK;
    }

    std::vector<int64_t> getFriends(int64_t id) {
        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();
        
        SQLite::Statement stmt(db, R"(
            SELECT CASE WHEN user1 = ? THEN user2 ELSE user1 END AS friend
            FROM friendships WHERE (user1 = ? OR user2 = ?) AND status = 'accepted'
        )");
        stmt.bind(1, id);
        stmt.bind(2, id);
        stmt.bind(3, id);

        std::vector<int64_t> friends;
        while(stmt.executeStep()) {
            friends.push_back(stmt.getColumn(0).getInt64());
        }
        return friends;
    }

    std::vector<int64_t> getPendingIncoming(int64_t id) {
        std::lock_guard lock(mtx);
        
        auto& db = Database::getInstance().get_db();

        SQLite::Statement stmt(db, R"(
            SELECT initiator FROM friendships WHERE (user1 = ? OR user2 = ?) AND status = 'pending' AND initiator != ?    
        )");

        stmt.bind(1, id);
        stmt.bind(2, id);
        stmt.bind(3, id);
        
        std::vector<int64_t> requests;
        while(stmt.executeStep()) {
            requests.push_back(stmt.getColumn(0).getInt64());
        }
        return requests;
    }

    ErrorCode areFriends(int64_t user1, int64_t user2) {
        std::lock_guard lock(mtx);
        auto& db = Database::getInstance().get_db();

        const auto& [u1, u2] = normalize(user1, user2);
        
        SQLite::Statement check(db, "SELECT 1 FROM friendships WHERE user1 = ? AND user2 = ? AND status = 'accepted'");
        check.bind(1, u1);
        check.bind(2, u2);

        return check.executeStep() ? Error::Base::OK : Error::Friends::NotFriends;
    }
};