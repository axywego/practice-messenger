#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantHash>
#include <memory>
#include "../include/client_async_handler.hpp"

class NetworkBridge : public QObject {
    Q_OBJECT
private:
    std::shared_ptr<ClientAsyncHandler> handler;
signals:
    void tokenVerify(quint32 errorCode);
    void authResult(QString token, quint32 errorCode);
    void chatMessageReceived(QString senderLogin, QString message);
    void directMessageReceived(QString senderLogin, QString message, qint64 timestamp);
    void friendRequestResult(quint32 errorCode);
    void friendListReceived(QStringList friends, QStringList pending);
    void historyReceived(QVariantList messages);
    void chatListReceived(QVariantList lastMessages);

    void friendNewRequest();
public:
    explicit NetworkBridge(std::shared_ptr<ClientAsyncHandler> handler, QObject* parent = nullptr): QObject(parent), handler(handler) {
        
        handler->on_token_response = [this](TokenVerifyResponse res) {
            QMetaObject::invokeMethod(this, [this, res] () {
                emit tokenVerify(res.error.value);
            }, Qt::QueuedConnection);
        };

        handler->on_auth_response = [this](AuthResponse res) {
            QMetaObject::invokeMethod(this, [this, res] () {
                emit authResult(QString::fromStdString(res.token), res.error.value);
            }, Qt::QueuedConnection);
        };

        handler->on_chat_incoming = [this](ChatIncoming res) {
            QMetaObject::invokeMethod(this, [this, res] () {
                emit chatMessageReceived(QString::fromStdString(res.sender_login), QString::fromStdString(res.message));
            }, Qt::QueuedConnection);
        };

        handler->on_dm_incoming = [this](DirectMessageIncoming res) {
            QMetaObject::invokeMethod(this, [this, res] () {
                emit directMessageReceived(QString::fromStdString(res.sender_login), QString::fromStdString(res.message), res.timestamp);
            }, Qt::QueuedConnection);
        };

        handler->on_friend_response = [this](FriendResponse res) {
            QMetaObject::invokeMethod(this, [this, res] () {
                emit friendRequestResult(res.error.value);
            }, Qt::QueuedConnection);
        };

        handler->on_friend_list_response = [this](FriendListResponse res) {
            QStringList friends, pending;
            for(const auto& f : res.friends) {
                friends << QString::fromStdString(f);
            }
            for(const auto& p : res.pending_incoming) {
                pending << QString::fromStdString(p);
            }
            QMetaObject::invokeMethod(this, [this, friends, pending] () {
                emit friendListReceived(friends, pending);
            }, Qt::QueuedConnection);
        };

        handler->on_history_response = [this](HistoryResponse res) {
            QVariantList list;
            for(const auto& m : res.messages) {
                QVariantHash entry;
                entry["from"] = QString::fromStdString(m.from_login);
                entry["message"] = QString::fromStdString(m.message);
                entry["timestamp"] = static_cast<qint64>(m.timestamp);
                list << entry;
            }
            QMetaObject::invokeMethod(this, [this, list] () {
                emit historyReceived(list);
            }, Qt::QueuedConnection);
        };

        handler->on_friend_new_request = [this]() {
            QMetaObject::invokeMethod(this, [this] () {
                emit friendNewRequest();
            }, Qt::QueuedConnection);
        };

        handler->on_chat_list = [this](ChatListResponse res) {
            QVariantList lastMessages;
            for(const auto& m : res.chats) {
                QVariantHash message;
                message["peer"] = QString::fromStdString(m.peer_login);
                message["message"] = QString::fromStdString(m.message);
                message["timestamp"] = static_cast<qint64>(m.timestamp);
                lastMessages << message;
            }
            QMetaObject::invokeMethod(this, [this, lastMessages] () {
                emit chatListReceived(lastMessages);
            }, Qt::QueuedConnection);
        };
    }
};