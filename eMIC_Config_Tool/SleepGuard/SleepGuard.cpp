#include "SleepGuard.h"
#include <QCoreApplication>

SleepGuard::SleepGuard(QObject *parent)
    : QObject(parent)
{
    QObject::connect(
        qApp,
        &QCoreApplication::aboutToQuit,
        this,
        &SleepGuard::allowSleep
    );
}

void SleepGuard::preventSleep()
{
#ifdef Q_OS_WIN
    if(!active)
    {
        SetThreadExecutionState(
            ES_CONTINUOUS |
            ES_SYSTEM_REQUIRED |
            ES_DISPLAY_REQUIRED
        );
        active = true;
    }
#endif
}

void SleepGuard::allowSleep()
{
#ifdef Q_OS_WIN
    if(active)
    {
        SetThreadExecutionState(ES_CONTINUOUS);
        active = false;
    }
#endif
}