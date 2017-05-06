#include "Arduino.h"
//Pin definition


// Register Read Commands
#define RREG    0x20    //Read n nnnn registers starting at address r rrrr
//first byte 001r rrrr (2xh)(2) - second byte 000n nnnn(2)
#define WREG    0x40    //Write n nnnn registers starting at address r rrrr
//first byte 010r rrrr (2xh)(2) - second byte 000n nnnn(2)

// System Commands
#define WAKEUP    0x02    //Wake-up from standby mode
#define STANDBY 0x04    //Enter standby mode
#define RESET   0x06    //Reset the device
#define START   0x08    //Start/restart (synchronize) conversions
#define STOP    0x0A    //Stop conversion

// Data Read Commands
#define RDATAC    0x10    //Enable Read Data Continuous mode.
//This mode is the default mode at power-up.
#define SDATAC    0x11    //Stop Read Data Continuously mode
#define RDATA   0x12    //Read data by command; supports multiple read back.

#define ADS1292_REG_ID      0x00
#define ADS1292_REG_CONFIG1   0x01
#define ADS1292_REG_CONFIG2   0x02
#define ADS1292_REG_LOFF    0x03
#define ADS1292_REG_CH1SET    0x04
#define ADS1292_REG_CH2SET    0x05
#define ADS1292_REG_RLDSENS   0x06
#define ADS1292_REG_LOFFSENS  0x07
#define ADS1292_REG_LOFFSTAT  0x08
