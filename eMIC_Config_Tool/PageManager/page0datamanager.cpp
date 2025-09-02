#include "page0datamanager.h"

Page0DataManager::Page0DataManager() {
    pageList = {
        { REG_READ_WRITE,"0x00", "PAGE_CFG",     "0x00" },
        { REG_READ_WRITE,"0x01", "SW_RESET",     "0x00" },
        { REG_READ_WRITE,"0x02", "SLEEP_CFG",    "0x01" },
        { REG_READ_WRITE,"0x07", "ASI_CFG0",     "0x60" },
        { REG_READ_WRITE,"0x08", "ASI_CFG1",     "0x00" },
        { REG_READ_WRITE,"0x09", "ASI_CFG2",     "0x00" },
        { REG_READ_WRITE,"0x0A", "ASI_MIX_CFG",  "0x00" },
        { REG_READ_WRITE,"0x0B", "ASI_CH1",      "0x00" },
        { REG_READ_WRITE,"0x0C", "ASI_CH2",      "0x20" },
        { REG_READ_WRITE,"0x0D", "ASI_CH3",      "0x00" },
        { REG_READ_WRITE,"0x0E", "ASI_CH4",      "0x00" },
        { REG_READ_WRITE,"0x13", "MST_CFG0",     "0x02" },
        { REG_READ_WRITE,"0x14", "MST_CFG1",     "0x48" },
        { REG_READ_ONLY ,"0x15", "ASI_STS",      "0xFF" },
        { REG_READ_WRITE,"0x16", "CLK_SRC",      "0x10" },
        { REG_READ_WRITE,"0x1F", "PDMCLK_CFG",   "0x40" },
        { REG_READ_WRITE,"0x20", "PDMIN_CFG",    "0x00" },
        { REG_READ_WRITE,"0x21", "GPIO_CFG0",    "0x22" },
        { REG_READ_WRITE,"0x22", "GPO_CFG0",     "0x41" },
        { REG_READ_WRITE,"0x29", "GPO_VAL",      "0x00" },
        { REG_READ_ONLY ,"0x2A", "GPIO_MON",     "0x00" },
        { REG_READ_WRITE,"0x2B", "GPI_CFG0",     "0x40" },
        { REG_READ_ONLY ,"0x2F", "GPI_MON",      "0x00" },
        { REG_READ_WRITE,"0x32", "INT_CFG",      "0x00" },
        { REG_READ_WRITE,"0x33", "INT_MASK0",    "0xFF" },
        { REG_READ_ONLY ,"0x36", "INT_LTCH0",    "0x00" },
        { REG_READ_WRITE,"0x3B", "BIAS_CFG",     "0x60" },
        { REG_READ_WRITE,"0x3E", "CH1_CFG2",     "0xC9" },
        { REG_READ_WRITE,"0x3F", "CH1_CFG3",     "0x80" },
        { REG_READ_WRITE,"0x40", "CH1_CFG4",     "0x00" },
        { REG_READ_WRITE,"0x41", "CH2_CFG0",     "0x40" },
        { REG_READ_WRITE,"0x43", "CH2_CFG2",     "0xC9" },
        { REG_READ_WRITE,"0x44", "CH2_CFG3",     "0x80" },
        { REG_READ_WRITE,"0x45", "CH2_CFG4",     "0x00" },
        { REG_READ_WRITE,"0x48", "CH3_CFG2",     "0xC9" },
        { REG_READ_WRITE,"0x49", "CH3_CFG3",     "0x80" },
        { REG_READ_WRITE,"0x4A", "CH3_CFG4",     "0x00" },
        { REG_READ_WRITE,"0x4D", "CH4_CFG2",     "0xC9" },
        { REG_READ_WRITE,"0x4E", "CH4_CFG3",     "0x80" },
        { REG_READ_WRITE,"0x4F", "CH4_CFG4",     "0x00" },
        { REG_READ_WRITE,"0x6B", "DSP_CFG0",     "0x01" },
        { REG_READ_WRITE,"0x6C", "DSP_CFG1",     "0x60" },
        { REG_READ_WRITE,"0x73", "IN_CH_EN",     "0xC0" },
        { REG_READ_WRITE,"0x74", "ASI_OUT_CH_EN","0xC0" },
        { REG_READ_WRITE,"0x75", "PWR_CFG",      "0xE0" },
        { REG_READ_ONLY ,"0x76", "DEV_STS0",     "0x00" },
        { REG_READ_ONLY ,"0x77", "DEV_STS1",     "0x80" },
        { REG_UNUSED    ,"0x7E", "I2C_CKSUM",    "0x00" },

    };
}

int Page0DataManager::count() const { return pageList.size(); }

PageElement Page0DataManager::get(int index) const {
    return (index >= 0 && index < pageList.size()) ? pageList.at(index) : PageElement();
}

void Page0DataManager::set(int index, const PageElement& element) {
    if (index >= 0 && index < pageList.size()) {
        pageList[index] = element;
    }
}

uint8_t Page0DataManager::getProperty(int index) const { return get(index).Property; }
QString Page0DataManager::getAddress(int index) const { return get(index).Address; }
QString Page0DataManager::getAcronym(int index) const { return get(index).Acronym; }
QString Page0DataManager::getValue(int index) const { return get(index).Value; }

void Page0DataManager::setValue(int index, const QString& hexValue) {
    if (index >= 0 && index < pageList.size()) {
        pageList[index].Value = hexValue.toUpper();
    }
}

void Page0DataManager::setValueFromByte(int index, uint8_t byteValue) {
    QString hex = QString("0x%1").arg(byteValue, 2, 16, QLatin1Char('0')).toUpper();
    setValue(index, hex);
}

uint8_t Page0DataManager::getValueAsByte(int index) const {
    bool ok = false;
    QString valStr = getValue(index);
    return static_cast<uint8_t>(valStr.toUInt(&ok, 16));
}

int Page0DataManager::findIndexByAcronym(const QString& acronym) const {
    for (int i = 0; i < pageList.size(); ++i) {
        if (pageList[i].Acronym == acronym)
            return i;
    }
    return -1;
}

const QList<PageElement>& Page0DataManager::getAll() const {
    return pageList;
}
