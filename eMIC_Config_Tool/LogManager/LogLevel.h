#ifndef LOGLEVEL_H
#define LOGLEVEL_H
#include "qdebug.h"
#include <QString>
#include <QSet>
#include <QStringList>

enum class LogLevel { Debug, Info, Warn, Error, UartRecv, UartInfo, FWdebug };

inline QString logLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::Debug:    return "  DEBUG  ";
    case LogLevel::Info:     return "  INFO   ";
    case LogLevel::Warn:     return "  WARN   ";
    case LogLevel::Error:    return "  ERROR  ";
    case LogLevel::UartRecv: return "UART_RECV";
    case LogLevel::UartInfo: return "UART_INFO";
    case LogLevel::FWdebug:  return " FWDEBUG ";
    }
    return {};
}

inline LogLevel logLevelFromString(const QString& str) {
    QString s = str.trimmed().toUpper();
    if (s == "DEBUG")     return LogLevel::Debug;
    if (s == "INFO")      return LogLevel::Info;
    if (s == "WARN")      return LogLevel::Warn;
    if (s == "ERROR")     return LogLevel::Error;
    if (s == "UART_RECV") return LogLevel::UartRecv;
    if (s == "UART_INFO") return LogLevel::UartInfo;
    if (s == "FWDEBUG")   return LogLevel::FWdebug;
    return LogLevel::Debug; // 預設 fallback
}

inline QList<LogLevel> parseEnabledLogLevels(const QStringList& levels) {
    QList<LogLevel> enabled;

    for (const QString& levelStr : levels) {
        QString trimmed = levelStr.trimmed().toUpper();

        enabled.append(logLevelFromString(trimmed));
    }

    return enabled;
}

#endif
