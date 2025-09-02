#ifndef PAGEELEMENT_H
#define PAGEELEMENT_H

#include <QString>

#define REG_UNUSED        0x00
#define REG_READ_ONLY   0x01
#define REG_READ_WRITE 0x02

struct PageElement {
    uint8_t Property;
    QString Address;
    QString Acronym;
    QString Value;

    PageElement() = default;
    PageElement(const uint8_t& property, const QString& address, const QString& acronym, const QString& value)
        : Property(property), Address(address), Acronym(acronym), Value(value) { }
};

#endif // PAGEELEMENT_H
