#include "mainwindow.h"
#include <windows.h>
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <QDir>
#include "LogManager/LogManager.h"
#include "LogManager/LogLevel.h"

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* exceptionInfo) {
    QFile f("logs/crash.txt");
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream ts(&f);
        ts << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ")
           << "Crash Detected, Exception Code: 0x"
           << QString::number(exceptionInfo->ExceptionRecord->ExceptionCode, 16)
           << "\n";
        f.flush();
    }

    return EXCEPTION_EXECUTE_HANDLER;
}


void qtMsgHandler(QtMsgType type, const QMessageLogContext&, const QString &msg) {
    switch(type) {
    case QtDebugMsg:
        LogManager::instance()->log(LogLevel::Debug, msg); break;
    case QtInfoMsg:
        LogManager::instance()->log(LogLevel::Info, msg); break;
    case QtWarningMsg:
        LogManager::instance()->log(LogLevel::Warn, msg); break;
    case QtCriticalMsg:
    case QtFatalMsg:
        LogManager::instance()->log(LogLevel::Error, msg);
        if(type == QtFatalMsg) abort();
        break;
    }
}

void checkINIexist()
{
    QString iniFileName = "AppConfig.ini";

    // 檢查當前目錄下是否已有 ini
    QString iniPath = QCoreApplication::applicationDirPath() + "/" + iniFileName;
    QFile iniFile(iniPath);

    if (!iniFile.exists()) {
        qDebug() << "INI 不存在，從 QRC 複製備份 ini";

        // QRC 檔案路徑
        QFile qrcIni(":/default_AppConfig.ini");

        if (qrcIni.exists()) {
            if (qrcIni.open(QIODevice::ReadOnly)) {
                if (iniFile.open(QIODevice::WriteOnly)) {
                    iniFile.write(qrcIni.readAll());
                    iniFile.close();
                    qDebug() << "成功複製 default_config.ini 到當前目錄";
                } else {
                    qDebug() << "無法寫入目標 INI 檔：" << iniFile.errorString();
                }
                qrcIni.close();
            } else {
                qDebug() << "無法開啟 QRC INI：" << qrcIni.errorString();
            }
        } else {
            qDebug() << "QRC 中沒有 default_config.ini";
        }
    } else {
        qDebug() << "INI 已存在：" << iniPath;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    checkINIexist();


    QSettings settings("AppConfig.ini", QSettings::IniFormat);

    int enableLog = settings.value("Log/Enable", "0").toInt();
    QStringList levelsList = settings.value("Log/Levels", "DEBUG,INFO,WARN,ERROR").toString().split(',', Qt::SkipEmptyParts);
    QList<LogLevel> enabledLevels = parseEnabledLogLevels(levelsList);
    QString format = settings.value("Log/Format", "txt12").toString();

    LogManager::instance()->setEnabledLevels(enabledLevels);
    LogManager::instance()->setLogFormat(format);

    if(enableLog)
    {
        qInstallMessageHandler(qtMsgHandler);
    }

    MainWindow w;
    //w.setWindowIcon(QIcon(":/app.ico"));
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();

    int x = (screenGeometry.width() - w.width()) / 2;
    int y = (screenGeometry.height() - w.height()) / 2;
    w.move(x, y);

    w.show();
    return a.exec();
}
