#pragma once

#include <unordered_map>
#include <mutex>
#include <memory>
#include <boost/asio.hpp>

#include "../protocol.hpp"

using boost::asio::ip::tcp;

class NetworkSession;

class SessionManager {
private:
    std::mutex mtx;
    std::unordered_map<std::string, std::weak_ptr<NetworkSession>> online_users;

    SessionManager() = default;
    ~SessionManager() = default;

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    
public:
    static SessionManager& getInstance() {
        static SessionManager instance;
        return instance;
    }

    void addUser(const std::string& login, std::shared_ptr<NetworkSession> session) {
        std::lock_guard lock(mtx);
        online_users[login] = session;
    }

    void removeUser(const std::string& login) {
        std::lock_guard lock(mtx);
        online_users.erase(login);
    }

    void broadcast(const std::string& sender_login, const std::vector<uint8_t>& packet_body);
};