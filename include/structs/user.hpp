#pragma once

#include <string>

struct User {
    std::string login;
    std::string salt;
    std::string password;
};