#pragma once

#include <cstdint>

constexpr unsigned long long operator""_KB(unsigned long long num) {
    return num * 1024;
}

constexpr unsigned long long operator""_MB(unsigned long long num) {
    return num * 1024 * 1024;
}