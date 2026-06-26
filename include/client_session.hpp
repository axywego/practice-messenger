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
#include "repositories/friend_repository.hpp"
#include "repositories/message_repository.hpp"
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
    std::string current_token;
    
    ErrorCode errorCode;

    Bytes handle_register(const Bytes& body, std::string& login, bool& is_auth_success) {
        Bytes response;

        auto& user_repo = UserRepository::getInstance();

        AuthRequest req = AuthRequest::deserialize(body);

        auto err = user_repo.registerUser(req.login, req.password);

        if (err.ok()) {
            auto token = user_repo.verifyUser(req.login, req.password);
            if(token.has_value())
                current_token = token.value();

            AuthResponse res {.token = token.value(), .error = token.error()};
            response = res.serialize();

            login = req.login;
            is_auth_success = true;

            errorCode = Error::Base::OK;
        }
        else {
            AuthResponse res {.token = "", .error = err};
            response = res.serialize();

            errorCode = err;
        }

        return response;
    }

    Bytes handle_login(const Bytes& body, std::string& login, bool& is_auth_success) {
        Bytes response;

        auto& user_repo = UserRepository::getInstance();
        
        AuthRequest req = AuthRequest::deserialize(body);

        auto token = user_repo.verifyUser(req.login, req.password);

        if (token.has_value()) {
            current_token = token.value();

            AuthResponse res { .token = token.value(), .error = Error::Base::OK};
            response = res.serialize();

            login = req.login;
            is_auth_success = true;

            errorCode = Error::Base::OK;
        }
        else {
            AuthResponse res {.token = "", .error = token.error()};
            response = res.serialize();

            errorCode = token.error();
        }

        return response;
    }

    Bytes handle_message(const Bytes& body) {
        Bytes response;

        auto& user_repo = UserRepository::getInstance();

        ChatSendRequest req = ChatSendRequest::deserialize(body);

        auto login = user_repo.getLoginByToken(req.token);

        if(login.has_value()) {
            ChatResponse res {.error = Error::Base::OK};
            
            response = res.serialize();
            
            // std::println("[{}]: {}", login, req.message);

            ChatIncoming incoming{.sender_login = login.value(), .message = req.message};
            SessionManager::getInstance().broadcast(login.value(), PacketType::MESSAGE, incoming.serialize());

            errorCode = Error::Base::OK;
        }
        else {
            ChatResponse res {.error = login.error()};
            response = res.serialize();

            errorCode = login.error();
        }

        return response;
    }

    Bytes handle_upload(const Bytes& body) {
        Bytes response;

        auto& user_repo = UserRepository::getInstance();

        FileUploadRequest req = FileUploadRequest::deserialize(body);

        auto login = user_repo.getLoginByToken(req.token);
        if(login.has_value()) {
            auto encrypted = encryptAES(req.file_data, SERVER_KEY);
            bytesToFile(req.file_name, encrypted);

            FileUploadResponse res{.error = Error::Base::OK};
            response = res.serialize();

            errorCode = Error::Base::OK;
        }
        else {
            FileUploadResponse res{.error = login.error()};
            response = res.serialize();

            errorCode = login.error();
        }

        return response;
    }

    Bytes handle_download(const Bytes& body) {
        Bytes response;

        auto& user_repo = UserRepository::getInstance();

        FileDownloadRequest req = FileDownloadRequest::deserialize(body);

        auto login = user_repo.getLoginByToken(req.token);

        Bytes bytes;
        if(login.has_value()) {
            bytes = fileToBytes(req.file_name);
            auto decrypted = decryptAES(bytes, SERVER_KEY);

            FileDownloadResponse res {.file_data = decrypted, .error = Error::Base::OK};
            response = res.serialize();

            errorCode = Error::Base::OK;
        }
        else {
            FileDownloadResponse res {.file_data = bytes, .error = login.error()};
            response = res.serialize();

            errorCode = login.error();
        }

        return response;
    }

    Bytes handle_friend_request(const Bytes& body) {
        FriendRequest req = FriendRequest::deserialize(body);

        auto& user_repo = UserRepository::getInstance();

        auto sender_login = user_repo.getLoginByToken(req.token);

        if(!sender_login.has_value()) {
            FriendResponse res {.error = sender_login.error()};

            errorCode = sender_login.error();
            
            return res.serialize();
        }

        auto user_exists_err = user_repo.userExists(req.target_login);
        if(!user_exists_err.ok()) {
            FriendResponse res {.error = user_exists_err};
            errorCode = user_exists_err;
            return res.serialize();
        }

        auto sender_id = user_repo.getIdByLogin(sender_login.value());
        auto target_id = user_repo.getIdByLogin(req.target_login);

        if(!(sender_id.has_value() && target_id.has_value())) {
            FriendResponse res {.error = sender_id.error().ok() ? target_id.error() : sender_id.error()};
            errorCode = res.error;
            return res.serialize();
        }

        auto err = FriendRepository::getInstance().sendRequest(sender_id.value(), target_id.value());
        if(!err.ok()) {
            FriendResponse res {.error = err};
            errorCode = err;
            return res.serialize();
        }
        
        FriendResponse res {.error = Error::Base::OK};
        return res.serialize();
    }

    Bytes handle_friend_accept(const Bytes& body) {
        FriendRequest req = FriendRequest::deserialize(body);

        auto& user_repo = UserRepository::getInstance();

        auto sender_login = user_repo.getLoginByToken(req.token);

        if(!sender_login.has_value()) {
            FriendResponse res {.error = sender_login.error()};
            errorCode = sender_login.error();
            return res.serialize();
        }

        auto user_exists_err = user_repo.userExists(req.target_login);
        if(!user_exists_err.ok()) {
            FriendResponse res {.error = user_exists_err};
            errorCode = user_exists_err;
            return res.serialize();
        }

        auto acceptor_id = user_repo.getIdByLogin(sender_login.value());
        auto requester_id = user_repo.getIdByLogin(req.target_login);

        if(!(acceptor_id.has_value() && requester_id.has_value())) {
            FriendResponse res {.error = acceptor_id.error().ok() ? requester_id.error() : acceptor_id.error()};
            errorCode = res.error;
            return res.serialize();
        }

        auto err = FriendRepository::getInstance().acceptRequest(acceptor_id.value(), requester_id.value());
        if(!err.ok()) {
            FriendResponse res {.error = err};
            errorCode = err;
            return res.serialize();
        }

        FriendResponse res {.error = Error::Base::OK};
        return res.serialize();
    }

    Bytes handle_friend_reject(const Bytes& body) {
        FriendRequest req = FriendRequest::deserialize(body);

        auto& user_repo = UserRepository::getInstance();
        
        auto sender_login = user_repo.getLoginByToken(req.token);

        if(!sender_login.has_value()) {
            FriendResponse res {.error = sender_login.error()};
            errorCode = sender_login.error();
            return res.serialize();
        }

        auto user_exists_err = user_repo.userExists(req.target_login);
        if(!user_exists_err.ok()) {
            FriendResponse res {.error = user_exists_err};
            errorCode = user_exists_err;
            return res.serialize();
        }

        auto rejecter_id = user_repo.getIdByLogin(sender_login.value());
        auto requester_id = user_repo.getIdByLogin(req.target_login);

        if(!(rejecter_id.has_value() && requester_id.has_value())) {
            FriendResponse res {.error = rejecter_id.error().ok() ? requester_id.error() : rejecter_id.error()};
            errorCode = res.error;
            return res.serialize();
        }

        auto err = FriendRepository::getInstance().rejectRequest(rejecter_id.value(), requester_id.value());
        if(!err.ok()) {
            FriendResponse res {.error = err};
            errorCode = err;
            return res.serialize();
        }

        FriendResponse res {.error = Error::Base::OK};
        return res.serialize();
    }

    Bytes handle_friend_list(const Bytes& body) {
        FriendListRequest req = FriendListRequest::deserialize(body);

        auto& user_repo = UserRepository::getInstance();
        
        auto sender_login = user_repo.getLoginByToken(req.token);

        if(!sender_login.has_value()) {
            FriendListResponse res {.friends = {}, .pending_incoming = {}, .error = sender_login.error()};
            errorCode = sender_login.error();
            return res.serialize();
        }

        auto sender_id = user_repo.getIdByLogin(sender_login.value());

        auto friend_list = FriendRepository::getInstance().getFriends(sender_id.value());
        std::vector<std::string> friend_list_login;
        friend_list_login.reserve(friend_list.size());
        
        for(const auto& el : friend_list) {
            friend_list_login.push_back(UserRepository::getInstance().getLoginById(el).value());
        }

        auto pending_list = FriendRepository::getInstance().getPendingIncoming(sender_id.value());
        std::vector<std::string> pending_list_login;
        pending_list_login.reserve(pending_list.size());

        for(const auto& el : pending_list) {
            pending_list_login.push_back(UserRepository::getInstance().getLoginById(el).value());
        }
        
        FriendListResponse res {.friends = friend_list_login, .pending_incoming = pending_list_login, .error = Error::Base::OK};
        return res.serialize();
    }

    Bytes handle_message_history(const Bytes& body) {
        HistoryRequest req = HistoryRequest::deserialize(body);

        auto& user_repo = UserRepository::getInstance();
        
        auto sender_login = user_repo.getLoginByToken(req.token);

        if(!sender_login.has_value()) {
            HistoryResponse res {.messages = {}, .error = sender_login.error()};
            errorCode = sender_login.error();
            return res.serialize();
        }

        auto sender_id = user_repo.getIdByLogin(sender_login.value());
        auto peer_id = user_repo.getIdByLogin(req.peer_login);

        auto messages = MessageRepository::getInstance().getHistory(sender_id.value(), peer_id.value(), 100);

        HistoryResponse res;
        for(const auto& m : messages) {
            HistoryResponse::Entry entry;
            entry.from_login = user_repo.getLoginById(m.sender).value();
            entry.message = m.message;
            entry.timestamp = m.timestamp;
            res.messages.push_back(entry);
        }
        res.error = Error::Base::OK;
        return res.serialize();
    }
 
    Bytes handle_direct_message(const Bytes& body) {
        DirectMessageRequest req = DirectMessageRequest::deserialize(body);

        auto& user_repo = UserRepository::getInstance();

        auto sender_login = user_repo.getLoginByToken(req.token);

        if(!sender_login.has_value()) {
            DirectMessageResponse res {.error = sender_login.error()};
            errorCode = sender_login.error();
            return res.serialize();
        }

        if(const auto err = user_repo.userExists(req.recipient_login); !err.ok()) {
            DirectMessageResponse res {.error = err};
            errorCode = err;
            return res.serialize();
        }

        auto sender_id = UserRepository::getInstance().getIdByLogin(sender_login.value());
        auto recipient_id = UserRepository::getInstance().getIdByLogin(req.recipient_login);

        if(const auto err = FriendRepository::getInstance().areFriends(sender_id.value(), recipient_id.value()); !err.ok()) {
            DirectMessageResponse res {.error = err};
            errorCode = err;
            return res.serialize();
        }

        auto sess = SessionManager::getInstance().getSession(req.recipient_login);
        bool online = (sess != nullptr);

        int64_t ts = MessageRepository::getInstance().saveMessage(sender_id.value(), recipient_id.value(), req.message);

        if(online) {
            DirectMessageIncoming incoming{.sender_login = sender_login.value(), .message = req.message, .timestamp = ts};
            sess->send_packet(PacketType::DIRECT_MESSAGE, incoming.serialize());
        }

        DirectMessageResponse res{.success = true, .error = Error::Base::OK};
        return res.serialize();
    }

    void deliver_pending() {
        auto& user_repo = UserRepository::getInstance();

        auto id = user_repo.getIdByLogin(current_login);

        auto pending = MessageRepository::getInstance().getUndelivered(id.value());

        if(pending.empty()) return;

        std::vector<int64_t> ids;
        for(const auto& el : pending) {
            auto sender_login = user_repo.getLoginById(el.sender);                

            DirectMessageIncoming incoming{
                .sender_login = sender_login.value(), .message = el.message, .timestamp = el.timestamp
            };
            
            send_packet(PacketType::DIRECT_MESSAGE, incoming.serialize());
            ids.push_back(el.id);
        }

        MessageRepository::getInstance().markDelivered(ids);
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

            case PacketType::FRIEND_REQUEST:{
                response = handle_friend_request(body); 
                break;
            }

            case PacketType::FRIEND_ACCEPT:{
                response = handle_friend_accept(body); 
                break;
            }

            case PacketType::FRIEND_REJECT:{
                response = handle_friend_reject(body); 
                break;
            }

            case PacketType::FRIEND_LIST:{
                response = handle_friend_list(body); 
                break;
            }

            case PacketType::DIRECT_MESSAGE: {
                response = handle_direct_message(body);
                break;
            }

            case PacketType::MESSAGE_HISTORY:{
                response = handle_message_history(body); 
                break;
            }

            default: {
                Logger::getInstance().logPacket(packet_type, errorCode, "я проебал этот пакет");
                break;
            }
        }

        if(is_auth_success) {
            current_login = login;
            deliver_pending();
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
            UserRepository::getInstance().logout(current_token);
            std::println("Клиент {} отключился", current_login);
        }
    }
};