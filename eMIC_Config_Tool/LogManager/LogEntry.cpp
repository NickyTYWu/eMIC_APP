#include "LogEntry.h"
#include <QJsonDocument>
#include <QJsonObject>

QString LogEntry::toPlainText() const {
    return QString("[%1] [%2] %3")
    .arg(timestamp.toString("yyyy-MM-dd hh:mm:ss"))
        .arg(logLevelToString(level))
        .arg(message);
}

QString LogEntry::toJsonString() const {
    QJsonObject obj;
    obj["timestamp"] = timestamp.toString(Qt::ISODate);
    obj["level"] = logLevelToString(level);
    obj["message"] = message;
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QString LogEntry::toCsvString() const {
    QString m = message;
    m.replace("\"", "\"\"");
    return QString("\"%1\",\"%2\",\"%3\"")
        .arg(timestamp.toString(Qt::ISODate))
        .arg(logLevelToString(level))
        .arg(m);
}
