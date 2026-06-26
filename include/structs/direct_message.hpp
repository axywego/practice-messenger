#pragma once

#include <cstdint>
#include <string>

struct DirectMessage {
    int64_t id;
    int64_t sender;
    int64_t recipient;
    int64_t timestamp;
    std::string message;
    bool delivered;
};