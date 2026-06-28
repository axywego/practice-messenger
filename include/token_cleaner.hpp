#pragma once

#include <boost/asio.hpp>
#include "repositories/user_repository.hpp"
#include "logger.hpp"

class TokenCleaner {
private:
    boost::asio::steady_timer timer;
    std::chrono::seconds interval;

    void schedule() {
        timer.expires_after(interval);
        timer.async_wait([this] (boost::system::error_code e) {
            if(e) return;

            UserRepository::getInstance().cleanExpiredTokens();
            Logger::getInstance().log("[TokenCleaner]: Истекшие токены очищены.");

            schedule();
        });
    }
public:
    explicit TokenCleaner(boost::asio::io_context& io, std::chrono::seconds new_interval = std::chrono::minutes(5)) 
        : timer(io), interval(new_interval) {
        schedule();
    }

    void stop() {
        timer.cancel();
    }
};