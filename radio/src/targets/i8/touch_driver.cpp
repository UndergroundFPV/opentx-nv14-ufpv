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

#include "touch_driver.h"
#include "i2c_driver.h"
#include "debug.h"
#include <string.h>

touchPointRef_t touchData;
volatile bool dataMutex = false;
uint8_t touchBuffer[FT6236_READ_DATA_LEN] FT6236_BUFFER_LOC;

void touchParsePoint(uint8_t * pBuffer, uint8_t idx)
{
  touchData.evt[idx] = FT6236_EVENT_FLAG(pBuffer[0]);
  touchData.tid[idx] = FT6236_TOUCH_ID(pBuffer[2]);
  touchData.press[idx] = (touchData.evt[idx] == FT6236_EVT_PRESS || touchData.evt[idx] == FT6236_EVT_CONTACT);
  if (touchData.evt[idx] != FT6236_EVT_NONE) {
    touchData.rawPoint[idx].x = (FT6236_COORD_MSB(pBuffer[0]) | pBuffer[1]);
    touchData.rawPoint[idx].y = (FT6236_COORD_MSB(pBuffer[2]) | pBuffer[3]);

    int16_t * xptr = &(TOUCH_SWAP_XY ? touchData.point[idx].y : touchData.point[idx].x);
    int16_t * yptr = &(TOUCH_SWAP_XY ? touchData.point[idx].x : touchData.point[idx].y);

    *xptr = touchData.rawPoint[idx].x;
    *yptr = touchData.rawPoint[idx].y;

    if (TOUCH_INVERT_X)
      *xptr = TOUCH_RESOLUTION_X - *xptr;
    if (TOUCH_INVERT_Y)
      *yptr = TOUCH_RESOLUTION_Y - *yptr;
  }
}

void touchParseData(void)
{
  if (dataMutex)
    return;

  if (TOUCH_READ_MODE == 2)
    NVIC_DisableIRQ(TOUCH_INT_EXTI_IRQ);

  dataMutex = true;

  memset(&touchData, 0, FT6236_READ_DATA_LEN);

  touchData.status = FT6236_STATUS(touchBuffer[0]);  // status may be zero if a point was released

  //TRACE("stat: %d;", touchData.status);
  for (int i = 0; i < TOUCH_POINTS; ++i) {
    touchParsePoint(&touchBuffer[1 + (6*i)], i);
    //TRACE("  p[%d]: x: %3d; y: %3d; rawX: %3d; rawY: %3d; evt: %d; tid: %d; prs: %d", i, touchData.point[i].x, touchData.point[i].y, touchData.rawPoint[i].x, touchData.rawPoint[i].y, touchData.evt[i], touchData.tid[i], touchData.press[i]);
  }
  //DUMP(touchBuffer, FT6236_READ_DATA_LEN);

  touchData.dataReady = true;
  dataMutex = false;

  if (TOUCH_READ_MODE == 2)
    NVIC_EnableIRQ(TOUCH_INT_EXTI_IRQ);
}

uint8_t touchReadReg(uint8_t regAddr)
{
  return i2cReadByte(TOUCH_I2C_ADDRESS, regAddr, 8);
}

uint8_t touchWriteReg(uint8_t regAddr, uint8_t value)
{
  uint8_t ret;
  do {
    i2cWriteByte(TOUCH_I2C_ADDRESS, regAddr, 8, value);
  }
  while ((ret = touchReadReg(regAddr)) != value);
  return ret;
}

// external interrupt for data ready signal
bool touchHwInit(void)
{
  if (!i2cWaitBusReady()) {
    if (!i2cInit()) {
      TRACE_ERROR("touch: Init failed, I2C bus not ready.");
      return false;
    }
  }
  GPIO_InitTypeDef GPIO_InitStructure;

  // init interrupt pin
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = TOUCH_INT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TOUCH_INT_GPIO, &GPIO_InitStructure);

  // init reset pin
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = TOUCH_RST_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(TOUCH_RST_GPIO, &GPIO_InitStructure);

#if TOUCH_READ_MODE == 2  // interrupt mode
  SYSCFG_EXTILineConfig(TOUCH_INT_EXTI_PortSource, TOUCH_INT_GPIO_PinSource);

  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = TOUCH_INT_EXTI_Line;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TOUCH_INT_EXTI_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TOUCH_INT_EXTI_IRQPriority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif

  return true;
}

bool touchInit()
{
  if (!touchHwInit())
      return false;

  // reset touch controller
  GPIO_ResetBits(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(50);
  GPIO_SetBits(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(310);

  uint8_t pId = touchReadReg(FT6236_FOCALTECH_ID);
  if (pId != FT6236_VENDOR_ID) {
    TRACE_ERROR("touch: Init failed, FT6236 returned wrong ID (expected: 0x%X, got 0x%X).", FT6236_VENDOR_ID, pId);
    return false;
  }

  // touch sensor config
  touchWriteReg(FT6236_G_MODE, TOUCH_USE_INT_MODE);
  touchWriteReg(FT6236_CTRL, TOUCH_USE_CTRL_MODE);

  TRACE("touchInit(): FT6236 ID 0x%X; \n"
        "THRSH: %d;\n"
        "FILTR: %d;\n"
        "CTRL: %d;\n"
        "TE-MON: %d;\n"
        "P-ACT: %d;\n"
        "P-MON: %d;\n"
        "G_MODE: %d;\n"
        "P_MODE: %d;\n",
        pId,
        touchReadReg(FT6236_TH_GROUP),
        touchReadReg(FT6236_TH_DIFF),
        touchReadReg(FT6236_CTRL),
        touchReadReg(FT6236_TIMEENTERMONITOR),
        touchReadReg(FT6236_PERIODACTIVE),
        touchReadReg(FT6236_PERIODMONITOR),
        touchReadReg(FT6236_G_MODE),
        touchReadReg(FT6236_PWR_MODE)
  );

  return true;
}

void touchDmaRxComplete()
{
  i2cLeaveMutexSection();
  dataMutex = false;
  touchParseData();
}

void touchReadData()
{
  bool busDataReady = true;
  if (TOUCH_READ_MODE == 1)  // polling mode
    busDataReady = (GPIO_ReadInputDataBit(TOUCH_INT_GPIO, TOUCH_INT_GPIO_PIN) == 0);  // has data if pulled low

  if (!busDataReady || dataMutex)
    return;

  dataMutex = true;
  i2cEnterMutexSection();
  if (TOUCH_USE_DMA) {
    i2cDmaRead(TOUCH_I2C_ADDRESS, FT6236_TD_STATUS, 8, touchBuffer, FT6236_READ_DATA_LEN, true, &touchDmaRxComplete);
  }
  else {
    i2cRead(TOUCH_I2C_ADDRESS, FT6236_TD_STATUS, 8, touchBuffer, FT6236_READ_DATA_LEN);
    busDataReady = false;
    dataMutex = false;
    i2cLeaveMutexSection();
    touchParseData();
  }
}

#if TOUCH_READ_MODE == 2  // interrupt mode
extern "C" void TOUCH_INT_EXTI_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_EXTI0);

  if (EXTI_GetITStatus(TOUCH_INT_EXTI_Line) != RESET) {
    touchReadData();
    EXTI_ClearITPendingBit(TOUCH_INT_EXTI_Line);
  }
}
#endif
