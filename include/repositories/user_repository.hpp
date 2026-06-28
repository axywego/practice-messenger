#pragma once 

#include <mutex>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <expected>
#include <chrono>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/hash2/sha2.hpp>

#include "../database.hpp"

#include "../structs/user.hpp"

#include "../crypto.hpp"

inline constexpr int64_t TOKEN_TTL = 7 * 24 * 60 * 60;

class UserRepository {
private:
    std::mutex mtx;

    boost::uuids::random_generator generator_uuids;
    
    UserRepository() = default;
    ~UserRepository() = default;
    UserRepository(const UserRepository&) = delete;
    UserRepository& operator=(const UserRepository&) = delete;

    bool verifyPassword(const std::string& login, const std::string& password) {
        auto& db = Database::getInstance().get_db();

        SQLite::Statement q(db, "SELECT salt, password FROM users WHERE login = ?");
        q.bind(1, login);
        if(!q.executeStep()) return false;

        std::string salt = q.getColumn(0).getText();
        std::string hash = q.getColumn(1).getText();

        return sha256(password + salt) == hash;
    }

public:

    static UserRepository& getInstance() {
        static UserRepository instance;
        return instance;
    }

    std::string generateSalt(){
        boost::uuids::uuid id = generator_uuids();
        return boost::uuids::to_string(id);
    }

    ErrorCode registerUser(const std::string& login, const std::string& password) {
        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();

        SQLite::Statement check(db, "SELECT 1 FROM users WHERE login = ?");
        check.bind(1, login);
        if(check.executeStep()) return Error::Auth::LoginExists;

        std::string salt = generateSalt();
        std::string hash = sha256(password + salt);

        SQLite::Statement ins(db, "INSERT INTO users (login, salt, password) VALUES (?, ?, ?)");
        ins.bind(1, login);
        ins.bind(2, salt);
        ins.bind(3, hash);
        ins.exec();

        return Error::Base::OK;
    }

    std::expected<std::string, ErrorCode> verifyUser(const std::string& login, const std::string& password) {
        std::lock_guard lock(mtx);

        if(!verifyPassword(login, password)) return std::unexpected(Error::Auth::IncorrectCredentials);

        boost::uuids::uuid uuid = generator_uuids();
        std::string token = boost::uuids::to_string(uuid);

        auto& db = Database::getInstance().get_db();

        int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        SQLite::Statement q(db, "SELECT id FROM users WHERE login = ?");
        q.bind(1, login);

        int64_t id;
        if(q.executeStep())
            id = q.getColumn(0).getInt64();

        SQLite::Statement ins(db, "INSERT INTO sessions (token, id, created_at) VALUES (?, ?, ?)");
        ins.bind(1, token);
        ins.bind(2, id);
        ins.bind(3, now);
        ins.exec();

        return token;
    }

    std::expected<std::string, ErrorCode> getLoginByToken(const std::string& token) {
        std::lock_guard lock(mtx);

        auto& db = Database::getInstance().get_db();

        SQLite::Statement check(db, "SELECT id, created_at FROM sessions WHERE token = ?");
        check.bind(1, token);

        if(!check.executeStep())
            return std::unexpected(Error::Auth::NonAuthorized);
        
        const auto id = check.getColumn(0).getInt64();
        const auto created_at = check.getColumn(1).getInt64();

        int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        if(now - created_at > TOKEN_TTL) {
            SQLite::Statement del(db, "DELETE FROM sessions WHERE token = ?");
            del.bind(1, token);
            del.exec();
            return std::unexpected(Error::Auth::TokenExpired);
        }

        SQLite::Statement q(db, "SELECT login FROM users WHERE id = ?");
        q.bind(1, id);

        if(q.executeStep())
            return q.getColumn(0).getText();

        return std::unexpected(Error::User::NotFound);
    }

    ErrorCode userExists(const std::string& login) {
        std::lock_guard lock(mtx);
        auto& db = Database::getInstance().get_db();

        SQLite::Statement q(db, "SELECT 1 FROM users WHERE login = ?");
        q.bind(1, login);
        return q.executeStep() ? Error::Base::OK : Error::User::NotFound;
    }

    std::expected<int64_t, ErrorCode> getIdByLogin(const std::string& login) {
        std::lock_guard lock(mtx);
        auto& db = Database::getInstance().get_db();

        SQLite::Statement q(db, "SELECT id FROM users WHERE login = ?");
        q.bind(1, login);

        if(q.executeStep())
            return q.getColumn(0).getInt64();
        return std::unexpected(Error::User::NotFound);
    }

    std::expected<std::string, ErrorCode> getLoginById(int64_t id) {
        std::lock_guard lock(mtx);
        auto& db = Database::getInstance().get_db();

        SQLite::Statement q(db, "SELECT login FROM users WHERE id = ?");
        q.bind(1, id);

        if(q.executeStep())
            return q.getColumn(0).getText();
        return std::unexpected(Error::User::NotFound);
    }

    void logout(const std::string& token) {
        std::lock_guard lock(mtx);
        auto& db = Database::getInstance().get_db();
        SQLite::Statement del(db, "DELETE FROM sessions WHERE token = ?");
        del.bind(1, token);
        del.exec();
    }

    void cleanExpiredTokens() {
        std::lock_guard lock(mtx);
        auto& db = Database::getInstance().get_db();

        int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        SQLite::Statement del(db, "DELETE FROM sessions WHERE ? - created_at > ?");
        del.bind(1, now);
        del.bind(2, TOKEN_TTL);
        del.exec();
    }
};