#ifndef CHECKWORKERTHREAD_H
#define CHECKWORKERTHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

class CheckWorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit CheckWorkerThread(QObject* parent = nullptr);
    void stop();
    void wake();

signals:
    void checkFunctionCalled(int count);
    void workFinished();

protected:
    void run() override;

private:
    QWaitCondition condition;
    QMutex mutex;
    bool shouldStop = false;
    int counter = 0;
};

#endif // CHECKWORKERTHREAD_H
