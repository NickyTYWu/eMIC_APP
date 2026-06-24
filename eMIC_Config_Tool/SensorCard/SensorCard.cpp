#include "SensorCard.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QLocale>

SensorCard::SensorCard(const QString &title,
                       const QString &unit1,
                       const QString &unit2,
                       QWidget *parent)
    : QFrame(parent),
      m_unit1(unit1),
      m_unit2(unit2)
{
    setMinimumSize(100, 60);

    setStyleSheet(
        "QFrame {"
        "background:qlineargradient("
        "x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #2B2B2B,"
        "stop:1 #1E1E1E);"

        "border:1px solid rgba(255,255,255,40);"
        "border-radius:14px;"
        "}"

        "QFrame:hover {"
        "border:1px solid #00D7FF;"
        "}"
    );

    m_titleLabel = new QLabel(title);

    m_titleLabel->setStyleSheet(
        "color:#00D7FF;"
        "font-size:11px;"
        "font-weight:bold;"
        "border:none;"
    );

    //if(unit1.isEmpty()&&unit2.isEmpty())
    //{
        m_deviceidLabel= new QLabel("----------------");
    //}
    //else
    //{
        m_value1Label = new QLabel("--");
        m_value2Label = new QLabel("--");

        m_unit1Label = new QLabel(unit1);
        m_unit2Label = new QLabel(unit2);
    //}

    QString deviceIdValueStyle =
        "color:white;"
        "font-size:18px;"
        "font-weight:bold;"
        "border:none;";

    QString valueStyle =
        "color:white;"
        "font-size:14px;"
        "font-weight:bold;"
        "border:none;";

    QString unitStyle =
        "color:#AAAAAA;"
        "font-size:12px;"
        "border:none;";

    m_deviceidLabel->setStyleSheet(deviceIdValueStyle);

    m_value1Label->setStyleSheet(valueStyle);
    m_value2Label->setStyleSheet(valueStyle);

    m_unit1Label->setStyleSheet(unitStyle);
    m_unit2Label->setStyleSheet(unitStyle);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->setContentsMargins(20,5,20,5);

    mainLayout->addWidget(m_titleLabel);

    if(unit1.isEmpty()&&unit2.isEmpty())
    {
        QHBoxLayout *valueLayout = new QHBoxLayout;
        QVBoxLayout *leftLayout = new QVBoxLayout;
        leftLayout->addWidget(m_deviceidLabel);

        valueLayout->addLayout(leftLayout);

        mainLayout->addSpacing(1);
        mainLayout->addLayout(valueLayout);
        mainLayout->addStretch();
    }
    else
    {
    QHBoxLayout *valueLayout = new QHBoxLayout;

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(m_value1Label);
    leftLayout->addWidget(m_unit1Label);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(m_value2Label);
    rightLayout->addWidget(m_unit2Label);

    valueLayout->addLayout(leftLayout);
    valueLayout->addSpacing(40);
    valueLayout->addLayout(rightLayout);

    mainLayout->addSpacing(1);
    mainLayout->addLayout(valueLayout);
    mainLayout->addStretch();
    }
}

void SensorCard::setDeviceId(QString value)
{
    m_deviceidLabel->setText(value);
}

void SensorCard::setValue1(float value)
{
    m_value1Label->setText(
        QString::number(value, 'f', 3));

    //m_value1Label->setText(QLocale().toString(value));
}

void SensorCard::setValue2(float value)
{
    m_value2Label->setText(
        QString::number(value, 'f', 3));
    //m_value2Label->setText(QLocale().toString(value));
}

void SensorCard::clearValue1()
{
    m_value1Label->setText("--");
}
void SensorCard::clearValue2()
{
    m_value2Label->setText("--");
}

void SensorCard::clearDeviceId()
{
    m_deviceidLabel->setText("----------------");
}
