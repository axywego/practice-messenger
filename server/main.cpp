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

#include "../include/handlers/client_handler.hpp"

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::endpoint endpoint(tcp::v4(), 12345);
        tcp::acceptor acceptor(io_context, endpoint);

        std::println("Сервер запущен по адресу {}. Ждем клиентов.", endpoint.address().to_string());

        while(true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::println("Клиент подключился!");

            std::thread(handle_client, std::move(socket)).detach();

        }
    }
    catch(std::exception& e) {
        std::cerr << "Exception: " << e.what() << '\n';
    }
}