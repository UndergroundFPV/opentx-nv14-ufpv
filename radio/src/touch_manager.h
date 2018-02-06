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

#include "board.h"
#include "definitions.h"
#include "geometry_types.h"
#include "fifo.h"
#include "tasks_arm.h"

#include <cinttypes>
#include <unordered_map>
#include <utility>
#include <vector>
//#include <functional>

#ifndef TOUCH_COORD_UNIT_TYPE
  #define TOUCH_COORD_UNIT_TYPE  int16_t  //! size of type used for coordinates, should be signed or real
#endif
#define TOUCH_MAX_QUEUE_LEN      16    //! Maximum number of events to buffer into queue. Queue if FIFO-style. Must be power of two.
#define TOUCH_MAX_QUEUE_TIME     2000  //! [ms] Expire queued events after this time (if not handled by any processors).

#define TOUCH_TAP_MIN_TM         35    //! [ms] Min time before a touch counts as a tap (debounce).
#define TOUCH_LONG_EVT_TM        900   //! [ms[ Time after which an event is considered "long" (eg. for a long tap).
#define TOUCH_TAP_COUNT_TM       150   //! [ms] Max time between taps to count them as consecutive, set to zero to disable double-tap detection and report single taps immediately. TODO
#define TOUCH_HOLD_MIN_TM        240   //! [ms] Min time a stationary touch counts as being held.
#define TOUCH_SWIPE_MAX_TM       300   //! [ms] Max time while a move counts as a swipe.
#define TOUCH_SWIPE_MIN_DIST     5     //! [px] Minimum distance for a move to count as a swipe.
#define TOUCH_TAP_RADIUS_SZ      4     //! [px] Radius within which a touch is still a tap vs. drag, (consecutive taps must be within same radius).
#define TOUCH_MIN_MOVE_DIST      2     //! [px] Minimum normalized distance before a move event is registered (prevents jitter).


namespace Touch {

struct Event;  // forward

typedef TOUCH_COORD_UNIT_TYPE tCoord_t;  //! coordinates type unit, should be signed or real
typedef Vector2T<tCoord_t> tVect_t;      //! vector type unit
typedef uint32_t tTime_t;                //! time type unit (also event serial IDs are based on time)

/*!
   Touch event processor callback function signature. The callback should return true if it handled the event, to avoid further propagation. \sa TouchManager
*/
typedef bool (*eventCallback_t)(const Touch::Event &);
// std::function uses dynamic memory allocation, though otherwise it would be "cleaner" for lambda callbacks than the TouchManager::eventCallbackLambda() trick (see below)
//typedef std::function<bool (const Touch::Event &)> eventCallback_t;

//! There can be many gesture types... WIP. Note that gesture is ongoing unless GEST_RELEASE flag is set.
enum GestureType {
  GEST_NONE       = 0,
  GEST_PRESS      = 0x0001,  //! initial touch down, happens only once per series
  GEST_RELEASE    = 0x0002,  //! point was released, this event should be last in series
  GEST_HOLD       = 0x0004,  //! touch is being held stationary (opposite of MOVE)
  GEST_MOVE       = 0x0008,  //! touchpoint is being moved (opposite of HOLD)
  GEST_TAP        = 0x0010,  //! a tap was registed (a touch event which has not moved outside a maximum radius)
  GEST_DBLTAP     = 0x0020,  //! a double tap was registed (two quick sequential touches/releases)   TODO
  GEST_DRAG       = 0x0040,  //! touchpoint was dragged (opposite of TAP)
  GEST_SWIPE      = 0x0080,  //! swipe gesture (a quick drag)
  GEST_PINCH      = 0x0100,  //! multi-finger pinch
  GEST_ROTATE     = 0x0200,  //! multi-finger rotate  (only if enabled, see \e setDetectRotations() )

  GEST_DIR_N      = 0x0400,  //! predominant gesture direction North (up)
  GEST_DIR_E      = 0x0800,  //! predominant gesture direction East (right)
  GEST_DIR_S      = 0x1000,  //! predominant gesture direction South (down)
  GEST_DIR_W      = 0x2000,  //! predominant gesture direction West (left)

  GEST_DOUBLE     = 0x4000,  //! two points were used in the gesture, eg. two-finger tap or swipe (NOTE: PINCH and ROTATE do not set this flag)
  GEST_LONG       = 0x8000,  //! gesture has exceeded "long" hold time (only used while HOLD or TAP are set)

  // save some bits since these are exlusive anyway
  GEST_DIR_IN     = GEST_DIR_N,  //! multi-touch with predominantly inward movement (zoom out)
  GEST_DIR_OUT    = GEST_DIR_E,  //! multi-touch with predominantly outward movement (zoom in)
  GEST_DIR_CW     = GEST_DIR_S,  //! multi-touch with predominantly clockwise movement (rotation)
  GEST_DIR_CCW    = GEST_DIR_W,  //! multi-touch with predominantly counter-clockwise movement (rotation)

  // completed events (these could be used for direct == comparison)
  GEST_TAPPED     = GEST_TAP | GEST_RELEASE,                 //! single tap (quick touch and release)
  GEST_DBLTAPPED  = GEST_DBLTAP | GEST_RELEASE,              //! tapped with two fingers
  GEST_LONGTAPPED = GEST_TAP | GEST_RELEASE | GEST_LONG,     //! a long tap (long touch and release)
  GEST_SWIPED_N   = GEST_SWIPE | GEST_DIR_N | GEST_RELEASE,
  GEST_SWIPED_E   = GEST_SWIPE | GEST_DIR_E | GEST_RELEASE,
  GEST_SWIPED_S   = GEST_SWIPE | GEST_DIR_S | GEST_RELEASE,
  GEST_SWIPED_W   = GEST_SWIPE | GEST_DIR_W | GEST_RELEASE,
  // potentially ongoing events (will be combined with other flags, use bitwise comparisons only)
  GEST_IS_DOWN    = GEST_PRESS | GEST_HOLD | GEST_MOVE,   //! gesture is still active/ongoing if one of these flags is set
  GEST_DRAG_N     = GEST_DRAG | GEST_DIR_N,
  GEST_DRAG_E     = GEST_DRAG | GEST_DIR_E,
  GEST_DRAG_S     = GEST_DRAG | GEST_DIR_S,
  GEST_DRAG_W     = GEST_DRAG | GEST_DIR_W,
  GEST_PINCH_IN   = GEST_PINCH | GEST_DIR_IN,
  GEST_PINCH_OUT  = GEST_PINCH | GEST_DIR_OUT,
  GEST_ZOOM_OUT   = GEST_PINCH_IN,
  GEST_ZOOM_IN    = GEST_PINCH_OUT,
  GEST_ROTATE_CW  = GEST_ROTATE | GEST_DIR_CW,
  GEST_ROTATE_CCW = GEST_ROTATE | GEST_DIR_CCW,
  // masks
  GEST_MSK_EVENT  = 0x03FF,  // transient: 0x000F, extended: 0x03F0
  GEST_MSK_DIR    = 0x3C00,  //! predominant direction of movement
  GEST_MSK_FLAGS  = 0xC000,  //! other flags
};

enum TpState {
  ST_UP       = 0,     //! not touching (after release) or otherwise unknown
  ST_PRESS    = 0x01,  //! initial touch down (first event in series)
  ST_RELEASE  = 0x02,  //! touch release (final event in series)
  ST_HOLD     = 0x04,  //! touch is being held stationary (opposite of MOVE) - only reported if enabled in driver
  ST_MOVE     = 0x08,  //! touchpoint is being moved (opposite of HOLD) - only reported if enabled in driver
  ST_TOUCH    = 0x40,  //! is being touched (== !(UP || RELEASE))
  ST_IGNORE   = 0x80,  //! point is being ignored (internal use)
};

/*! \brief The Touch::RawTrackingPoint is generated for each touch point by the low-level touch driver or other input interface. */
struct RawTrackingPoint
{
  uint8_t state;             // TpState
  uint8_t index;             // touchpoint index
  Touch::tTime_t serId;      // unique ID of event chain (start time ^ (index + 1))
  Touch::tVect_t pos;        // touch coordinates
  Touch::tVect_t rawPos;     // before flip/rotate
  Touch::tTime_t ts;         // timestamp of this event (as reported by TouchManager::getTime())

  void debug() const;
};

/*! \brief The Touch::TouchPoint is generated by the TouchManager for each touch point based on RawTrackingPoint events from driver/input. */
struct TouchPoint {
  uint8_t state;               //! TpState
  uint8_t index;               //! touchpoint index
  Touch::tTime_t serId;        //! unique event series ID (corresponds to RawTrackingPoint series ID)
  Touch::tVect_t pos;          //! [px] current calibrated screen position of event
  Touch::tVect_t rawPos;       //! current position in raw uncalibrated coordinates (eg. used for calibration)
  Touch::tTime_t timestamp;    //! [ms] timestamp of current event
  Touch::tTime_t startTm;      //! [ms] event start timestamp
  Touch::tVect_t startPos;     //! [px] event start position (calibrated screen coordinates)
  Touch::tTime_t lastTm;       //! [ms] timestamp of previous event in this series (zero on series start)
  Touch::tVect_t lastPos;      //! [px] position of previous event in this series in calibrated coordinates (invalid on series start)
  Touch::tTime_t holdTm;       //! [ms] elapsed time w/out moving
  Touch::tTime_t moveStartTm;  //! [ms] time first move detected

  //! Returns current speed of movement in each direction as absolute [px/s].
  inline Touch::tVect_t velocity() const
  {
    return pos.distVect(lastPos) / ((timestamp - lastTm) / 1e3f);
  }

  //! Returns scale factor relative to another point since start of history. Values > 1 are outward (points are getting further apart), < 1 are inward.
  inline float scaleFactor(const TouchPoint & other) const
  {
    if (startPos.isNull() && other.startPos.isNull())
      return 0.0f;
    return Vector2F(pos).dist(other.pos) / Vector2F(startPos).dist(other.startPos);
  }

  //! Returns rotation angle in [rad] relative to another point since start of history. Positive values are CW, negative are CCW.
  inline float rotation(const TouchPoint & other) const
  {
    tVect_t startDir = startPos - other.startPos;
    tVect_t currDir = pos - other.pos;
    if (startDir.isNull() && currDir.isNull())
      return 0.0f;
    float crossNorm = startDir.cross(currDir) / (startDir.len() * currDir.len());
    return asinf(crossNorm);
  }

  void debug(bool terse = false, bool wp = false) const;
};

/*! \brief The Touch::Event structure describes a touch event generated by this class based on TouchPoint input. */
struct Event
{
  uint16_t gesture;     //! GestureType
  uint8_t pointCount;   //! number of touchpoint(s) involved in event
  tTime_t timestamp;    //! [ms] a time stamp for the event in units reported by TouchManager::getTime()
  tTime_t seriesId;     //! a unique ID assigned when this touch event first started, could be used to track a series (corresponds to TouchPoint series ID)
  std::vector<TouchPoint> touchPoints;  //! touch point(s) participating in this event, always at least \e pointCount in size

  Event() : Event(GEST_NONE) {}
  Event(uint16_t type, uint8_t count = 0, tTime_t ts = 0, tTime_t serId = 0) :
    gesture(type), pointCount(count), timestamp(ts), seriesId(serId)
  {
    if (pointCount)
      touchPoints.resize(pointCount);
  }

  //! \a format (0=terse; 1=print touchPoints in terse format; 2=print tPs in verbose format
  void debug(uint8_t format = 0) const;
};

//! touchpoint struct, used in calibration screen and reusableBuffer union (must stay simple type)
typedef struct {
  int16_t x;
  int16_t y;
} tsPoint_t;

//! calibration matrix (packed for use in radio data struct)
PACK(struct CalibMatrix {
  int32_t An;
  int32_t Bn;
  int32_t Cn;
  int32_t Dn;
  int32_t En;
  int32_t Fn;
  int32_t Div;
  uint16_t crc;
});

}  // namespace Touch

/*!
  \brief TouchManager class processes raw touch input from a basic touchscreen driver and converts that data into more meaninful events.

  The current interface to the touch events is via \e TouchManager::processQueue(eventCallback_t) which takes a callback function as argument. This is called in turn for each event
  currently in the queue. The callback function must return true or false to indicate if it handled the event or not.  If the event was not hanlded by the callback, it is added back
  to the queue, otherwise it is dicarded.

  The callback function signature (typedef eventCallback_t) is: \e bool(*)(const Touch::Event &)

  Callbacks can be regular or lambda functions. If the lambda does not need to capture, then it can be used like any other function. (Note: Global and any static vars do not need to be captured.)

  \example Example using a non-capturing lambda callback function:
  \code
    static tVect_t lastCoords = {-1, -1};  // example local static variable
    // callback for touch event processor (no capture)
    eventCallback_t cb = [](const Event & ev) {
      if (ev.type == GEST_TAPPED) {
        lastCoords = ev.point;
        lcdDrawNumber(0, 0, ev.point.x);  // a global function
        return true;
      }
      return false;
    };
    // process touch events, note the lambda is passed directly
    TouchManager::instance()->processQueue(cb);
    lcdDrawNumber(0, 10, lastCoords.x);
  \endcode

  \example Example using a capturing lambda callback function. It is necessary to use the \e eventCallbackLambda() wrapper.
  \code
    tVect_t tapCoords = {-1, -1};  // example local variable
    // callback for touch event processor (note "&" capture, needs to be declared "auto" type instead of eventCallback_t because of this)
    auto cb = [&](const Event & ev) {
      if (ev.type == GEST_TAPPED) {
        tapCoords = ev.point;
        return true;
      }
      return false;
    };
    // process touch events, note that we wrap the lambda in TouchManager::eventCallbackLambda()
    TouchManager::instance()->processQueue(TouchManager::eventCallbackLambda(cb));
    // continue with function code
    if (tapCoords.x > -1) {
      ....
    }
  \endcode
*/
class TouchManager
{
  public:
    TouchManager();
    ~TouchManager() {}

    /*! \brief Return a static instance of this object. Although TouchManager could be instantiated multiple times,
       there is proably no reason to do so unless one has mutliple touch devices. */
    static TouchManager * instance();

    //! Initializes the touch driver and creates TouchManager task.
    bool init();

    //! Process the touch event queue using given \a cb as callback function. The callback should return \e true if it handled the event, or \e false otherwise.
    void processQueue(Touch::eventCallback_t cb);
    //! Remove all touch events from pending queue.
    void clearQueue();
    //! Place an event in the queue
    void enqueue(const Touch::Event & ev);
    //! Used by touch driver to signal that new data has arrived.
    void driverDataReady(uint8_t numPoints);
    //! Register a raw touch event. This is typically used only internally after driver signals that data is available.
    bool rawEvent(const Touch::RawTrackingPoint & rtp);

    //! Rotation gesture detection is computationally expensive and is disabled by default.
    void setDetectRotations(bool on) { m_detectRotations = on; }

    //! A utility function for wrapping capturing lambda callbacks for processQueue() (this trick avoids dynamic memory allocations of using std::function)
    //! This is not reentrant, so TouchManager protects all callback calls with an exclusive mutex.
    template <typename Tfunc>
    static inline Touch::eventCallback_t eventCallbackLambda(Tfunc func) {
      static Tfunc fn = func;
      return [](const Touch::Event & ev) -> bool { return fn(ev); };
    }

    //! A central place to get a timestamp for all touch-related activities, this ensures consistency.
    static Touch::tTime_t getTime();

    // These functions are used by the main task management system to get process and stack info.
    static OS_TID taskId() { return m_taskId; }
    static TaskStack<TOUCH_STACK_SIZE> & taskStack() { return m_taskStack; }
    //! TouchManager task code. Do not call this directly, it's only public because it needs to be for task manager.
    void run(void * /*pdata*/);

    // calibration
    static Touch::CalibMatrix * getCalibration();
    static bool setCalibration(Touch::CalibMatrix matrix);
    static void initCalibrationMatrix(Touch::CalibMatrix * m);
    static bool isCalibMatrixValid(const Touch::CalibMatrix * m);
    static bool isCurrentCalibrationValid() { return isCalibMatrixValid(getCalibration()); }
    static uint16_t calibrationCrc(const Touch::CalibMatrix * matrix);
    static void dumpCalibMatrix(const Touch::CalibMatrix * m);

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
    static bool calcCalibrationMatrix(Touch::tsPoint_t * screenPoints, Touch::tsPoint_t * touchPoints, Touch::CalibMatrix * matrix);

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
    static bool calibratedPoint(Touch::tVect_t * resultPoint, const Touch::tVect_t * rawPoint,  const Touch::CalibMatrix * matrix, bool constrain = true);

  protected:
    void generateEvent(uint8_t numPoints);

    bool m_detectRotations;
    Touch::TouchPoint m_points[TOUCH_POINTS];
    Fifo<Touch::Event, TOUCH_MAX_QUEUE_LEN> m_eventQue;

    OS_MutexID m_eventQueMtxId;
    OS_MutexID m_callbackMtxId;
    static OS_TID m_taskId;
    static TaskStack<TOUCH_STACK_SIZE> _ALIGNED(8) m_taskStack;
};


/*!
   \brief The KeyEventEmulator class is for converting touch events to \e EVT_KEY_*(KEY_*) events.
 */
class KeyEventEmulator
{
    enum EventFlags {
      EFLAG_NONE        = 0,
      EFLAG_IGNORE      = 0x01,
      EFLAG_RPT_DLY     = 0x02,
      EFLAG_ST_FIRST    = 0x04,
    };

    // NOTE: all times are actually system ticks, or whatever is reported by TouchManager::getTime()
    struct KeyEventMap {
      uint16_t keyEvt;        //! (KEY_EVT & KEY)
      uint16_t touchEvt;      //! Touch::Event::serialId
      uint8_t flags;          //! EventFlags
      uint16_t delay;         //! [ms] current delay time
      Touch::tTime_t lastTm;  //! [ms] Touch::Event::timestamp

      void clear() { *this = {0, 0, 0, 0, 0}; }
      void debug();
    };

  public:
    KeyEventEmulator() {}

    //! Suspend/resume mapping of touch events to key events.
    static void suspendKeyEvents(bool suspend = true) { m_suspendKeyEvents = suspend; }
    static bool keyEventsSuspended() { return m_suspendKeyEvents; }
    static void killEvents(uint16_t event);
    static bool mapToKeyEvent(const Touch::Event & touchEv);

  protected:
    typedef std::unordered_map<uint16_t, uint16_t> keymap_t;

    static int8_t findKeyInEventMap(uint16_t key, int16_t flags = -1);
    static int8_t findSeriesInEventMap(uint16_t evt, int16_t flags = -1);
    static uint8_t addToEventMap(uint16_t key, uint16_t evt, uint8_t f = 0);
    static KeyEventMap * eventMap() NOINLINE;
    static const keymap_t & map() NOINLINE;
    //static KeyEventMap * getKeyEvent(const Touch::Event & touchEv);
    //static const keymap_t & evt2KeyMap() NOINLINE;

    static bool m_suspendKeyEvents;
};

#endif // TOUCH_MANAGER_H
