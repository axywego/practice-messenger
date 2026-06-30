#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QHash>
#include <optional>
#include "../include/codes.hpp"

class ErrorCodeHandler : public QObject {
    Q_OBJECT
public:
    explicit ErrorCodeHandler(QObject* parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE bool isOk(quint32 code) {
        ErrorCode e {code};
        return e.ok();
    }

    Q_INVOKABLE QString textError(quint32 code) {
        ErrorCode e {code};
        return QString::fromStdString(codeErrorText(e));
    }
};