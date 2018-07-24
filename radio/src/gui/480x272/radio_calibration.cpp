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
#include "radio_calibration.h"

class StickCalibrationWindow: public Window {
  public:
    StickCalibrationWindow(Window * parent, const rect_t & rect, uint8_t stickX, uint8_t stickY):
      Window(parent, rect),
      stickX(stickX),
      stickY(stickY)
    {
    }

    virtual void checkEvents() override
    {
      // will always force a full monitor window refresh
      invalidate();
    }

    void paint(BitmapBuffer * dc)
    {
      dc->drawBitmap(0, 0, calibStickBackground);
      int16_t x = calibratedAnalogs[stickX];
      int16_t y = calibratedAnalogs[stickY];
      dc->drawBitmap(width() / 2 - 9 + (bitmapSize / 2 * x) / RESX, height() / 2 - 9 - (bitmapSize / 2 * y) / RESX, calibStick);
    }

  protected:
    static constexpr coord_t bitmapSize = 68;
    uint8_t stickX, stickY;
};

class PotCalibrationWindow: public Window {
  public:
    PotCalibrationWindow(Window * parent, const rect_t & rect, uint8_t potIndex):
      Window(parent, rect),
      potIndex(potIndex)
    {
    }

    virtual void checkEvents() override
    {
      // will always force a full monitor window refresh
      invalidate();
    }

    virtual void paint(BitmapBuffer * dc)
    {
      drawVerticalSlider(5, 8, rect.h - 18, calibratedAnalogs[potIndex], -RESX, RESX, 40, OPTION_SLIDER_TICKS | OPTION_SLIDER_BIG_TICKS | OPTION_SLIDER_SQUARE_BUTTON);
    }

  protected:
    static constexpr coord_t bitmapSize = 68;
    uint8_t potIndex;
};

RadioCalibrationPage::RadioCalibrationPage():
  PageTab(STR_MENUCALIBRATION, ICON_RADIO_CALIBRATION)
{
}

void RadioCalibrationPage::build(Window * window)
{
  menuCalibrationState = 0;

  // The two sticks
  new StickCalibrationWindow(window, {20, 20, 90, 90}, STICK1, STICK2);
  new StickCalibrationWindow(window, {LCD_W-110, 20, 90, 90}, STICK4, STICK3);

  // The two pots
  new PotCalibrationWindow(window, {20, 140, 20, 178}, POT1);
  new PotCalibrationWindow(window, {LCD_W-40, 140, 20, 178}, POT2);

  button = new TextButton(window, {LCD_W/2-125, window->height() - 50, 250, 30}, "START", [=]() -> int8_t {
    nextStep();
    return 0;
  }, 0);
}


enum CalibrationState {
  CALIB_START = 0,
  CALIB_SET_MIDPOINT,
  CALIB_MOVE_STICKS,
  CALIB_STORE,
  CALIB_FINISHED
};

void RadioCalibrationPage::checkEvents()
{
  for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_MOUSE_ANALOGS; i++) { // get low and high vals for sticks and trims
    int16_t vt = i<TX_VOLTAGE ? anaIn(i) : anaIn(i+1);
    reusableBuffer.calib.loVals[i] = min(vt, reusableBuffer.calib.loVals[i]);
    reusableBuffer.calib.hiVals[i] = max(vt, reusableBuffer.calib.hiVals[i]);
    if (i >= POT1 && i <= POT_LAST) {
      if (IS_POT_WITHOUT_DETENT(i)) {
        reusableBuffer.calib.midVals[i] = (reusableBuffer.calib.hiVals[i] + reusableBuffer.calib.loVals[i]) / 2;
      }
#if NUM_XPOTS > 0
      uint8_t idx = i - POT1;
      int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
      if (IS_POT_MULTIPOS(i) && count <= XPOTS_MULTIPOS_COUNT) {
        // use raw analog value for multipos calibraton, anaIn() already has multipos decoded value
        vt = getAnalogValue(i) >> 1;
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == 0 || vt < reusableBuffer.calib.xpotsCalib[idx].lastPosition - XPOT_DELTA || vt > reusableBuffer.calib.xpotsCalib[idx].lastPosition + XPOT_DELTA) {
          reusableBuffer.calib.xpotsCalib[idx].lastPosition = vt;
          reusableBuffer.calib.xpotsCalib[idx].lastCount = 1;
        }
        else {
          if (reusableBuffer.calib.xpotsCalib[idx].lastCount < 255) {
            reusableBuffer.calib.xpotsCalib[idx].lastCount++;
          }
        }
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == XPOT_DELAY) {
          int16_t position = reusableBuffer.calib.xpotsCalib[idx].lastPosition;
          bool found = false;
          for (int j=0; j<count; j++) {
            int16_t step = reusableBuffer.calib.xpotsCalib[idx].steps[j];
            if (position >= step-XPOT_DELTA && position <= step+XPOT_DELTA) {
              found = true;
              break;
            }
          }
          if (!found) {
            if (count < XPOTS_MULTIPOS_COUNT) {
              reusableBuffer.calib.xpotsCalib[idx].steps[count] = position;
            }
            reusableBuffer.calib.xpotsCalib[idx].stepsCount += 1;
          }
        }
      }
#endif
    }
  }

  if (menuCalibrationState == CALIB_SET_MIDPOINT) {
    for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS + NUM_MOUSE_ANALOGS; i++) {
      reusableBuffer.calib.loVals[i] = 15000;
      reusableBuffer.calib.hiVals[i] = -15000;
      reusableBuffer.calib.midVals[i] = i < TX_VOLTAGE ? anaIn(i) : anaIn(i + 1);
#if NUM_XPOTS > 0
      if (i < NUM_XPOTS) {
        reusableBuffer.calib.xpotsCalib[i].stepsCount = 0;
        reusableBuffer.calib.xpotsCalib[i].lastCount = 0;
      }
#endif
    }
  }
  else if (menuCalibrationState == CALIB_MOVE_STICKS) {
    for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS + NUM_MOUSE_ANALOGS; i++) {
      if (abs(reusableBuffer.calib.loVals[i] - reusableBuffer.calib.hiVals[i]) > 50) {
        g_eeGeneral.calib[i].mid = reusableBuffer.calib.midVals[i];
        int16_t v = reusableBuffer.calib.midVals[i] - reusableBuffer.calib.loVals[i];
        g_eeGeneral.calib[i].spanNeg = v - v / STICK_TOLERANCE;
        v = reusableBuffer.calib.hiVals[i] - reusableBuffer.calib.midVals[i];
        g_eeGeneral.calib[i].spanPos = v - v / STICK_TOLERANCE;
      }
    }
#if NUM_XPOTS > 0
    for (int i=POT1; i<=POT_LAST; i++) {
      int idx = i - POT1;
      int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
      if (IS_POT_MULTIPOS(i)) {
        if (count > 1 && count <= XPOTS_MULTIPOS_COUNT) {
          for (int j=0; j<count; j++) {
            for (int k=j+1; k<count; k++) {
              if (reusableBuffer.calib.xpotsCalib[idx].steps[k] < reusableBuffer.calib.xpotsCalib[idx].steps[j]) {
                SWAP(reusableBuffer.calib.xpotsCalib[idx].steps[j], reusableBuffer.calib.xpotsCalib[idx].steps[k]);
              }
            }
          }
          StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[i];
          calib->count = count - 1;
          for (int j=0; j<calib->count; j++) {
            calib->steps[j] = (reusableBuffer.calib.xpotsCalib[idx].steps[j+1] + reusableBuffer.calib.xpotsCalib[idx].steps[j]) >> 5;
          }
        }
        else {
          // g_eeGeneral.potsConfig &= ~(0x03<<(2*idx));
        }
      }
    }
#endif
  }
}

void RadioCalibrationPage::nextStep()
{
  menuCalibrationState++;

  switch (menuCalibrationState) {
    case CALIB_SET_MIDPOINT:
      button->setText(STR_SETMIDPOINT);
      break;

    case CALIB_MOVE_STICKS:
      button->setText(STR_MOVESTICKSPOTS);
      break;

    case CALIB_STORE:
      button->setText("Done!");
      g_eeGeneral.chkSum = evalChkSum();
      storageDirty(EE_GENERAL);
      menuCalibrationState = CALIB_FINISHED;
      break;

    default:
      button->setText("START");
      menuCalibrationState = CALIB_START;
      break;
  }
}

#define XPOT_DELTA                     10
#define XPOT_DELAY                     5 /* cycles */
#define STICKS_WIDTH                   90
#define STICKS_Y                       60
#define STICK_LEFT_X                   25
#define STICK_RIGHT_X                  (LCD_W-STICK_LEFT_X-STICKS_WIDTH)

bool menuCommonCalib(event_t event)
{
  drawMenuTemplate(NULL, ICON_RADIO_CALIBRATION, NULL, OPTION_MENU_NO_FOOTER);

  for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_MOUSE_ANALOGS; i++) { // get low and high vals for sticks and trims
    int16_t vt = i<TX_VOLTAGE ? anaIn(i) : anaIn(i+1);
    reusableBuffer.calib.loVals[i] = min(vt, reusableBuffer.calib.loVals[i]);
    reusableBuffer.calib.hiVals[i] = max(vt, reusableBuffer.calib.hiVals[i]);
    if (i >= POT1 && i <= POT_LAST) {
      if (IS_POT_WITHOUT_DETENT(i)) {
        reusableBuffer.calib.midVals[i] = (reusableBuffer.calib.hiVals[i] + reusableBuffer.calib.loVals[i]) / 2;
      }

#if NUM_XPOTS > 0
      uint8_t idx = i - POT1;
      int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
      if (IS_POT_MULTIPOS(i) && count <= XPOTS_MULTIPOS_COUNT) {
        // use raw analog value for multipos calibraton, anaIn() already has multipos decoded value
        vt = getAnalogValue(i) >> 1;
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == 0 || vt < reusableBuffer.calib.xpotsCalib[idx].lastPosition - XPOT_DELTA || vt > reusableBuffer.calib.xpotsCalib[idx].lastPosition + XPOT_DELTA) {
          reusableBuffer.calib.xpotsCalib[idx].lastPosition = vt;
          reusableBuffer.calib.xpotsCalib[idx].lastCount = 1;
        }
        else {
          if (reusableBuffer.calib.xpotsCalib[idx].lastCount < 255) {
            reusableBuffer.calib.xpotsCalib[idx].lastCount++;
          }
        }
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == XPOT_DELAY) {
          int16_t position = reusableBuffer.calib.xpotsCalib[idx].lastPosition;
          bool found = false;
          for (int j=0; j<count; j++) {
            int16_t step = reusableBuffer.calib.xpotsCalib[idx].steps[j];
            if (position >= step-XPOT_DELTA && position <= step+XPOT_DELTA) {
              found = true;
              break;
            }
          }
          if (!found) {
            if (count < XPOTS_MULTIPOS_COUNT) {
              reusableBuffer.calib.xpotsCalib[idx].steps[count] = position;
            }
            reusableBuffer.calib.xpotsCalib[idx].stepsCount += 1;
          }
        }
      }
#endif
    }
  }

  switch (event) {
    case EVT_ENTRY:
    case EVT_KEY_FIRST(KEY_EXIT):
      menuCalibrationState = CALIB_START;
      break;

    case EVT_KEY_FIRST(KEY_ENTER):
      killEvents(event);
      menuCalibrationState++;
      break;
  }

  switch (menuCalibrationState) {
    case CALIB_START:
      // START CALIBRATION
      if (!READ_ONLY()) {
        lcdDrawText(50, 3, STR_MENUCALIBRATION, MENU_TITLE_COLOR);
        lcdDrawText(50, 3+FH, STR_MENUTOSTART, MENU_TITLE_COLOR);
      }
      break;

    case CALIB_SET_MIDPOINT:
      // SET MIDPOINT
      lcdDrawText(50, 3, STR_MENUCALIBRATION, MENU_TITLE_COLOR);
      lcdDrawText(50, 3+FH, STR_SETMIDPOINT, MENU_TITLE_COLOR);
      for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_MOUSE_ANALOGS; i++) {
        reusableBuffer.calib.loVals[i] = 15000;
        reusableBuffer.calib.hiVals[i] = -15000;
        reusableBuffer.calib.midVals[i] = i < TX_VOLTAGE ? anaIn(i) : anaIn(i+1);
#if NUM_XPOTS > 0
        if (i < NUM_XPOTS) {
          reusableBuffer.calib.xpotsCalib[i].stepsCount = 0;
          reusableBuffer.calib.xpotsCalib[i].lastCount = 0;
        }
#endif
      }
      break;

    case CALIB_MOVE_STICKS:
      // MOVE STICKS/POTS
      lcdDrawText(50, 3, STR_MENUCALIBRATION, MENU_TITLE_COLOR);
      lcdDrawText(50, 3+FH, STR_MOVESTICKSPOTS, MENU_TITLE_COLOR);
      for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_MOUSE_ANALOGS; i++) {
        if (abs(reusableBuffer.calib.loVals[i]-reusableBuffer.calib.hiVals[i]) > 50) {
          g_eeGeneral.calib[i].mid = reusableBuffer.calib.midVals[i];
          int16_t v = reusableBuffer.calib.midVals[i] - reusableBuffer.calib.loVals[i];
          g_eeGeneral.calib[i].spanNeg = v - v/STICK_TOLERANCE;
          v = reusableBuffer.calib.hiVals[i] - reusableBuffer.calib.midVals[i];
          g_eeGeneral.calib[i].spanPos = v - v/STICK_TOLERANCE;
        }
      }
#if NUM_XPOTS > 0
      for (int i=POT1; i<=POT_LAST; i++) {
        int idx = i - POT1;
        int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
        if (IS_POT_MULTIPOS(i)) {
          if (count > 1 && count <= XPOTS_MULTIPOS_COUNT) {
            for (int j=0; j<count; j++) {
              for (int k=j+1; k<count; k++) {
                if (reusableBuffer.calib.xpotsCalib[idx].steps[k] < reusableBuffer.calib.xpotsCalib[idx].steps[j]) {
                  SWAP(reusableBuffer.calib.xpotsCalib[idx].steps[j], reusableBuffer.calib.xpotsCalib[idx].steps[k]);
                }
              }
            }
            StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[i];
            calib->count = count - 1;
            for (int j=0; j<calib->count; j++) {
              calib->steps[j] = (reusableBuffer.calib.xpotsCalib[idx].steps[j+1] + reusableBuffer.calib.xpotsCalib[idx].steps[j]) >> 5;
            }
          }
          else {
            // g_eeGeneral.potsConfig &= ~(0x03<<(2*idx));
          }
        }
      }
#endif
      break;

    case CALIB_STORE:
      g_eeGeneral.chkSum = evalChkSum();
      storageDirty(EE_GENERAL);
      menuCalibrationState = CALIB_FINISHED;
      break;

    default:
      menuCalibrationState = CALIB_START;
      break;
  }

  return true;
}


bool menuRadioCalibration(event_t event)
{
  if (event == EVT_ENTRY || event == EVT_ENTRY_UP) TRACE("Menu %s displayed ...", STR_MENUCALIBRATION);
  if (menuCalibrationState == CALIB_FINISHED || (menuCalibrationState == CALIB_START && event == EVT_KEY_FIRST(KEY_EXIT))) {
    menuCalibrationState = CALIB_START;
    killEvents(event);
    popMenu();
    return false;
  }
  if (!check_submenu_simple(event, 0)) {
    return false;
  }

  menuVerticalPosition = -1;

  return menuCommonCalib(READ_ONLY() ? 0 : event);
}

bool menuFirstCalib(event_t event)
{
  event = 0;
  
  if (event == EVT_KEY_FIRST(KEY_EXIT) || menuCalibrationState == CALIB_FINISHED) {
    menuCalibrationState = CALIB_START;
    // chainMenu(menuMainView);
    return false;
  }
  else {
    return menuCommonCalib(event);
  }
}
