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

#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include "board.h"
#include "touch_manager.h"

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
#define FT6236_INT_MODE_POLL    (0x00)      // Interrupt polling mode
#define FT6236_INT_MODE_TRIG    (0x01)      // Interrupt trigger mode
#define FT6236_CTRL_MODE_ACT    (0x00)      // Keep active mode
#define FT6236_CTRL_MODE_MON    (0x01)      // Switch from active to monitor mode after timeout in TIMEENTERMONITOR

// expected device ID
#define FT6236_VENDOR_ID        (0x11)      // Value of FT Panel ID

// touch event IDs
#define FT6236_EVT_PRESS        (0x00)
#define FT6236_EVT_RELEASE      (0x01)
#define FT6236_EVT_CONTACT      (0x02)
#define FT6236_EVT_NONE         (0x03)

// read value masks
#define FT6236_STATUS(v)        (v & 0x0F)
#define FT6236_EVENT_FLAG(v)    ((v & 0xC0) >> 6)
#define FT6236_TOUCH_ID(v)      ((v & 0xF0) >> 4)
#define FT6236_COORD_MSB(v)     ((v & 0x0F) << 8)
#define FT6236_TOUCH_AREA(v)    ((v & 0xF0) >> 4)

// setup
#define TOUCH_READ_MODE         1           // 1 = polled mode (read data on demand); 2 = immediate (read data as soon as ready, in ISR)
#define FT6236_READ_DATA_LEN    11          // total bytes to read at a time into buffer (TD_STATUS  + (P1_XH + P1_XL + P1_YH + P1_YL + P1_W + P1_A + P2_XH + P2_XL + P2_YH + P2_YL))

#define TOUCH_USE_CTRL_MODE     FT6236_CTRL_MODE_ACT
#if TOUCH_READ_MODE == 1
  #define TOUCH_USE_INT_MODE    FT6236_INT_MODE_POLL
#else
  #define TOUCH_USE_INT_MODE    FT6236_INT_MODE_TRIG
#endif

#if TOUCH_USE_DMA
  #define FT6236_BUFFER_LOC     __DMA
#else
  #define FT6236_BUFFER_LOC
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  // TODO: remove/consolidate these as needed
  bool      dataReady;
  uint8_t   status;               // number of valid touchpoints
  tsPoint_t point[TOUCH_POINTS];
  tsPoint_t rawPoint[TOUCH_POINTS];
  bool      press[TOUCH_POINTS];  // is touchpoint pressed
  uint8_t   evt[TOUCH_POINTS];    // Event Flag (2b)
  uint8_t   tid[TOUCH_POINTS];    // Touch ID   (4b)
} touchPointRef_t;
extern touchPointRef_t touchData;

bool touchInit(void);
void touchReadData(void);

#ifdef __cplusplus
}
#endif

#endif // TOUCH_DRIVER_H
