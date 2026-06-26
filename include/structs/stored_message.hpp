#pragma once

#include <cstdint>
#include <string>

struct StoredMessage {
    int64_t id;
    int64_t sender;
    int64_t timestamp;
    std::string message;
};