#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <vector>
#include <array>

using boost::asio::ip::tcp;

enum class PacketType : uint32_t {
    REGISTER = 1,
    LOGIN = 2,
    UPLOAD = 3,
    DOWNLOAD = 4,
    MESSAGE = 5,
    RESULT = 6
};

inline void send_packet_sync(tcp::socket& socket, PacketType type, const std::vector<uint8_t>& body) {
    std::vector<uint8_t> full_buffer;

    uint32_t type_net = htonl(static_cast<uint32_t>(type));
    uint32_t size_net = htonl(body.size());
    
    full_buffer.resize(8 + body.size());

    memcpy(full_buffer.data(), &type_net, 4);
    memcpy(full_buffer.data() + 4, &size_net, 4);
    memcpy(full_buffer.data() + 8, body.data(), body.size());

    boost::system::error_code error;

    boost::asio::write(socket, boost::asio::buffer(full_buffer), error);
    if(error) {
        throw std::runtime_error("Send failed: " + error.message());
    }
}

inline std::pair<PacketType, std::vector<uint8_t>> recv_packet_sync(tcp::socket& socket) {
    boost::system::error_code error;

    std::array<char, 8> header;
    boost::asio::read(socket, boost::asio::buffer(header), error);
    if(error) {
        throw std::runtime_error("Recv failed: " + error.message());
    }
    
    uint32_t type_net, length_net;
    memcpy(&type_net, header.data(), 4);
    memcpy(&length_net, header.data() + 4, 4);

    auto type = static_cast<PacketType>(ntohl(type_net));
    auto length = ntohl(length_net);

    std::vector<uint8_t> body(length);
    if(length > 0) {
        boost::asio::read(socket, boost::asio::buffer(body), boost::asio::transfer_exactly(length));
        if(error) {
            throw std::runtime_error("Recv failed: " + error.message());
        }
    }

    return {type, body};
}