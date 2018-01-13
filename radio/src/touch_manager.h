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

#ifndef TOUCH_MANAGER_H
#define TOUCH_MANAGER_H

#include "definitions.h"

struct point_t;

// touchpoint struct
typedef struct
{
  int16_t x;
  int16_t y;
} tsPoint_t;

// calibration matrix (packed for use in radio data struct)
PACK(struct TouchCalibMatrix {
  int32_t An;
  int32_t Bn;
  int32_t Cn;
  int32_t Dn;
  int32_t En;
  int32_t Fn;
  int32_t Div;
  uint16_t crc;
});

class TouchManager
{
  public:
    TouchManager() {}
    static bool getTouchPoint(tsPoint_t * point, uint8_t index = 0, bool raw = false);
    static bool getTouchPoint(point_t * point, uint8_t index = 0);
    static bool waitTouchRelease(int ticks = 100, uint8_t index = 0);

    // util
    static point_t tsPointToLcdPoint(const tsPoint_t & pt);
    static tsPoint_t lcdPointToTsPoint(const point_t & pt);

    // calibration
    static TouchCalibMatrix * getCalibration();
    static bool setCalibration(TouchCalibMatrix matrix);
    static void initCalibrationMatrix(TouchCalibMatrix * m);
    static bool isCalibMatrixValid(const TouchCalibMatrix * m);
    static bool isCurrentCalibrationValid() { return isCalibMatrixValid(getCalibration()); }
    static uint16_t calibrationCrc(const TouchCalibMatrix * matrix);
    static void dumpCalibMatrix(const TouchCalibMatrix * m);

    /*!
      \brief Calculates the difference between the touch screen and the
             actual screen co-ordinates, taking into account misalignment
             and any physical offset of the touch screen.

      \param[in]  screenPoints  Pointer to array of 3 points representing the actual screen coordinates
                                which were expected to be touched.
      \param[in]  touchPoints   Pointer to array of 3 points representing the touch sensor coordinates
                                which are expected to correspond to the screen coordinates in \a screenPoints.
      \param[out] matrix        Pointer to the resulting calibration matrix coefficients used for
                                mapping touch coordinates to screen coordinates.
      \return bool true on success or false on failure. Failure is result of divider calculation resulting in zero,
              which suggests poor data.

      \note  This is based on the public domain touch screen calibration code
             written by Carlos E. Vidales (copyright (c) 2001).
             https://www.embedded.com/design/system-integration/4023968/How-To-Calibrate-Touch-Screens
    */
    static bool calcCalibrationMatrix(tsPoint_t * screenPoints, tsPoint_t * touchPoints, TouchCalibMatrix * matrix);

    /*!
      \brief  Converts raw touch screen locations (screenPtr) into actual
              pixel locations on the display (displayPtr) using the
              supplied matrix.

      \param[out] resultPoint Pointer to the tsPoint_t object that will hold
                              the compensated pixel location on the display
      \param[in]  rawPoint    Pointer to the tsPoint_t object that contains the
                              raw touch screen co-ordinates (before the
                              calibration calculations are made)
      \param[in]  matrix      Pointer to the calibration matrix coefficients
                              used during the calibration process.

      \note  This is based on the public domain touch screen calibration code
             written by Carlos E. Vidales (copyright (c) 2001).
             https://www.embedded.com/design/system-integration/4023968/How-To-Calibrate-Touch-Screens
    */
    static bool calibratedPoint(tsPoint_t * resultPoint, tsPoint_t * rawPoint, TouchCalibMatrix * matrix);

};

#endif // TOUCH_MANAGER_H
