#ifndef __Command_H__
#define __Command_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

#define RCV_CMD_MAX_LEN 132

#define PAGE0_REG_MAX_SIZE 49
#define PAGE1_REG_MAX_SIZE 2
#define PAGE2_REG_MAX_SIZE 121
#define PAGE3_REG_MAX_SIZE 121
#define PAGE4_REG_MAX_SIZE 77

#define hBYTE 0x55
#define flash_base_address 0x08000000
#define Bootloader1_StartAddress 0x00000000
#define Bootloader1_EndAddress   0x00001FFF

#define APP_StartAddress 0x00002000
#define APP_EndAddress   0x0000FBFF

#define Version_StartAddress 0x0000FC00
#define Version_EndAddress   0x0000FFFF

#define MAXBUFFERSIZE   256


#define CMD_FAIL                     0x00
#define CMD_SUCCESS                  0x01
#define CMD_CHECKSUMERROR            0x02
#define CMD_PAGE0_WRITE_SUCCESS      0x03
#define CMD_PAGE2_WRITE_SSUCCESS     0x04
#define CMD_PAGE3_WRITE_SSUCCESS     0x05
#define CMD_PAGE4_WRITE_SSUCCESS     0x06
#define CMD_TBD1_WRITE_SSUCCESS      0x07
#define CMD_TBD2_WRITE_SSUCCESS      0x08
#define CMD_INFO1_WRITE_SSUCCESS     0x09
#define CMD_INFO2_WRITE_SSUCCESS     0x0A

#define STARTBYTE         0x00
#define PAYLOADLEN        0x01
#define CMDID             0x02
#define CMDDATA           0x03


#define BLOCK_ID_PAGE0 0x00
#define BLOCK_ID_PAGE2 0x01
#define BLOCK_ID_PAGE3 0x02
#define BLOCK_ID_PAGE4 0x03
#define BLOCK_ID_TBD1  0x04
#define BLOCK_ID_TBD2  0x05
#define BLOCK_ID_INFO1 0x06
#define BLOCK_ID_INFO2 0x07

#define ACK_CMD                                          0x00
#define GET_FW_VERSION_CMD                               0x01
#define GET_FW_VERSION_RESPONSE_CMD                      0x02
#define GET_TH_CMD                                       0x03
#define GET_TH_RESPONSE_CMD                              0x04
#define WRITE_EEPROM_BLOCK_CMD                           0x05
#define READ_EEPROM_BLOCK_CMD                            0x06
#define READ_EEPROM_BLOCK_RESPONSE_CMD                   0x07
#define WRITE_EEPROM_CMD                                 0x08
#define READ_EEPROM_CMD                                  0x09
#define READ_EEPROM_RESPONSE_CMD                         0x0A
#define WRITE_PCMD_BLOCK_CMD                             0x0B
#define READ_PCMD_BLOCK_CMD                              0x0C
#define READ_PCMD_BLOCK_RESPONSE_CMD                     0x0D
#define WRITE_PCMD_REG_CMD                               0x0E
#define READ_PCMD_REG_CMD                                0x0F
#define READ_PCMD_REG_RESPONSE_CMD                       0x10
#define WRITE_PCMD_REG_WITH_PAGE_PARAMETERS_CMD          0x11
#define READ_PCMD_REG_WITH_PAGE_PARAMETERS_CMD           0x12
#define READ_PCMD_REG_WITH_PAGE_PARAMETERS_RESPONSE_CMD  0x13
#define REBOOT_CMD                                       0xAA
#define GATMODE_CMD                                      0xB0
#define OTA_UPGRADE_INIT_CMD                             0xB2
#define OTA_UPGRADE_START_CMD                            0xB3
#define OTA_UPGRADE_END_CMD                              0xB4
#define DEBUG_MSG_RESPONE_CMD                            0xFF
  
  
  
  
  
  
  
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif
