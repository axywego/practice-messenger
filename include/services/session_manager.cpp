#include "session_manager.hpp"

#include "../network_session.hpp"

void SessionManager::broadcast(
    const std::string& sender_login, PacketType packet_type, const std::vector<uint8_t>& packet_body
){
    std::lock_guard lock(mtx);
    
    std::println("[SessionManager] broadcast от '{}', всего онлайн: {}", 
                 sender_login, online_users.size());

    for(auto& [login, weak_sess] : online_users){
        std::println("[SessionManager] проверяем '{}'", login);
        if(login != sender_login) {
            if(auto sess = weak_sess.lock()) {
                std::println("[SessionManager] отправляем '{}'", login);
                sess->send_packet(packet_type, packet_body);
            } else {
                std::println("[SessionManager] сессия '{}' мертва", login);
            }
        }
    }
}