#include <boost/asio.hpp>
#include <boost/hash2/sha2.hpp>
#include <iostream>
#include <thread>
#include <print>
#include <fstream>
#include <vector>
#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <unordered_map>
#include <mutex>

#include "../include/client_session.hpp"

class Server {
private:
    boost::asio::io_context io_context;
    tcp::endpoint endpoint;
    tcp::acceptor acceptor;
public:
    Server() : endpoint(tcp::v4(), 12345), acceptor(io_context, endpoint) {
        std::println("Сервер создан на адресе {}", endpoint.address().to_string());
    }

    void start_accept() {
        acceptor.async_accept(io_context, 
            [this](boost::system::error_code error, tcp::socket socket) {
                if(!error) {
                    std::println("Клиент подключился!");
                    std::make_shared<ClientSession>(std::move(socket))->start();
                }
                else {
                    std::cerr << "Ошибка accept: " << error.message() << '\n';
                }
                start_accept();
            }
        );
    }
    void run(){
        io_context.run();
    }
};

int main() {
    try {
        Server server;

        server.start_accept();

        server.run();
    }
    catch(std::exception& e) {
        std::cerr << "Exception: " << e.what() << '\n';
    }
}