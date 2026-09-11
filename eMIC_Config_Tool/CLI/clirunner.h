#ifndef CLIRUNNER_H
#define CLIRUNNER_H

#include <QObject>
#include <QStringList>
#include "mainwindow.h"

class CliRunner : public QObject
{
    Q_OBJECT
public:
    explicit CliRunner(const QStringList &args, QObject *parent = nullptr);
     ~CliRunner();
public slots:
    void run();

signals:
    void finished(bool success);

private:
    QStringList m_args;
    MainWindow* m_w;
    void doFinished(bool success);
};

#endif // CLIRUNNER_H
