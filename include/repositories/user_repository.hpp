#pragma once 

#include <mutex>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/hash2/sha2.hpp>

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

    std::vector<User> users;

    // token to login
    std::unordered_map<std::string, std::string> sessions;

    boost::uuids::random_generator generator_uuids;
    

    UserRepository() {
        std::ifstream in("users.txt");
        
        std::string line;
        while(std::getline(in, line)){
            std::istringstream iss(line);
            std::string login, salt, password;
            std::getline(iss, login, ':');
            std::getline(iss, salt, ':');
            std::getline(iss, password, ':');

            users.emplace_back(login, salt, password);
        }
    }

    ~UserRepository() = default;

    UserRepository(const UserRepository&) = delete;
    UserRepository& operator=(const UserRepository&) = delete;

    bool isUserInDbUnsafe(const std::string& login) {
        return std::find_if(users.begin(), users.end(), [login](const User& u) { return u.login == login; }) != users.end();
    }


    bool isUserInDbUnsafe(const std::string& login, const std::string& password) {
        auto it = std::find_if(users.begin(), users.end(), 
            [login](const User& u) { 
                return u.login == login;
             });

        if(it != users.end()) {
            return sha256(password + it->salt) == it->password;
        }

        return false;
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

    bool registerUser(const std::string& login, const std::string& password) {
        std::lock_guard lock(mtx);

        if(!isUserInDbUnsafe(login)) {
            std::string salt = generateSalt();
            std::string hash = sha256(password + salt);

            std::string line = std::format("{}:{}:{}\n", login, salt, hash);

            std::ofstream out("users.txt", std::ios::app);
            out << line;

            users.emplace_back(login, salt, hash);

            return true;
        }

        return false;
    }

    std::string verifyUser(const std::string& login, const std::string& password) {
        std::lock_guard lock(mtx);
        if(isUserInDbUnsafe(login, password)){
            boost::uuids::uuid id = generator_uuids();
            std::string token = boost::uuids::to_string(id);

            sessions[token] = login;
            return token;
        }
        return "";
    }

    std::string getLoginByToken(const std::string& token) {
        std::lock_guard lock(mtx);
        auto it = sessions.find(token);
        if(it != sessions.end()) return it->second;
        return "";
    }
};