#pragma once

#include <vector>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <print>

std::vector<uint8_t> fileToBytes(const std::string& file_name) {
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    auto size = std::filesystem::file_size(file_name);

    std::vector<uint8_t> file_buffer(size);

    file.seekg(0, std::ios::beg);

    file.read(reinterpret_cast<char*>(file_buffer.data()), size);

    return file_buffer;
}

void bytesToFile(const std::string& file_name, const std::vector<uint8_t>& bytes) {
    std::ofstream file(file_name, std::ios::binary);
    file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}