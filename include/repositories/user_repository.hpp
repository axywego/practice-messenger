#pragma once 

#include <mutex>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <expected>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/hash2/sha2.hpp>

#include "../database.hpp"

#include "../structs/user.hpp"

std::string sha256(const std::string& input) {
    boost::hash2::sha2_256 hash;

    hash.update(input.data(), input.size());

    auto res = hash.result();

    std::string hash_string;
    for(const auto& el : res) {
        hash_string += std::format("{:02x}", el);
    }

    return hash_string;
}

class UserRepository {
private:
    std::mutex mtx;

    // token to login
    std::unordered_map<std::string, std::string> sessions;

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
        if(verifyPassword(login, password)){
            boost::uuids::uuid id = generator_uuids();
            std::string token = boost::uuids::to_string(id);

            sessions[token] = login;
            return token;
        }
        return std::unexpected(Error::Auth::IncorrectCredentials);
    }

    std::expected<std::string, ErrorCode> getLoginByToken(const std::string& token) {
        std::lock_guard lock(mtx);
        auto it = sessions.find(token);
        if(it != sessions.end()) return it->second;
        return std::unexpected(Error::Auth::NonAuthorized);
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
        sessions.erase(token);
    }
};