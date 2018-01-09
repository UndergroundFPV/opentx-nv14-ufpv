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
#include "i2c_driver.h"
#include "debug.h"

// FT6236 touch driver

#if defined(I2C_DMA)
  #define TOUCH_READ_MODE              0     // DMA mode:
#else
  #define TOUCH_READ_MODE              1     // 0 = DMA mode: 1 = polled mode (read data on demand); 2 = immediate (read data as soon as ready, in IRQ)
#endif

#define TOUCH_USE_DMA           (TOUCH_READ_MODE == 0)

#define DEVICE_MODE_VALUE       (00)        // Device WORKING mode
#define TOUCH_THRESHOLD_VALUE   (22)        // Touch threshold value
#define ACTIVE_PERIOD_VALUE     (12)        // Active period value

#define FT6236_DEV_MODE         (0x00)      // device mode register (R/W)
#define FT6236_GEST_ID          (0x01)      // Gesture ID (R)
#define FT6236_TD_STATUS        (0x02)      // Touch points count register (R)

#define FT6236_P1_XH            (0X03)      // 1st Touch X Position (R)
#define FT6236_P2_XH            (0X09)      // 2nd Touch X Position (R)

#define FT6236_TH_GROUP         (0x80)      // Threshold for touch detection (R/W)
#define FT6236_TH_DIFF          (0x85)      // Filter function coefficient (R/W)
#define FT6236_CTRL             (0x86)      // 0: will keep active mode. 1: switch to monitor mode when idle (R/W)
#define FT6236_TIMEENTERMONITOR (0x87)      // Idle timeout before switching to monitor mode (R/W)
#define FT6236_PERIODACTIVE     (0x88)      // Report rate in active mode (R/W)
#define FT6236_PERIODMONITOR    (0x89)      // Report rate in monitor mode (R/W)
#define FT6236_CIPHER           (0xA3)      // Chip Selecting (R)
#define FT6236_G_MODE           (0xA4)      // Interrupt mode regitster (R/W)
#define FT6236_PWR_MODE         (0xA5)      // Current power mode (R/W)
#define FT6236_FIRMID           (0xA6)      // Firmware version (R)
#define FT6236_FOCALTECH_ID     (0xA8)      // FT Panel ID (R)
#define FT6236_RELEASE_CODE_ID  (0xAF)      // Release code version (R)
#define FT6236_STATE            (0xBC)      // Current Operating mode (R/W)

#define FT6236_Vendor_ID        (0x11)      // Value of FT Panel ID

#define FT6236_STATUS(v)        (v & 0x0F)
#define FT6236_EVENT_FLAG(v)    ((v & 0xC0) >> 6)
#define FT6236_TOUCH_ID(v)      ((v & 0xF0) >> 4)
#define FT6236_COORD_MSB(v)     ((v & 0x0F) << 8)
#define FT6236_TOUCH_AREA(v)    ((v & 0xF0) >> 4)

#define FT6236_READ_DATA_LEN    14  // total bytes to read at a time into buffer

#if defined(TOUCH_USE_DMA)
  #define FT6236_BUFFER_LOC     __DMA  // if available
#else
  #define FT6236_BUFFER_LOC
#endif

touchPointRef_t touchData;
static bool busDataReady = false;
//static bool bufferDataReady = false;
static bool dataMutex = false;
uint8_t touchBuffer[FT6236_READ_DATA_LEN] FT6236_BUFFER_LOC;

void touchParsePoint(uint8_t * pBuffer, uint8_t idx)
{
  touchData.evt[idx] = FT6236_EVENT_FLAG(pBuffer[0]);
  touchData.y[idx] = (FT6236_COORD_MSB(pBuffer[0]) | pBuffer[1]);
  //touchData.y[idx] = LCD_H - (touchData.y[idx] >> 1);

  touchData.tid[idx] = FT6236_TOUCH_ID(pBuffer[2]);
  touchData.x[idx] = (FT6236_COORD_MSB(pBuffer[2]) | pBuffer[3]);
  //touchData.x[idx] = LCD_W - (0xFF - (touchData.x[idx] >> 1));

  touchData.z[idx] = pBuffer[4];
  touchData.area[idx] = FT6236_TOUCH_AREA(pBuffer[5]);
}

void touchParseData(void)
{
  if (dataMutex)
    return;

  dataMutex = true;
  NVIC_DisableIRQ(TOUCH_INT_EXTI_IRQ);
  touchData.dataReady = false;

  touchData.gid = touchBuffer[0];
  touchData.status = FT6236_STATUS(touchBuffer[1]);

  for (int i = 0; i < touchData.status; ++i) {
    touchParsePoint(&touchBuffer[2 + (6*i)], i);
    //TRACE("p[%d]: x: %d; y: %d; z: %d; evt: %d; tid: %d; area: %d; gid: %d; ", i, touchData.x[i], touchData.y[i], touchData.z[i], touchData.evt[i], touchData.tid[i], touchData.area[i], touchData.gid);
    TRACE("p[%d]: x: %d; y: %d; evt: %X; id: %X;", i, touchData.x[i], touchData.y[i], touchData.evt[i], touchData.tid[i]);
  }
  NVIC_EnableIRQ(TOUCH_INT_EXTI_IRQ);
  touchData.dataReady = true;
  dataMutex = false;
}

uint8_t touchReadReg(uint8_t regAddr)
{
  return i2cReadByte(TOUCH_I2C_ADDRESS, regAddr, 8);
}

uint8_t touchWriteReg(uint8_t regAddr, uint8_t value)
{
  return i2cWriteByte(TOUCH_I2C_ADDRESS, regAddr, 8, value);
}

// external interrupt for data ready signal
static void EXTILine0_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = TOUCH_INT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(TOUCH_INT_GPIO, &GPIO_InitStructure);

  SYSCFG_EXTILineConfig(TOUCH_INT_EXTI_PortSource, TOUCH_INT_GPIO_PinSource);

  EXTI_StructInit(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = TOUCH_INT_EXTI_Line;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = TOUCH_INT_EXTI_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TOUCH_INT_EXTI_IRQPriority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

bool touchInit()
{
  if (!i2cWaitEventCleared(I2C_FLAG_BUSY)) {
    if (!i2cInit()) {
      TRACE_ERROR("Touch Driver: Init failed, I2C bus not ready.");
      return false;
    }
  }

  // init reset pin
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = TOUCH_RST_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(TOUCH_RST_GPIO, &GPIO_InitStructure);

  // reset touch controller
  GPIO_SetBits(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(100);
  GPIO_ResetBits(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(500);
  GPIO_SetBits(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(300);

	// touch sensor config
//  touchWriteReg(FT6236_DEV_MODE, DEVICE_MODE_VALUE);
//  touchWriteReg(FT6236_TH_GROUP, TOUCH_THRESHOLD_VALUE);
//  touchWriteReg(FT6236_PERIODACTIVE, ACTIVE_PERIOD_VALUE);

  TRACE("touchInit(): FocalTech Panel ID 0x%X; THG: %X; THD: %X; CTRL: %X; TEM: %X; PA: %X; PM: %X; GM: %X; FWId: %X; RELID: %X; STATE: %X;",
        touchReadReg(FT6236_FOCALTECH_ID),
        touchReadReg(FT6236_TH_GROUP),
        touchReadReg(FT6236_TH_DIFF),
        touchReadReg(FT6236_CTRL),
        touchReadReg(FT6236_TIMEENTERMONITOR),
        touchReadReg(FT6236_PERIODACTIVE),
        touchReadReg(FT6236_PERIODMONITOR),
        touchReadReg(FT6236_G_MODE),
        touchReadReg(FT6236_FIRMID),
        touchReadReg(FT6236_RELEASE_CODE_ID),
        touchReadReg(FT6236_STATE)
        );

  if (touchReadReg(FT6236_FOCALTECH_ID) != FT6236_Vendor_ID) {
    TRACE_ERROR("Touch Driver: Init failed, FT6236 returned wrong ID.");
    return false;
  }

  // enable external interrupt
  EXTILine0_Config();

	return true;
}

void touchReadData()
{
  if (!busDataReady || dataMutex || TOUCH_USE_DMA)
    return;

  dataMutex = true;
  i2cRead(TOUCH_I2C_ADDRESS, FT6236_GEST_ID, 8, touchBuffer, FT6236_READ_DATA_LEN);
  busDataReady = false;
  dataMutex = false;
  touchParseData();
}

void touchDmaRxComplete()
{
  dataMutex = false;
  touchParseData();
}

extern "C" void TOUCH_INT_EXTI_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_EXTI0);

  if (EXTI_GetITStatus(TOUCH_INT_EXTI_Line) != RESET) {

    if (TOUCH_USE_DMA) {
      dataMutex = true;
      i2cDmaRead(TOUCH_I2C_ADDRESS, FT6236_GEST_ID, 8, touchBuffer, FT6236_READ_DATA_LEN, true, &touchDmaRxComplete);
    }
    else {
      busDataReady = true;
      if (TOUCH_READ_MODE == 2)
        touchReadData();
    }
    EXTI_ClearITPendingBit(TOUCH_INT_EXTI_Line);
  }
}
