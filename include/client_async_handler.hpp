#pragma once

#include <mutex>

#include "network_session.hpp"
#include "messages.hpp"

class ClientAsyncHandler : public NetworkSession {
    using NetworkSession::NetworkSession;

protected:
    void handle_packet(PacketType packet_type, std::vector<uint8_t> body) override {
        std::lock_guard lock(console_mutex);

        if(packet_type == PacketType::MESSAGE) {
            ChatIncoming req = parseChatIncoming(body);
            std::println("\n[{}]: {}", req.sender_login, req.message);
            std::print(">>> ");
        }
        else if(packet_type == PacketType::RESULT) {
            std::println("\n[Получен ответ от сервера]");
            std::print(">>> ");
        }
    }
public:

    static std::mutex console_mutex;
};

std::mutex ClientAsyncHandler::console_mutex;
