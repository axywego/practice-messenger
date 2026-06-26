#include "session_manager.hpp"

#include "../network_session.hpp"

void SessionManager::broadcast(
    const std::string& sender_login, PacketType packet_type, const std::vector<uint8_t>& packet_body
){
    std::lock_guard lock(mtx);

    for(auto& [login, weak_sess] : online_users){
        if(login != sender_login) {
            if(auto sess = weak_sess.lock())
                sess->send_packet(packet_type, packet_body);
        }
    }
}