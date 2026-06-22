#pragma once

#include <cstdint>
#include <format>

struct ErrorCode {
    uint32_t value{ 0x0000'0000 };

    constexpr explicit ErrorCode(uint32_t v) noexcept : value(v) {}
    constexpr ErrorCode() = default;

    constexpr operator uint32_t() const noexcept {
        return value;
    }

    constexpr uint32_t category() const noexcept {
        return value & 0xF000'0000;
    }

    constexpr uint32_t code() const noexcept {
        return value & 0x0FFF'FFFF;
    }

    constexpr bool ok() const noexcept {
        return value == 0;
    }

    constexpr void clear() noexcept {
        value = 0;
    }

    constexpr bool operator==(const ErrorCode&) const = default;

    std::string toString() const;
};

namespace ErrorCategory {
    inline constexpr uint32_t Auth = 0x1000'0000;
    inline constexpr uint32_t Message = 0x2000'0000;
}

constexpr ErrorCode makeCode(uint32_t c, uint32_t n) noexcept {
    return ErrorCode {c | n};
}

namespace Error {
    namespace Auth {
        inline constexpr ErrorCode LoginExists = makeCode(ErrorCategory::Auth, 1);
        inline constexpr ErrorCode IncorrectCredentials = makeCode(ErrorCategory::Auth, 2);
    }

    namespace Message {

    }
}

inline constexpr std::string_view categoryName(uint32_t c) noexcept {
    switch (c) {
        case ErrorCategory::Auth:    return "Auth";
        case ErrorCategory::Message: return "Message";
        default:              return "Unknown";
    }
}

inline constexpr std::string_view codeName(ErrorCode e) noexcept {
    switch (e.value) {
        case Error::Auth::LoginExists:          return "LoginExists";
        case Error::Auth::IncorrectCredentials: return "IncorrectCredentials";
        default:                              return "Unknown";
    }
}

inline std::string ErrorCode::toString() const {
    if(ok()) return "OK";
    else return std::format("{}.{}", categoryName(category()), codeName(*this));
}