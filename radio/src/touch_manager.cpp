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

#include "opentx.h"
#include "touch_manager.h"
#include "touch_driver.h"
#include "debug.h"
#if !defined(SIMU)
  #include "CoOS.h"
#endif

#define MS_PER_TICK         (CFG_CPU_FREQ / CFG_SYSTICK_FREQ / (CFG_CPU_FREQ / 1000))
#define TIME_TO_TICKS(_ms)  ((_ms) / MS_PER_TICK)
#define TICKS_TO_TIME(_tk)  ((_tk) * MS_PER_TICK)

#define TOUCH_TASK_PERIOD   TIME_TO_TICKS(2)    // 2ms (= 1 systick)

RTOS_TASK_HANDLE TouchManager::m_taskId;
TaskStack<TOUCH_STACK_SIZE> __ALIGNED(8) TouchManager::m_taskStack;

using namespace Touch;

// This is simply to trigger the TouchManager::run() function since CoOS/pthread can't seem to access it otherwise (compiler errors or warnings generated).
void touchManagerTask(void * pdata)
{
  if (TouchManager::instance())
    TouchManager::instance()->run(pdata);
}

TouchManager * TouchManager::instance()
{
  static TouchManager instance;
  return &instance;
}

TouchManager::TouchManager() :
  m_detectRotations(false)
{
}

bool TouchManager::init()
{
  // init driver
  if (!touchData.initialized)
    touchInit();
  touchData.reportHoldEvents = true;
  touchData.reportMoveEvents = true;
  //touchData.touchManager = this;    // if this is set then the touch driver calls TouchManager::driverDataReady() (pushes data vs. pulling it)

  m_taskId = CoCreateTask(touchManagerTask, NULL, TOUCH_TASK_PRIO, &m_taskStack.stack[TOUCH_STACK_SIZE-1], TOUCH_STACK_SIZE);
  m_eventQueMtxId = CoCreateMutex();
  m_callbackMtxId = CoCreateMutex();

  return touchData.initialized;
}

void TouchManager::run(void * /*pdata*/)
{
  int8_t tdStat;
  //tTime_t lastQueCheck = getTime();

  // wait for radio settings to be read and other vital startup tasks to complete
  (void)CoWaitForSingleFlag(openTxInitCompleteFlag, 0);

  while(1)
  {
#if defined(SIMU)
    if (!main_thread_running)
      break;
#endif
//  tTime_t now = getTime();

    if ((tdStat = touchReadData())) {
      // touchReadData() returns -1 for delayed read, >0 for immediate read, 0 for no data
      // in case of delayed read (DMA), wait for ready flag
      if (tdStat > 0 || CoWaitForSingleFlag(touchData.dataReadyFlag, TOUCH_TASK_PERIOD * 3) == E_OK)
        driverDataReady(touchData.status);
    }

    // We map to key events in menus task before GUI runs...
    // OR
    //    if (!KeyEventEmulator::keyEventsSuspended())
    //      processQueue(&KeyEventEmulator::mapToKeyEvent);
    // OR
    //    if (now - lastQueCheck >= 10) {
    //      if (!KeyEventEmulator::keyEventsSuspended())
    //        processQueue(&KeyEventEmulator::mapToKeyEvent);
    //      lastQueCheck = now;
    //    }

    CoTickDelay(TOUCH_TASK_PERIOD);
  }
}

void TouchManager::driverDataReady(uint8_t numPoints)
{
  if (!numPoints || !touchGetDataMutex())
    return;

  uint8_t actualActive = 0;
  for (uint8_t i=0; i < numPoints; ++i) {
    if (rawEvent(touchData.touchPt[i]))
      ++actualActive;
  }
  touchRelDataMutex();

  if (actualActive)
    generateEvent(actualActive);
}

bool TouchManager::rawEvent(const RawTrackingPoint & rtp)
{
  if (rtp.index >= TOUCH_POINTS)
    return false;

  TouchPoint & pt = m_points[rtp.index];

  if (rtp.state == ST_UP) {
    // reset point
    pt.state = ST_UP;
    return false;
  }

  const bool newEvt = ((rtp.state & ST_PRESS) || pt.serId != rtp.serId);

  if (newEvt) {
    pt.state = (rtp.state | ST_PRESS);
    pt.serId = rtp.serId;
    pt.index = rtp.index;
    pt.startTm = rtp.ts;
    pt.lastTm = 0;
    pt.lastPos = {0, 0};
    pt.holdTm = 0;
    pt.moveStartTm = 0;
    calibratedPoint(&pt.startPos, &rtp.pos, getCalibration());
    pt.pos = pt.startPos;
  }
  // HOLD, MOVE, or RELEASE
  else {
    pt.state = (pt.state & ST_IGNORE) | (rtp.state & 0x7F);  // do not wipe out IGNORE flag
    pt.lastTm = pt.timestamp;
    pt.lastPos = pt.pos;
    calibratedPoint(&pt.pos, &rtp.pos, getCalibration());

    if (rtp.state & ST_TOUCH) {
      if (rtp.state & ST_MOVE) {
        pt.holdTm = 0;
        if (!pt.moveStartTm)
          pt.moveStartTm = rtp.ts;
      }
      else if (rtp.state & ST_HOLD) {
        pt.holdTm += rtp.ts - pt.lastTm;
      }
    }
  }

  pt.rawPos = rtp.pos;
  pt.timestamp = rtp.ts;

  ++inactivity.sum;  // update global activity tracker

  //rtp.debug();
  //pt.debug(false);
  return true;
}

uint32_t dirFromAngle(int16_t angle)
{
  if (angle < 0 || angle > 360)
    return 0;

  //E = 0, S = 90, W = 180, N = 270
  if (angle > 45 && angle < 135)
    return GEST_DIR_S;
  else if (angle >= 135 && angle <= 225)
    return GEST_DIR_W;
  else if (angle > 225 && angle < 315)
    return GEST_DIR_N;

  return GEST_DIR_E;  // (angle >= 315 || angle <= 45)
}

//enum TpState
//  ST_UP       = 0,     //! not touching (after release) or otherwise unknown
//  ST_PRESS    = 0x01,  //! initial touch down (first event in series)
//  ST_RELEASE  = 0x02,  //! touch release (final event in series)
//  ST_HOLD     = 0x04,  //! touch is being held stationary (opposite of MOVE) - only reported if enabled in driver
//  ST_MOVE     = 0x08,  //! touchpoint is being moved (opposite of HOLD) - only reported if enabled in driver
//  ST_TOUCH    = 0x40,  //! is being touched (== !(UP || RELEASE))
//  ST_IGNORE   = 0x80,  //! point is being ignored (internal use)

//enum GestureType
//  GEST_PRESS      = 0x0001,  //! initial touch down, happens only once per series
//  GEST_RELEASE    = 0x0002,  //! point was released, this event should be last in series
//  GEST_HOLD       = 0x0004,  //! touch is being held stationary (opposite of MOVE)
//  GEST_MOVE       = 0x0008,  //! touchpoint is being moved (opposite of HOLD)
//  GEST_TAP        = 0x0010,  //! a tap was registed (a touch event which has not moved outside a maximum radius)
//  GEST_DBLTAP     = 0x0020,  //! a double tap was registed (two quick sequential touches/releases)   TODO
//  GEST_DRAG       = 0x0040,  //! touchpoint was dragged (opposite of TAP)
//  GEST_SWIPE      = 0x0080,  //! swipe gesture (a quick drag)
//  GEST_PINCH      = 0x0100,  //! two-finger pinch
//  GEST_ROTATE     = 0x0200,  //! two-finger rotate
//  GEST_DIR_N      = 0x0400,
// ....
//  GEST_DOUBLE     = 0x4000,  // two points were used in the gesture, eg. two-finger tap or swipe (NOTE: PINCH and ROTATE do not set this flag)
//  GEST_LONG       = 0x8000,  // gesture has exceeded "long" hold time (only used while HOLD or TAP are set)

void TouchManager::generateEvent(uint8_t numPoints)
{
  static tTime_t hapticFiredForEvt = 0;

//  uint8_t numPoints = 0;
//  for (uint8_t i=0; i < TOUCH_POINTS; ++i) {
//    if (m_points[i].state != ST_UP)
//      ++numPoints;
//  }

  if (!numPoints)
    return;

  Event event(GEST_NONE, numPoints, getTime());

  uint8_t nextPtIdx = 0, evtTpIdx = 0;
  TouchPoint * pt = nullptr;

  // Find the first active point and populate touchPoints vector.
  for (uint8_t i=0; i < TOUCH_POINTS && evtTpIdx < numPoints; ++i) {
    if (m_points[i].state == ST_UP)
      continue;
    event.touchPoints[evtTpIdx++] = m_points[i];
    if (!pt) {
      pt = &m_points[i];  // use this as first event point
      ++nextPtIdx;
    }
  }
  // check for valid point (shouldn't happen since numPoints > 0)
  if (!pt)
    return;

  event.seriesId = pt->serId;

  if ((pt->state & ST_IGNORE)) {
    // if we're ignoring this point, then queue the touchPoints and bail out early
    enqueue(event);
    return;
  }

  tCoord_t distance = pt->startPos.dist(pt->pos);

  if ((pt->state & ST_PRESS))
    event.gesture |= GEST_PRESS;
  else if ((pt->state & ST_RELEASE))
    event.gesture |= GEST_RELEASE;

  // actively moving = MOVE
  if (pt->state & ST_MOVE) {
    event.gesture |= GEST_MOVE;
  }
  // hold timer expired and still being touched = HOLD
  else if ((pt->state & ST_HOLD) && pt->holdTm >= TOUCH_HOLD_MIN_TM) {
    event.gesture |= GEST_HOLD;
  }

  // is within rap radius?
  if (distance <= TOUCH_TAP_RADIUS_SZ) {
    // meets minimum tap time? = TAP
    if (pt->timestamp - pt->startTm >= TOUCH_TAP_MIN_TM)
      event.gesture |= GEST_TAP;
  }
  // has miminum swipe distance and is quick enough? = SWIPE
  else if (distance >= TOUCH_SWIPE_MIN_DIST && (pt->timestamp - pt->moveStartTm) <= TOUCH_SWIPE_MAX_TM) {
    event.gesture |= GEST_SWIPE;
  }
  // moved outside tap radius and not a swipe = DRAG
  else {
    event.gesture |= GEST_DRAG;
  }

  // exceeded LONG hold time
  if (pt->holdTm >= TOUCH_LONG_EVT_TM && (event.gesture & (GEST_HOLD | GEST_TAP))) {
    event.gesture |= GEST_LONG;
#if defined(HAPTIC)
    // TODO: not sure this is the best place to fire haptic events, but at least we provide some feedback for now...
    //   it probably makes more sense for event handlers to provide feedback (eg. the key mapper fires on first repeat event, etc)
    //if (g_eeGeneral.hapticMode == e_mode_all)  TODO new haptic mode?
    if (hapticFiredForEvt != event.seriesId) {
      haptic.play(4, 0, PLAY_BACKGROUND);
      hapticFiredForEvt = event.seriesId;
    }
#endif
  }

  // Check what other points might be doing (this is pretty rough, and only supports 2 touch points)
  if (event.gesture != GEST_NONE && numPoints == 2) {
    TouchPoint * pt2 = nullptr;
    // find next active point starting from first one already found
    for ( ; nextPtIdx < TOUCH_POINTS; ++nextPtIdx) {
      if (m_points[nextPtIdx].state != ST_UP) {
        pt2 = &m_points[nextPtIdx];
        break;
      }
    }
    if (pt2) {
      bool pinchRot = false;
      // if next point has also moved, determine its direction relative to 1st point
      if (!(event.gesture & GEST_TAP) && pt2->startPos.dist(pt2->pos) > TOUCH_TAP_RADIUS_SZ) {
        float scalFact = 0.0f;
        float rotFact = 0.0f;
        //tCoord_t dltaDist = pt->startPos.dist(pt2->startPos) - pt->pos.dist(pt2->pos);
        if (m_detectRotations && fabsf((rotFact = pt->rotation(*pt2))) > 0.1f) {
          event.gesture |= (GEST_ROTATE | (rotFact < 1 ? GEST_DIR_CCW : GEST_DIR_CW));
          pinchRot = true;
        }
        else if ((scalFact = pt->scaleFactor(*pt2)) && fabsf(1.0f - scalFact) > 0.1f) {
          //if (dltaDist && abs(dltaDist) > 5)
          event.gesture |= (GEST_PINCH | (scalFact < 1 ? GEST_DIR_IN : GEST_DIR_OUT));
          pinchRot = true;
        }
      }

      // if we have a pinch or rotate gesture then it's not a drag or swipe
      if (pinchRot) {
        event.gesture &= ~(GEST_DRAG | GEST_SWIPE);
      }
      // else assume 2nd touch point is doing a "complimentary" gesture (eg. tap or drag) and simply set the DOUBLE flag ... this could probably be improved!
      else {
        event.gesture |= GEST_DOUBLE;
      }
      // if our first point is released then we ignore the 2nd point for the rest of the gesture (the raw points will still be queued)
      if (event.gesture & GEST_RELEASE)
        pt2->state |= ST_IGNORE;
    }
  }

  // if we're still dragging/swiping, determine direction
  if (event.gesture & (GEST_DRAG | GEST_SWIPE))
    event.gesture |= dirFromAngle(pt->startPos.angle(pt->pos));

  //if (event.gesture != GEST_NONE)
  enqueue(event);

  //event.debug(0);
}

void TouchManager::enqueue(const Event & ev)
{
  RTOS_LOCK_MUTEX(m_eventQueMtxId);
  if (m_eventQue.isFull())
    m_eventQue.pop();
  m_eventQue.push(ev);
  RTOS_UNLOCK_MUTEX(m_eventQueMtxId);
}

void TouchManager::processQueue(eventCallback_t cb)
{
  //processQueue((eventCallbackL_t)cb, NULL);
  if (!cb || m_eventQue.isEmpty())
    return;

  Event ev;
  tTime_t now = getTime();
  Fifo<Touch::Event, TOUCH_MAX_QUEUE_LEN> tempQ;

  RTOS_LOCK_MUTEX(m_eventQueMtxId);  // lock while copy
  while (m_eventQue.pop(ev))
    tempQ.push(ev);
  RTOS_UNLOCK_MUTEX(m_eventQueMtxId);

  while (tempQ.pop(ev)) {
    if (!ev.pointCount)
      continue;
    RTOS_LOCK_MUTEX(m_callbackMtxId);  // current callback has priority
    const bool ret = cb(ev);
    RTOS_UNLOCK_MUTEX(m_callbackMtxId);
    if (!ret && (now - ev.timestamp) < TOUCH_MAX_QUEUE_TIME)
      enqueue(ev);   // if callback doesn't handle event and it is not too old, add it back to queue
  }
}

void TouchManager::clearQueue()
{
  RTOS_LOCK_MUTEX(m_eventQueMtxId);
  m_eventQue.clear();
  RTOS_UNLOCK_MUTEX(m_eventQueMtxId);
}


//
// Utils
//

tTime_t TouchManager::getTime()
{
  return (tTime_t)TICKS_TO_TIME(CoGetOSTime());
}


//
// Calibration
//

bool TouchManager::setCalibration(CalibMatrix matrix)
{
  if (!isCalibMatrixValid(&matrix))
    return false;

  g_eeGeneral.touchCalib = matrix;
  storageDirty(EE_GENERAL);
  return true;
}

bool TouchManager::isCalibMatrixValid(const CalibMatrix * m)
{
  return (m->Div != 0 && calibrationCrc(m) == m->crc);
}

void TouchManager::initCalibrationMatrix(CalibMatrix * m)
{
  *m = {0, 0, 0, 0, 0, 0, 0};
}

CalibMatrix * TouchManager::getCalibration()
{
  return &g_eeGeneral.touchCalib;
}

bool TouchManager::calibratedPoint(tVect_t * resultPoint, const tVect_t * rawPoint, const CalibMatrix * matrix, bool constrain)
{
  if (!isCalibMatrixValid(matrix))
    return false;

  tVect_t raw(*rawPoint);
  resultPoint->x = ((matrix->An * raw.x) + (matrix->Bn * raw.y) + matrix->Cn) / matrix->Div;
  resultPoint->y = ((matrix->Dn * raw.x) + (matrix->En * raw.y) + matrix->Fn) / matrix->Div;

  if (constrain) {
    resultPoint->x = limit<tVect_t::value_type>(0, resultPoint->x, LCD_W-1);
    resultPoint->y = limit<tVect_t::value_type>(0, resultPoint->y, LCD_H-1);
  }

  return true;
}

bool TouchManager::calcCalibrationMatrix(tsPoint_t * screenPoints, tsPoint_t * touchPoints, CalibMatrix * matrix)
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

uint16_t TouchManager::calibrationCrc(const CalibMatrix * matrix)
{
  uint16_t crc = 0;
  uint8_t * bytes = (uint8_t *)matrix;
  for (int i=0; i < (7*4); ++i)
    crc += *bytes++;
  return crc;
}

void TouchManager::dumpCalibMatrix(const CalibMatrix * m)
{
  TRACE_DEBUG("[CalibMatrix] { %d, %d, %d, %d, %d, %d, %d } crc: 0x%X\n", m->An, m->Bn, m->Cn, m->Dn, m->En, m->Fn, m->Div, m->crc);
}

void RawTrackingPoint::debug() const {
  TRACE_DEBUG("RawTrackingPoint: {st:0x%02X, id:%d, i:%d, ts:%d, pos:(%3d, %2d), rawP:(%3d, %2d);}\n", state, serId, index, ts, pos.x, pos.y, rawPos.x, rawPos.y);
}

void TouchPoint::debug(bool terse, bool wp) const {
  if (!wp)
    TRACE_DEBUG("");

  TRACE_DEBUG_WP("TouchPoint: {id: %d, idx:%d, st:0x%02X, ts:%d, pos:(%3d, %2d), dur:%d, dst:%d%s",
              serId, index,  state, timestamp, pos.x, pos.y, timestamp - startTm, startPos.dist(pos), (terse ? "}\n" : ", "));

  if (terse)
    return;

  const tVect_t vel = velocity();
  TRACE_DEBUG_WP("sTm:%d, sPos:(%3d, %2d), rPos:(%3d, %2d), lPos:(%3d, %2d), lTs:%d, v:(%2d, %2d), hTm:%d, mTm:%d}\n",
                 startTm, startPos.x, startPos.y, rawPos.x, rawPos.y, lastPos.x, lastPos.y, lastTm, vel.x, vel.y, holdTm, moveStartTm);
}

// format (0=terse; 1=tPs terse; 2=tPs verbose
void Event::debug(uint8_t format) const {
  TRACE_DEBUG("Event: {g:0x%04X, pts:%d, id:%d, ts:%d}\n", gesture, pointCount, seriesId, timestamp);

  if (!format)
    return;

  int i = 0;
  for (const TouchPoint & pt: touchPoints) {
    if (i < pointCount-1) {
      TRACE_DEBUG_WP(" (To[%d] rot: %.5f, fact: %f)\n", i+1, pt.rotation(touchPoints.at(i+1)), pt.scaleFactor(touchPoints.at(i+1)));
    }
    TRACE_DEBUG("  p[%d] ", i++);
    pt.debug((format == 1), true);
  }

}


//
// KeyEventEmulator
//

#define KEYEVENTMAP_LEN    10   // key event history size
#define REPT_MIN_DELAY_TM  175  // [ms]

bool KeyEventEmulator::m_suspendKeyEvents = false;

/* NOTES:

  The current event mapping strategy is to use the lookup table (at bottom) to map gestures directly to KEY_EVENT(KEY_n),
   with minimal work by the mapper function. We do track the state somewhat, enough for KillEvents() to work and
   also to provide decrementing repeat rates (like the physical key handler does).

  Another idea is to just map certain gestures to keys (see stub @ bottom), and then determine the appropriate KEY_EVENT to
   fire based on other parameters, like tracking the state of each key.  However I'm not sure this will work any better
   since the basic issue remains that in some situations the KEY_EVENT we're mapping to still isn't what feels natural
   to the user. With physical keys the difference between button press and release are mostly too subtle to notice, but
   when trying to emulate them the difference becomes much more apparent.

  So far I think the best solution lies in the UI responding to touch gestures/events directly, just like key events now.
   This way the most natural gesture choice is determined in the correct place.  The gesture ID has been kept deliberately
   to 16bits so that it could be combined with the current 16b key event type for a 32b total "event type" bitfield.
*/

bool KeyEventEmulator::mapToKeyEvent(const Event & touchEv)
{
  if (touchEv.gesture == GEST_NONE || m_suspendKeyEvents)
    return false;   // signal that we didn't handle the event

  uint16_t keyEvt = 0;
  bool keepKem = false;

  // find key event in gesture mapping
  keymap_t::const_iterator kmi = map().find(touchEv.gesture);
  if (kmi == map().end())
    return false;
  else
    keyEvt = kmi->second;

  // find this touch series in recent history
  int8_t idx = findSeriesInEventMap(touchEv.seriesId);
  if (idx < 0)
    idx = addToEventMap(keyEvt, touchEv.seriesId);

  KeyEventMap * kem = &eventMap()[idx];

  // Out of sequence?  (should not happen)
  if (kem->lastTm > touchEv.timestamp)
    return true;  // maybe better to reject?

  // Event killed?
  if (kem->flags & EFLAG_IGNORE) {
    //TRACE_DEBUG("mapToKeyEvent(): event ignored ");
    //kem->debug();
    return true;
  }

  // if new key then reset some flags (shouldn't really happen, but...)
  if (EVT_KEY_MASK(kem->keyEvt) != EVT_KEY_MASK(keyEvt))
    kem->flags &= ~(EFLAG_ST_FIRST | EFLAG_RPT_DLY);

#if 0
  // Check to make sure there is/was a FIRST event
  // Experimental! works but has side-effects which would need to be compensated for in menu handlers... or something.
  if (IS_KEY_FIRST(keyEvt)) {
    kem->flags |= EFLAG_ST_FIRST;
  }
  else if (!(kem->flags & EFLAG_ST_FIRST)) {
    // If the FIRST event was never fired for this key then we insert one...
    keyEvt = EVT_KEY_FIRST(EVT_KEY_MASK(keyEvt));
    kem->flags |= EFLAG_ST_FIRST;
    // ... and requeue the original event. If other events in this sequence
    // follow before this requeued one, this one will be ignored due to timestamp mismatch.
    TouchManager::instance()->enqueue(Event(touchEv));
    doNotClear = true;
  }
#endif

  // check for repeats and delay them in steps of decreasing length
  if ((kem->flags & EFLAG_RPT_DLY)) {
    if (!IS_KEY_REPT(keyEvt)) {
      // no longer repeating
      kem->flags &= ~EFLAG_RPT_DLY;
    }
    else {
      // skip event if still delaying
      if (touchEv.timestamp - kem->lastTm < kem->delay) {
        //TRACE_DEBUG("mapToKeyEvent: event delay \n");
        //kem->debug();
        return true;
      }
      // event will fire, do we also reduce the delay time?
      else if (kem->delay >= REPT_MIN_DELAY_TM * 2) {
        // reduce delay time in increments
        kem->delay -= REPT_MIN_DELAY_TM;
      }
    }
  }
  else if (IS_KEY_REPT(keyEvt)) {
    // start repeat delay but still fire current event
    kem->flags |= EFLAG_RPT_DLY;
    kem->delay = REPT_MIN_DELAY_TM * 5;  // 5 steps
  }

  kem->keyEvt = keyEvt;
  kem->lastTm = touchEv.timestamp;

  //TRACE_DEBUG("mapToKeyEvent: Event{t:0x%04X, id:%d, dst:%d, dir:0x%02X, dur:%2d} ", touchEv.type, touchEv.seriesId, touchEv.distance, ((touchEv.type & GEST_MSK_DIR) >> 16), touchEv.duration);
  //kem->debug();

  // register the key event
  putEvent(kem->keyEvt);

  // clear the event record if it is finished
  if ((touchEv.gesture & GEST_RELEASE) && !keepKem)
    kem->clear();

  return true;  // signal that we handled the event
}

void KeyEventEmulator::killEvents(uint16_t event)
{
  int8_t idx = findKeyInEventMap(event, EFLAG_NONE);
  if (idx > -1) {
    eventMap()[idx].flags |= EFLAG_IGNORE;
  }
}

int8_t KeyEventEmulator::findKeyInEventMap(uint16_t key, int16_t flags)
{
  int8_t ret = -1;
  for (uint8_t i = 0; i < KEYEVENTMAP_LEN; ++i) {
    if (eventMap()[i].keyEvt == key && (flags == -1 || flags == eventMap()[i].flags)) {
      ret = i;
      break;
    }
  }
  return ret;
}

int8_t KeyEventEmulator::findSeriesInEventMap(uint16_t evt, int16_t flags)
{
  int8_t ret = -1;
  for (uint8_t i = 0; i < KEYEVENTMAP_LEN; ++i) {
    if (eventMap()[i].touchEvt == evt && (flags == -1 || flags == eventMap()[i].flags)) {
      ret = i;
      break;
    }
  }
  return ret;
}

uint8_t KeyEventEmulator::addToEventMap(uint16_t key, uint16_t evt, uint8_t f)
{
  static uint8_t nextIdx = KEYEVENTMAP_LEN - 1;
  nextIdx = (nextIdx + 1) % KEYEVENTMAP_LEN;
  eventMap()[nextIdx] = {key, evt, f, 0, 0};
  return nextIdx;
}

KeyEventEmulator::KeyEventMap * KeyEventEmulator::eventMap()
{
  static KeyEventMap emap[KEYEVENTMAP_LEN];
  return &emap[0];
}

void KeyEventEmulator::KeyEventMap::debug() {
  TRACE_DEBUG_WP("KEM: {keyEvt:0x%04X, tchId:%d, lstTm:%d, f:0x%02X, dly:%d}\n", keyEvt, touchEvt, lastTm, flags, delay);
}


//enum GestureType {
//  GEST_NONE       = 0,
//  GEST_PRESS      = 0x0001,  //! initial touch down, happens only once per series
//  GEST_RELEASE    = 0x0002,  //! point was released, this event should be last in series
//  GEST_HOLD       = 0x0004,  //! touch is being held stationary (opposite of MOVE)
//  GEST_MOVE       = 0x0008,  //! touchpoint is being moved (opposite of HOLD)
//  GEST_TAP        = 0x0010,  //! a tap was registed (a touch event which has not moved outside a maximum radius)
//  GEST_DBLTAP     = 0x0020,  //! a double tap was registed (two quick sequential touches/releases)   TODO
//  GEST_DRAG       = 0x0040,  //! touchpoint was dragged (opposite of TAP)
//  GEST_SWIPE      = 0x0080,  //! swipe gesture (a quick drag)
//  GEST_PINCH      = 0x0100,  //! two-finger pinch
//  GEST_ROTATE     = 0x0200,  //! two-finger rotate  TODO
//  GEST_DIR_N      = 0x0400,
// ....
//  GEST_DOUBLE     = 0x4000,  // two points were used in the gesture, eg. two-finger tap or swipe (NOTE: PINCH and ROTATE do not set this flag)
//  GEST_LONG       = 0x8000,  // gesture has exceeded "long" hold time (only used while HOLD or TAP are set)

//#define _MSK_KEY_BREAK                 0x20
//#define _MSK_KEY_REPT                  0x40
//#define _MSK_KEY_FIRST                 0x60
//#define _MSK_KEY_LONG                  0x80

//! Here we map gestures to key events. But it is difficult to get the "expected" behavior in all situations (menus/popups/editing/etc).
const KeyEventEmulator::keymap_t & KeyEventEmulator::map() {
  static const keymap_t map = {
    // ENTER/MENU key, don't need a REPT event here since nothing uses it
    {GEST_TAPPED,                      EVT_KEY_FIRST(KEY_ENTER)},
    {GEST_LONGTAPPED,                     EVT_KEY_BREAK(KEY_ENTER)},
    {GEST_TAP | GEST_HOLD | GEST_LONG, EVT_KEY_LONG(KEY_ENTER)},
    {GEST_TAPPED | GEST_DOUBLE,        EVT_KEY_BREAK(KEY_ENTER)},
    {GEST_LONGTAPPED | GEST_DOUBLE,       EVT_KEY_LONG(KEY_ENTER)},

    // for UP and DOWN, FIRST makes more sense the way it is used in menus
    {GEST_DRAG_N | GEST_HOLD, EVT_KEY_FIRST(KEY_UP)},
    {GEST_DRAG_S | GEST_HOLD, EVT_KEY_FIRST(KEY_DOWN)},
    // but for LEFT/RIGHT we need to keep LONG, or use some other way to get into settings from main view
    {GEST_DRAG_E | GEST_HOLD, EVT_KEY_LONG(KEY_RIGHT)},
    {GEST_DRAG_W | GEST_HOLD, EVT_KEY_LONG(KEY_LEFT)},

    // repeat on long drag-and-hold
    {GEST_DRAG_N | GEST_HOLD | GEST_LONG,  EVT_KEY_REPT(KEY_UP)},
    {GEST_DRAG_S | GEST_HOLD | GEST_LONG,  EVT_KEY_REPT(KEY_DOWN)},
    {GEST_DRAG_E | GEST_HOLD | GEST_LONG,  EVT_KEY_REPT(KEY_RIGHT)},
    {GEST_DRAG_W | GEST_HOLD | GEST_LONG,  EVT_KEY_REPT(KEY_LEFT)},

    // UP/DOWN long on double drag-hold N/S (need this to get into eg. telemetry screens)
    {GEST_DRAG_N | GEST_DOUBLE | GEST_HOLD, EVT_KEY_LONG(KEY_UP)},
    {GEST_DRAG_S | GEST_DOUBLE | GEST_HOLD, EVT_KEY_LONG(KEY_DOWN)},

    // break on release...
    {GEST_DRAG_N | GEST_RELEASE,  EVT_KEY_BREAK(KEY_UP)},
    {GEST_DRAG_S | GEST_RELEASE,  EVT_KEY_BREAK(KEY_DOWN)},
    {GEST_DRAG_E | GEST_RELEASE,  EVT_KEY_BREAK(KEY_RIGHT)},
    {GEST_DRAG_W | GEST_RELEASE,  EVT_KEY_BREAK(KEY_LEFT)},

    // ...and also for swipe (but L & R are reversed!)
    {GEST_SWIPED_N, EVT_KEY_BREAK(KEY_UP)},
    {GEST_SWIPED_S, EVT_KEY_BREAK(KEY_DOWN)},
    {GEST_SWIPED_E, EVT_KEY_BREAK(KEY_LEFT)},
    {GEST_SWIPED_W, EVT_KEY_BREAK(KEY_RIGHT)},

    // all the ways to exit....  all use 2 fingers and none work great if the touch controller doesn't detect multi-fingers well :(
    // This should be tuned and/or alternate EXIT strategy implemented.
    {GEST_DRAG_W | GEST_DOUBLE | GEST_MOVE,                 EVT_KEY_FIRST(KEY_EXIT)},
    {GEST_DRAG_W | GEST_DOUBLE | GEST_HOLD,                 EVT_KEY_LONG(KEY_EXIT)},
    {GEST_DRAG_W | GEST_DOUBLE | GEST_HOLD | GEST_LONG,     EVT_KEY_REPT(KEY_EXIT)},
    {GEST_DRAG_W | GEST_DOUBLE | GEST_RELEASE,              EVT_KEY_BREAK(KEY_EXIT)},

    {GEST_SWIPED_W | GEST_DOUBLE,    EVT_KEY_BREAK(KEY_EXIT)},

    {GEST_PINCH_IN | GEST_MOVE,     EVT_KEY_FIRST(KEY_EXIT)},
    {GEST_PINCH_IN | GEST_HOLD,     EVT_KEY_LONG(KEY_EXIT)},
    {GEST_PINCH_IN | GEST_RELEASE,  EVT_KEY_BREAK(KEY_EXIT)},
  };

  return map;
}

#if 0
KeyEventEmulator::KeyEventMap * KeyEventEmulator::getKeyEvent(const Event & touchEv)
{
  uint16_t keyEvt = 0;

  // find key event in gesture mapping
  keymap_t::const_iterator kmi = map().find(touchEv.type);
  if (kmi == map().end())
    return NULL;
  else
    keyEvt = kmi->second;

  // find this touch series in recent history
  int8_t idx = findSeriesInEventMap(touchEv.seriesId);
  if (idx < 0)
    idx = addToEventMap(keyEvt, touchEv.seriesId);

  KeyEventMap * kem = eventMap()[idx];
  kem->keyEvt = keyEvt;
  return kem;

  //  event_t evt = getEvent(false);

  //  if ((touchEv.type & GEST_HOLD) && touchEv.duration > 150 && !evt /*(evt & 0xF) == KEY_ENTER*/) {
  //    key = KEY_ENTER;
  //    keyEvt = EVT_KEY_FIRST(key);
  //  }
  //  else if ((touchEv.type & GEST_SWIPE)) {
  //    keyEvt = EVT_KEY_BREAK(key);
  //  }
  //  //if (touchEv.type & GEST_PRESS)
  //   // keyEvt = EVT_KEY_FIRST(key);
  //  else if (touchEv.type & GEST_RELEASE)
  //    keyEvt = EVT_KEY_BREAK(key);
  //  else if (touchEv.duration >= KEY_REPEAT_DELAY * 10)
  //    keyEvt = EVT_KEY_REPT(key);
  //  else if (touchEv.duration >= KEY_LONG_DELAY * 10)
  //    keyEvt = EVT_KEY_LONG(key);
}

const KeyEventEmulator::keymap_t & KeyEventEmulator::evt2KeyMap()
{
  static const keymap_t map = {
    {GEST_TAP,    KEY_ENTER},

    {GEST_DRAG_N, KEY_UP},
    {GEST_DRAG_S, KEY_DOWN},
    {GEST_DRAG_E, KEY_RIGHT},
    {GEST_DRAG_W, KEY_LEFT},

    {GEST_SWIPE_N, KEY_UP},
    {GEST_SWIPE_S, KEY_DOWN},
    {GEST_SWIPE_E, KEY_RIGHT},
    {GEST_SWIPE_W, KEY_LEFT},

    {GEST_DRAG_W | GEST_DOUBLE,  KEY_EXIT},
    {GEST_SWIPE_W | GEST_DOUBLE, KEY_EXIT},
  };

  return map;
}
#endif
