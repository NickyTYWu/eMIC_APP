#include "Page3datamanager.h"

Page3DataManager::Page3DataManager() {
    pageList = {
        {REG_READ_WRITE, "0x03", "PAGE_CFG"        , "0x03"},
        {REG_READ_WRITE, "0x08", "BQ7_N0_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE, "0x09", "BQ7_N0_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x0A", "BQ7_N0_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x0B", "BQ7_N0_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x0C", "BQ7_N1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x0D", "BQ7_N1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x0E", "BQ7_N1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x0F", "BQ7_N1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x10", "BQ7_N2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x11", "BQ7_N2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x12", "BQ7_N2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x13", "BQ7_N2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x14", "BQ7_D1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x15", "BQ7_D1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x16", "BQ7_D1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x17", "BQ7_D1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x18", "BQ7_D2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x19", "BQ7_D2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x1A", "BQ7_D2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x1B", "BQ7_D2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x1C", "BQ8_N0_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE, "0x1D", "BQ8_N0_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x1E", "BQ8_N0_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x1F", "BQ8_N0_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x20", "BQ8_N1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x21", "BQ8_N1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x22", "BQ8_N1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x23", "BQ8_N1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x24", "BQ8_N2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x25", "BQ8_N2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x26", "BQ8_N2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x27", "BQ8_N2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x28", "BQ8_D1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x29", "BQ8_D1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x2A", "BQ8_D1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x2B", "BQ8_D1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x2C", "BQ8_D2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x2D", "BQ8_D2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x2E", "BQ8_D2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x2F", "BQ8_D2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x30", "BQ9_N0_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE, "0x31", "BQ9_N0_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x32", "BQ9_N0_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x33", "BQ9_N0_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x34", "BQ9_N1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x35", "BQ9_N1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x36", "BQ9_N1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x37", "BQ9_N1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x38", "BQ9_N2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x39", "BQ9_N2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x3A", "BQ9_N2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x3B", "BQ9_N2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x3C", "BQ9_D1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x3D", "BQ9_D1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x3E", "BQ9_D1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x3F", "BQ9_D1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x40", "BQ9_D2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x41", "BQ9_D2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x42", "BQ9_D2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x43", "BQ9_D2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x44", "BQ10_N0_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE, "0x45", "BQ10_N0_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x46", "BQ10_N0_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x47", "BQ10_N0_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x48", "BQ10_N1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x49", "BQ10_N1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x4A", "BQ10_N1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x4B", "BQ10_N1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x4C", "BQ10_N2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x4D", "BQ10_N2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x4E", "BQ10_N2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x4F", "BQ10_N2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x50", "BQ10_D1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x51", "BQ10_D1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x52", "BQ10_D1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x53", "BQ10_D1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x54", "BQ10_D2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x55", "BQ10_D2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x56", "BQ10_D2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x57", "BQ10_D2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x58", "BQ11_N0_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE, "0x59", "BQ11_N0_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x5A", "BQ11_N0_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x5B", "BQ11_N0_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x5C", "BQ11_N1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x5D", "BQ11_N1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x5E", "BQ11_N1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x5F", "BQ11_N1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x60", "BQ11_N2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x61", "BQ11_N2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x62", "BQ11_N2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x63", "BQ11_N2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x64", "BQ11_D1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x65", "BQ11_D1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x66", "BQ11_D1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x67", "BQ11_D1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x68", "BQ11_D2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x69", "BQ11_D2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x6A", "BQ11_D2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x6B", "BQ11_D2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x6C", "BQ12_N0_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE, "0x6D", "BQ12_N0_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x6E", "BQ12_N0_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x6F", "BQ12_N0_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE, "0x70", "BQ12_N1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x71", "BQ12_N1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x72", "BQ12_N1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x73", "BQ12_N1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x74", "BQ12_N2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x75", "BQ12_N2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x76", "BQ12_N2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x77", "BQ12_N2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x78", "BQ12_D1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x79", "BQ12_D1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x7A", "BQ12_D1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x7B", "BQ12_D1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE, "0x7C", "BQ12_D2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE, "0x7D", "BQ12_D2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE, "0x7E", "BQ12_D2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE, "0x7F", "BQ12_D2_BYT4[7:0]", "0x00"}

    };
}

int Page3DataManager::count() const { return pageList.size(); }

PageElement Page3DataManager::get(int index) const {
    return (index >= 0 && index < pageList.size()) ? pageList.at(index) : PageElement();
}

void Page3DataManager::set(int index, const PageElement& element) {
    if (index >= 0 && index < pageList.size()) {
        pageList[index] = element;
    }
}

uint8_t Page3DataManager::getProperty(int index) const { return get(index).Property; }
QString Page3DataManager::getAddress(int index) const { return get(index).Address; }
QString Page3DataManager::getAcronym(int index) const { return get(index).Acronym; }
QString Page3DataManager::getValue(int index) const { return get(index).Value; }

void Page3DataManager::setValue(int index, const QString& hexValue) {
    if (index >= 0 && index < pageList.size()) {
        pageList[index].Value = hexValue.toUpper();
    }
}

void Page3DataManager::setValueFromByte(int index, uint8_t byteValue) {
    QString hex = QString("0x%1").arg(byteValue, 2, 16, QLatin1Char('0')).toUpper();
    setValue(index, hex);
}

uint8_t Page3DataManager::getValueAsByte(int index) const {
    bool ok = false;
    QString valStr = getValue(index);
    return static_cast<uint8_t>(valStr.toUInt(&ok, 16));
}

int Page3DataManager::findIndexByAcronym(const QString& acronym) const {
    for (int i = 0; i < pageList.size(); ++i) {
        if (pageList[i].Acronym == acronym)
            return i;
    }
    return -1;
}

const QList<PageElement>& Page3DataManager::getAll() const {
    return pageList;
}
