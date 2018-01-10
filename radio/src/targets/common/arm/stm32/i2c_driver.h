/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <stdint.h>
#include <atomic>

#ifndef   I2C_I2Cx
  #define I2C_I2Cx             I2C  // I2C peripheral
#endif
#ifndef   I2C_TIMEOUT
  #define I2C_TIMEOUT_MAX      1000
#endif
#ifndef   I2C_BUS_RST_MAX
  #define I2C_BUS_RST_MAX      300  // maximum attempts to reset a hung bus before giving up
#endif
#ifndef   I2C_ERROR_HANDLER
  #define I2C_ERROR_HANDLER()  i2cErrorHandler()
#endif

#define TRACE_I2C_ERR(f_, ...)     debugPrintf(("-E- [I2C] " f_ "\r\n"), ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

typedef void i2cDmaCallback_t (void);
typedef struct {
  bool dataInit;
  bool hwInit;
  i2cDmaCallback_t * dmaRxCallback;
  i2cDmaCallback_t * dmaTxCallback;
} i2cData_t;

bool i2cInit();
bool i2cWaitBusReady();
bool i2cWaitStandbyState(uint8_t addr);
bool i2cWrite(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t * pBuffer, uint16_t len);
bool i2cWriteByte(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t data);
bool i2cRead(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t * pBuffer, uint16_t len);
uint8_t i2cReadByte(uint8_t addr, uint16_t loc, uint8_t regSz);
bool i2cDmaWrite(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t * pBuffer, uint16_t len, bool oneShot, i2cDmaCallback_t * callback);
bool i2cDmaRead(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t * pBuffer, uint16_t len, bool oneShot, i2cDmaCallback_t * callback);

#ifdef __cplusplus
}
#endif

#endif // I2C_DRIVER_H
