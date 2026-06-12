#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <print>
#include <sstream>
#include "../include/protocol.hpp"
#include "../include/messages.hpp"
#include "../include/cast.hpp"

#include <fstream>
#include <filesystem>

constexpr size_t MAX_FILE_SIZE = 1024 * 1024;

using boost::asio::ip::tcp;

std::string authorize(tcp::socket& socket) {
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

        send_packet(socket, command == "REGISTER" ? PacketType::REGISTER : PacketType::LOGIN, makeAuthRequest(req));

        const auto& [result_type, result_body] = recv_packet(socket);

        AuthResponse res = parseAuthResponse(result_body);

        if(res.success) {
            token = res.token;
            break;
        } 
    }
    return token;
}


void chat(tcp::socket& socket, const std::string& token) {

    while(true) {

        std::println("=========================");
        std::println("1. Отправить сообщение");
        std::println("2. Отравить файл");
        std::println("3. Скачать файл");
        std::println("=========================");
        std::print(">>> ");
        
        std::string choice;
        std::getline(std::cin, choice);

        if(choice == "1") {
            std::print("MSG >>> ");

            std::string msg;
            std::getline(std::cin, msg);

            ChatRequest req {token, msg};

            send_packet(socket, PacketType::MESSAGE, makeChatRequest(req));

            const auto& [result_type, result_body] = recv_packet(socket);

            ChatResponse res = parseChatResponse(result_body);

            if(!res.success) {
                std::println("Сообщение не было отправлено!");
            }
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

            send_packet(socket, PacketType::UPLOAD, makeFileRequest(req));

            const auto& [result_type, result_body] = recv_packet(socket);
            FileResponse res = parseFileResponse(result_body);

            if(!res.success) {
                std::println("Файл не был отправлен!");
            }
        }

        else if(choice == "3") {
            std::print("FILE NAME >>> ");

            std::string file_name;
            std::getline(std::cin, file_name);

            DownloadRequest req {token, file_name};

            send_packet(socket, PacketType::DOWNLOAD, makeDownloadRequest(req));

            const auto& [result_type, result_body] = recv_packet(socket);

            DownloadResponse res = parseDownloadResponse(result_body);

            if(res.success) {
                bytesToFile(file_name, res.file_data);
            }
            else {
                std::println("Не удалось скачать файл!");
            }
        }
    }
}

void connect_to_server(const std::string& server_ip) {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address(server_ip), 12345));

        std::println("Подключение к серверу {}", server_ip);

        auto token = authorize(socket);

        std::println("Успешный вход! Вам стал доступен чат.");

        chat(socket, token);
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << '\n';
    }
}

int main() {
    std::string server_ip;
    std::print("Введите адрес сервера в формате xx.xx.xx.xx: ");
    std::getline(std::cin, server_ip);
    connect_to_server(server_ip);
    return 0;
}