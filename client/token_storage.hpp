#pragma once

#include <QString>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

class TokenStorage {

public:
    static QString filePath() {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        return dir + "/token";
    }
public:
    static void save(const QString& token) {
        QFile file(filePath());
        if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(token.toUtf8());
        }
    }

    static QString load() {
        QFile file(filePath());
        if(!file.exists() || !file.open(QIODevice::ReadOnly)) {
            return "";
        }
        return QString::fromUtf8(file.readAll()).trimmed();
    }

    static void clear() {
        QFile::remove(filePath());
    }
};
