#pragma once

#include "protocol.hpp"
#include "messages.hpp"

using boost::asio::ip::tcp;

std::vector<uint8_t> handle_register(const std::vector<uint8_t>& body) {
    std::vector<uint8_t> response;
    AuthRequest req = parseAuthRequest(body);

    bool result = UserStorage::getInstance().registerUser(req.login, req.password);
    if (result) {
        std::string token = UserStorage::getInstance().verifyUser(req.login, req.password);
        AuthResponse res {true, token};
        response = makeAuthResponse(res);
    }
    else {
        AuthResponse res {false, ""};
        response = makeAuthResponse(res);
    }

    return response;
}

std::vector<uint8_t> handle_login(const std::vector<uint8_t>& body) {
    std::vector<uint8_t> response;
    AuthRequest req = parseAuthRequest(body);

    std::string token = UserStorage::getInstance().verifyUser(req.login, req.password);
    if (!token.empty()) {
        AuthResponse res {true, token};
        response = makeAuthResponse(res);
    }
    else {
        AuthResponse res {false, ""};
        response = makeAuthResponse(res);
    }

    return response;
}

std::vector<uint8_t> handle_message(const std::vector<uint8_t>& body) {
    std::vector<uint8_t> response;
    ChatRequest req = parseChatRequest(body);

    std::string login = UserStorage::getInstance().getLoginByToken(req.token);
    if(!login.empty()) {
        ChatResponse res{true};
        response = makeChatResponse(res);
        std::println("[{}]: {}", login, req.message);
    }
    else {
        ChatResponse res{false};
        response = makeChatResponse(res);
    }

    return response;
}

std::vector<uint8_t> handle_upload(const std::vector<uint8_t>& body) {
    std::vector<uint8_t> response;
    FileRequest req = parseFileRequest(body);

    std::string login = UserStorage::getInstance().getLoginByToken(req.token);
    if(!login.empty()) {
        auto encrypted = encryptAES(req.file_data, SERVER_KEY);
        bytesToFile(req.file_name, encrypted);

        FileResponse res{true};
        response = makeFileResponse(res);
    }
    else {
        FileResponse res{false};
        response = makeFileResponse(res);
    }

    return response;
}

std::vector<uint8_t> handle_download(const std::vector<uint8_t>& body) {
    std::vector<uint8_t> response;
    DownloadRequest req = parseDownloadRequest(body);

    std::string login = UserStorage::getInstance().getLoginByToken(req.token);

    std::vector<uint8_t> bytes;
    if(!login.empty()) {
        bytes = fileToBytes(req.file_name);
        auto decrypted = decryptAES(bytes, SERVER_KEY);

        DownloadResponse res {true, decrypted};
        response = makeDownloadResponse(res);
    }
    else {
        DownloadResponse res {false, bytes};
        response = makeDownloadResponse(res);
    }

    return response;
}

void handle_client(tcp::socket socket) {
    try {
        while(true) {

            const auto& [req_type, req_body] = recv_packet(socket);

            std::vector<uint8_t> response;

            switch(req_type) {
                case PacketType::REGISTER: {
                    response = handle_register(req_body);
                    break;
                }
                case PacketType::LOGIN: {
                    response = handle_login(req_body);
                    break;
                }
                case PacketType::MESSAGE: {
                    response = handle_message(req_body);
                    break;
                }
                case PacketType::UPLOAD: {
                    response = handle_upload(req_body);
                    break;
                }

                case PacketType::DOWNLOAD: {
                    response = handle_download(req_body);
                    break;
                }

                default: {
                    // такого не может тупо быть
                    break;
                }
            }

            send_packet(socket, PacketType::RESULT, response);
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << '\n';
    }
}