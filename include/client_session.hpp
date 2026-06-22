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
#include "logger.hpp"
#include "codes.hpp"

using boost::asio::ip::tcp;


class ClientSession : public NetworkSession {
private:
    using NetworkSession::NetworkSession;

    std::string current_login;
    
    ErrorCode errorCode;

    std::vector<uint8_t> handle_register(const std::vector<uint8_t>& body, std::string& login, bool& is_auth_success) {
        std::vector<uint8_t> response;

        AuthRequest req = AuthRequest::deserialize(body);

        bool result = UserRepository::getInstance().registerUser(req.login, req.password);

        if (result) {
            std::string token = UserRepository::getInstance().verifyUser(req.login, req.password);

            AuthResponse res {.success = true, .token = token};
            response = res.serialize();

            login = req.login;
            is_auth_success = true;
        }
        else {
            errorCode = Error::Auth::LoginExists;
            AuthResponse res {.success = false, .token = ""};
            response = res.serialize();
        }

        return response;
    }

    std::vector<uint8_t> handle_login(const std::vector<uint8_t>& body, std::string& login, bool& is_auth_success) {
        std::vector<uint8_t> response;
        
        AuthRequest req = AuthRequest::deserialize(body);

        std::string token = UserRepository::getInstance().verifyUser(req.login, req.password);

        if (!token.empty()) {
            AuthResponse res {.success = true, .token = token};
            response = res.serialize();

            login = req.login;
            is_auth_success = true;
        }
        else {
            errorCode = Error::Auth::IncorrectCredentials;
            AuthResponse res {.success = false, .token = ""};
            response = res.serialize();
        }

        return response;
    }

    std::vector<uint8_t> handle_message(const std::vector<uint8_t>& body) {
        std::vector<uint8_t> response;
        ChatSendRequest req = ChatSendRequest::deserialize(body);

        std::string login = UserRepository::getInstance().getLoginByToken(req.token);
        if(!login.empty()) {
            ChatResponse res {.success = true};
            response = res.serialize();
            
            std::println("[{}]: {}", login, req.message);

            ChatIncoming incoming{.sender_login = login, .message = req.message};
            SessionManager::getInstance().broadcast(login, incoming.serialize());
        }
        else {
            ChatResponse res {.success = false};
            response = res.serialize();
        }

        return response;
    }

    std::vector<uint8_t> handle_upload(const std::vector<uint8_t>& body) {
        std::vector<uint8_t> response;
        FileUploadRequest req = FileUploadRequest::deserialize(body);

        std::string login = UserRepository::getInstance().getLoginByToken(req.token);
        if(!login.empty()) {
            auto encrypted = encryptAES(req.file_data, SERVER_KEY);
            bytesToFile(req.file_name, encrypted);

            FileUploadResponse res{.success = true};
            response = res.serialize();
        }
        else {
            FileUploadResponse res{.success = false};
            response = res.serialize();
        }

        return response;
    }

    std::vector<uint8_t> handle_download(const std::vector<uint8_t>& body) {
        std::vector<uint8_t> response;
        FileDownloadRequest req = FileDownloadRequest::deserialize(body);

        std::string login = UserRepository::getInstance().getLoginByToken(req.token);

        std::vector<uint8_t> bytes;
        if(!login.empty()) {
            bytes = fileToBytes(req.file_name);
            auto decrypted = decryptAES(bytes, SERVER_KEY);

            FileDownloadResponse res {.success = true, .file_data = decrypted};
            response = res.serialize();
        }
        else {
            FileDownloadResponse res {.success = false, .file_data = bytes};
            response = res.serialize();
        }

        return response;
    }

protected:

    void handle_packet(PacketType packet_type, std::vector<uint8_t> body) override {
        auto self = shared_from_this();
        std::vector<uint8_t> response;

        errorCode.clear();
        
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
            current_login = login;
            SessionManager::getInstance().addUser(current_login, self);
        }

        if(!response.empty()){
            send_packet(PacketType::RESULT, response);
            Logger::getInstance().logPacket(packet_type, errorCode);
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