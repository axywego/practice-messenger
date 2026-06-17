#pragma once

#include <cstdint>

constexpr unsigned long long operator""_MB(unsigned long long num) {
    return num * 1024;
}