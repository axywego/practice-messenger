#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <vector>

using boost::asio::ip::tcp;

enum class PacketType : uint32_t {
    REGISTER = 1,
    LOGIN = 2,
    UPLOAD = 3,
    DOWNLOAD = 4,
    MESSAGE = 5,
    RESULT = 6
};

void send_packet(tcp::socket& socket, PacketType type, const std::vector<uint8_t>& body) {
    boost::asio::streambuf buf;

    boost::system::error_code error;

    std::ostream os(&buf);

    auto type_int32 = static_cast<uint32_t>(type);

    uint32_t type_net = htonl(type_int32);
    uint32_t size_net = htonl(body.size());
    os.write(reinterpret_cast<const char*>(&type_net), 4);
    os.write(reinterpret_cast<const char*>(&size_net), 4);
    os.write(reinterpret_cast<const char*>(body.data()), body.size());
    
    boost::asio::write(socket, buf, error);
}

std::pair<PacketType, std::vector<uint8_t>> recv_packet(tcp::socket& socket) {
    boost::asio::streambuf buf;

    std::istream is(&buf);

    boost::asio::read(socket, buf, boost::asio::transfer_exactly(4));
    
    uint32_t received_type, type;
    is.read(reinterpret_cast<char*>(&received_type), 4);
    type = ntohl(received_type);

    boost::asio::read(socket, buf, boost::asio::transfer_exactly(4));

    uint32_t received_length, length;

    is.read(reinterpret_cast<char*>(&received_length), 4);
    length = ntohl(received_length);

    boost::asio::read(socket, buf, boost::asio::transfer_exactly(length));

    std::vector<uint8_t> body(length);

    is.read(reinterpret_cast<char*>(body.data()), length);

    return {static_cast<PacketType>(type), body};
}