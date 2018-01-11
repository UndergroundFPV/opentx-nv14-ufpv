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
#include "opentx.h"

// FT6236 touch driver

// device registers
#define FT6236_DEV_MODE         (0x00)      // device mode (R/W) 0=WORKING; 100b=FACTORY
#define FT6236_GEST_ID          (0x01)      // Gesture ID (R) 0x10: Mv U; 0x14 Mv R; 0x18: Mv D; 0x1C Mv L; 0x48 Zm In; 0x49: Zm Out
#define FT6236_TD_STATUS        (0x02)      // Number of touch points detected (1 or 2) (R)
#define FT6236_P1_XH            (0X03)      // 1st Touch X Position High 4b & Event Flag 2b (R)
#define FT6236_P1_XL            (0X04)      // 1st Touch X Position Low Byte (R)
#define FT6236_P1_YH            (0X05)      // 1st Touch Y Position High 4b & Touch ID 4b (R)
#define FT6236_P1_YL            (0X06)      // 1st Touch Y Position Low Byte (R)
#define FT6236_P1_WEIGHT        (0X07)      // 1st Touch Weight (R)
#define FT6236_P1_AREA          (0X08)      // 1st Touch Area (R)
#define FT6236_P2_XH            (0X09)      // 2nd Touch X Position High 4b & Event Flag 2b (R)
#define FT6236_P2_XL            (0X0A)      // 2nd Touch X Position Low Byte (R)
#define FT6236_P2_YH            (0X0B)      // 2nd Touch Y Position High 4b & Touch ID 4b (R)
#define FT6236_P2_YL            (0X0C)      // 2nd Touch Y Position Low Byte (R)
#define FT6236_P2_WEIGHT        (0X0D)      // 2nd Touch Weight (R)
#define FT6236_P2_AREA          (0X0E)      // 2nd Touch Area (R)

#define FT6236_TH_GROUP         (0x80)      // Threshold for touch detection (R/W)
#define FT6236_TH_DIFF          (0x85)      // Filter function coefficient (R/W)
#define FT6236_CTRL             (0x86)      // 0: will keep active mode. 1: switch to monitor mode when idle (R/W)
#define FT6236_TIMEENTERMONITOR (0x87)      // Idle timeout before switching to monitor mode (R/W)
#define FT6236_PERIODACTIVE     (0x88)      // Report rate in active mode (R/W)
#define FT6236_PERIODMONITOR    (0x89)      // Report rate in monitor mode (R/W)

#define FT6236_RADIAN_VALUE     (0x91)      // Min. allowed angle for rotate gesture (R/W)
#define FT6236_OFFSET_LFT_RGT   (0x92)      // Max. offset for moving left/right gesture (R/W)
#define FT6236_OFFSET_UP_DN     (0x93)      // Max. offset for moving up/down gesture (R/W)
#define FT6236_DISTANCE_LFT_RGT (0x94)      // Min. distance for moving left/right gesture (R/W)
#define FT6236_DISTANCE_UP_DN   (0x95)      // Min. distance for moving up/down gesture (R/W)
#define FT6236_DISTANCE_ZOOM    (0x96)      // Max. distance for zoom in/out gesture (R/W)

#define FT6236_LIB_VER_H        (0xA1)      // LIB version high byte (R)
#define FT6236_LIB_VER_L        (0xA2)      // LIB version low byte (R)
#define FT6236_CIPHER           (0xA3)      // Chip Selecting (R)
#define FT6236_G_MODE           (0xA4)      // Interrupt mode regitster (R/W)
#define FT6236_PWR_MODE         (0xA5)      // Current power mode (R/W)
#define FT6236_FIRMID           (0xA6)      // Firmware version (R)
#define FT6236_FOCALTECH_ID     (0xA8)      // FT Panel ID (R)
#define FT6236_RELEASE_CODE_ID  (0xAF)      // Release code version (R)
#define FT6236_STATE            (0xBC)      // Current Operating mode (R/W)

// register values
#define TOUCH_INT_MODE_POLL     (0x00)      // Interrupt polling mode
#define TOUCH_INT_MODE_TRIG     (0x01)      // Interrupt trigger mode
#define TOUCH_CTRL_MODE_ACT     (0x00)      // Keep active mode
#define TOUCH_CTRL_MODE_MON     (0x01)      // Switch from active to monitor mode after timeout in TIMEENTERMONITOR

// expected device ID
#define FT6236_VENDOR_ID        (0x11)      // Value of FT Panel ID

// read value masks
#define FT6236_STATUS(v)        (v & 0x0F)
#define FT6236_EVENT_FLAG(v)    ((v & 0xC0) >> 6)
#define FT6236_TOUCH_ID(v)      ((v & 0xF0) >> 4)
#define FT6236_COORD_MSB(v)     ((v & 0x0F) << 8)
#define FT6236_TOUCH_AREA(v)    ((v & 0xF0) >> 4)

// setup
#define TOUCH_READ_MODE         1           // 1 = polled mode (read data on demand); 2 = immediate (read data as soon as ready, in ISR)
#define FT6236_READ_DATA_LEN    14          // total bytes to read at a time into buffer (GEST_ID + TD_STATUS  + (6 * 2))

#define TOUCH_USE_CTRL_MODE     TOUCH_CTRL_MODE_ACT
#if TOUCH_READ_MODE == 1
  #define TOUCH_USE_INT_MODE    TOUCH_INT_MODE_POLL
#else
  #define TOUCH_USE_INT_MODE    TOUCH_INT_MODE_TRIG
#endif

#if TOUCH_USE_DMA
  #define FT6236_BUFFER_LOC     __DMA
#else
  #define FT6236_BUFFER_LOC
#endif

touchPointRef_t touchData;
volatile bool dataMutex = false;
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

  for (int i = 0, end = min<uint8_t>(touchData.status, TOUCH_POINTS); i < end; ++i) {
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
  GPIO_SetBits(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(100);
  GPIO_ResetBits(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(500);
  GPIO_SetBits(TOUCH_RST_GPIO, TOUCH_RST_GPIO_PIN);
  delay_ms(300);

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
  if (TOUCH_USE_DMA) {
    i2cDmaRead(TOUCH_I2C_ADDRESS, FT6236_GEST_ID, 8, touchBuffer, FT6236_READ_DATA_LEN, true, &touchDmaRxComplete);
  }
  else {
    i2cRead(TOUCH_I2C_ADDRESS, FT6236_GEST_ID, 8, touchBuffer, FT6236_READ_DATA_LEN);
    busDataReady = false;
    dataMutex = false;
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
