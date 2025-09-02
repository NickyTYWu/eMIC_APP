#ifndef Page3DATAMANAGER_H
#define Page3DATAMANAGER_H

#include <QString>
#include <QList>
#include <QDebug>
#include "pageElement.h"


class Page3DataManager {
public:
    Page3DataManager();

    int count() const;

    PageElement get(int index) const;
    void set(int index, const PageElement& element);

    uint8_t getProperty(int index) const;
    QString getAddress(int index) const;
    QString getAcronym(int index) const;
    QString getValue(int index) const;

    void setValue(int index, const QString& hexValue);
    void setValueFromByte(int index, uint8_t byteValue);
    uint8_t getValueAsByte(int index) const;

    int findIndexByAcronym(const QString& acronym) const;

    const QList<PageElement>& getAll() const;

private:
    QList<PageElement> pageList;
};

#endif // Page3DATAMANAGER_H
