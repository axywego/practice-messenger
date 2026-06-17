#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <print>
#include <sstream>
#include "../include/protocol.hpp"
#include "../include/messages.hpp"
#include "../include/cast.hpp"
#include "../include/client_async_handler.hpp"

#include <fstream>
#include <filesystem>

constexpr size_t MAX_FILE_SIZE = 1024 * 1024;

using boost::asio::ip::tcp;

std::string authorize(std::shared_ptr<ClientAsyncHandler> handler) {
    std::println("Для входа в систему вам необходимо войти/зарегистрироваться. Введите LOGIN/REGISTER соотвественно.");
    std::println("Пример команды: REGISTER login password");

    std::string token;

    while(true) {
        boost::system::error_code error;
        
        std::string request;
        std::print(">>> ");
        std::getline(std::cin, request);

        std::istringstream iss_req(request);

        std::string command;
        iss_req >> command;
        if(command != "REGISTER" && command != "LOGIN") {
            std::println("НЕВЕРНАЯ КОМАНДА. ПОВТОРИТЕ ПОПЫТКУ!");
            continue;
        }

        std::string login, password;
        iss_req >> login >> password;

        AuthRequest req {login, password};

        std::println("SEND REQUEST TO AUTH...");
        send_packet_sync(handler->get_socket(), command == "REGISTER" ? PacketType::REGISTER : PacketType::LOGIN, makeAuthRequest(req));

        std::println("WAITING RESPONSE...");
        const auto& [result_type, result_body] = recv_packet_sync(handler->get_socket());
        std::println("RESPONSE HERE...");

        AuthResponse res = parseAuthResponse(result_body);

        if(res.success) {
            token = res.token;
            break;
        } 
    }
    return token;
}


void chat_loop(std::shared_ptr<ClientAsyncHandler> handler, const std::string& token) {

    while(true) {
        std::string choice;

        {
            std::lock_guard lock(ClientAsyncHandler::console_mutex);

            std::println("=========================");
            std::println("1. Отправить сообщение");
            std::println("2. Отравить файл");
            std::println("3. Скачать файл");
            std::println("=========================");
            std::print(">>> ");
        }

        std::getline(std::cin, choice);

        if(choice == "1") {
            std::print("MSG >>> ");

            std::string msg;
            std::getline(std::cin, msg);

            ChatSendRequest req {token, msg};

            handler->send_packet(PacketType::MESSAGE, makeChatSendRequest(req));
        }

        else if(choice == "2") {
            std::print("FILE NAME >>> ");

            std::string file_name;
            std::getline(std::cin, file_name);
            auto size = std::filesystem::file_size(file_name);

            if(size >= MAX_FILE_SIZE) {
                std::println("Слишком большой размер файла!");
                continue;
            }

            auto file_buffer = fileToBytes(file_name);

            FileRequest req {token, file_name, file_buffer};

            handler->send_packet(PacketType::UPLOAD, makeFileRequest(req));
        }

        else if(choice == "3") {
            std::print("FILE NAME >>> ");

            std::string file_name;
            std::getline(std::cin, file_name);

            DownloadRequest req {token, file_name};

            handler->send_packet(PacketType::DOWNLOAD, makeDownloadRequest(req));
        }
    }
}

int main() {
    try {
        boost::asio::io_context io_context;

        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address_v4("0.0.0.0"), 12345));
        auto handler = std::make_shared<ClientAsyncHandler>(std::move(socket));
        std::println("Подключение к серверу!");

        auto token = authorize(handler);
        std::println("Успешный вход! Вам стал доступен чат.");

        std::thread input_thread(chat_loop, handler, token);
        input_thread.detach();

        handler->start();

        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << '\n';
    }
}