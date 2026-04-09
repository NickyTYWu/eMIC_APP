#pragma once

#include <QObject>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class SleepGuard : public QObject
{
    Q_OBJECT

public:
    explicit SleepGuard(QObject *parent = nullptr);

    void preventSleep();
    void allowSleep();

private:
    bool active = false;
};