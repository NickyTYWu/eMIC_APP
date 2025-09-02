#ifndef LOGENTRY_H
#define LOGENTRY_H

#include "LogLevel.h"
#include <QString>
#include <QDateTime>

struct LogEntry {
    QDateTime timestamp;
    LogLevel level;
    QString message;

    QString toPlainText() const;
    QString toJsonString() const;
    QString toCsvString() const;
};

#endif