#include "../include/crypto.hpp"
#include <print>

int main() {
    std::vector<uint8_t> key(32, 0x42);
    std::string original = "HI brodddddddddddddddddddd";
    std::vector<uint8_t> data(original.begin(), original.end());

    auto encrypted = encryptAES(data, key);
    auto decrypted = decryptAES(encrypted, key);

    std::string result(decrypted.begin(), decrypted.end());

    std::println("Оригинал: {}", original);
    std::println("Расшифровка: {}", result);
    std::println("Совпадает: {}", original == result);
}