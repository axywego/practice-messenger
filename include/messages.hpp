#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <memory.h>

struct AuthRequest {
    std::string login;
    std::string password;
};

struct ChatSendRequest {
    std::string token;
    std::string message;
};

struct ChatIncoming {
    std::string sender_login;
    std::string message;
};

struct FileRequest {
    std::string token;
    std::string file_name;
    std::vector<uint8_t> file_data;
};

struct DownloadRequest {
    std::string token;
    std::string file_name;
};

struct AuthResponse {
    bool success;
    std::string token;
};

struct ChatResponse {
    bool success;
};

struct FileResponse {
    bool success;
};

struct DownloadResponse {
    bool success;
    std::vector<uint8_t> file_data;
};

// -------------------------UTILS------------------------------------

void pack(std::vector<uint8_t>& body, const std::vector<uint8_t>& data){
    auto length = htonl(static_cast<uint32_t>(data.size()));

    const uint8_t* len_ptr = reinterpret_cast<const uint8_t*>(&length);
    body.insert(body.end(), len_ptr, len_ptr + 4);

    body.insert(body.end(), data.begin(), data.end());
}

std::string unpack(size_t& offset, const std::vector<uint8_t>& body) {
    uint32_t length;
    memcpy(&length, body.data() + offset, 4);

    length = ntohl(length);

    offset += 4;

    std::string value(body.begin() + offset, body.begin() + offset + length);

    offset += value.size();

    return value;
}

std::vector<std::uint8_t> unpackBytes(size_t& offset, const std::vector<uint8_t>& body) {
    uint32_t length;
    memcpy(&length, body.data() + offset, 4);

    length = ntohl(length);

    offset += 4;

    std::vector<uint8_t> value(body.begin() + offset, body.begin() + offset + length);

    offset += value.size();

    return value;
}

std::vector<uint8_t> stringToBytes(const std::string& str){
    return std::vector<uint8_t>(str.begin(), str.end());
}

// -------------------------MAKE REQUESTS----------------------------

std::vector<std::uint8_t> makeAuthRequest(const AuthRequest& req) {
    std::vector<uint8_t> body;

    pack(body, stringToBytes(req.login));
    pack(body, stringToBytes(req.password));

    return body;
}

std::vector<uint8_t> makeChatSendRequest(const ChatSendRequest& req) {
    std::vector<uint8_t> body;

    pack(body, stringToBytes(req.token));
    pack(body, stringToBytes(req.message));

    return body;
}

std::vector<uint8_t> makeChatIncoming(const ChatIncoming& req) {
    std::vector<uint8_t> body;

    pack(body, stringToBytes(req.sender_login));
    pack(body, stringToBytes(req.message));

    return body;
}

std::vector<uint8_t> makeFileRequest(const FileRequest& req) {
    std::vector<uint8_t> body;

    pack(body, stringToBytes(req.token));
    pack(body, stringToBytes(req.file_name));
    pack(body, req.file_data);

    return body;
}

std::vector<uint8_t> makeDownloadRequest(const DownloadRequest& req) {
    std::vector<uint8_t> body;

    pack(body, stringToBytes(req.token));
    pack(body, stringToBytes(req.file_name));

    return body;
}

// -----------------------PARSE REQUESTS-----------------------------

// [4 байта: длина логина][логин][4 байта: длина пароля][пароль]
AuthRequest parseAuthRequest(const std::vector<uint8_t>& body) {
    size_t offset = 0;

    const auto login = unpack(offset, body);
    const auto password = unpack(offset, body);

    return AuthRequest {login, password};
}

// [4 байта: длина токена][токен][4 байта: длина сообщения][сообщение]
ChatSendRequest parseChatSendRequest(const std::vector<uint8_t>& body) {
    size_t offset = 0;

    const auto token = unpack(offset, body);
    const auto message = unpack(offset, body);

    return ChatSendRequest {token, message};
}

// [4 байта: длина логина][логин][4 байта: длина сообщения][сообщение]
ChatIncoming parseChatIncoming(const std::vector<uint8_t>& body) {
    size_t offset = 0;

    const auto sender_login = unpack(offset, body);
    const auto message = unpack(offset, body);

    return ChatIncoming {sender_login, message};
}

// [4 байта: длина токена][токен][4 байта: длина названия][название][4 байта: длина файла][данные файл]
FileRequest parseFileRequest(const std::vector<uint8_t>& body) {
    size_t offset = 0;

    const auto token = unpack(offset, body);
    const auto file_name = unpack(offset, body);
    const auto file_data = unpackBytes(offset, body);

    return FileRequest {token, file_name, file_data};
}

// [4 байта: длина токена][токен][4 байта: длина названия][название]
DownloadRequest parseDownloadRequest(const std::vector<uint8_t>& body) {
    size_t offset = 0;

    const auto token = unpack(offset, body);
    const auto file_name = unpack(offset, body);

    return DownloadRequest {token, file_name};
}

// -------------------------MAKE RESPONSES----------------------------

std::vector<uint8_t> makeAuthResponse(const AuthResponse& res) {
    std::vector<uint8_t> body;

    pack(body, stringToBytes(res.success ? "true" : "false"));
    pack(body, stringToBytes(res.token));

    return body;
}

std::vector<uint8_t> makeChatResponse(const ChatResponse& res) {
    std::vector<uint8_t> body;

    pack(body, stringToBytes(res.success ? "true" : "false"));

    return body;
}

std::vector<uint8_t> makeFileResponse(const FileResponse& res) {
    std::vector<uint8_t> body;

    pack(body, stringToBytes(res.success ? "true" : "false"));

    return body;
}

std::vector<uint8_t> makeDownloadResponse(const DownloadResponse& res) {
    std::vector<uint8_t> body;

    pack(body, stringToBytes(res.success ? "true" : "false"));
    pack(body, res.file_data);

    return body;
}

// -------------------------PARSE RESPONSES---------------------------

AuthResponse parseAuthResponse(const std::vector<uint8_t>& body) {
    size_t offset = 0;

    const auto success = unpack(offset, body);
    const auto token = unpack(offset, body);

    return AuthResponse {success == "true" ? true : false, token};
}

ChatResponse parseChatResponse(const std::vector<uint8_t>& body) {
    size_t offset = 0;

    const auto success = unpack(offset, body);

    return ChatResponse {success == "true" ? true : false};
}

FileResponse parseFileResponse(const std::vector<uint8_t>& body) {
    size_t offset = 0;

    const auto success = unpack(offset, body);

    return FileResponse {success == "true" ? true : false};
}

DownloadResponse parseDownloadResponse(const std::vector<uint8_t>& body) {
    size_t offset = 0;

    const auto success = unpack(offset, body);
    const auto file_data = unpackBytes(offset, body);

    return DownloadResponse {success == "true" ? true : false, file_data};
}