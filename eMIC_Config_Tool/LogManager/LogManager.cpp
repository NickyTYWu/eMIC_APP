#include "LogManager.h"
#include <QDateTime>
#include <QDir>
#include <QMetaObject>
#include <QDebug>
#include <QTextStream>

LogManager* LogManager::instance() {
    static LogManager inst;
    return &inst;
}

LogManager::LogManager(QObject* parent) : QObject(parent) {
    m_enabled = { LogLevel::Debug, LogLevel::Info, LogLevel::Warn, LogLevel::Error };
}

void LogManager::setOutputWidget(QTextEdit* w) {
    QMutexLocker l(&m_mutex);
    m_widget = w;
}

void LogManager::setEnabledLevels(const QList<LogLevel>& levels) {
    QMutexLocker l(&m_mutex);
    m_enabled = levels;
}

void LogManager::setLogFormat(const QString& fmt) {
    QMutexLocker l(&m_mutex);
    m_format = fmt;
}

void LogManager::rotateLogFileIfNeeded() {
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    if (today == m_curDate && m_file.isOpen()) return;
    m_curDate = today;
    QDir().mkpath(m_logDir);
    if (m_file.isOpen()) m_file.close();
    QString fn = QString("%1/log_%2.%3").arg(m_logDir, m_curDate, m_format);
    m_file.setFileName(fn);
    m_file.open(QIODevice::Append | QIODevice::Text);
}

void LogManager::closeLog()
{
    if (m_file.isOpen())
    {
        m_file.flush();
        m_file.close();
    }
}
void LogManager::log(LogLevel level, const QString& msg) {

    if (!m_enabled.contains(level))
        return;

    LogEntry e{ QDateTime::currentDateTime(), level, msg };
    QMutexLocker l(&m_mutex);
    m_entries.append(e);
    QString text = e.toPlainText();

    if (m_widget) {
        QColor col;
        switch (level) {
            case LogLevel::Debug: col = Qt::gray; break;
            case LogLevel::Info:  col = Qt::black; break;
            case LogLevel::Warn:  col = Qt::darkYellow; break;
            case LogLevel::Error: col = Qt::red; break;
            default:
                col = Qt::black; break;
                break;
        }
        QMetaObject::invokeMethod(m_widget, [this, text, col]() {
            m_widget->setTextColor(col);
            m_widget->append(text);
        }, Qt::QueuedConnection);
    }

    rotateLogFileIfNeeded();

    if (m_file.isOpen()) {
        QTextStream out(&m_file);
        if (m_format == "json")
            out << e.toJsonString() << "\n";
        else if (m_format == "csv")
            out << e.toCsvString() << "\n";
        else
            out << text << "\n";
    }

    //qDebug().noquote() << text;
}
