#pragma once

#include <cstdint>
#include <format>

struct ErrorCode {
    uint32_t value{};

    constexpr ErrorCode() : value(1) {}

    constexpr explicit ErrorCode(uint32_t v) noexcept : value(v) {}

    constexpr ErrorCode(int v) : value(v) {}

    constexpr ErrorCode operator=(int new_value) noexcept {
        value = new_value;
        return *this;
    }

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
        return value == 1;
    }

    constexpr void clear() noexcept {
        value = 1;
    }

    constexpr bool operator==(const ErrorCode&) const = default;

    std::string toString() const;
};

namespace ErrorCategory {
    inline constexpr uint32_t Base = 0x0000'0000;
    inline constexpr uint32_t Auth = 0x1000'0000;
    inline constexpr uint32_t Message = 0x2000'0000;
    inline constexpr uint32_t Friends = 0x3000'0000;
    inline constexpr uint32_t User = 0x4000'0000;
}

constexpr ErrorCode makeCode(uint32_t c, uint32_t n) noexcept {
    return ErrorCode {c | n};
}

namespace Error {
    namespace Base {
        inline constexpr ErrorCode Unknown = 0;
        inline constexpr ErrorCode OK = 1;
    }
    
    namespace Auth {
        inline constexpr ErrorCode LoginExists = makeCode(ErrorCategory::Auth, 1);
        inline constexpr ErrorCode IncorrectCredentials = makeCode(ErrorCategory::Auth, 2);
        inline constexpr ErrorCode NonAuthorized = makeCode(ErrorCategory::Auth, 3);
    }

    namespace Message {

    }

    namespace Friends {
        inline constexpr ErrorCode AlreadyFriends = makeCode(ErrorCategory::Friends, 1);
        inline constexpr ErrorCode AlreadySent = makeCode(ErrorCategory::Friends, 2);
        inline constexpr ErrorCode RequestToYourself = makeCode(ErrorCategory::Friends, 3);
        inline constexpr ErrorCode AcceptNonExistentRequest = makeCode(ErrorCategory::Friends, 4);
        inline constexpr ErrorCode AcceptRequestToYourself = makeCode(ErrorCategory::Friends, 5);
        inline constexpr ErrorCode RejectNonExistentRequest = makeCode(ErrorCategory::Friends, 6);
        inline constexpr ErrorCode RejectRequestToYourself = makeCode(ErrorCategory::Friends, 7);
        inline constexpr ErrorCode NotFriends = makeCode(ErrorCategory::Friends, 8);
    }

    namespace User {
        inline constexpr ErrorCode NotFound = makeCode(ErrorCategory::User, 1);
    }
}

inline constexpr std::string_view categoryName(uint32_t c) noexcept {
    switch (c) {
        case ErrorCategory::Base: return "Base";
        case ErrorCategory::Auth:    return "Auth";
        case ErrorCategory::Message: return "Message";
        case ErrorCategory::Friends: return "Friends";
        case ErrorCategory::User: return "User";
        default:              return "Unknown";
    }
}

inline constexpr std::string_view codeName(ErrorCode e) noexcept {
    switch (e.value) {
        case Error::Base::OK: return "OK";
        case Error::Base::Unknown: return "Unknown";

        case Error::Auth::LoginExists:          return "LoginExists";
        case Error::Auth::IncorrectCredentials: return "IncorrectCredentials";
        case Error::Auth::NonAuthorized: return "NonAuthorized";
        
        case Error::Friends::AlreadyFriends: return "AlreadyFriends";
        case Error::Friends::AlreadySent: return "AlreadySent";
        case Error::Friends::RequestToYourself: return "RequestToYourself";
        case Error::Friends::AcceptNonExistentRequest: return "AcceptNonExistentRequest";
        case Error::Friends::AcceptRequestToYourself: return "AcceptRequestToYourself";
        case Error::Friends::RejectNonExistentRequest: return "RejectNonExistentRequest";
        case Error::Friends::RejectRequestToYourself: return "RejectRequestToYourself";
        case Error::Friends::NotFriends: return "NotFriends";

        case Error::User::NotFound: return "NotFound";

        default:                              return "Unknown";
    }
}

inline std::string ErrorCode::toString() const {
    return std::format("{}.{}", categoryName(category()), codeName(*this));
}