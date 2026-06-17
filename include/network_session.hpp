#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <print>
#include <memory>
#include <queue>
#include <array>

#include "protocol.hpp"
#include "literals.hpp"

using boost::asio::ip::tcp;

class NetworkSession : public std::enable_shared_from_this<NetworkSession> {
protected:
    virtual void handle_packet(PacketType packet_type, std::vector<uint8_t> body) = 0;
private:
    tcp::socket socket;
    std::array<char, 8> header_buffer;
    std::queue<std::vector<uint8_t>> write_queue;

    void read_header() {
        auto self(shared_from_this());

        boost::asio::async_read(socket, boost::asio::buffer(header_buffer), 
            [this, self](boost::system::error_code error, size_t ) {
                if(!error)
                    parse_header_and_read_body();
                else 
                    socket.close();
            }
        );
    }

    void parse_header_and_read_body() {
        uint32_t type_net, length_net;
        memcpy(&type_net, header_buffer.data(), 4);
        memcpy(&length_net, header_buffer.data() + 4, 4);

        auto packet_type = static_cast<PacketType>(ntohl(type_net));
        auto body_length = ntohl(length_net);

        // if(body_length > 10_MB) {
        //     std::println("Слишком большой пакет от клиента!");
        //     socket.close();
        //     return;
        // }

        auto body_buffer = std::make_shared<std::vector<uint8_t>>(body_length);

        auto self(shared_from_this());

        boost::asio::async_read(socket, boost::asio::buffer(*body_buffer),
            [this, self, packet_type, body_buffer](boost::system::error_code error, size_t length) {
                if(!error) {
                    handle_packet(packet_type, std::move(*body_buffer));
                    read_header();
                }
                else {
                    socket.close();
                }
            });
    }

    void do_write() {
        auto self(shared_from_this());

        boost::asio::async_write(socket, boost::asio::buffer(write_queue.front()),
            [this, self] (boost::system::error_code error, size_t) {
                if(!error) {
                    write_queue.pop();
                    if(!write_queue.empty()) {
                        do_write();
                    }
                }
                else {
                    socket.close();
                }
            }
        );
    }
public:
    explicit NetworkSession(tcp::socket socket) : socket(std::move(socket)) {}

    virtual ~NetworkSession() = default;

    tcp::socket& get_socket() {
        return socket;
    }

    void start() {
        read_header();
    }

    void send_packet(PacketType packet_type, const std::vector<uint8_t>& body) {
        auto self(shared_from_this());

        std::vector<uint8_t> full_packet;
        uint32_t type_net = htonl(static_cast<uint32_t>(packet_type));
        uint32_t size_net = htonl(body.size());

        full_packet.resize(8 + body.size());

        memcpy(full_packet.data(), &type_net, 4);
        memcpy(full_packet.data() + 4, &size_net, 4);
        memcpy(full_packet.data() + 8, body.data(), body.size());

        boost::asio::post(socket.get_executor(), 
            [this, self, p = std::move(full_packet)]() mutable {
                bool write_in_progress = !write_queue.empty();
                write_queue.push(std::move(p));
                if(!write_in_progress) {
                    do_write();
                }
            }
        );
    }
};