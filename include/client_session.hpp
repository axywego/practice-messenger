#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <print>
#include <memory>
#include <queue>
#include <array>

#include "network_session.hpp"
#include "messages.hpp"
#include "repositories/user_repository.hpp"
#include "crypto.hpp"
#include "cast.hpp"
#include "services/session_manager.hpp"

using boost::asio::ip::tcp;

class ClientSession : public NetworkSession {
private:
    using NetworkSession::NetworkSession;

    std::string current_login;

    std::vector<uint8_t> handle_register(const std::vector<uint8_t>& body, std::string& login, bool& is_auth_success) {
        std::vector<uint8_t> response;
        AuthRequest req = parseAuthRequest(body);

        bool result = UserRepository::getInstance().registerUser(req.login, req.password);
        if (result) {
            std::string token = UserRepository::getInstance().verifyUser(req.login, req.password);

            AuthResponse res {true, token};
            response = makeAuthResponse(res);

            login = req.login;
            is_auth_success = true;
        }
        else {
            AuthResponse res {false, ""};
            response = makeAuthResponse(res);
        }

        return response;
    }

    std::vector<uint8_t> handle_login(const std::vector<uint8_t>& body, std::string& login, bool& is_auth_success) {
        std::println("GET REQUEST ON LOGIN...");
        std::vector<uint8_t> response;
        AuthRequest req = parseAuthRequest(body);

        std::string token = UserRepository::getInstance().verifyUser(req.login, req.password);
        if (!token.empty()) {
            AuthResponse res {true, token};
            response = makeAuthResponse(res);

            login = req.login;
            is_auth_success = true;
        }
        else {
            AuthResponse res {false, ""};
            response = makeAuthResponse(res);
        }
        std::println("RESPONSE WAS CREATED...");

        return response;
    }

    std::vector<uint8_t> handle_message(const std::vector<uint8_t>& body) {
        std::vector<uint8_t> response;
        ChatSendRequest req = parseChatSendRequest(body);

        std::string login = UserRepository::getInstance().getLoginByToken(req.token);
        if(!login.empty()) {
            ChatResponse res{true};
            response = makeChatResponse(res);
            
            std::println("[{}]: {}", login, req.message);

            ChatIncoming incoming{login, req.message};
            SessionManager::getInstance().broadcast(login, makeChatIncoming(incoming));
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

        std::string login = UserRepository::getInstance().getLoginByToken(req.token);
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

        std::string login = UserRepository::getInstance().getLoginByToken(req.token);

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

protected:

    void handle_packet(PacketType packet_type, std::vector<uint8_t> body) override {
        auto self = shared_from_this();
        std::vector<uint8_t> response;

        bool is_auth_success = false;
        std::string login;

        switch(packet_type) {
            case PacketType::REGISTER: {
                response = handle_register(body, login, is_auth_success);
                break;
            }
            case PacketType::LOGIN: {
                response = handle_login(body, login, is_auth_success);
                break;
            }
            case PacketType::MESSAGE: {
                response = handle_message(body);
                break;
            }
            case PacketType::UPLOAD: {
                response = handle_upload(body);
                break;
            }

            case PacketType::DOWNLOAD: {
                response = handle_download(body);
                break;
            }

            default: {
                // такого не может тупо быть
                break;
            }
        }

        if(is_auth_success) {
            std::println("Успешный вход пользователя {}", login);
            current_login = login;
            SessionManager::getInstance().addUser(current_login, self);
            std::println("Добавление пользователя в список онлайн юзеров");
        }

        if(!response.empty()){
            send_packet(PacketType::RESULT, response);
        }
    }

public:

    ~ClientSession() override {
        if(!current_login.empty()){
            SessionManager::getInstance().removeUser(current_login);
            std::println("Клиент {} отключился", current_login);
        }
    }
};