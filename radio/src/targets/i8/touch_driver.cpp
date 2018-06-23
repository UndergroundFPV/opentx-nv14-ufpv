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
#include "opentx.h"

#include <algorithm>
#include <cstring>
#include <list>

using namespace Touch;

touchData_t touchData;
uint8_t touchBuffer[FT6236_READ_DATA_LEN] FT6236_BUFFER_LOC;
OS_EventID touchDriverSem;
OS_MutexID touchDataMutex;

bool touchGetBufferMutex()
{
  return (pendSemaphore(touchDriverSem, 1000) == E_OK);
}

void touchRelBufferMutex()
{
  (void)postSemaphore(touchDriverSem);
}

void touchRelBufferMutex_isr()
{
  (void)postSemaphore_isr(touchDriverSem);
}

bool touchGetDataMutex()
{
  return (enterMutexSection(touchDataMutex) == E_OK);
}

void touchRelDataMutex()
{
  leaveMutexSection(touchDataMutex);
}

uint8_t touchParsePoint(uint8_t * pBuffer, RawTrackingPoint & ptRef)
{
  uint8_t evt = FT6236_EVENT_FLAG(pBuffer[0]);
  ptRef.index = FT6236_TOUCH_ID(pBuffer[2]);
  if (evt != FT6236_EVT_NONE) {
    ptRef.rawPos.x = (FT6236_COORD_MSB(pBuffer[0]) | pBuffer[1]);
    ptRef.rawPos.y = (FT6236_COORD_MSB(pBuffer[2]) | pBuffer[3]);
    ptRef.pos = ptRef.rawPos;
    if (TOUCH_INVERT_X)
      ptRef.pos.x *= -1;
    if (TOUCH_INVERT_Y)
      ptRef.pos.y *= -1;
    if (TOUCH_SWAP_XY)
      ptRef.pos.swap();
  }
  return evt;
}

int8_t touchParseData(bool hasBusData = true)
{
  if ((hasBusData && !touchGetBufferMutex()) || !touchGetDataMutex())
    return 0;

  if (TOUCH_READ_MODE == 2)
    NVIC_DisableIRQ(TOUCH_INT_EXTI_IRQ);

  int8_t activePts = 0;
  touchData.needLastRead = false;

  touchData.status = (hasBusData ? FT6236_STATUS(touchBuffer[0]) : 0);  // status may be zero if a point was released
  touchData.lastEvent = TouchManager::getTime();

  //DUMP(touchBuffer, FT6236_READ_DATA_LEN);
  if (!hasBusData) TRACE_DEBUG("stat: %d; bd: %d\n", touchData.status, hasBusData);
  std::list<uint8_t> ptList(touchData.ptsIdxList, touchData.ptsIdxList + TOUCH_POINTS);
  uint8_t i = 0;
  while (!ptList.empty() && i < TOUCH_POINTS) {
    int8_t tpIdx = -1;
    uint8_t evt = FT6236_EVT_MAX + 1;  // "unknown"
    RawTrackingPoint touchPt;
    touchPt.state = Touch::ST_UP;
    touchPt.serId = 0;

    // have valid data for this touchpoint?
    if (i < touchData.status) {
      evt = touchParsePoint(&touchBuffer[1 + (6*i)], touchPt);

      if (touchPt.index < TOUCH_POINTS) {
        tpIdx = touchPt.index;
        ptList.remove(tpIdx);

        if (evt == FT6236_EVT_PRESS || evt == FT6236_EVT_CONTACT)
          touchPt.state |= ST_TOUCH;
      }
    }

    // if no valid data, assume point is not pressed on first unused index in list
    if (tpIdx < 0) {
      tpIdx = ptList.front();
      ptList.pop_front();
      touchPt.index = tpIdx;
    }

    // get reference to correct touchpoint in touchData
    const RawTrackingPoint & ptRef = touchData.touchPt[tpIdx];

    // is pressed?
    if (touchPt.state & Touch::ST_TOUCH) {
      if (ptRef.state & Touch::ST_TOUCH) {
        touchPt.serId = ptRef.serId;
        // already touching, so either a hold or a move event
        if (touchData.reportMoveEvents && touchPt.pos.dist(ptRef.pos) >= TOUCH_MIN_MOVE_DIST)
          touchPt.state |= Touch::ST_MOVE;
        else if (touchData.reportHoldEvents)
          touchPt.state |= Touch::ST_HOLD;
      }
      else {
        // new touch
        touchPt.serId = (touchData.lastEvent ^ (tpIdx + 1));  // set the series ID
        touchPt.state |= Touch::ST_PRESS;
      }
    }
    // was pressed?
    else if (ptRef.state & Touch::ST_TOUCH) {
      // touch was released
      touchPt.serId = ptRef.serId;
      // recover last known position
      touchPt.pos = ptRef.pos;
      touchPt.rawPos = ptRef.rawPos;
      touchPt.state |= Touch::ST_RELEASE;
    }
    // had been just released on last pass?
    else if (ptRef.state & Touch::ST_RELEASE) {
      // keep UP state but set changed flag
      activePts = std::max<int8_t>(i, tpIdx) + 1;
    }
    // else invalid touch

    if (touchPt.state != Touch::ST_UP) {
      activePts = std::max<int8_t>(i, tpIdx) + 1;
      touchPt.ts = touchData.lastEvent;
      touchData.needLastRead = (touchPt.state & Touch::ST_TOUCH);  // if pressed, make sure we run at least one more time to detect touch-end events!

      //TRACE_DEBUG(" p[%d]: ts:%d;\n", i, touchPt.ts, touchPt.serId);
      //TRACE_DEBUG(" p[%d]: idx:%d; evt:%d; st:0x%02X; ts:%d; sID:%d;\n", i, touchPt.index, evt, touchPt.state, touchPt.ts, touchPt.serId);
      //TRACE_DEBUG(" p[%d]: idx:%d; evt:%d; st:0x%02X; ts:%d; sID:%d; {%3d, %2d} {raw:%3d, %3d}\n", i, touchPt.index, evt, touchPt.state, touchPt.ts, touchPt.serId, touchPt.pos.x, touchPt.pos.y, touchPt.rawPos.x, touchPt.rawPos.y);
      //TRACE_DEBUG( " last: idx:%d;         st:0x%02X; ts:%d; sID:%d; {%3d, %3d}\n", ptRef.index, ptRef.state, ptRef.ts, ptRef.serId, ptRef.pos.x, ptRef.pos.y);
    }

    // assign updated point data back to the tracking array
    touchData.touchPt[tpIdx] = touchPt;
    ++i;
  }

  touchData.status = activePts;

  touchRelBufferMutex();
  touchRelDataMutex();

  if (activePts && touchData.touchManager)
    touchData.touchManager->driverDataReady(activePts);

  if (TOUCH_READ_MODE == 2)
    NVIC_EnableIRQ(TOUCH_INT_EXTI_IRQ);

  return touchData.status;
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
  //i2cEnterMutexSection();
  if (!i2cWaitBusReady()) {
    if (!i2cInit()) {
      TRACE_ERROR("touch: Init failed, I2C bus not ready.");
      return false;
    }
  }
  //i2cLeaveMutexSection();

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
  touchData.initialized = false;
  touchData.touchManager = NULL;
  touchData.reportHoldEvents = false;
  touchData.reportMoveEvents = true;
  for (int i=0; i < TOUCH_POINTS; ++i)
    touchData.ptsIdxList[i] = i;

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
  //i2cEnterMutexSection();
  touchWriteReg(FT6236_G_MODE, TOUCH_USE_INT_MODE);
  touchWriteReg(FT6236_CTRL, TOUCH_USE_CTRL_MODE);
  touchWriteReg(FT6236_TH_GROUP, TOUCH_USE_GROUP_VAL);
  touchWriteReg(FT6236_TH_DIFF, TOUCH_USE_DIFF_VAL);
  touchWriteReg(FT6236_PERIODACTIVE, TOUCH_USE_PER_ACT);
  touchWriteReg(FT6236_PERIODMONITOR, TOUCH_USE_PER_MON);
  touchWriteReg(FT6236_TIMEENTERMONITOR, TOUCH_USE_ENTER_MON);
  //i2cLeaveMutexSection();

  TRACE("touchInit(): FT6236 ID 0x%X;"
//        "\n THRSH: %d;"
//        "\n FILTR: %d;"
//        "\n CTRL: %d;"
//        "\n TE-MON: %d;"
//        "\n P-ACT: %d;"
//        "\n P-MON: %d;"
//        "\n G_MODE: %d;"
        , pId
//        ,
//        touchReadReg(FT6236_TH_GROUP),
//        touchReadReg(FT6236_TH_DIFF),
//        touchReadReg(FT6236_CTRL),
//        touchReadReg(FT6236_TIMEENTERMONITOR),
//        touchReadReg(FT6236_PERIODACTIVE),
//        touchReadReg(FT6236_PERIODMONITOR),
//        touchReadReg(FT6236_G_MODE)
  );

  touchDriverSem = createSemaphore(1, 1, EVENT_SORT_TYPE_FIFO);
  touchDataMutex = createMutex();
  touchData.dataReadyFlag = CoCreateFlag(true, false);
  if (TOUCH_READ_MODE == 2)
    touchData.busDataReadyFlag = CoCreateFlag(true, false);
  touchData.initialized = true;
  return touchData.initialized;
}

void touchDmaRxComplete()
{
  i2cLeaveMutexSection_isr();
  touchRelBufferMutex_isr();
  if (touchParseData(true))
    isr_SetFlag(touchData.dataReadyFlag);
}

int8_t touchTriggerDataParse(bool hasBusData)
{
  int8_t ret = touchParseData(hasBusData);
  if (ret > 0)
    CoSetFlag(touchData.dataReadyFlag);
  return ret;
}

int8_t touchReadData()
{
  bool busDataReady = false;
  if (TOUCH_READ_MODE == 1)  // polling mode
    busDataReady = (GPIO_ReadInputDataBit(TOUCH_INT_GPIO, TOUCH_INT_GPIO_PIN) == 0);  // has data if pulled low
  else if (TOUCH_READ_MODE == 2)  // interrupt mode
    busDataReady = (CoAcceptSingleFlag(touchData.busDataReadyFlag) == E_OK);

  // If we have no new data AND the last check left us with an active touchpoint AND it's been longer than sensor refresh period, then run
  //  data parsing anyway to signal the touch UP event (since apparently we never got the notice otherwise). This shouldn't really happen much if at all.
  if (!busDataReady && touchData.needLastRead && (TouchManager::getTime() - touchData.lastEvent) > TOUCH_MAX_WAIT_TIME)
    return touchTriggerDataParse(false);

  if (!busDataReady || !touchGetBufferMutex())
    return 0;  // no data

  i2cEnterMutexSection();
  if (TOUCH_USE_DMA) {
    i2cDmaRead(TOUCH_I2C_ADDRESS, FT6236_TD_STATUS, 8, touchBuffer, FT6236_READ_DATA_LEN, true, &touchDmaRxComplete);
    return -1;  // delayed read
  }
  else {
    i2cRead(TOUCH_I2C_ADDRESS, FT6236_TD_STATUS, 8, touchBuffer, FT6236_READ_DATA_LEN);
    i2cLeaveMutexSection();
    touchRelBufferMutex();
    return touchTriggerDataParse(true);  // immediate read
  }
}

#if TOUCH_READ_MODE == 2  // interrupt mode
extern "C" void TOUCH_INT_EXTI_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_EXTI0);
  CoEnterISR();
  if (EXTI_GetITStatus(TOUCH_INT_EXTI_Line) != RESET) {
    EXTI_ClearITPendingBit(TOUCH_INT_EXTI_Line);
    isr_SetFlag(touchData.busDataReadyFlag);
  }
  CoExitISR();
}
#endif
