#include "CheckWorkerThread.h"
#include <QDebug>

CheckWorkerThread::CheckWorkerThread(QObject* parent)
    : QThread(parent)
{}

void CheckWorkerThread::run()
{
    qDebug() << "[Thread] Start waiting and checking loop";
    mutex.lock();
    while (!shouldStop) {
        bool woke = condition.wait(&mutex, 1000); // Wait or wake up every 50ms
        Q_UNUSED(woke);
        counter++;
        emit checkFunctionCalled(counter);

        //if (counter >= 20) {
        //    emit workFinished();
        //    break;
        //}
    }
    mutex.unlock();
    qDebug() << "[Thread] Mission ended";
}

void CheckWorkerThread::stop()
{
    qDebug() << "[Thread] stop";
    mutex.lock();
    shouldStop = true;
    condition.wakeOne();
    mutex.unlock();
}

void CheckWorkerThread::wake()
{
    //qDebug() << "[Thread] wake";
    mutex.lock();
    condition.wakeOne();
    mutex.unlock();
}
