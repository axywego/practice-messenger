#pragma once

#include <string>
#include <cstdint>

struct LastMessage {
    std::string peer_login;
    std::string message;
    int64_t timestamp;
};