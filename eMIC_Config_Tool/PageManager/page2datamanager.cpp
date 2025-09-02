#include "Page2datamanager.h"

Page2DataManager::Page2DataManager() {
    pageList = {
         {REG_READ_WRITE,"0x00", "PAGE_CFG"        , "0x02"},
         {REG_READ_WRITE,"0x08", "BQ1_N0_BYT1[7:0]", "0x7F"},
         {REG_READ_WRITE,"0x09", "BQ1_N0_BYT2[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x0A", "BQ1_N0_BYT3[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x0B", "BQ1_N0_BYT4[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x0C", "BQ1_N1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x0D", "BQ1_N1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x0E", "BQ1_N1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x0F", "BQ1_N1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x10", "BQ1_N2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x11", "BQ1_N2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x12", "BQ1_N2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x13", "BQ1_N2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x14", "BQ1_D1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x15", "BQ1_D1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x16", "BQ1_D1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x17", "BQ1_D1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x18", "BQ1_D2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x19", "BQ1_D2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x1A", "BQ1_D2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x1B", "BQ1_D2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x1C", "BQ2_N0_BYT1[7:0]", "0x7F"},
         {REG_READ_WRITE,"0x1D", "BQ2_N0_BYT2[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x1E", "BQ2_N0_BYT3[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x1F", "BQ2_N0_BYT4[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x20", "BQ2_N1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x21", "BQ2_N1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x22", "BQ2_N1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x23", "BQ2_N1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x24", "BQ2_N2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x25", "BQ2_N2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x26", "BQ2_N2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x27", "BQ2_N2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x28", "BQ2_D1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x29", "BQ2_D1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x2A", "BQ2_D1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x2B", "BQ2_D1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x2C", "BQ2_D2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x2D", "BQ2_D2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x2E", "BQ2_D2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x2F", "BQ2_D2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x30", "BQ3_N0_BYT1[7:0]", "0x7F"},
         {REG_READ_WRITE,"0x31", "BQ3_N0_BYT2[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x32", "BQ3_N0_BYT3[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x33", "BQ3_N0_BYT4[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x34", "BQ3_N1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x35", "BQ3_N1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x36", "BQ3_N1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x37", "BQ3_N1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x38", "BQ3_N2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x39", "BQ3_N2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x3A", "BQ3_N2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x3B", "BQ3_N2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x3C", "BQ3_D1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x3D", "BQ3_D1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x3E", "BQ3_D1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x3F", "BQ3_D1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x40", "BQ3_D2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x41", "BQ3_D2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x42", "BQ3_D2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x43", "BQ3_D2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x44", "BQ4_N0_BYT1[7:0]", "0x7F"},
         {REG_READ_WRITE,"0x45", "BQ4_N0_BYT2[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x46", "BQ4_N0_BYT3[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x47", "BQ4_N0_BYT4[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x48", "BQ4_N1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x49", "BQ4_N1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x4A", "BQ4_N1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x4B", "BQ4_N1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x4C", "BQ4_N2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x4D", "BQ4_N2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x4E", "BQ4_N2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x4F", "BQ4_N2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x50", "BQ4_D1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x51", "BQ4_D1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x52", "BQ4_D1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x53", "BQ4_D1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x54", "BQ4_D2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x55", "BQ4_D2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x56", "BQ4_D2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x57", "BQ4_D2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x58", "BQ5_N0_BYT1[7:0]", "0x7F"},
         {REG_READ_WRITE,"0x59", "BQ5_N0_BYT2[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x5A", "BQ5_N0_BYT3[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x5B", "BQ5_N0_BYT4[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x5C", "BQ5_N1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x5D", "BQ5_N1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x5E", "BQ5_N1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x5F", "BQ5_N1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x60", "BQ5_N2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x61", "BQ5_N2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x62", "BQ5_N2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x63", "BQ5_N2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x64", "BQ5_D1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x65", "BQ5_D1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x66", "BQ5_D1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x67", "BQ5_D1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x68", "BQ5_D2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x69", "BQ5_D2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x6A", "BQ5_D2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x6B", "BQ5_D2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x6C", "BQ6_N0_BYT1[7:0]", "0x7F"},
         {REG_READ_WRITE,"0x6D", "BQ6_N0_BYT2[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x6E", "BQ6_N0_BYT3[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x6F", "BQ6_N0_BYT4[7:0]", "0xFF"},
         {REG_READ_WRITE,"0x70", "BQ6_N1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x71", "BQ6_N1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x72", "BQ6_N1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x73", "BQ6_N1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x74", "BQ6_N2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x75", "BQ6_N2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x76", "BQ6_N2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x77", "BQ6_N2_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x78", "BQ6_D1_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x79", "BQ6_D1_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x7A", "BQ6_D1_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x7B", "BQ6_D1_BYT4[7:0]", "0x00"},
         {REG_READ_WRITE,"0x7C", "BQ6_D2_BYT1[7:0]", "0x00"},
         {REG_READ_WRITE,"0x7D", "BQ6_D2_BYT2[7:0]", "0x00"},
         {REG_READ_WRITE,"0x7E", "BQ6_D2_BYT3[7:0]", "0x00"},
         {REG_READ_WRITE,"0x7F", "BQ6_D2_BYT4[7:0]", "0x00"}

    };
}

int Page2DataManager::count() const { return pageList.size(); }

PageElement Page2DataManager::get(int index) const {
    return (index >= 0 && index < pageList.size()) ? pageList.at(index) : PageElement();
}

void Page2DataManager::set(int index, const PageElement& element) {
    if (index >= 0 && index < pageList.size()) {
        pageList[index] = element;
    }
}

uint8_t Page2DataManager::getProperty(int index) const { return get(index).Property; }
QString Page2DataManager::getAddress(int index) const { return get(index).Address; }
QString Page2DataManager::getAcronym(int index) const { return get(index).Acronym; }
QString Page2DataManager::getValue(int index) const { return get(index).Value; }

void Page2DataManager::setValue(int index, const QString& hexValue) {
    if (index >= 0 && index < pageList.size()) {
        pageList[index].Value = hexValue.toUpper();
    }
}

void Page2DataManager::setValueFromByte(int index, uint8_t byteValue) {
    QString hex = QString("0x%1").arg(byteValue, 2, 16, QLatin1Char('0')).toUpper();
    setValue(index, hex);
}

uint8_t Page2DataManager::getValueAsByte(int index) const {
    bool ok = false;
    QString valStr = getValue(index);
    return static_cast<uint8_t>(valStr.toUInt(&ok, 16));
}

int Page2DataManager::findIndexByAcronym(const QString& acronym) const {
    for (int i = 0; i < pageList.size(); ++i) {
        if (pageList[i].Acronym == acronym)
            return i;
    }
    return -1;
}

const QList<PageElement>& Page2DataManager::getAll() const {
    return pageList;
}
