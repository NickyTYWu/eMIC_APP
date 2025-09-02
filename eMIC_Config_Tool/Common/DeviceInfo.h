#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QMainWindow>
#include <cstdint>
#include <cstring>

#pragma pack(push,1)

struct DeviceInfo {

        char cartridge1ModelNumber[8];
        char cartridge1VersionLetter[1];
        char cartridge1SerialNumber[16];
        float cartridge1Sensitivity;
        float cartridge1ReferenceFrequency;
        uint8_t cartridge1UnitsCode;
        int16_t cartridge1FrequencyRangeMin;
        int16_t cartridge1FrequencyRangeMax;
        uint8_t cartridge1ChannelAssignment;

        char cartridge2ModelNumber[8];
        char cartridge2VersionLetter[1];
        char cartridge2SerialNumber[16];
        float cartridge2Sensitivity;
        float cartridge2ReferenceFrequency;
        uint8_t cartridge2UnitsCode;
        int16_t cartridge2FrequencyRangeMin;
        int16_t cartridge2FrequencyRangeMax;
        uint8_t cartridge2ChannelAssignment;

        char systemDigitInterfaceType[4];
        float systemBitClockFrequency;
        uint8_t systemWordLength;
        float systemSampleRate;
        char systemSerialnumber[16];
        float systemSensitivity;
        uint8_t systemCalibrationDate[6];
        uint16_t systemManufactuerID;
        uint16_t systemFirmwareVersion;
};


struct DeviceInfoNote {
    char note[128];
};
#pragma pack(pop)
#endif // DEVICEINFO_H
