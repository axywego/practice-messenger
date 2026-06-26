#pragma once

#include <print>
#include "protocol.hpp"
#include "messages.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include "codes.hpp"

inline std::string currentTimeFormatted() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::tm* tm = std::localtime(&time_t_now);
    
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << tm->tm_mday << "-"
        << std::setw(2) << (tm->tm_mon + 1) << "-"
        << (tm->tm_year + 1900) << " "
        << std::setw(2) << tm->tm_hour << ":"
        << std::setw(2) << tm->tm_min << ":"
        << std::setw(2) << tm->tm_sec << ":"
        << std::setw(3) << ms.count();
    
    return oss.str();
}

class Logger {
public:
    static Logger getInstance() {
        static Logger instanse;
        return instanse;
    }

    void log(const std::string& msg) {
        std::println("[{}] : {}", currentTimeFormatted(), msg);
    }

    void logPacket(PacketType type, ErrorCode e, std::string msg = "") {
        std::string color = (e.ok() ? "\x1b[32m" : "\x1b[31m");
        if(msg.empty())
            std::println("{}[{}] : {} = {}\x1b[0m", color, currentTimeFormatted(), packetTypeToString(type), e.toString());
        else 
            std::println("{}[{}] : {} = {}. {}\x1b[0m", color, currentTimeFormatted(), packetTypeToString(type), e.toString(), msg);
    }

    void logSerializable(const std::string& struct_string, const std::string& msg) {
        std::println("\x1b[33m[{}] : {} : {}\x1b[0m", currentTimeFormatted(), struct_string, msg);
    }
};