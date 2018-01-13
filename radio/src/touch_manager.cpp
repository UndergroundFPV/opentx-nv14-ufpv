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

#include "touch_manager.h"
#include "touch_driver.h"
#include "opentx.h"
#include "debug.h"
#if !defined(SIMU)
  #include "CoOS.h"
#endif

//! non-blocking
bool TouchManager::getTouchPoint(tsPoint_t * point, uint8_t index, bool raw)
{
  if (index >= TOUCH_POINTS)
    return false;

  //memset(point, 0, sizeof(tsPoint_t));

  if (touchData.dataReady && touchData.press[index]) {
    touchData.dataReady = false;
    ++inactivity.sum;
    if (point) {
      *point = touchData.point[index];
      if (!raw) {
        calibratedPoint(point, &touchData.point[0], getCalibration());
        point->x = limit<int16_t>(0, point->x, LCD_W-1);
        point->y = limit<int16_t>(0, point->y, LCD_H-1);
      }
    }
    //TRACE_DEBUG("p[%d]: x: %3d; y: %3d; rawX: %3d; rawY: %3d; evt: %d; tid: %d;\n", index, point->x, point->y, touchData.rawPoint[index].x, touchData.rawPoint[index].y, touchData.evt[index], touchData.tid[index]);
    return true;
  }
  return false;
}

bool TouchManager::getTouchPoint(point_t * point, uint8_t index)
{
  tsPoint_t * pt = new tsPoint_t;
  pt->x = point->x;
  pt->y = point->y;
  if (!getTouchPoint(pt, index, false))
    return false;

  point->x = pt->x;
  point->y = pt->y;

  delete pt;
  return true;
}

//! blocks thread for ticks*2 ms
bool TouchManager::waitTouchRelease(int ticks, uint8_t index)
{
  while (getTouchPoint(NULL, index, false) || --ticks) {
    CoTickDelay(1);
  };
  return true;
}

//
// Utils
//

point_t TouchManager::tsPointToLcdPoint(const tsPoint_t & pt)
{
  return point_t({(coord_t)pt.x, (coord_t)pt.y});
}

tsPoint_t TouchManager::lcdPointToTsPoint(const point_t & pt) {
  return tsPoint_t({(int16_t)pt.x, (int16_t)pt.y});
}

//
// Calibration
//

bool TouchManager::setCalibration(TouchCalibMatrix matrix)
{
  if (!isCalibMatrixValid(&matrix))
    return false;

  g_eeGeneral.touchCalib = matrix;
  storageDirty(EE_GENERAL);
  return true;
}

bool TouchManager::isCalibMatrixValid(const TouchCalibMatrix * m)
{
  return (m->Div != 0 && calibrationCrc(m) == m->crc);
}

void TouchManager::initCalibrationMatrix(TouchCalibMatrix * m)
{
  *m = {0, 0, 0, 0, 0, 0, 0};
}

TouchCalibMatrix * TouchManager::getCalibration()
{
  return &g_eeGeneral.touchCalib;
}

bool TouchManager::calibratedPoint(tsPoint_t * resultPoint, tsPoint_t * rawPoint, TouchCalibMatrix * matrix)
{
  if (!isCalibMatrixValid(matrix))
    return false;

  resultPoint->x = ((matrix->An * rawPoint->x) + (matrix->Bn * rawPoint->y) + matrix->Cn) / matrix->Div;
  resultPoint->y = ((matrix->Dn * rawPoint->x) + (matrix->En * rawPoint->y) + matrix->Fn) / matrix->Div;
  return true;
}

bool TouchManager::calcCalibrationMatrix(tsPoint_t * screenPoints, tsPoint_t * touchPoints, TouchCalibMatrix * matrix)
{
  matrix->Div = ((touchPoints[0].x - touchPoints[2].x) * (touchPoints[1].y - touchPoints[2].y)) -
      ((touchPoints[1].x - touchPoints[2].x) * (touchPoints[0].y - touchPoints[2].y)) ;

  if (!matrix->Div)
    return false ;

  matrix->An = ((screenPoints[0].x - screenPoints[2].x) * (touchPoints[1].y - touchPoints[2].y)) -
               ((screenPoints[1].x - screenPoints[2].x) * (touchPoints[0].y - touchPoints[2].y)) ;

  matrix->Bn = ((touchPoints[0].x - touchPoints[2].x) * (screenPoints[1].x - screenPoints[2].x)) -
               ((screenPoints[0].x - screenPoints[2].x) * (touchPoints[1].x - touchPoints[2].x)) ;

  matrix->Cn = (touchPoints[2].x * screenPoints[1].x - touchPoints[1].x * screenPoints[2].x) * touchPoints[0].y +
               (touchPoints[0].x * screenPoints[2].x - touchPoints[2].x * screenPoints[0].x) * touchPoints[1].y +
               (touchPoints[1].x * screenPoints[0].x - touchPoints[0].x * screenPoints[1].x) * touchPoints[2].y ;

  matrix->Dn = ((screenPoints[0].y - screenPoints[2].y) * (touchPoints[1].y - touchPoints[2].y)) -
               ((screenPoints[1].y - screenPoints[2].y) * (touchPoints[0].y - touchPoints[2].y)) ;

  matrix->En = ((touchPoints[0].x - touchPoints[2].x) * (screenPoints[1].y - screenPoints[2].y)) -
               ((screenPoints[0].y - screenPoints[2].y) * (touchPoints[1].x - touchPoints[2].x)) ;

  matrix->Fn = (touchPoints[2].x * screenPoints[1].y - touchPoints[1].x * screenPoints[2].y) * touchPoints[0].y +
               (touchPoints[0].x * screenPoints[2].y - touchPoints[2].x * screenPoints[0].y) * touchPoints[1].y +
               (touchPoints[1].x * screenPoints[0].y - touchPoints[0].x * screenPoints[1].y) * touchPoints[2].y ;

  matrix->crc = calibrationCrc(matrix);
  return true;
}

uint16_t TouchManager::calibrationCrc(const TouchCalibMatrix * matrix)
{
  uint16_t crc = 0;
  uint8_t * bytes = (uint8_t *)matrix;
  for (int i=0; i < (7*4); ++i)
    crc += *bytes++;
  return crc;
}

void TouchManager::dumpCalibMatrix(const TouchCalibMatrix * m)
{
  TRACE_DEBUG("[TouchCalibMatrix] { %d, %d, %d, %d, %d, %d, %d } crc: 0x%X\n", m->An, m->Bn, m->Cn, m->Dn, m->En, m->Fn, m->Div, m->crc);
}

