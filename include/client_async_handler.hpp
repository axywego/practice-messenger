#pragma once

#include <mutex>
#include <deque>
#include <string>
#include <functional>

#include "network_session.hpp"
#include "messages.hpp"
#include <functional>

class ClientAsyncHandler : public NetworkSession {
    using NetworkSession::NetworkSession;

public:
    std::function<void(TokenVerifyResponse)> on_token_response;
    std::function<void(AuthResponse)> on_auth_response;
    std::function<void(ChatIncoming)> on_chat_incoming;
    std::function<void(DirectMessageIncoming)> on_dm_incoming;
    std::function<void(FriendResponse)> on_friend_response;
    std::function<void(FriendListResponse)> on_friend_list_response;
    std::function<void(HistoryResponse)> on_history_response;
    std::function<void(ChatListResponse)> on_chat_list;

    // вызов в том случае, если нас добавляют в друзья или отклоняют запрос ну и да
    std::function<void()> on_friend_new_request;

    std::function<void(std::string)> on_message;

    std::deque<std::string> messages;
    std::mutex messages_mutex;

    std::queue<PacketType> pending_requests;
    std::mutex pending_mutex;

    void send_request(PacketType type, const Bytes& body) {
        {
            std::lock_guard lock(pending_mutex);
            pending_requests.push(type);
        }
        send_packet(type, body);
    }

protected:
    void handle_packet(PacketType packet_type, std::vector<uint8_t> body) override {
        std::string line;

        switch(packet_type) {
            case PacketType::MESSAGE: {
                auto msg = ChatIncoming::deserialize(body);
                if(on_chat_incoming) on_chat_incoming(msg);
                break;
            }
            case PacketType::DIRECT_MESSAGE: {
                auto msg = DirectMessageIncoming::deserialize(body);
                if(on_dm_incoming) on_dm_incoming(msg);
                break;
            }
            case PacketType::FRIEND_NEW_REQUEST:
            case PacketType::FRIEND_REQUEST_ACCEPTED:
            case PacketType::FRIEND_REQUEST_REJECTED: {
                if(on_friend_new_request) on_friend_new_request();
                break;
            }
            case PacketType::RESULT: {
                PacketType original;
                {
                    std::lock_guard lock(pending_mutex);
                    if(pending_requests.empty()) break;
                    original = pending_requests.front();
                    pending_requests.pop();
                }

                switch(original) {
                    case PacketType::TOKEN_VERIFY: {
                        auto res = TokenVerifyResponse::deserialize(body);
                        if(on_token_response) on_token_response(res);
                        break;
                    }
                    case PacketType::REGISTER:
                    case PacketType::LOGIN: {
                        auto res = AuthResponse::deserialize(body);
                        if(on_auth_response) on_auth_response(res);
                        break;
                    }
                    case PacketType::FRIEND_REQUEST:
                    case PacketType::FRIEND_ACCEPT:
                    case PacketType::FRIEND_REJECT: {
                        auto res = FriendResponse::deserialize(body);
                        if(on_friend_response) on_friend_response(res);
                        break;
                    }
                    case PacketType::FRIEND_LIST: {
                        auto res = FriendListResponse::deserialize(body);
                        if(on_friend_list_response) on_friend_list_response(res);
                        break;
                    }
                    case PacketType::MESSAGE_HISTORY: {
                        auto res = HistoryResponse::deserialize(body);
                        if(on_history_response) on_history_response(res);
                        break;
                    }
                    case PacketType::CHAT_LIST: {
                        auto res = ChatListResponse::deserialize(body);
                        if(on_chat_list) on_chat_list(res);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
        }

        {
            std::lock_guard lock(messages_mutex);
            messages.push_back(line);
            if (messages.size() > 200)
                messages.pop_front();
        }
    }
};