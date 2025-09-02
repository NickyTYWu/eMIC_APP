#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QFile>
#include <QVector>
#include <QMutex>
#include "LogEntry.h"

class LogManager : public QObject {
    Q_OBJECT
public:
    static LogManager* instance();

    const QVector<LogEntry>& entries() const { return m_entries; }
    void setOutputWidget(QTextEdit* w);
    void setEnabledLevels(const QList<LogLevel>& levels);
    void setLogFormat(const QString& fmt); // "txt", "json", "csv"
    void log(LogLevel level, const QString& msg);
    void closeLog();
private:
    explicit LogManager(QObject* parent = nullptr);
    void rotateLogFileIfNeeded();

    QTextEdit* m_widget = nullptr;
    QList<LogLevel> m_enabled;
    QVector<LogEntry> m_entries;
    QString m_logDir = "logs";
    QString m_curDate;
    QString m_format = "txt";
    QFile m_file;
    QMutex m_mutex;
};

#endif
