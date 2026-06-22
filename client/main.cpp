// #include <boost/asio.hpp>
// #include <iostream>
// #include <thread>
// #include <print>
// #include <sstream>
// #include "../include/protocol.hpp"
// #include "../include/messages.hpp"
// #include "../include/cast.hpp"
// #include "../include/client_async_handler.hpp"

// #include <fstream>
// #include <filesystem>

// constexpr size_t MAX_FILE_SIZE = 1024 * 1024;

// using boost::asio::ip::tcp;

// std::string authorize(std::shared_ptr<ClientAsyncHandler> handler) {
//     std::println("Для входа в систему вам необходимо войти/зарегистрироваться. Введите LOGIN/REGISTER соотвественно.");
//     std::println("Пример команды: REGISTER login password");

//     std::string token;

//     while(true) {
//         boost::system::error_code error;
        
//         std::string request;
//         std::print(">>> ");
//         std::getline(std::cin, request);

//         std::istringstream iss_req(request);

//         std::string command;
//         iss_req >> command;
//         if(command != "REGISTER" && command != "LOGIN") {
//             std::println("НЕВЕРНАЯ КОМАНДА. ПОВТОРИТЕ ПОПЫТКУ!");
//             continue;
//         }

//         std::string login, password;
//         iss_req >> login >> password;

//         AuthRequest req {login, password};

//         std::println("SEND REQUEST TO AUTH...");
//         send_packet_sync(handler->get_socket(), command == "REGISTER" ? PacketType::REGISTER : PacketType::LOGIN, makeAuthRequest(req));

//         std::println("WAITING RESPONSE...");
//         const auto& [result_type, result_body] = recv_packet_sync(handler->get_socket());
//         std::println("RESPONSE HERE...");

//         AuthResponse res = parseAuthResponse(result_body);

//         if(res.success) {
//             token = res.token;
//             break;
//         } 
//     }
//     return token;
// }


// void chat_loop(std::shared_ptr<ClientAsyncHandler> handler, const std::string& token) {

//     while(true) {
//         std::string choice;

//         {
//             std::lock_guard lock(ClientAsyncHandler::console_mutex);

//             std::println("=========================");
//             std::println("1. Отправить сообщение");
//             std::println("2. Отравить файл");
//             std::println("3. Скачать файл");
//             std::println("=========================");
//             std::print(">>> ");
//         }

//         std::getline(std::cin, choice);

//         if(choice == "1") {
//             std::print("MSG >>> ");

//             std::string msg;
//             std::getline(std::cin, msg);

//             ChatSendRequest req {token, msg};

//             handler->send_packet(PacketType::MESSAGE, makeChatSendRequest(req));
//         }

//         else if(choice == "2") {
//             std::print("FILE NAME >>> ");

//             std::string file_name;
//             std::getline(std::cin, file_name);
//             auto size = std::filesystem::file_size(file_name);

//             if(size >= MAX_FILE_SIZE) {
//                 std::println("Слишком большой размер файла!");
//                 continue;
//             }

//             auto file_buffer = fileToBytes(file_name);

//             FileRequest req {token, file_name, file_buffer};

//             handler->send_packet(PacketType::UPLOAD, makeFileRequest(req));
//         }

//         else if(choice == "3") {
//             std::print("FILE NAME >>> ");

//             std::string file_name;
//             std::getline(std::cin, file_name);

//             DownloadRequest req {token, file_name};

//             handler->send_packet(PacketType::DOWNLOAD, makeDownloadRequest(req));
//         }
//     }
// }

// int main() {
//     try {
//         boost::asio::io_context io_context;

//         tcp::socket socket(io_context);
//         socket.connect(tcp::endpoint(boost::asio::ip::make_address_v4("0.0.0.0"), 12345));
//         auto handler = std::make_shared<ClientAsyncHandler>(std::move(socket));
//         std::println("Подключение к серверу!");

//         auto token = authorize(handler);
//         std::println("Успешный вход! Вам стал доступен чат.");

//         std::thread input_thread(chat_loop, handler, token);
//         input_thread.detach();

//         handler->start();

//         io_context.run();
//     }
//     catch (std::exception& e) {
//         std::cerr << "Exception: " << e.what() << '\n';
//     }
// }
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

// ftxui
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

constexpr size_t MAX_FILE_SIZE = 1024 * 1024;

using boost::asio::ip::tcp;
using namespace ftxui;

// ---------------------------------------------------------------------------
// Авторизация — остаётся в обычном консольном режиме (до запуска TUI)
// ---------------------------------------------------------------------------
std::string authorize(std::shared_ptr<ClientAsyncHandler> handler) {
    std::println("Для входа введите LOGIN/REGISTER.");
    std::println("Пример: REGISTER login password");

    std::string token;
    while (true) {
        std::string request;
        std::print(">>> ");
        std::getline(std::cin, request);

        std::istringstream iss(request);
        std::string command;
        iss >> command;

        if (command != "REGISTER" && command != "LOGIN") {
            std::println("Неверная команда. Повторите попытку.");
            continue;
        }

        std::string login, password;
        iss >> login >> password;

        AuthRequest req{.login = login, .password = password};
        send_packet_sync(handler->get_socket(),
                         command == "REGISTER" ? PacketType::REGISTER : PacketType::LOGIN,
                         req.serialize());

        const auto& [result_type, result_body] = recv_packet_sync(handler->get_socket());
        AuthResponse res = AuthResponse::deserialize(result_body);

        if (res.success) {
            token = res.token;
            break;
        }
        std::println("Ошибка авторизации. Попробуйте снова.");
    }
    return token;
}

// ---------------------------------------------------------------------------
// TUI — главный экран чата
// ---------------------------------------------------------------------------
void run_tui(std::shared_ptr<ClientAsyncHandler> handler, const std::string& token) {
    auto screen = ScreenInteractive::Fullscreen();

    // --- Состояние ---
    std::vector<std::string> menu_entries = {
        "Отправить сообщение",
        "Отправить файл",
        "Скачать файл",
    };
    int menu_selected = 0;
    bool menu_active = false;  // показываем ли поле ввода

    std::string input_value;
    std::string input_label = ">>> ";

    // Локальная копия сообщений для рендера (обновляется из on_message)
    std::vector<std::string> displayed_messages;

    // --- Колбэк из сетевого потока ---
    handler->on_message = [&](std::string) {
        {
            std::lock_guard lock(handler->messages_mutex);
            displayed_messages.assign(handler->messages.begin(), handler->messages.end());
        }
        // Просим ftxui перерисовать экран из любого потока
        screen.PostEvent(Event::Custom);
    };

    // --- Обработка отправки (Enter в поле ввода) ---
    auto do_send = [&]() {
        if (input_value.empty()) return;

        switch (menu_selected) {
        case 0: {
            ChatSendRequest req{.token = token, .message = input_value};
            handler->send_packet(PacketType::MESSAGE, req.serialize());

            std::lock_guard lock(handler->messages_mutex);
            handler->messages.push_back("[Вы]: " + input_value);
            displayed_messages.assign(handler->messages.begin(), handler->messages.end());
            break;
        }
        case 1: {
            auto size = std::filesystem::file_size(input_value);
            if (size >= MAX_FILE_SIZE) {
                std::lock_guard lock(handler->messages_mutex);
                handler->messages.push_back("[!] Файл слишком большой.");
                displayed_messages.assign(handler->messages.begin(), handler->messages.end());
                break;
            }
            auto buf = fileToBytes(input_value);
            FileUploadRequest req{.token = token, .file_name = input_value, .file_data = buf};
            handler->send_packet(PacketType::UPLOAD, req.serialize());
            break;
        }
        case 2: {
            FileDownloadRequest req{.token = token, .file_name = input_value};
            handler->send_packet(PacketType::DOWNLOAD, req.serialize());
            break;
        }
        }

        input_value.clear();
        menu_active = false;  // после отправки сбрасываем в выбор действия
    };

    // --- Компоненты ---

    // Меню действий (правая панель)
    auto menu = Menu(&menu_entries, &menu_selected);

    // Поле ввода (нижняя панель)
    InputOption input_opt;
    input_opt.on_enter = [&]{ do_send(); };
    auto input = Input(&input_value, &input_label, input_opt);

    // Корневой контейнер: перехватываем клавиши
    auto root = Container::Vertical({
        Container::Horizontal({
            // left — пустышка для сообщений (они рендерятся в Renderer ниже)
            Renderer([]{ return text(""); }),
            menu,
        }),
        input,
    });

    root = CatchEvent(root, [&](Event event) -> bool {
        // Ctrl+W — отмена выбора / закрыть ввод
        if (event == Event::Special("\x17")) {
            input_value.clear();
            menu_active = false;
            return true;
        }
        // Enter на меню — активировать поле ввода
        if (!menu_active && event == Event::Return) {
            menu_active = true;
            // Подсказка зависит от выбранного пункта
            switch (menu_selected) {
            case 0: input_label = "Сообщение >>> "; break;
            case 1: input_label = "Имя файла   >>> "; break;
            case 2: input_label = "Имя файла   >>> "; break;
            }
            input->TakeFocus();
            return true;
        }
        return false;
    });

    // --- Renderer ---
    auto renderer = Renderer(root, [&]() -> Element {
        // Левая панель: история сообщений
        Elements msg_elements;
        for (const auto& m : displayed_messages)
            msg_elements.push_back(text(m));
        if (msg_elements.empty())
            msg_elements.push_back(text("— нет сообщений —") | dim);

        auto left_pane = vbox(std::move(msg_elements)) | yframe | flex;

        // Правая панель: меню
        auto right_pane = vbox({
            text("[ действия ]") | bold | color(Color::GrayLight),
            separator(),
            menu->Render(),
            filler(),
            text("enter — выбрать") | dim,
            text("ctrl+w — отмена") | dim,
        }) | size(WIDTH, EQUAL, 28);

        // Нижняя панель: ввод
        Element bottom;
        if (menu_active) {
            bottom = hbox({
                input->Render() | flex,
            }) | border;
        } else {
            bottom = hbox({
                text("выберите действие в меню →") | dim | flex,
            }) | border;
        }

        return vbox({
            hbox({
                left_pane | border | flex,
                right_pane | border,
            }) | flex,
            bottom,
        });
    });

    screen.Loop(renderer);
}

// ---------------------------------------------------------------------------
// Консольный режим (--console) — оригинальный chat_loop без TUI
// ---------------------------------------------------------------------------
void chat_loop_console(std::shared_ptr<ClientAsyncHandler> handler, const std::string& token) {
    static std::mutex console_mtx;

    handler->on_message = [](std::string line) {
        std::lock_guard lock(console_mtx);
        std::println("{}", line);
        std::print(">>> ");
    };

    while (true) {
        std::string choice;

        {
            std::lock_guard lock(console_mtx);
            std::println("=========================");
            std::println("1. Отправить сообщение");
            std::println("2. Отправить файл");
            std::println("3. Скачать файл");
            std::println("=========================");
            std::print(">>> ");
        }

        std::getline(std::cin, choice);

        if (choice == "1") {
            std::print("MSG >>> ");
            std::string msg;
            std::getline(std::cin, msg);
            ChatSendRequest req{.token = token, .message = msg};
            handler->send_packet(PacketType::MESSAGE, req.serialize());
        }
        else if (choice == "2") {
            std::print("FILE NAME >>> ");
            std::string file_name;
            std::getline(std::cin, file_name);

            auto size = std::filesystem::file_size(file_name);
            if (size >= MAX_FILE_SIZE) {
                std::println("Слишком большой размер файла!");
                continue;
            }
            auto buf = fileToBytes(file_name);
            FileUploadRequest req{.token = token, .file_name = file_name, .file_data = buf};
            handler->send_packet(PacketType::UPLOAD, req.serialize());
        }
        else if (choice == "3") {
            std::print("FILE NAME >>> ");
            std::string file_name;
            std::getline(std::cin, file_name);
            FileDownloadRequest req{.token = token, .file_name = file_name};
            handler->send_packet(PacketType::DOWNLOAD, req.serialize());
        }
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
enum class Mode { TUI, Console };

Mode parse_args(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--console" || arg == "-c")
            return Mode::Console;
        if (arg == "--tui" || arg == "-t")
            return Mode::TUI;
        if (arg == "--help" || arg == "-h") {
            std::println("Использование: {} [опции]", argv[0]);
            std::println("  --tui,     -t   Графический режим в терминале (по умолчанию)");
            std::println("  --console, -c   Простой консольный режим");
            std::println("  --help,    -h   Показать эту справку");
            std::exit(0);
        }
        std::println("Неизвестный аргумент: {}. Используйте --help.", arg);
        std::exit(1);
    }
    return Mode::TUI; // по умолчанию
}

int main(int argc, char* argv[]) {
    Mode mode = parse_args(argc, argv);

    try {
        boost::asio::io_context io_context;

        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address_v4("0.0.0.0"), 12345));

        auto handler = std::make_shared<ClientAsyncHandler>(std::move(socket));
        std::println("Подключение к серверу!");

        auto token = authorize(handler);
        std::println("Успешный вход! Запускаем чат...");

        handler->start();
        std::thread io_thread([&]{ io_context.run(); });

        if (mode == Mode::Console) {
            io_thread.detach();
            chat_loop_console(handler, token);
        } else {
            io_thread.detach();
            run_tui(handler, token);
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << '\n';
    }
}