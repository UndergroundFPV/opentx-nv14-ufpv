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

#include "i2c_driver.h"
#include "board.h"
#include "debug.h"

#if !defined(UNUSED)
#define UNUSED(x)	((void)(x))
#endif

i2cData_t i2cData;

bool i2cErrorHandler()
{
  return i2cInit();
}

bool i2cWaitEvent(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (!I2C_CheckEvent(I2C_I2Cx, event)) {
    if (!(--timeout))
      return false;
  }
  return true;
}

bool i2cWaitEventCleared(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (I2C_CheckEvent(I2C_I2Cx, event)) {
    if (!(--timeout))
      return false;
  }
  return true;
}

bool i2cWaitFlagCleared(uint32_t flag)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (I2C_GetFlagStatus(I2C_I2Cx, flag)) {
    if (!(--timeout))
      return false;
  }
  return true;

}

bool i2cWaitBusReady()
{
  if (!i2cWaitFlagCleared(I2C_FLAG_BUSY)) {
    TRACE_I2C_ERR("i2cWaitBusReady(): Bus timeout, I2C_FLAG_BUSY still set.");
    return I2C_ERROR_HANDLER();
  }

  return true;
}

void i2cDataInit() {
  if (i2cData.dataInit)
    return;

  i2cData.dataInit = true;
  i2cData.hwInit = false;
  i2cData.dmaRxCallback = NULL;
  i2cData.dmaTxCallback = NULL;
}

void i2cDmaInit(uint32_t direction)
{
  I2C_DMACmd(I2C_I2Cx, DISABLE);

  DMA_InitTypeDef DMA_InitStructure;
  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&I2C_I2Cx->DR);
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
  DMA_InitStructure.DMA_DIR = direction;
  // these are set right before a transfer request, in i2cDmaSetBuffer()
  DMA_InitStructure.DMA_Memory0BaseAddr = 0;
  DMA_InitStructure.DMA_BufferSize = 0xFFFF;

  if (direction == DMA_DIR_PeripheralToMemory) {
#if defined(I2C_DMA) && defined(I2C_DMA_RX_Stream)
    DMA_InitStructure.DMA_Channel = I2C_DMA_RX_Channel;
    DMA_ClearFlag(I2C_DMA_RX_Stream, I2C_DMA_RX_FLAG_FEIF | I2C_DMA_RX_FLAG_DMEIF | I2C_DMA_RX_FLAG_TEIF | I2C_DMA_RX_FLAG_HTIF | I2C_DMA_RX_FLAG_TCIF);
    DMA_Cmd(I2C_DMA_RX_Stream, DISABLE);
    DMA_DeInit(I2C_DMA_RX_Stream);
    DMA_Init(I2C_DMA_RX_Stream, &DMA_InitStructure);
#endif
  }
  else {
#if defined(I2C_DMA) && defined(I2C_DMA_TX_Stream)
    DMA_InitStructure.DMA_Channel = I2C_DMA_TX_Channel;
    DMA_ClearFlag(I2C_DMA_TX_Stream, I2C_DMA_TX_FLAG_FEIF | I2C_DMA_TX_FLAG_DMEIF | I2C_DMA_TX_FLAG_TEIF | I2C_DMA_TX_FLAG_HTIF | I2C_DMA_TX_FLAG_TCIF);
    DMA_Cmd(I2C_DMA_TX_Stream, DISABLE);
    DMA_DeInit(I2C_DMA_TX_Stream);
    DMA_Init(I2C_DMA_TX_Stream, &DMA_InitStructure);
#endif
  }
}

bool i2cHwInit()
{
  i2cData.hwInit = false;
  I2C_Cmd(I2C_I2Cx, DISABLE);
  I2C_DeInit(I2C_I2Cx);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = I2C_SCL_GPIO_PIN | I2C_SDA_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;  // GPIO_Speed_50MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2C_GPIO, &GPIO_InitStructure);

  GPIO_PinAFConfig(I2C_GPIO, I2C_SCL_GPIO_PinSource, I2C_GPIO_AF);
  GPIO_PinAFConfig(I2C_GPIO, I2C_SDA_GPIO_PinSource, I2C_GPIO_AF);

#if defined(I2C_DMA) && defined(I2C_DMA_RX_Stream)
  i2cDmaInit(DMA_DIR_PeripheralToMemory);
#endif
#if defined(I2C_DMA) && defined(I2C_DMA_TX_Stream)
  i2cDmaInit(DMA_DIR_MemoryToPeripheral);
#endif

  I2C_InitTypeDef I2C_InitStructure;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

  //I2C_AnalogFilterCmd(I2C_I2Cx, ENABLE);
  //I2C_DigitalFilterConfig(I2C_I2Cx, 0x0F);
  I2C_DeInit(I2C_I2Cx);
  I2C_Cmd(I2C_I2Cx, ENABLE);
  I2C_Init(I2C_I2Cx, &I2C_InitStructure);

  delay_us(100);
  i2cData.hwInit = i2cWaitBusReady();
  return i2cData.hwInit;
}

bool i2cInit()
{
  i2cDataInit();

  uint16_t maxTries = I2C_BUS_RST_MAX;
  while (!i2cHwInit()) {
    // if recovering from an error, we may need to retry the bus several times
    if (!(--maxTries)) {
      TRACE_I2C_ERR("i2cInit(): Init failed, bus hung on startup.");
      return false;
    }
    // re-init
    I2C_GenerateSTOP(I2C_I2Cx, ENABLE);
    I2C_SoftwareResetCmd(I2C_I2Cx, ENABLE);
    I2C_SoftwareResetCmd(I2C_I2Cx, DISABLE);
  }

  TRACE("[I2C] i2cInit() (@ %dHz)", I2C_SPEED);
  return true;
}

bool i2cWaitStandbyState(uint8_t addr)
{
  if (!i2cWaitBusReady())
    return false;

  uint32_t maxTries = I2C_TIMEOUT_MAX;
  do {
    if (!(--maxTries)) {
      TRACE_I2C_ERR("i2cWaitStandbyState(): Maximum tries exceeded.");
      return I2C_ERROR_HANDLER();
    }

    I2C_GenerateSTART(I2C_I2Cx, ENABLE);
    if (!i2cWaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) {
      TRACE_I2C_ERR("i2cWaitStandbyState(%d): !i2cWaitEvent(I2C_EVENT_MASTER_MODE_SELECT)", addr);
      return false;
    }

    I2C_Send7bitAddress(I2C_I2Cx, addr, I2C_Direction_Transmitter);
  } while (!i2cWaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_GenerateSTOP(I2C_I2Cx, ENABLE);
  return true;
}

bool i2cPrepareWrite(uint8_t addr, uint16_t loc, uint8_t regSz)
{
  if (!i2cWaitBusReady())
    return false;

  I2C_GenerateSTART(I2C_I2Cx, ENABLE);
  if (!i2cWaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) {
    TRACE_I2C_ERR("i2cPrepareWrite(%d, %d, %d): !i2cWaitEvent(MASTER_MODE_SELECTED)", addr, loc, regSz);
    return false;
  }

  I2C_Send7bitAddress(I2C_I2Cx, addr, I2C_Direction_Transmitter);
  if (!i2cWaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
    TRACE_I2C_ERR("i2cPrepareWrite(%d, %d, %d): !i2cWaitEvent(MASTER_TX_MODE_SELECTED)", addr, loc, regSz);
    return false;
  }

  if (regSz == 16) {
    I2C_SendData(I2C_I2Cx, (uint8_t)((loc & 0xFF00) >> 8));
    if (!i2cWaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) {
      TRACE_I2C_ERR("i2cPrepareWrite(%d, %d, %d): !i2cWaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING) on destination address", addr, loc, regSz);
      return false;
    }
  }
  I2C_SendData(I2C_I2Cx, (uint8_t)(loc & 0xFF));
  if (!i2cWaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
    TRACE_I2C_ERR("i2cPrepareWrite(%d, %d, %d): !i2cWaitEvent(MASTER_BYTE_TRANSMITTED) on destination address", addr, loc, regSz);
    return false;
  }

  return true;
}

bool i2cWrite(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t * pBuffer, uint16_t len)
{
  if (!i2cPrepareWrite(addr, loc, regSz))
    return false;

  while (pBuffer && len) {
    I2C_SendData(I2C_I2Cx, *pBuffer++);
    --len;

    if (!i2cWaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
      TRACE_I2C_ERR("i2cWrite(%d, %d, %d): !i2cWaitEvent(MASTER_BYTE_TRANSMITTED)", addr, loc, len);
      return false;
    }
  }

  I2C_GenerateSTOP(I2C_I2Cx, ENABLE);
  return true;
}

bool i2cWriteByte(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t data)
{
  uint8_t tmp = data;
  return i2cWrite(addr, loc, regSz, &tmp, 1);
}

bool i2cPrepareRead(uint8_t addr, uint16_t loc, uint8_t regSz)
{
  if (!i2cWrite(addr, loc, regSz, 0, 0))
    return false;

  I2C_GenerateSTART(I2C_I2Cx, ENABLE);
  if (!i2cWaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) {
    TRACE_I2C_ERR("i2cPrepareRead(%d, %d): !i2cWaitEvent(MASTER_MODE_SELECTED)", addr, loc);
    return false;
  }

  I2C_Send7bitAddress(I2C_I2Cx, addr, I2C_Direction_Receiver);
  if (!i2cWaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
    TRACE_I2C_ERR("i2cPrepareRead(%d, %d): !i2cWaitEvent(MASTER_RX_MODE_SELECTED)", addr, loc);
    return false;
  }

  return true;
}

bool i2cRead(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t * pBuffer, uint16_t len)
{
  if (!i2cPrepareRead(addr, loc, regSz))
    return false;

  while (pBuffer && len) {
    I2C_AcknowledgeConfig(I2C_I2Cx, len > 1 ? ENABLE : DISABLE);

    if (!i2cWaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) {
      TRACE_I2C_ERR("i2cRead(%d, %d, %d): !i2cWaitEvent(MASTER_BYTE_RECEIVED)", addr, loc, len);
      return false;
    }

    *pBuffer++ = I2C_ReceiveData(I2C_I2Cx);
    --len;
  }
  I2C_GenerateSTOP(I2C_I2Cx, ENABLE);

  return true;
}

uint8_t i2cReadByte(uint8_t addr, uint16_t loc, uint8_t regSz)
{
  uint8_t tmp;
  if (i2cRead(addr, loc, regSz, &tmp, 1))
    return tmp;
  else
    return 0;
}

// TODO: DMA write untested
bool i2cDmaWrite(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t * pBuffer, uint16_t len, bool oneShot, i2cDmaCallback_t * callback)
{
#if defined(I2C_DMA) && defined(I2C_DMA_TX_Stream)
  if (!i2cPrepareWrite(addr, loc, regSz))
    return false;

  I2C_DMA_TX_Stream->NDTR = (uint32_t)len;  // BufferSize;
  I2C_DMA_TX_Stream->M0AR = CONVERT_PTR_UINT(pBuffer);  // Memory0BaseAddr;

  DMA_Cmd(I2C_DMA_TX_Stream, DISABLE);

  if (oneShot)
    DMA_SetCurrDataCounter(I2C_DMA_TX_Stream, len);

  if (callback) {
    DMA_ITConfig(I2C_DMA_TX_Stream, DMA_IT_TC, ENABLE);
    NVIC_EnableIRQ(I2C_DMA_TX_IRQn);
    NVIC_SetPriority(I2C_DMA_TX_IRQn, I2C_DMA_TX_IRQPriority);
  }
  else {
    DMA_ITConfig(I2C_DMA_TX_Stream, DMA_IT_TC, DISABLE);
  }
  i2cData.i2cDmaTxCallback = callback;

  I2C_DMALastTransferCmd(I2C_I2Cx, ENABLE);
  DMA_Cmd(I2C_DMA_TX_Stream, ENABLE);
  I2C_DMACmd(I2C_I2Cx, ENABLE);
  return true;
#else
  UNUSED(addr); UNUSED(loc); UNUSED(regSz); UNUSED(pBuffer); UNUSED(len); UNUSED(oneShot); UNUSED(callback);
  return false;
#endif
}

bool i2cDmaRead(uint8_t addr, uint16_t loc, uint8_t regSz, uint8_t * pBuffer, uint16_t len, bool oneShot, i2cDmaCallback_t * callback)
{
#if defined(I2C_DMA) && defined(I2C_DMA_RX_Stream)
  if (!i2cPrepareRead(addr, loc, regSz))
    return false;

  I2C_DMA_RX_Stream->NDTR = (uint32_t)len;  // BufferSize;
  I2C_DMA_RX_Stream->M0AR = CONVERT_PTR_UINT(pBuffer);  // Memory0BaseAddr;

  DMA_Cmd(I2C_DMA_RX_Stream, DISABLE);

  if (oneShot)
    DMA_SetCurrDataCounter(I2C_DMA_RX_Stream, len);

  if (callback) {
    DMA_ITConfig(I2C_DMA_RX_Stream, DMA_IT_TC, ENABLE);
    NVIC_EnableIRQ(I2C_DMA_RX_IRQn);
    NVIC_SetPriority(I2C_DMA_RX_IRQn, I2C_DMA_RX_IRQPriority);
  }
  else {
    DMA_ITConfig(I2C_DMA_RX_Stream, DMA_IT_TC, DISABLE);
  }
  i2cData.dmaRxCallback = callback;

  if (len > 1)
    I2C_AcknowledgeConfig(I2C_I2Cx, ENABLE);
  I2C_DMALastTransferCmd(I2C_I2Cx, ENABLE);
  DMA_Cmd(I2C_DMA_RX_Stream, ENABLE);
  I2C_DMACmd(I2C_I2Cx, ENABLE);
  return true;
#else
  UNUSED(addr); UNUSED(loc); UNUSED(regSz); UNUSED(pBuffer); UNUSED(len); UNUSED(oneShot); UNUSED(callback);
  return false;
#endif
}

#if defined(I2C_DMA) && defined(I2C_DMA_RX_IRQHandler)
extern "C" void I2C_DMA_RX_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_I2C_DMA_RX);
  if (DMA_GetFlagStatus(I2C_DMA_RX_Stream, I2C_DMA_RX_FLAG_TCIF) != RESET) {
    I2C_GenerateSTOP(I2C_I2Cx, ENABLE);
    DMA_Cmd(I2C_DMA_RX_Stream, DISABLE);
    //I2C_DMACmd(I2C_I2Cx, DISABLE);
    DMA_ClearFlag(I2C_DMA_RX_Stream, I2C_DMA_RX_FLAG_TCIF);

    if (i2cData.dmaRxCallback)
      i2cData.dmaRxCallback();
  }
}
#endif  // defined(I2C_DMA) && defined(I2C_DMA_RX_IRQHandler)

#if defined(I2C_DMA) && defined(I2C_DMA_TX_IRQHandler)
extern "C" void I2C_DMA_TX_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_I2C_DMA_TX);
  if (DMA_GetFlagStatus(I2C_DMA_TX_Stream, I2C_DMA_TX_FLAG_TCIF) != RESET) {
    DMA_Cmd(I2C_DMA_TX_Stream, DISABLE);
    //I2C_DMACmd(I2C_I2Cx, DISABLE);
    DMA_ClearFlag(I2C_DMA_TX_Stream, I2C_DMA_TX_FLAG_TCIF);

    // Wait till all data have been physically transferred on the bus
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
      TRACE_I2C_ERR("I2C_DMA_TX_IRQHandler(): !i2cWaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)");
      return;
    }

    I2C_GenerateSTOP(I2C_I2Cx, ENABLE);

    if (i2cData.i2cDmaTxCallback)
      i2cData.i2cDmaTxCallback();
  }
}
#endif  // defined(I2C_DMA) && defined(I2C_DMA_TX_IRQHandler)
