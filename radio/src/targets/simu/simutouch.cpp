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


// NOTE: These Qt includes need to come first, before opentx.h, due to macro naming conflicts in ARM libs.
//#include <QtCore>
//#include <QTouchEvent>
//#include <QDebug>

#include "opentx.h"

#if IS_TOUCH_ENABLED()

#include "simutouch.h"
//#include "simpgmspace.h"
#include "targets/i8/touch_driver.h"
#include "touch_manager.h"

//static QTouchEvent * lastTouchEvent = NULL;
//QMutex touchMutex;

using namespace Touch;

touchData_t touchData;

// TODO: Stubs!

bool touchGetDataMutex()
{
  return true;
}

void touchRelDataMutex()
{
 // touchMutex.unlock();
}

uint8_t touchParseData(void)
{
//  if (!touchData.dataReady || !lastTouchEvent /*|| !touchGetMutex()*/)
//    return;
//  touchData.dataReady = false;
//  QTouchEvent evt = QTouchEvent(lastTouchEvent->type(), lastTouchEvent->device(), lastTouchEvent->modifiers(), lastTouchEvent->touchPointStates(), lastTouchEvent->touchPoints());

//  foreach (const QTouchEvent::TouchPoint & tp, evt.touchPoints()) {
//    qDebug() << tp;
//  }
//  touchReleaesMutex();
  return 0;
}

void simuSetTouchEvent()
{
  if (!touchGetDataMutex())
    return;

  //lastTouchEvent = event; //QTouchEvent(event->type(), event->device(), event->modifiers(), event->touchPointStates(), event->touchPoints());

  //touchReleaesMutex();
  touchParseData();

}

int8_t touchReadData()
{
//  if (touchData.dataReady) {
//    //touchParseData();
//    return 1;  // immediate read
//  }
  return 0;
}

bool touchInit(void)
{
  touchData.touchManager = NULL;
  touchData.reportHoldEvents = false;
  touchData.reportMoveEvents = true;
  touchData.initialized = true;

  return true;
}

#endif
