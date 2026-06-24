#ifndef SENSORCARD_H
#define SENSORCARD_H

#include <QFrame>

class QLabel;

class SensorCard : public QFrame
{
    Q_OBJECT

public:
    explicit SensorCard(const QString &title,
                        const QString &unit1,
                        const QString &unit2,
                        QWidget *parent = nullptr);

    void setValue1(float value);
    void setValue2(float value);
    void clearValue1();
    void clearValue2();
    void setDeviceId(QString value);
    void clearDeviceId();


private:
    QLabel *m_titleLabel;

    QLabel *m_deviceidLabel;

    QLabel *m_value1Label;
    QLabel *m_unit1Label;

    QLabel *m_value2Label;
    QLabel *m_unit2Label;

    QString m_unit1;
    QString m_unit2;
};

#endif // SENSORCARD_H
