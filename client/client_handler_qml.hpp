#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QHash>
#include <optional>

#include "../include/client_async_handler.hpp"

class ClientHandlerQml : public QObject {
    Q_OBJECT
private:
    std::weak_ptr<ClientAsyncHandler> handler;
public:
    explicit ClientHandlerQml(std::shared_ptr<ClientAsyncHandler> handler, QObject* parent = nullptr);
    Q_INVOKABLE bool registerUser(const QVariantHash& data);
    Q_INVOKABLE bool loginUser(const QVariantHash& data);

    Q_INVOKABLE bool uploadFile(const QVariantHash& data);
    Q_INVOKABLE bool downloadFile(const QVariantHash& data);

    Q_INVOKABLE bool sendMessageGeneralChat(const QVariantHash& data);
    Q_INVOKABLE bool getMessageFromGeneralChat(const QVariantHash& data);

    Q_INVOKABLE bool sendFriendRequest(const QVariantHash& data);
    Q_INVOKABLE bool acceptFriendRequest(const QVariantHash& data);
    Q_INVOKABLE bool rejectFriendRequest(const QVariantHash& data);
    Q_INVOKABLE bool getFriendList(const QVariantHash& data);

    Q_INVOKABLE QVariantHash sendDirectMessage(const QVariantHash& data);
    Q_INVOKABLE QVariantList getHistoryDirectMessage(const QVariantHash& data);
signals:
    void errorOccurred(const QString&);
    void dataChanged();
};