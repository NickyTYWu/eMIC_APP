#include "Page4datamanager.h"

Page4DataManager::Page4DataManager() {
    pageList = {
        {REG_READ_WRITE,"0x00", "PAGE_CFG"          , "0x04"},
        {REG_READ_WRITE,"0x08", "MIX1_CH1_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE,"0x09", "MIX1_CH1_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x0A", "MIX1_CH1_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x0B", "MIX1_CH1_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x0C", "MIX1_CH2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x0D", "MIX1_CH2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x0E", "MIX1_CH2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x0F", "MIX1_CH2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x10", "MIX1_CH3_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x11", "MIX1_CH3_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x12", "MIX1_CH3_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x13", "MIX1_CH3_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x14", "MIX1_CH4_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x15", "MIX1_CH4_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x16", "MIX1_CH4_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x17", "MIX1_CH4_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x18", "MIX2_CH1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x19", "MIX2_CH1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x1A", "MIX2_CH1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x1B", "MIX2_CH1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x1C", "MIX2_CH2_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE,"0x1D", "MIX2_CH2_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x1E", "MIX2_CH2_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x1F", "MIX2_CH2_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x20", "MIX2_CH3_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x21", "MIX2_CH3_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x22", "MIX2_CH3_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x23", "MIX2_CH3_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x24", "MIX2_CH4_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x25", "MIX2_CH4_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x26", "MIX2_CH4_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x27", "MIX2_CH4_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x28", "MIX3_CH1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x29", "MIX3_CH1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x2A", "MIX3_CH1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x2B", "MIX3_CH1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x2C", "MIX3_CH2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x2D", "MIX3_CH2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x2E", "MIX3_CH2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x2F", "MIX3_CH2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x30", "MIX3_CH3_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE,"0x31", "MIX3_CH3_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x32", "MIX3_CH3_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x33", "MIX3_CH3_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x34", "MIX3_CH4_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x35", "MIX3_CH4_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x36", "MIX3_CH4_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x37", "MIX3_CH4_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x38", "MIX4_CH1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x39", "MIX4_CH1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x3A", "MIX4_CH1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x3B", "MIX4_CH1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x3C", "MIX4_CH2_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x3D", "MIX4_CH2_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x3E", "MIX4_CH2_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x3F", "MIX4_CH2_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x40", "MIX4_CH3_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x41", "MIX4_CH3_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x42", "MIX4_CH3_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x43", "MIX4_CH3_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x44", "MIX4_CH4_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE,"0x45", "MIX4_CH4_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x46", "MIX4_CH4_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x47", "MIX4_CH4_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x48", "IIR_N0_BYT1[7:0]", "0x7F"},
        {REG_READ_WRITE,"0x49", "IIR_N0_BYT2[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x4A", "IIR_N0_BYT3[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x4B", "IIR_N0_BYT4[7:0]", "0xFF"},
        {REG_READ_WRITE,"0x4C", "IIR_N1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x4D", "IIR_N1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x4E", "IIR_N1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x4F", "IIR_N1_BYT4[7:0]", "0x00"},
        {REG_READ_WRITE,"0x50", "IIR_D1_BYT1[7:0]", "0x00"},
        {REG_READ_WRITE,"0x51", "IIR_D1_BYT2[7:0]", "0x00"},
        {REG_READ_WRITE,"0x52", "IIR_D1_BYT3[7:0]", "0x00"},
        {REG_READ_WRITE,"0x53", "IIR_D1_BYT4[7:0]", "0x00"}

    };
}

int Page4DataManager::count() const { return pageList.size(); }

PageElement Page4DataManager::get(int index) const {
    return (index >= 0 && index < pageList.size()) ? pageList.at(index) : PageElement();
}

void Page4DataManager::set(int index, const PageElement& element) {
    if (index >= 0 && index < pageList.size()) {
        pageList[index] = element;
    }
}

uint8_t Page4DataManager::getProperty(int index) const { return get(index).Property; }
QString Page4DataManager::getAddress(int index) const { return get(index).Address; }
QString Page4DataManager::getAcronym(int index) const { return get(index).Acronym; }
QString Page4DataManager::getValue(int index) const { return get(index).Value; }

void Page4DataManager::setValue(int index, const QString& hexValue) {
    if (index >= 0 && index < pageList.size()) {
        pageList[index].Value = hexValue.toUpper();
    }
}

void Page4DataManager::setValueFromByte(int index, uint8_t byteValue) {
    QString hex = QString("0x%1").arg(byteValue, 2, 16, QLatin1Char('0')).toUpper();
    setValue(index, hex);
}

uint8_t Page4DataManager::getValueAsByte(int index) const {
    bool ok = false;
    QString valStr = getValue(index);
    return static_cast<uint8_t>(valStr.toUInt(&ok, 16));
}

int Page4DataManager::findIndexByAcronym(const QString& acronym) const {
    for (int i = 0; i < pageList.size(); ++i) {
        if (pageList[i].Acronym == acronym)
            return i;
    }
    return -1;
}

const QList<PageElement>& Page4DataManager::getAll() const {
    return pageList;
}
