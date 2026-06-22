#pragma once

#include <mutex>
#include <deque>
#include <string>
#include <functional>

#include "network_session.hpp"
#include "messages.hpp"

class ClientAsyncHandler : public NetworkSession {
    using NetworkSession::NetworkSession;

public:
    std::function<void(std::string)> on_message;

    std::deque<std::string> messages;
    std::mutex messages_mutex;

protected:
    void handle_packet(PacketType packet_type, std::vector<uint8_t> body) override {
        std::string line;

        if (packet_type == PacketType::MESSAGE) {
            ChatIncoming req = ChatIncoming::deserialize(body);
            line = "[" + req.sender_login + "]: " + req.message;
        }
        // else if (packet_type == PacketType::RESULT) {
        //     line = "[сервер]: операция выполнена";
        // }
        {
            std::lock_guard lock(messages_mutex);
            messages.push_back(line);
            if (messages.size() > 200)
                messages.pop_front();
        }

        if (on_message)
            on_message(line);
    }
};