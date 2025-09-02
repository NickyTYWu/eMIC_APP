QT       += core gui
QT       += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CheckWorkerThread.cpp \
    LogManager/LogEntry.cpp \
    LogManager/LogManager.cpp \
    MCP2221/error_list.cpp \
    PageManager/page0datamanager.cpp \
    PageManager/page2datamanager.cpp \
    PageManager/page3datamanager.cpp \
    PageManager/page4datamanager.cpp \
    main.cpp \
    mainwindow.cpp


HEADERS += \
    CheckWorkerThread.h \
    Common/Command.h \
    Common/DeviceInfo.h \
    LogManager/LogEntry.h \
    LogManager/LogLevel.h \
    LogManager/LogManager.h \
    MCP2221/GetErrName.h \
    MCP2221/mcp2221_dll_um.h \
    PageManager/page0datamanager.h \
    PageManager/page2datamanager.h \
    PageManager/page3datamanager.h \
    PageManager/page4datamanager.h \
    PageManager/pageElement.h \
    mainwindow.h \

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/MCP2221/./ -lmcp2221_dll_um_x64

INCLUDEPATH += $$PWD/. \
               $$PWD/PageManager \
               $$PWD/LogManager \
               $$PWD/MCP2221 \
               $$PWD/Common
DEPENDPATH += $$PWD/. \
              $$PWD/MCP2221

RESOURCES += \
    Resources.qrc

RC_FILE += Icon.rc
