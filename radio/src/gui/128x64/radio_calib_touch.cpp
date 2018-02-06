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
#include "touch_driver.h"
#include "touch_manager.h"
#include "geometry_types.h"
#include "stdio.h"
#include <cmath>

// TODO: Translations!
#define STR_MENU_TOUCH_CALIB             "TOUCH CALIBRATION"
#define STR_MENU_TOUCH_CALIB_LEN         17
#define STR_MENU_TOUCH_TEST              "TOUCH DIAGNOSTIC"
#define STR_MENU_TOUCH_TEST_LEN          16

// TODO: Move useful GUI helpers to central location

enum Alignment {
  ALIGN_NONE     = 0,
  ALIGN_RIGHT    = 0x001,
  ALIGN_CTR      = 0x002,
  ALIGN_LEFT     = 0x004,
  ALIGN_TOP      = 0x008,
  ALIGN_MID      = 0x010,
  ALIGN_BOT      = 0x020,

  ALIGN_TOP_RIGHT   = ALIGN_TOP | ALIGN_RIGHT,
  ALIGN_TOP_LEFT    = ALIGN_TOP | ALIGN_LEFT,
  ALIGN_TOP_CTR     = ALIGN_TOP | ALIGN_CTR,
  ALIGN_MID_RIGHT   = ALIGN_MID | ALIGN_RIGHT,
  ALIGN_MID_LEFT    = ALIGN_MID | ALIGN_LEFT,
  ALIGN_MID_CENTER  = ALIGN_MID | ALIGN_CTR,
  ALIGN_BOT_RIGHT   = ALIGN_BOT | ALIGN_RIGHT,
  ALIGN_BOT_LEFT    = ALIGN_BOT | ALIGN_LEFT,
  ALIGN_BOT_CENTER  = ALIGN_BOT | ALIGN_CTR,
};

enum ButtonState {
  STATE_NORMAL = 0,
  STATE_ACTIVE,
  STATE_DISABLED,
  STATE_HIDDEN,
  STATE_ENUM_COUNT
};

enum ButtonCheckState {
  CHKSTATE_UNCHECKED = 0,
  CHKSTATE_CHECKED,
  CHKSTATE_PARTCHECKED,
  CHKSTATE_ENUM_COUNT
};

struct WidgetStyle {
  uint8_t lcdPatt;
  LcdFlags lcdFlags;
  LcdFlags lblLcdFlags;
};

#define BTN_MAX_LBL_LEN   LCD_COLS

struct ButtonData {
  uint8_t id;
  ButtonState state;
  ButtonCheckState chkState;
  Rect geo;
  char label[BTN_MAX_LBL_LEN];
  Margins labelPadding;  // [px] padding size around label text when auto-determining h/w of btn
  bool checkable;
  WidgetStyle style[STATE_ENUM_COUNT];
  Rect origGeo;
  uint8_t align;  // Alignment
  Rect checkboxGeo;
  uint8_t lblLen;

  ButtonData(uint8_t id, const Rect & geo, const char * label = nullptr, uint8_t align = ALIGN_LEFT, bool checkable = false, LcdFlags dfltLblFlg = 0, const Margins & labelPadding = Margins(3, 2, 3, 2)) :
    id(id), state(STATE_NORMAL), chkState(CHKSTATE_UNCHECKED), geo(geo), labelPadding(labelPadding), checkable(checkable), origGeo(geo), align(align), lblLen(0)
  {
    const WidgetStyle defStyle[] = { {DOTTED, FORCE|ROUND, dfltLblFlg}, {SOLID, FORCE|ROUND, dfltLblFlg|INVERS}, {DOTTED, FORCE|ROUND, dfltLblFlg} };
    memcpy(&this->style, &defStyle,  sizeof(defStyle));
    if (checkable)
      checkboxGeo = Rect(0, 0, 7 + labelPadding.left(), 7 + labelPadding.vertical());

    setLabel(label, true);
  }

  void setLabel(const char * lbl = 0, bool resetWidth = false)
  {
    if (lbl) {
      strncpy(label, lbl, BTN_MAX_LBL_LEN);
      lblLen = strlen(label);
    }
    else {
      label[0] = 0;
      lblLen = 0;
    }

    if (resetWidth) {
      resetSize();
    }
  }

  void resetSize()
  {
    if (lblLen) {
      if (!origGeo.w)
        geo.w = getTextWidth(label, lblLen, style[state].lblLcdFlags) + labelPadding.horizontal();
      if (!origGeo.h)
        geo.h = lcdFontScaledSize('h', style[state].lblLcdFlags, 1) + labelPadding.vertical();
    }
    if (checkable) {
      checkboxGeo = Rect(0, 0, 7 + (lblLen ? labelPadding.left() : labelPadding.horizontal()), 7 + labelPadding.vertical());
      if (!origGeo.w)
        geo.w += checkboxGeo.w;
      if (!origGeo.h && geo.h < checkboxGeo.h)
        geo.h = checkboxGeo.h;
    }
    resetPos();
  }

  void resetPos()
  {
    if (align & ALIGN_CTR)
      geo.x = origGeo.x - geo.w / 2;
    else if (align & ALIGN_RIGHT)
      geo.x = origGeo.x - geo.w;

    if (align & ALIGN_MID)
      geo.y = origGeo.y - geo.h / 2;
    else if (align & ALIGN_BOT)
      geo.y = origGeo.y - geo.h;

    checkboxGeo.setPosition(geo.x + labelPadding.left(), geo.center().y - roundf(checkboxGeo.h / 2) + 1);
  }
};

/*!
   \brief Draws a string centered horizontally and vertically inside given rectangle \a within.
   \param within Screen coordinates rectangle to position the text within, with \e within.x and \e within.y
                 representing the top-left corner and \e within.x + \e within.w the right side, etc. This could
                  be e.g. a button, screen area, {0, 0, LCD_W, LCD_H} (to center on whole screen), etc.
   \param margin Add extra padding around the text.
   \param len    Pass the string length if it already known (saves time).

   TODO: Properly calculate string height, for now assumes one line of font size specified in \a flags.
*/
void drawCenteredText(Rect within, const char * txt, LcdFlags flags = 0, uint8_t margin = 0, uint16_t len = 0)
{
  const uint8_t txtW = getTextWidth(txt, (len ? len : strlen(txt)), flags);
  const coord_t x = within.x + (within.w - margin * 2 - txtW) / 2 + margin;
  const coord_t y = within.y + (within.h - margin * 2 - lcdFontScaledSize('h', flags, 1)) / 2 + margin;
  lcdDrawText(x, y, txt, flags);
}

/*! \brief Draws a string centered horizontally within \e LCD_W at vertical position \a y.
  \sa drawCenteredText(Rect, const char *, LcdFlags, uint8_t, uint16_t)  */
void drawCenteredText(coord_t y, const char * txt, LcdFlags flags = 0, uint16_t len = 0)
{
  drawCenteredText({0, y, LCD_W, lcdFontScaledSize('h', flags, 1)}, txt, flags, 0, len);
}

void drawButton(ButtonData & btnData)
{
  if (btnData.geo.w < 2 || btnData.geo.h < 2)
    return;

  if (btnData.state != STATE_NORMAL)
    lcdDrawFilledRect(btnData.geo.x, btnData.geo.y, btnData.geo.w, btnData.geo.h, btnData.style[btnData.state].lcdPatt, btnData.style[btnData.state].lcdFlags);
  else
    lcdDrawRect(btnData.geo.x, btnData.geo.y, btnData.geo.w, btnData.geo.h, btnData.style[btnData.state].lcdPatt, btnData.style[btnData.state].lcdFlags);
  if (btnData.checkable) {
    if (btnData.chkState == CHKSTATE_PARTCHECKED)
      lcdDrawChar(btnData.checkboxGeo.x+1, btnData.checkboxGeo.y, '*');
    drawCheckBox(btnData.checkboxGeo.x, btnData.checkboxGeo.y, (btnData.chkState == CHKSTATE_CHECKED), ((btnData.style[btnData.state].lblLcdFlags & INVERS) ? 1 : 0));
  }
  if (btnData.lblLen && btnData.state != STATE_DISABLED) {  // disabled state text is hard on 1bit display  :/
    Rect g = btnData.geo;
    g.setLeft(g.x + btnData.checkboxGeo.w);
    g -= btnData.labelPadding;
    drawCenteredText(g, btnData.label, btnData.style[btnData.state].lblLcdFlags, 1, btnData.lblLen);
  }
}


//
// Actual calibration screen stuff starts here
//

using namespace Touch;

enum CalibrationState {
  CALIB_RESTART = 0,
  CALIB_START,
  CALIB_POINT_1,
  CALIB_POINT_2,
  CALIB_POINT_3,
  CALIB_STORE,
  CALIB_COMPLETE,
  CALIB_TEST,
  CALIB_TEST_DRAW,
  CALIB_TEST_CTRLS,
  CALIB_FINISHED,
  CALIB_PG_PREV,
  CALIB_PG_NEXT
};

enum {
  MENU_BTN_NONE,
  MENU_BTN_START   = CALIB_POINT_1 + 1,
  MENU_BTN_RECAL   = CALIB_START + 1,
  MENU_BTN_TEST    = CALIB_TEST + 1,
  MENU_BTN_DRAW    = CALIB_TEST_DRAW + 1,
  MENU_BTN_CNTRLS  = CALIB_TEST_CTRLS + 1,
  MENU_BTN_EXIT    = CALIB_FINISHED + 1,
  MENU_BTN_PG_PREV = CALIB_PG_PREV + 1,
  MENU_BTN_PG_NEXT = CALIB_PG_NEXT + 1,
  MENU_BTN_BACK
};

void nextState(uint8_t state)
{
  reusableBuffer.touchCal.state = state;
  reusableBuffer.touchCal.newState = true;
}

void calibDrawCoords(coord_t x, coord_t y, const Touch::tVect_t & point, uint8_t idx = 0, LcdFlags flg = SMLSIZE)
{
  lcdDrawText(x, y, (idx ? "X2:" :  "X1:"), flg);
  lcdDrawNumber(lcdLastRightPos, y, point.x, flg|LEADING0, 3);
  lcdDrawText(lcdLastRightPos+4, y, (idx ? "Y2:" :  "Y1:"), flg);
  lcdDrawNumber(lcdLastRightPos+1, y, point.y, flg|LEADING0, 2);
}

void drawCalibInitialScreen(uint8_t menuType)
{
  const coord_t btnW = 50, btnH = 15;

  static ButtonData buttons[] {
    ButtonData(MENU_BTN_EXIT, {5, LCD_H - btnH - 1, btnW, btnH}, "Exit"),
    ButtonData(MENU_BTN_TEST, {LCD_W - btnW - 5, LCD_H - btnH - 1, btnW, btnH}, "Test"),
    ButtonData(MENU_BTN_START, {0, 0, LCD_W, LCD_H}, 0),
  };

  // First enter of this screen, setup/reset
  if (reusableBuffer.touchCal.newState) {
    TouchManager::instance()->clearQueue();

    // reset buttons
    for (size_t i = 0, end = DIM(buttons); i < end; ++i)
      buttons[i].state = STATE_NORMAL;

    reusableBuffer.touchCal.newState = false;
  }

  if (menuType == 1) {
    TITLE(STR_MENU_TOUCH_CALIB);
    drawScreenIndex(MENU_RADIO_CALIB_TOUCH, DIM(menuTabGeneral), 0);
  }
  else {
    drawCenteredText(0, STR_MENU_TOUCH_CALIB, INVERS, STR_MENU_TOUCH_CALIB_LEN);
  }

  if (reusableBuffer.touchCal.state == CALIB_RESTART)
      drawCenteredText(MENU_HEADER_HEIGHT+2, "Calib. failed, try again.", 0, 25);
  drawCenteredText(MENU_HEADER_HEIGHT+2+FH+2, "Touch anywhere to start.", 0, 24);

  bool showBtns = TouchManager::isCurrentCalibrationValid();
  for (size_t i = 0, end = DIM(buttons) - 1; i < end; ++i) {
    if (showBtns)
      drawButton(buttons[i]);
    else
      buttons[i].state = STATE_HIDDEN;
  }

  // callback for touch event processor (no-capture lambda)
  Touch::eventCallback_t cb = [](const Event & ev) {
    if (!ev.pointCount)
      return false;

    bool accepted = false;
    const tVect_t pos = ev.touchPoints.at(0).pos;

    for (size_t i = 0, end = DIM(buttons); i < end; ++i) {
      if (!accepted && buttons[i].state < STATE_DISABLED && buttons[i].geo.contains(pos, -1)) {
        buttons[i].state = STATE_ACTIVE;
        if (ev.gesture == GEST_TAPPED) {
          nextState(buttons[i].id - 1);
          return true;
        }
      }
      else if (buttons[i].state == STATE_ACTIVE) {
        buttons[i].state = STATE_NORMAL;
      }
    }
    if (ev.gesture == GEST_TAPPED) {
      // tapped but somewhere outside normal screen coordinates, start calibration
      nextState(CALIB_POINT_1);
      return true;
    }

    return false;
  };

  // process touch events
  TouchManager::instance()->processQueue(cb);
}

bool drawCalibPointScreen(uint8_t seq)
{
  const uint8_t dia = 19, rad = (uint8_t)floorf(dia / 2);
  const tsPoint_t &center = reusableBuffer.touchCal.lcdPoint[seq];
  bool ret = false;

  // First enter of this screen, setup/reset
  if (reusableBuffer.touchCal.newState) {
    TouchManager::instance()->clearQueue();
    reusableBuffer.touchCal.newState = false;
  }

  lcdDrawFilledRect(center.x-1, center.y - rad, 3, dia, SOLID, ROUND);  // vert
  lcdDrawFilledRect(center.x - rad, center.y-1, dia, 3, SOLID, ROUND);  // horiz
  coord_t y = (LCD_H - FH * 3) / 2;
  if (seq == 2)
    y -= dia;
  drawCenteredText(y, "Calibration point: ", SMLSIZE, 18);
  lcdDrawNumber(lcdLastRightPos, y, seq + 1, 0);
  drawCenteredText(y+FH+2, "Touch in the center of the +", SMLSIZE, 28);

  // callback for touch event processor (needs to be auto because of capture, then wrapped in TouchManager::eventCallbackLambda())
  auto cb = [&ret, &seq](const Event & ev) {
    if (ev.gesture == GEST_TAPPED) {
      const tVect_t pos = ev.touchPoints.at(0).rawPos;
      reusableBuffer.touchCal.touchPoint[seq] = {pos.x, pos.y};
      ret = true;
    }
    return ret;  // return true if we handled the event
  };

  // process touch events
  TouchManager::instance()->processQueue(TouchManager::eventCallbackLambda(cb));

  if (ret)
    TRACE_DEBUG("TP%d: lcd: %3d, %3d; touch: %3d, %3d\n", seq, reusableBuffer.touchCal.lcdPoint[seq].x, reusableBuffer.touchCal.lcdPoint[seq].y, reusableBuffer.touchCal.touchPoint[seq].x, reusableBuffer.touchCal.touchPoint[seq].y);
  return ret;
}

// TODO: break up into multiple screens: eg. one for drawing, one for buttons/controls,
//  and main screen with page <- -> buttons along with exit/recal so don't have to exit every time.
void drawCalibTestScreen(uint8_t /*menuType*/)
{
  static tVect_t lastPoint = {-1, -1};
  const coord_t btnH = 12; //, btnW = 30;
  const coord_t x = 1, x2 = LCD_W - 2;
  coord_t y = LCD_H - 1, y2 = y - btnH - 3;

  static ButtonData buttons[] {
    ButtonData(MENU_BTN_PG_PREV, {0, 0, 0, 0}, "\177", ALIGN_TOP, false, SMLSIZE, Margins(3, 1, 3, 2)),
    ButtonData(MENU_BTN_PG_NEXT, {LCD_W, 0, 0, 0}, "\176", ALIGN_TOP_RIGHT, false, SMLSIZE, Margins(3, 1, 3, 2)),
    ButtonData(MENU_BTN_EXIT,    {x, y, 45, btnH}, "Exit", ALIGN_BOT),
    ButtonData(MENU_BTN_RECAL,   {x2, y, 70, btnH}, "Recalibrate", ALIGN_BOT_RIGHT),
    ButtonData(MENU_BTN_DRAW,    {x2, y2, 35, btnH}, "Draw", ALIGN_BOT_RIGHT),
    ButtonData(MENU_BTN_CNTRLS,  {coord_t(x2-38), y2, 0, btnH}, "Controls", ALIGN_BOT_RIGHT)
  };

  // First enter of this screen, setup/reset
  if (reusableBuffer.touchCal.newState) {
    TouchManager::instance()->clearQueue();

    // reset buttons
    for (size_t i = 0, end = DIM(buttons); i < end; ++i)
      buttons[i].state = STATE_NORMAL;
    lastPoint = {-1, -1};

#if defined(SIMU)
    buttons[3].state = STATE_DISABLED;
#endif
    reusableBuffer.touchCal.newState = false;
  }

  //if (menuType == 1)
  //  drawScreenIndex(MENU_RADIO_CALIB_TOUCH, DIM(menuTabGeneral), 0);
  //TITLE(STR_MENU_TOUCH_TEST);
  drawCenteredText(0, STR_MENU_TOUCH_TEST, INVERS, STR_MENU_TOUCH_TEST_LEN);

  y = MENU_HEADER_HEIGHT + 3;
  if (reusableBuffer.touchCal.state == CALIB_COMPLETE && lastPoint.x < 0) {
    drawCenteredText(y, "Calibration complete. Test", SMLSIZE, 26);
    drawCenteredText((y += FH), "by tapping on the screen.", SMLSIZE, 25);
  }

  for (size_t i = 0, end = DIM(buttons); i < end; ++i)
    drawButton(buttons[i]);

  lcdDrawText(buttons[5].geo.left()-1, buttons[5].geo.bottom() - FH_SML, "Tests:", RIGHT|SMLSIZE);

  // callback for touch event processor (no capture)
  Touch::eventCallback_t cb = [](const Event & ev) -> bool {
    if (!ev.pointCount)
      return false;

    const tVect_t tpoint0 = ev.touchPoints.at(0).pos;
    for (size_t i = 0, end = DIM(buttons); i < end; ++i) {
      if (buttons[i].state < STATE_DISABLED && buttons[i].geo.contains(tpoint0, -1)) {
        buttons[i].state = STATE_ACTIVE;
        if (ev.gesture == GEST_TAPPED) {
          nextState(buttons[i].id - 1);
          return true;
        }
      }
      else if (buttons[i].state == STATE_ACTIVE) {
        buttons[i].state = STATE_NORMAL;
      }
    }

    lastPoint = tpoint0;
    return true;  // accept anyway
  };

  // process touch events
  TouchManager::instance()->processQueue(cb);

  if (lastPoint.x > -1) {
    // draw coordinates display
    y += 2;
    lcdDrawText(2, y+3, "X:");
    lcdDrawNumber(lcdLastRightPos, y, lastPoint.x, MIDSIZE|LEADING0, 3);
    lcdDrawText(lcdLastRightPos, y+3, " Y:", 0);
    lcdDrawNumber(lcdLastRightPos+1, y, lastPoint.y, MIDSIZE|LEADING0, 2);
    lcdDrawFilledRect(max(lastPoint.x-2, 0), max(lastPoint.y-2, 0), 5, 5, SOLID, ROUND);
  }
}

void drawCalibControlsTestScreen()
{
  // TODO: add more controls to test with
  static tVect_t lastPoint = {-1, -1};
  const coord_t btnH = 14; //, btnW = 30; //, btnHs = MENU_HEADER_HEIGHT+1;
  const coord_t x = LCD_W - 4;  //, x2 = x - 3 - btnW;
  const coord_t y = LCD_H - btnH, y2 = y - btnH - 3;

  static ButtonData buttons[] {
    ButtonData(MENU_BTN_TEST, {0, 0, 0, 0}, "\177", ALIGN_TOP, false, SMLSIZE, Margins(3, 1, 3, 2)),
    // test buttons
    ButtonData(MENU_BTN_NONE, {3, y, 0, 0}, "Checkable", ALIGN_BOT_LEFT, true),
    ButtonData(MENU_BTN_NONE, {3, y2, 0, 0}, 0, ALIGN_BOT_LEFT, true),
    ButtonData(MENU_BTN_NONE, {x, y, 0, btnH}, "tap me!", ALIGN_BOT_RIGHT)
  };
  static const uint8_t BTN_TST_ANNOY = 3;

  // First enter of this screen, setup/reset
  if (reusableBuffer.touchCal.newState) {
    TouchManager::instance()->clearQueue();

    // reset buttons
    for (size_t i = 0, end = DIM(buttons); i < end; ++i) {
      buttons[i].state = STATE_NORMAL;
      buttons[i].chkState = CHKSTATE_UNCHECKED;
    }
    buttons[BTN_TST_ANNOY].setLabel("tap me!", true);

    reusableBuffer.touchCal.newState = false;
  }

  drawCenteredText(0, STR_MENU_TOUCH_TEST, INVERS, STR_MENU_TOUCH_TEST_LEN);
  for (size_t i = 0, end = DIM(buttons); i < end; ++i)
    drawButton(buttons[i]);

  // callback for touch event processor (no capture)
  Touch::eventCallback_t cb = [](const Event & ev) -> bool {
    if (!ev.pointCount)
      return false;

    const tVect_t tpoint0 = ev.touchPoints.at(0).pos;
    for (size_t i = 0, end = DIM(buttons); i < end; ++i) {
      if (buttons[i].state < STATE_DISABLED && buttons[i].geo.contains(tpoint0, -1)) {
        buttons[i].state = STATE_ACTIVE;
        if (ev.gesture == GEST_TAPPED) {
          if (buttons[i].id != MENU_BTN_NONE) {
            nextState(buttons[i].id - 1);
            return true;
          }
          if (buttons[i].checkable) {
            buttons[i].chkState = ButtonCheckState((buttons[i].chkState + 1) % CHKSTATE_ENUM_COUNT);
            buttons[i].state = STATE_NORMAL;
          }
        }
      }
      else if (buttons[i].state == STATE_ACTIVE) {
        buttons[i].state = STATE_NORMAL;
      }
    }

    // the "annoying" button
    if (ev.gesture == GEST_TAPPED) {
      if (buttons[BTN_TST_ANNOY].state == STATE_ACTIVE)
        buttons[BTN_TST_ANNOY].setLabel("thanks!", true);
      else
        buttons[BTN_TST_ANNOY].setLabel("no tap?", true);
    }
    else if (ev.gesture & GEST_RELEASE) {
      buttons[BTN_TST_ANNOY].setLabel("tap me!", true);
    }

    lastPoint = tpoint0;
    return true;  // accept touch event
  };

  // process touch events
  TouchManager::instance()->processQueue(cb);

  // draw coordinates display
  calibDrawCoords(1, LCD_H - FH, lastPoint);
}

void drawCalibDrawingScreen()
{
  static const int maxPts = 60;
  static tVect_t lastPoint[2] = {{-1, -1}, {-1, -1}};
  static tVect_t trail[2][maxPts] = {{}};
  static int trailHead[2] = {0, 0};
  static int trailTail[2] = {0, 0};

  static ButtonData backBtn(0, {0, 0, 0, 0}, "\177", ALIGN_TOP, false, SMLSIZE, Margins(3, 1, 3, 2));
  static ButtonData clsBtn(0, {LCD_W, 0, 0, 0}, "*", ALIGN_TOP_RIGHT, false, SMLSIZE, Margins(3, 1, 3, 2));

  if (reusableBuffer.touchCal.newState) {
    TouchManager::instance()->clearQueue();

    // reset history and snakes
    for (int i = 0; i < 2; ++i) {
      lastPoint[i] = {-1, -1};
      trailHead[i] = trailTail[i] = 0;
      trail[i][maxPts-1].x = -1;
    }

    reusableBuffer.touchCal.newState = false;
  }

  drawCenteredText(0, STR_MENU_TOUCH_TEST, INVERS, STR_MENU_TOUCH_TEST_LEN);
  drawButton(backBtn);
  drawButton(clsBtn);

  Touch::eventCallback_t cb = [](const Event & ev) -> bool {
    if (!ev.pointCount)
      return false;

    if (ev.gesture == GEST_TAPPED) {
      const tVect_t pos = ev.touchPoints.at(0).pos;
      if (backBtn.geo.contains(pos, -1)) {
        nextState(CALIB_TEST);
        return true;
      }
      if (clsBtn.geo.contains(pos, -1)) {
        nextState(CALIB_TEST_DRAW);
        return true;
      }
    }

    for (int i = 0; i < ev.pointCount; ++i) {
      const Touch::TouchPoint & tp = ev.touchPoints.at(i);
      int j = tp.index;
      if (j > 1 || lastPoint[j].dist(tp.pos) < 1)
        continue;
      lastPoint[j] = tp.pos;
      trail[j][trailTail[j]] = tp.pos;
      trailTail[j] = (trailTail[j] + 1) % maxPts;
      if (trail[j][maxPts-1].x > -1)
        trailHead[j] = (trailHead[j] + 1) % maxPts;
    }
    return true;
  };

  // process touch events
  TouchManager::instance()->processQueue(cb);

  // draw coordinates display
  coord_t y = LCD_H - FH;
  calibDrawCoords(1, y, lastPoint[0], 0);
  calibDrawCoords(lcdLastRightPos + 10, y, lastPoint[1], 1);

  // draw tracking snakes
  for (int i = 0; i < 2; ++i) {
    if (lastPoint[i].x < 0)
      continue;
    tVect_t prevPt = {-1, -1};
    int j = trailHead[i];
    while (j != trailTail[i]) {
      if ((prevPt.x > -1))
        lcdDrawLine(prevPt.x, prevPt.y, trail[i][j].x, trail[i][j].y, SOLID, FORCE);
      if (j == trailTail[i] - 1)
        lcdDrawFilledRect(max(trail[i][j].x-2, 0), max(trail[i][j].y-2, 0), 5, 5, SOLID, ROUND|FORCE);
      prevPt = trail[i][j];
      j = (j+1) % maxPts;
    }
  }
}

void menuCommonCalibTouch(event_t event, uint8_t menuType = 0)
{
  uint8_t stepResult;
  reusableBuffer.touchCal.lcdPoint[0] = { LCD_W / 10, LCD_H / 7 };
  reusableBuffer.touchCal.lcdPoint[1] = { LCD_W / 2, LCD_H - LCD_H / 7 };
  reusableBuffer.touchCal.lcdPoint[2] = { LCD_W - LCD_W / 10, LCD_H / 2 };

  // NOTE/TODO: We suspend gesture->key mapping (KeyEventEmulator::suspendKeyEvents(true)) and
  // handle all key & touch events here so that they don't get "swallowed".
  // We re-instate key mapping in CALIB_FINISHED step.
  // This is not ideal, need some way to capture touch events separately.

  switch (event)
  {
    case EVT_ENTRY:
    case EVT_KEY_LONG(KEY_ENTER):
      killEvents(event);
      KeyEventEmulator::suspendKeyEvents(true);
      TouchManager::instance()->clearQueue();
      // force start screen if enter-long or invalid calibration, otherwise go to test screen
      if (event == EVT_KEY_LONG(KEY_ENTER) || !TouchManager::isCurrentCalibrationValid())
        nextState(CALIB_START);
      else
        nextState(CALIB_TEST);
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      nextState(CALIB_FINISHED);
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      // Force calibration step 1 if enter-break and we're on start screen
      if (reusableBuffer.touchCal.state == CALIB_START || reusableBuffer.touchCal.state == CALIB_RESTART)
        nextState(CALIB_POINT_1);
      break;

    case EVT_KEY_PREVIOUS_PAGE:
      if (menuType == 1 && (reusableBuffer.touchCal.state < CALIB_POINT_1 || reusableBuffer.touchCal.state > CALIB_STORE)) {
        nextState(CALIB_PG_PREV);
        killEvents(event);
      }
      break;

    case EVT_KEY_NEXT_PAGE:
      if (menuType == 1 && (reusableBuffer.touchCal.state < CALIB_POINT_1 || reusableBuffer.touchCal.state > CALIB_STORE)) {
        nextState(CALIB_PG_NEXT);
        killEvents(event);
      }
      break;
  }

  switch (reusableBuffer.touchCal.state) {
    case CALIB_RESTART:
    case CALIB_START:
#if defined(SIMU)
      nextState(CALIB_TEST);  // TODO: default 1:1 calibration matrix for simu LCD.
      break;
#endif
      drawCalibInitialScreen(menuType);
      break;

    case CALIB_POINT_1:
      if (drawCalibPointScreen(0))
        nextState(CALIB_POINT_2);
      break;

    case CALIB_POINT_2:
      if (drawCalibPointScreen(1))
        nextState(CALIB_POINT_3);
      break;

    case CALIB_POINT_3:
      if (drawCalibPointScreen(2))
        nextState(CALIB_STORE);
      break;

    case CALIB_STORE:
#if defined(SIMU)
      nextState(CALIB_COMPLETE);
      break;
#endif
    {
      Touch::CalibMatrix matrix;
      if (!TouchManager::calcCalibrationMatrix(reusableBuffer.touchCal.lcdPoint, reusableBuffer.touchCal.touchPoint, &matrix) || !TouchManager::setCalibration(matrix))
        nextState(CALIB_RESTART);
      else
        nextState(CALIB_COMPLETE);
      TouchManager::dumpCalibMatrix(&matrix);  // DEBUG_TRACE
      break;
    }

    case CALIB_COMPLETE:
    case CALIB_TEST:
      drawCalibTestScreen(menuType);
      break;

    case CALIB_TEST_DRAW:
      drawCalibDrawingScreen();
      break;

    case CALIB_TEST_CTRLS:
      drawCalibControlsTestScreen();
      break;

    case CALIB_FINISHED:
    case CALIB_PG_PREV:
    case CALIB_PG_NEXT:
      KeyEventEmulator::suspendKeyEvents(false);
      stepResult = 255;
      if (reusableBuffer.touchCal.state == CALIB_PG_PREV)
        stepResult = MENU_RADIO_CALIB_TOUCH - 1;
      else if (reusableBuffer.touchCal.state == CALIB_PG_NEXT)
        stepResult = (MENU_RADIO_CALIB_TOUCH + 1) % MENU_RADIO_PAGES_COUNT;
      nextState(CALIB_START);
      if (menuType == 1 && stepResult != 255)
        chainMenu((MenuHandlerFunc)pgm_read_adr(&menuTabGeneral[stepResult]));
      else
        popMenu();
      break;

    default:
      nextState(CALIB_START);
      break;
  }
}

void menuRadioCalibTouch(event_t event)
{
  menuCommonCalibTouch(event, 1);
}

void menuFirstCalibTouch(event_t event)
{
  menuCommonCalibTouch(event, 2);
}
