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

#include "board.h"
#include "eeprom_driver.h"
#include "i2c_driver.h"
#include "debug.h"

/**
  * @brief  Reads a block of data from the EEPROM.
  * @param  pBuffer : pointer to the buffer that receives the data read
  *   from the EEPROM.
  * @param  ReadAddr : EEPROM's internal address to read from.
  * @param  NumByteToRead : number of bytes to read from the EEPROM.
  * @retval None
  */
void eepromReadBlock(uint8_t * buffer, size_t address, size_t size)
{
  i2cEnterMutexSection();
  if (!i2cRead(EEPROM_I2C_ADDRESS, (uint16_t)address, 16, buffer, (uint16_t)size))
    TRACE_ERROR("eeprom: I2C read failed in eepromReadBlock()!\r\n");
  i2cLeaveMutexSection();
}

uint8_t eepromIsTransferComplete()
{
  return 1;
}

/**
  * @brief  Writes more than one byte to the EEPROM with a single WRITE cycle.
  * @note   The number of byte can't exceed the EEPROM page size.
  * @param  pBuffer : pointer to the buffer containing the data to be
  *   written to the EEPROM.
  * @param  WriteAddr : EEPROM's internal address to write to.
  * @param  NumByteToWrite : number of bytes to write to the EEPROM.
  * @retval None
  */
void eepromPageWrite(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t NumByteToWrite)
{
  if (!i2cWrite(EEPROM_I2C_ADDRESS, WriteAddr, 16, pBuffer, NumByteToWrite))
    TRACE_ERROR("eeprom: I2C write failed in eepromPageWrite()!\r\n");
}

/**
  * @brief  Writes buffer of data to the I2C EEPROM.
  * @param  buffer : pointer to the buffer containing the data to be
  *   written to the EEPROM.
  * @param  address : EEPROM's internal address to write to.
  * @param  size : number of bytes to write to the EEPROM.
  * @retval None
  */
void eepromWriteBlock(uint8_t * buffer, size_t address, size_t size)
{
  uint8_t offset = address % EEPROM_PAGESIZE;
  uint8_t count = EEPROM_PAGESIZE - offset;
  if (size < count) {
    count = size;
  }
  while (count > 0) {
    i2cEnterMutexSection();
    eepromPageWrite(buffer, address, count);
    if (!i2cWaitStandbyState(EEPROM_I2C_ADDRESS)) {
      TRACE_WARNING("eeprom: eepromWriteBlock() I2C standby state failed, write may be incomplete.\r\n");
    }
    i2cLeaveMutexSection();
    address += count;
    buffer += count;
    size -= count;
    count = EEPROM_PAGESIZE;
    if (size < EEPROM_PAGESIZE) {
      count = size;
    }
  }
}
