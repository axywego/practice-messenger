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
private:
    std::weak_ptr<ClientAsyncHandler> handler;
    std::string token;
public:
    explicit ClientHandlerQml(std::shared_ptr<ClientAsyncHandler> handler, QObject* parent = nullptr) : QObject(parent), handler(handler) {}
    
    void setToken(const std::string& t) {
        token = t;
        QString qtoken = QString::fromStdString(token);   
        qDebug() << "setToken вызван, токен:" << qtoken;
        qDebug() << "Путь к файлу:" << TokenStorage::filePath();
        TokenStorage::save(qtoken);
        qDebug() << "Прочитали обратно:" << TokenStorage::load();
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

    Q_INVOKABLE void sendFriendRequest(const QVariantHash& data) {

    }
    Q_INVOKABLE void acceptFriendRequest(const QVariantHash& data) {

    }
    Q_INVOKABLE void rejectFriendRequest(const QVariantHash& data) {

    }
    Q_INVOKABLE void getFriendList(const QVariantHash& data) {

    }

    Q_INVOKABLE QVariantHash sendDirectMessage(const QVariantHash& data) {

    }
    Q_INVOKABLE QVariantList getHistoryDirectMessage(const QVariantHash& data) {

    }
};