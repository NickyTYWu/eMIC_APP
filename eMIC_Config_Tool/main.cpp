#include "mainwindow.h"
#include <windows.h>
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <QTimer>
#include <QDir>
#include "LogManager/LogManager.h"
#include "LogManager/LogLevel.h"
#include "clirunner.h"
#include "global.h"

QTextStream outMSG(stdout);
bool isCliMode = false;

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

    QString iniPath = QCoreApplication::applicationDirPath() + "/" + iniFileName;
    QFile iniFile(iniPath);

    if (!iniFile.exists()) {
        qDebug() << "INI does not exist; copy the backup INI from QRC.";

        // QRC 檔案路徑
        QFile qrcIni(":/default_AppConfig.ini");

        if (qrcIni.exists()) {
            if (qrcIni.open(QIODevice::ReadOnly)) {
                if (iniFile.open(QIODevice::WriteOnly)) {
                    iniFile.write(qrcIni.readAll());
                    iniFile.close();
                    qDebug() << "Successfully copied default_config.ini to the current directory.";
                } else {
                    qDebug() << "Unable to write to the target INI file:" << iniFile.errorString();
                }
                qrcIni.close();
            } else {
                qDebug() << "Unable to open QRC INI:" << qrcIni.errorString();
            }
        } else {
            qDebug() << "There is no default_config.ini in QRC.";
        }
    } else {
        qDebug() << "INI already exists:" << iniPath;
    }
}

int main(int argc, char *argv[])
{
    //bool isCliMode = false;
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--cli") == 0) {
            isCliMode = true;
            break;
        }
    }

    if (isCliMode) {
#ifdef Q_OS_WIN
        FreeConsole();

        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            freopen("CONIN$", "r", stdin);
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);

            SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
        }
#endif

        qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &) {

        });
    }

    QApplication a(argc, argv);
    //QApplication::setStyle("Fusion");

    if (!isCliMode) {
        checkINIexist();
        QSettings settings("AppConfig.ini", QSettings::IniFormat);
        int enableLog = settings.value("Log/Enable", "0").toInt();
        QStringList levelsList = settings.value("Log/Levels", "DEBUG,INFO,WARN,ERROR").toString().split(',', Qt::SkipEmptyParts);
        QList<LogLevel> enabledLevels = parseEnabledLogLevels(levelsList);
        QString format = settings.value("Log/Format", "txt12").toString();

        LogManager::instance()->setEnabledLevels(enabledLevels);
        LogManager::instance()->setLogFormat(format);

        if(enableLog) {
            qInstallMessageHandler(qtMsgHandler);
        }
    }

    QStringList args = QCoreApplication::arguments();

    if (args.contains("--cli")) {

        CliRunner* runner = new CliRunner(args, &a);

        QObject::connect(runner, &CliRunner::finished, &a, [](bool success) {
            QCoreApplication::exit(success ? 0 : -1);
        });

        QTimer::singleShot(0, runner, &CliRunner::run);

        return a.exec();
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
