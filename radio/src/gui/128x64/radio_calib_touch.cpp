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
#include "stdio.h"
#include <cmath>

// TODO: Translations!

#define STR_MENU_TOUCH_CALIB             "TOUCH CALIBRATION"
#define STR_MENU_TOUCH_TEST              "TOUCH TEST"

enum CalibrationState {
  CALIB_RESTART = 0,
  CALIB_START,
  CALIB_POINT_1,
  CALIB_POINT_2,
  CALIB_POINT_3,
  CALIB_STORE,
  CALIB_COMPLETE,
  CALIB_TEST,
  CALIB_FINISHED
};

enum {
  MENU_BTN_NONE,
  MENU_BTN_EXIT,
  MENU_BTN_START,
  MENU_BTN_TEST
};

// TODO: Move useful GUI helpers to central location

struct Rect {
  coord_t x;
  coord_t y;
  coord_t w;
  coord_t h;

  coord_t right()    const { return x + w; }
  coord_t bottom()   const { return y + h; }
  point_t topLeft()  const { return {x, y}; }
  point_t topRight() const { return {right(), y}; }
  point_t botLeft()  const { return {x, bottom()}; }
  point_t botRight() const { return {right(), bottom()}; }
  bool contains(point_t point, int8_t margin = 0) const { return (point.x >= x + margin && point.x <= right() - margin && point.y >= y + margin && point.y <= bottom() - margin); }
  bool operator ==(const Rect & other) { return (x == other.x && y == other.y && w == other.w && h == other.h); }
  bool operator !=(const Rect & other) { return !(*this == other); }
};

struct WidgetStyle {
  uint8_t lcdPatt;
  LcdFlags lcdFlags;
  LcdFlags lblLcdFlags;
};

enum ButtonState {
  STATE_NORMAL = 0,
  STATE_ACTIVE,
  STATE_DISABLED,
  STATE_ENUM_COUNT
};

enum ButtonCheckState {
  CHKSTATE_UNCHECKED = 0,
  CHKSTATE_CHECKED,
  CHKSTATE_PARTCHECKED,
  CHKSTATE_ENUM_COUNT
};

struct ButtonData {
  uint8_t id;
  ButtonState state;
  ButtonCheckState chkState;
  Rect geo;
  const char * label;
  bool checkable;
  WidgetStyle style[STATE_ENUM_COUNT];

  ButtonData(uint8_t id, const Rect & geo, const char * label = 0, bool checkable = false, WidgetStyle style[] = nullptr) :
    id(id), state(STATE_NORMAL), chkState(CHKSTATE_UNCHECKED), geo(geo), label(label), checkable(checkable)
  {
    static const WidgetStyle defStyle[] = {{DOTTED, FORCE|ROUND, 0}, {SOLID, FORCE|ROUND, INVERS}, {DOTTED, FORCE|ROUND, 0}};
    if (style)
      memcpy(&this->style, style, sizeof(WidgetStyle) * STATE_ENUM_COUNT);
    else
      memcpy(&this->style, &defStyle,  sizeof(WidgetStyle) * STATE_ENUM_COUNT);
  }
};

void drawCenteredText(Rect within, const char * txt, LcdFlags flags = 0, uint8_t margin = 0, uint16_t len = 0)
{
  const uint8_t txtW = getTextWidth(txt, (len ? len : strlen(txt)), flags);
  const coord_t x = within.x + (within.w - margin * 2 - txtW) / 2 + margin;
  const coord_t y = within.y + (within.h - margin * 2 - FH) / 2 + margin;
  lcdDrawText(x, y, txt, flags);
}

void drawCenteredText(coord_t y, const char * txt, LcdFlags flags = 0, uint16_t len = 0)
{
  drawCenteredText({0, y, LCD_W, FH}, txt, flags, 0, len);
}

void drawButton(ButtonData & btnData)
{
  uint16_t lblLen = 0;
  if (!btnData.geo.w && btnData.label) {
    lblLen = getTextWidth(btnData.label, strlen(btnData.label), btnData.style[btnData.state].lblLcdFlags);
    btnData.geo.w = lblLen + 6 + (btnData.checkable ? FW + 3 : 0);
    btnData.geo.x -= btnData.geo.w / 2;  // assume x is meant as center point
  }

  if (btnData.geo.w < 2 || btnData.geo.h < 2)
    return;

  if (btnData.state != STATE_NORMAL)
    lcdDrawFilledRect(btnData.geo.x, btnData.geo.y, btnData.geo.w, btnData.geo.h, btnData.style[btnData.state].lcdPatt, btnData.style[btnData.state].lcdFlags);
  else
    lcdDrawRect(btnData.geo.x, btnData.geo.y, btnData.geo.w, btnData.geo.h, btnData.style[btnData.state].lcdPatt, btnData.style[btnData.state].lcdFlags);
  if (btnData.checkable) {
    char mark[2] = {(btnData.chkState == CHKSTATE_CHECKED ? '#' : btnData.chkState == CHKSTATE_PARTCHECKED ? '*' : 'O'), 0};
    if (btnData.label)
      lcdDrawText(btnData.geo.x + 2, btnData.geo.y + (btnData.geo.h - FH) / 2, mark, btnData.style[btnData.state].lblLcdFlags);
    else
      drawCenteredText(btnData.geo, mark, btnData.style[btnData.state].lblLcdFlags, 1, 1);
  }
  if (btnData.label && btnData.state != STATE_DISABLED)  // disabled state text is hard on 1bit display  :/
    drawCenteredText(btnData.geo, btnData.label, btnData.style[btnData.state].lblLcdFlags, 1, lblLen);
}


// Actual calibration screen stuff starts here

uint8_t drawCalibInitialScreen(uint8_t menuType)
{
  uint8_t ret = MENU_BTN_NONE;  // 1=exit; 2=start; 3=test;
  const coord_t btnW = 50, btnH = 15;
  static ButtonData btnExit(MENU_BTN_EXIT, {5, LCD_H - btnH - 1, btnW, btnH}, "Exit");
  static ButtonData btnStart(MENU_BTN_START, {0, 0, LCD_W, LCD_H}, 0);
  static ButtonData btnTest(MENU_BTN_TEST, {LCD_W - btnW - 5, LCD_H - btnH - 1, btnW, btnH}, "Test");

  if (menuType == 1)
    TITLE(STR_MENU_TOUCH_CALIB);
  else
    drawCenteredText(0, STR_MENU_TOUCH_CALIB, INVERS);

  if (reusableBuffer.touchCal.state == CALIB_RESTART)
      drawCenteredText(MENU_HEADER_HEIGHT+2, "Calib. failed, try again.");
  drawCenteredText(MENU_HEADER_HEIGHT+2+FH+2, "Touch anywhere to start.");

  bool showBtns = TouchManager::isCurrentCalibrationValid();
  if (showBtns) {
    drawButton(btnExit);
    drawButton(btnTest);
  }

  point_t point;
  bool tp = TouchManager::getTouchPoint(&point);

  if (tp) {
    if (showBtns) {
      if (btnExit.geo.contains(point, -1))
        ret = btnExit.id;
      if (btnTest.geo.contains(point, -1))
        ret = btnTest.id;
    }
    if (!ret) {
      // anywhere else on the screen starts cali
      ret = btnStart.id;
    }
  }

  return ret;
}

bool drawCalibPointScreen(uint8_t seq)
{
  const uint8_t dia = 19, rad = (uint8_t)floorf(dia / 2);
  const tsPoint_t &center = reusableBuffer.touchCal.lcdPoint[seq];
  lcdDrawFilledRect(center.x-1, center.y - rad, 3, dia, SOLID, ROUND);  // vert
  lcdDrawFilledRect(center.x - rad, center.y-1, dia, 3, SOLID, ROUND);  // horiz
  coord_t y = (LCD_H - FH * 3) / 2;
  if (seq == 2)
    y -= dia;
  drawCenteredText(y, "Calibration point: ", SMLSIZE);
  lcdDrawNumber(lcdLastRightPos, y, seq + 1, 0);
  drawCenteredText(y+FH+2, "Touch in the center of the X.", SMLSIZE);

  bool ret = TouchManager::getTouchPoint(&reusableBuffer.touchCal.touchPoint[seq], 0, true);
  if (ret)
    TRACE_DEBUG("TP%d: lcd: %3d, %3d; touch: %3d, %3d\n", seq, reusableBuffer.touchCal.lcdPoint[seq].x, reusableBuffer.touchCal.lcdPoint[seq].y, reusableBuffer.touchCal.touchPoint[seq].x, reusableBuffer.touchCal.touchPoint[seq].y);
  return ret;
}

uint8_t drawCalibTestScreen(uint8_t menuType)
{
  static tsPoint_t lastPoint = {-1, -1};
  static tsPoint_t trail[30] = {{0}};
  static int trailHead = 0;
  static int trailTail = 0;
  uint8_t ret = MENU_BTN_NONE;  // 1=exit; 2=restart
  coord_t btnW = 30, btnH = 14;
  coord_t y = LCD_H - btnH - 1;

  static ButtonData btnExit(MENU_BTN_EXIT, {3, y, 45, btnH}, "Exit");
  static ButtonData btnStart(MENU_BTN_START, {LCD_W - 70 - 3, y, 70, btnH}, "Recalibrate");
  btnH += 2;
  y = y - btnH - 3;
  static ButtonData btnTest1(MENU_BTN_NONE, {coord_t(LCD_W - btnW - 3), y, btnW, btnH}, 0, true);
  static ButtonData btnTest2(MENU_BTN_NONE, Rect({coord_t(LCD_W - btnW - 3 - btnW - 3), y, btnW, btnH}));

  btnTest1.state = ButtonState::STATE_NORMAL;
#if defined(SIMU)
  btnStart.state = STATE_DISABLED;
  //btnExit.state = ButtonState::STATE_ACTIVE;  // test
#endif

  if (menuType == 1)
    TITLE(STR_MENU_TOUCH_TEST);
  else
    drawCenteredText(0, STR_MENU_TOUCH_TEST, INVERS);

  y = MENU_HEADER_HEIGHT + 1;
  if (reusableBuffer.touchCal.state == CALIB_COMPLETE) {
    drawCenteredText(y, "Calibration complete. Test", SMLSIZE);
    drawCenteredText((y += FH), "by drawing on the screen.", SMLSIZE);
  }

  drawButton(btnStart);
  drawButton(btnExit);
  drawButton(btnTest1);
  drawButton(btnTest2);

  bool btnPress = false;
  tsPoint_t point;
  bool gotTp = TouchManager::getTouchPoint(&point, 0, false);

  if (gotTp) {
    point_t pt = TouchManager::tsPointToLcdPoint(point);
    if (btnExit.geo.contains(pt, -1))
      ret = btnExit.id;
    else if (btnStart.geo.contains(pt, -1) && btnStart.state != STATE_DISABLED)
      ret = btnStart.id;
    else if (btnTest1.geo.contains(pt, -1)) {
      btnTest1.state = ButtonState::STATE_ACTIVE;
      btnTest1.chkState = ButtonCheckState((btnTest1.chkState + 1) % CHKSTATE_ENUM_COUNT);
      btnPress = true;
    }
    else if (btnTest2.geo.contains(pt, -1)) {
      btnTest2.state = ButtonState((btnTest2.state + 1) % STATE_ENUM_COUNT);
      btnPress = true;
    }

    if (ret)
      return ret;
    else if (btnPress)
      TouchManager::waitTouchRelease(100);  // debounce

    int aX = abs(lastPoint.x - point.x), aY = abs(lastPoint.y - point.y);
    if (aX > 2 || aY > 2) {
      if (aX > 25 || aY > 25) {
        // reset trail if new point too far away from last
        trailHead = trailTail = 0;
        trail[29].x = -1;
      }
      lastPoint = point;
      if (!btnPress) {
        trail[trailTail] = point;
        trailTail = (trailTail + 1) % 30;
        if (trail[29].x > -1)
          trailHead = (trailHead + 1) % 30;
      }
    }
  }

  if (lastPoint.x > -1 && lastPoint.y > -1) {
    y += FH + 2;
    lcdDrawText(2, y+3, "X:");
    lcdDrawNumber(lcdLastRightPos, y, lastPoint.x, MIDSIZE|LEADING0, 3);
    lcdDrawText(lcdLastRightPos, y+3, " Y:", 0);
    lcdDrawNumber(lcdLastRightPos+1, y, lastPoint.y, MIDSIZE|LEADING0, 2);

    tsPoint_t prevPt = {-1, -1};
    int i = trailHead;
    while (i != trailTail) {
      if ((prevPt.x > -1))
        lcdDrawLine(prevPt.x, prevPt.y, trail[i].x, trail[i].y, SOLID /*, FORCE*/);
      if (i == trailTail - 1)
        lcdDrawFilledRect(trail[i].x-2, trail[i].y-2, 5, 5, SOLID, /*FORCE|*/ROUND);
      prevPt = trail[i];
      i = (i+1) % 30;
    }
  }
  return ret;
}

void menuCommonCalibTouch(event_t event, uint8_t menuType = 0)
{
  uint8_t stepResult;
  reusableBuffer.touchCal.lcdPoint[0] = { LCD_W / 10, LCD_H / 7 };
  reusableBuffer.touchCal.lcdPoint[1] = { LCD_W / 2, LCD_H - LCD_H / 7 };
  reusableBuffer.touchCal.lcdPoint[2] = { LCD_W - LCD_W / 10, LCD_H / 2 };

  switch (event)
  {
    case EVT_ENTRY:
    case EVT_KEY_BREAK(KEY_ENTER):
      reusableBuffer.touchCal.state = CALIB_START;
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      reusableBuffer.touchCal.state = CALIB_FINISHED;
      break;
  }

  switch (reusableBuffer.touchCal.state) {
    case CALIB_RESTART:
    case CALIB_START:
#if defined(SIMU)
      reusableBuffer.touchCal.state = CALIB_TEST;
      break;
#endif
      stepResult = drawCalibInitialScreen(menuType);
      if (stepResult && TouchManager::waitTouchRelease()) {
        reusableBuffer.touchCal.state = (stepResult == MENU_BTN_START ? CALIB_POINT_1 : stepResult == MENU_BTN_TEST ? CALIB_TEST : stepResult == MENU_BTN_EXIT ? CALIB_FINISHED : reusableBuffer.touchCal.state);
      }
      break;

    case CALIB_POINT_1:
      if (drawCalibPointScreen(0) && TouchManager::waitTouchRelease())
        reusableBuffer.touchCal.state = CALIB_POINT_2;
      break;

    case CALIB_POINT_2:
      if (drawCalibPointScreen(1) && TouchManager::waitTouchRelease())
        reusableBuffer.touchCal.state = CALIB_POINT_3;
      break;

    case CALIB_POINT_3:
      if (drawCalibPointScreen(2) && TouchManager::waitTouchRelease())
        reusableBuffer.touchCal.state = CALIB_STORE;
      break;

    case CALIB_STORE:
#if defined(SIMU)
      reusableBuffer.touchCal.state = CALIB_COMPLETE;
      break;
#endif
    {
      TouchCalibMatrix matrix;
      if (!TouchManager::calcCalibrationMatrix(reusableBuffer.touchCal.lcdPoint, reusableBuffer.touchCal.touchPoint, &matrix) || !TouchManager::setCalibration(matrix))
        reusableBuffer.touchCal.state = CALIB_RESTART;
      else
        reusableBuffer.touchCal.state = CALIB_COMPLETE;
      TouchManager::dumpCalibMatrix(&matrix);  // DEBUG_TRACE
      break;
    }

    case CALIB_COMPLETE:
    case CALIB_TEST:
      stepResult = drawCalibTestScreen(menuType);
      if (stepResult && TouchManager::waitTouchRelease())
        reusableBuffer.touchCal.state = (stepResult == 1 ? CALIB_FINISHED : stepResult == MENU_BTN_START ? CALIB_START : reusableBuffer.touchCal.state);
      break;

    case CALIB_FINISHED:
      reusableBuffer.touchCal.state = CALIB_START;
      popMenu();
      break;

    default:
      reusableBuffer.touchCal.state = CALIB_START;
      break;
  }
}

void menuRadioCalibTouch(event_t event)
{
  if (reusableBuffer.touchCal.state < CALIB_POINT_1 || reusableBuffer.touchCal.state > CALIB_POINT_3)
    check_simple(event, MENU_RADIO_CALIB_TOUCH, menuTabGeneral, DIM(menuTabGeneral), 0);
  menuCommonCalibTouch(event, 1);
}

void menuFirstCalibTouch(event_t event)
{
  menuCommonCalibTouch(event, 2);
}
