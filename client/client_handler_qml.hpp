#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QHash>
#include <optional>

#include "../include/client_async_handler.hpp"
#include "token_storage.hpp"
#include "../include/crypto.hpp"

class ClientHandlerQml : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isTokenReady READ isTokenReady NOTIFY tokenReady)
private:
    std::weak_ptr<ClientAsyncHandler> handler;
    std::string token;
signals:
    void tokenReady();
public:
    explicit ClientHandlerQml(std::shared_ptr<ClientAsyncHandler> handler, QObject* parent = nullptr) : QObject(parent), handler(handler) {}
    
    bool isTokenReady() const {
        return !token.empty();
    }

    void setToken(const std::string& t) {
        token = t;
        QString qtoken = QString::fromStdString(t);   
        qDebug() << "setToken вызван, токен:" << qtoken;
        qDebug() << "Путь к файлу:" << TokenStorage::filePath();
        TokenStorage::save(qtoken);
        qDebug() << "Прочитали обратно:" << TokenStorage::load();
        emit tokenReady();
    }

    Q_INVOKABLE bool checkSavedToken() {
        QString saved = TokenStorage::load();
        qDebug() << "checkSavedToken, токен:" << saved;
        if(saved.isEmpty()) return false;

        auto p = handler.lock();
        if(!p) return false;
        TokenVerifyRequest req{ .old_token = saved.toStdString() };
        p->send_request(PacketType::TOKEN_VERIFY, req.serialize());
        qDebug() << "TOKEN_VERIFY отправлен";
        return true;
    }

    Q_INVOKABLE void logout() {
        auto p = handler.lock();
        if(!p) return;

        if(!token.empty()) {
            LogoutRequest req{ .old_token = token, };
            p->send_packet(PacketType::LOGOUT, req.serialize());
        }

        token = "";
        TokenStorage::clear();
    }

    Q_INVOKABLE void registerUser(const QString& login, const QString& password) {
        auto p = handler.lock();
        if(!p) return;
        AuthRequest req {
            .login = login.toStdString(),
            .password = sha256(password.toStdString()),
        };
        p->send_request(PacketType::REGISTER, req.serialize());
    }

    Q_INVOKABLE void loginUser(const QString& login, const QString& password) {
        auto p = handler.lock();
        if(!p) return;
        AuthRequest req {
            .login = login.toStdString(),
            .password = sha256(password.toStdString()),
        };
        p->send_request(PacketType::LOGIN, req.serialize());
    }

    Q_INVOKABLE void uploadFile(const QVariantHash& data){

    }

    Q_INVOKABLE void downloadFile(const QVariantHash& data) {

    }

    Q_INVOKABLE void sendMessageGeneralChat(const QString& message) {
        auto p = handler.lock();
        if(!p) return;
        ChatSendRequest req {
            .token = token,
            .message = message.toStdString(),
        };
        p->send_request(PacketType::MESSAGE, req.serialize());
    }

    Q_INVOKABLE void sendFriendRequest(const QString& target_login) {
        auto p = handler.lock();
        if(!p) return;
        FriendRequest req {
            .token = token,
            .target_login = target_login.toStdString(),
        };
        p->send_request(PacketType::FRIEND_REQUEST, req.serialize());
    }

    Q_INVOKABLE void acceptFriendRequest(const QString& target_login) {
        auto p = handler.lock();
        if(!p) return;
        FriendRequest req {
            .token = token,
            .target_login = target_login.toStdString(),
        };
        p->send_request(PacketType::FRIEND_ACCEPT, req.serialize());
    }

    Q_INVOKABLE void rejectFriendRequest(const QString& target_login) {
        auto p = handler.lock();
        if(!p) return;
        FriendRequest req {
            .token = token,
            .target_login = target_login.toStdString(),
        };
        p->send_request(PacketType::FRIEND_REJECT, req.serialize());
    }

    Q_INVOKABLE void getFriendList() {
        auto p = handler.lock();
        if(!p) return;
        FriendListRequest req {
            .token = token,
        };
        p->send_request(PacketType::FRIEND_LIST, req.serialize());
    }

    Q_INVOKABLE void sendDirectMessage(const QString& recipientLogin, const QString& message) {
        auto p = handler.lock();
        if(!p) return;
        DirectMessageRequest req {
            .token = token,
            .recipient_login = recipientLogin.toStdString(),
            .message = message.toStdString()
        };
        p->send_request(PacketType::DIRECT_MESSAGE, req.serialize());
    }

    Q_INVOKABLE void getHistoryDirectMessage(const QString& peerLogin) {
        auto p = handler.lock();
        if(!p) return;
        HistoryRequest req {
            .token = token,
            .peer_login = peerLogin.toStdString()
        };
        p->send_request(PacketType::MESSAGE_HISTORY, req.serialize());
    }

    Q_INVOKABLE void getLastMessages() {
        auto p = handler.lock();
        if(!p) return;
        ChatListRequest req {
            .token = token,
        };
        p->send_request(PacketType::CHAT_LIST, req.serialize());
    }
};