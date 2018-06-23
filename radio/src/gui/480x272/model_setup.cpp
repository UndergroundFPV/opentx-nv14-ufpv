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

#include <touch_driver.h>
#include "opentx.h"

uint8_t g_moduleIdx;

enum MenuModelSetupItems {
  ITEM_MODEL_NAME,
  ITEM_MODEL_BITMAP,
  ITEM_MODEL_TIMER1,
  ITEM_MODEL_TIMER1_NAME,
  ITEM_MODEL_TIMER1_PERSISTENT,
  ITEM_MODEL_TIMER1_MINUTE_BEEP,
  ITEM_MODEL_TIMER1_COUNTDOWN_BEEP,
#if TIMERS > 1
  ITEM_MODEL_TIMER2,
  ITEM_MODEL_TIMER2_NAME,
  ITEM_MODEL_TIMER2_PERSISTENT,
  ITEM_MODEL_TIMER2_MINUTE_BEEP,
  ITEM_MODEL_TIMER2_COUNTDOWN_BEEP,
#endif
#if TIMERS > 2
  ITEM_MODEL_TIMER3,
  ITEM_MODEL_TIMER3_NAME,
  ITEM_MODEL_TIMER3_PERSISTENT,
  ITEM_MODEL_TIMER3_MINUTE_BEEP,
  ITEM_MODEL_TIMER3_COUNTDOWN_BEEP,
#endif
  ITEM_MODEL_EXTENDED_LIMITS,
  ITEM_MODEL_EXTENDED_TRIMS,
  ITEM_MODEL_DISPLAY_TRIMS,
  ITEM_MODEL_TRIM_INC,
  ITEM_MODEL_THROTTLE_LABEL,
  ITEM_MODEL_THROTTLE_REVERSED,
  ITEM_MODEL_THROTTLE_TRACE,
  ITEM_MODEL_THROTTLE_TRIM,
  ITEM_MODEL_PREFLIGHT_LABEL,
  ITEM_MODEL_CHECKLIST_DISPLAY,
  ITEM_MODEL_THROTTLE_WARNING,
  ITEM_MODEL_SWITCHES_WARNING,
  ITEM_MODEL_SLIDPOT_WARNING_STATE,
  ITEM_MODEL_POTS_WARNING,
  ITEM_MODEL_SLIDERS_WARNING,
  ITEM_MODEL_BEEP_CENTER,
  ITEM_MODEL_USE_GLOBAL_FUNCTIONS,
  ITEM_MODEL_INTERNAL_MODULE_LABEL,
  ITEM_MODEL_INTERNAL_MODULE_MODE,
  ITEM_MODEL_INTERNAL_MODULE_CHANNELS,
  ITEM_MODEL_INTERNAL_MODULE_BIND,
  ITEM_MODEL_INTERNAL_MODULE_FAILSAFE,
  ITEM_MODEL_INTERNAL_MODULE_ANTENNA,
  ITEM_MODEL_EXTERNAL_MODULE_LABEL,
  ITEM_MODEL_EXTERNAL_MODULE_MODE,
#if defined(MULTIMODULE)
  ITEM_MODEL_EXTERNAL_MODULE_STATUS,
  ITEM_MODEL_EXTERNAL_MODULE_SYNCSTATUS,
#endif
  ITEM_MODEL_EXTERNAL_MODULE_CHANNELS,
  ITEM_MODEL_EXTERNAL_MODULE_BIND,
  ITEM_MODEL_EXTERNAL_MODULE_FAILSAFE,
  ITEM_MODEL_EXTERNAL_MODULE_OPTIONS,
  ITEM_MODEL_EXTERNAL_MODULE_BIND_OPTIONS,
#if defined(MULTIMODULE)
  ITEM_MODEL_EXTERNAL_MODULE_AUTOBIND,
#endif
  ITEM_MODEL_EXTERNAL_MODULE_POWER,
  ITEM_MODEL_TRAINER_LABEL,
  ITEM_MODEL_TRAINER_MODE,
  ITEM_MODEL_TRAINER_LINE1,
  ITEM_MODEL_TRAINER_LINE2,
  ITEM_MODEL_SETUP_MAX
};

#define MODEL_SETUP_2ND_COLUMN         160
#define MODEL_SETUP_3RD_COLUMN         230
#define MODEL_SETUP_4TH_COLUMN         300
#define MODEL_SETUP_BIND_OFS           40
#define MODEL_SETUP_RANGE_OFS          80
#define MODEL_SETUP_SET_FAILSAFE_OFS   100
#define MODEL_SETUP_SLIDPOT_SPACING    45

#define CURRENT_MODULE_EDITED(k)       (k>=ITEM_MODEL_TRAINER_LABEL ? TRAINER_MODULE : (k>=ITEM_MODEL_EXTERNAL_MODULE_LABEL ? EXTERNAL_MODULE : INTERNAL_MODULE))

void onBindMenu(const char * result)
{
  uint8_t moduleIdx = (menuVerticalPosition >= ITEM_MODEL_EXTERNAL_MODULE_LABEL ? EXTERNAL_MODULE : INTERNAL_MODULE);

  if (result == STR_BINDING_25MW_CH1_8_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.power = R9M_LBT_POWER_25;
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  }
  else if (result == STR_BINDING_25MW_CH1_8_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.power = R9M_LBT_POWER_25;
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = false;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  }
  else if (result == STR_BINDING_500MW_CH1_8_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.power = R9M_LBT_POWER_500;
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  }
  else if (result == STR_BINDING_500MW_CH9_16_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.power = R9M_LBT_POWER_500;
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = true;
  }
  else if (result == STR_BINDING_1_8_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = false;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  }
  else if (result == STR_BINDING_1_8_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  }
  else if (result == STR_BINDING_9_16_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = false;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = true;
  }
  else if (result == STR_BINDING_9_16_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = true;
  }
  else {
    return;
  }

  moduleFlag[moduleIdx] = MODULE_BIND;
}

void onModelSetupBitmapMenu(const char * result)
{
  if (result == STR_UPDATE_LIST) {
    if (!sdListFiles(BITMAPS_PATH, BITMAPS_EXT, sizeof(g_model.header.bitmap)-LEN_BITMAPS_EXT, NULL)) {
      POPUP_WARNING(STR_NO_BITMAPS_ON_SD);
    }
  }
  else {
    // The user choosed a bmp file in the list
    copySelection(g_model.header.bitmap, result, sizeof(g_model.header.bitmap));
    storageDirty(EE_MODEL);
  }
}

void editTimerMode(int timerIdx, coord_t y, LcdFlags attr, event_t event)
{
  TimerData & timer = g_model.timers[timerIdx];
  if (attr && menuHorizontalPosition < 0) {
    lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, 115+2*INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
  }
  drawStringWithIndex(&menuBodyWindow, MENUS_MARGIN_LEFT, y, STR_TIMER, timerIdx+1, BOLD);
  // drawTimerMode(&menuBodyWindow, MODEL_SETUP_2ND_COLUMN, y, timer.mode, (menuHorizontalPosition<=0 ? attr : 0));
  // drawTimer(&menuBodyWindow, MODEL_SETUP_2ND_COLUMN+50, y, timer.start, (menuHorizontalPosition!=0 ? attr|TIMEHOUR : TIMEHOUR));
  if (attr && s_editMode>0) {
    switch (menuHorizontalPosition) {
      case 0:
      {
        int32_t timerMode = timer.mode;
        if (timerMode < 0) timerMode -= TMRMODE_COUNT-1;
        CHECK_INCDEC_MODELVAR_CHECK(event, timerMode, -TMRMODE_COUNT-SWSRC_LAST+1, TMRMODE_COUNT+SWSRC_LAST-1, isSwitchAvailableInTimers);
        if (timerMode < 0) timerMode += TMRMODE_COUNT-1;
        timer.mode = timerMode;
#if defined(AUTOSWITCH)
        if (s_editMode>0) {
          int8_t val = timer.mode - (TMRMODE_COUNT-1);
          int8_t switchVal = checkIncDecMovedSwitch(val);
          if (val != switchVal) {
            timer.mode = switchVal + (TMRMODE_COUNT-1);
            storageDirty(EE_MODEL);
          }
        }
#endif
        break;
      }
      case 1:
      {
        const int stopsMinutes[] = { 8, 60, 120, 180, 240, 300, 600, 900, 1200 };
        timer.start = checkIncDec(event, timer.start, 0, TIMER_MAX, EE_MODEL, NULL, (const CheckIncDecStops&)stopsMinutes);
        break;
      }
    }
  }
}

int getSwitchWarningsCount()
{
  int count = 0;
  for (int i=0; i<NUM_SWITCHES; ++i) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      ++count;
    }
  }
  return count;
}

#define IF_INTERNAL_MODULE_ON(x)          (IS_INTERNAL_MODULE_ENABLED() ? (uint8_t)(x) : HIDDEN_ROW)
#define IF_EXTERNAL_MODULE_ON(x)          (IS_EXTERNAL_MODULE_ENABLED() ? (uint8_t)(x) : HIDDEN_ROW)

#define INTERNAL_MODULE_MODE_ROWS         (uint8_t)0
#define INTERNAL_MODULE_CHANNELS_ROWS     IF_INTERNAL_MODULE_ON(1)
#define PORT_CHANNELS_ROWS(x)             (x==INTERNAL_MODULE ? INTERNAL_MODULE_CHANNELS_ROWS : (x==EXTERNAL_MODULE ? EXTERNAL_MODULE_CHANNELS_ROWS : 1))

#define TIMER_ROWS(x)                     NAVIGATION_LINE_BY_LINE|1, 0, 0, 0, g_model.timers[x].countdownBeep != COUNTDOWN_SILENT ? (uint8_t)1 : (uint8_t)0

#define EXTERNAL_MODULE_MODE_ROWS         (IS_MODULE_PXX(EXTERNAL_MODULE) || IS_MODULE_DSM2(EXTERNAL_MODULE)) ? (uint8_t)1 : IS_MODULE_MULTIMODULE(EXTERNAL_MODULE) ? MULTIMODULE_MODE_ROWS(EXTERNAL_MODULE) : (uint8_t)0

#if TIMERS == 1
  #define TIMERS_ROWS                     TIMER_ROWS(0)
#elif TIMERS == 2
  #define TIMERS_ROWS                     TIMER_ROWS(0), TIMER_ROWS(1)
#elif TIMERS == 3
  #define TIMERS_ROWS                     TIMER_ROWS(0), TIMER_ROWS(1), TIMER_ROWS(2)
#endif

#define SW_WARN_ITEMS()                   uint8_t(NAVIGATION_LINE_BY_LINE|(getSwitchWarningsCount()-1))
#define POT_WARN_ROWS                     (uint8_t)0
#define POT_WARN_ITEMS()                  ((g_model.potsWarnMode) ? uint8_t(NAVIGATION_LINE_BY_LINE|(NUM_POTS-1)) : (uint8_t)0)
#define SLIDER_WARN_ITEMS()               ((g_model.potsWarnMode) ? uint8_t(NAVIGATION_LINE_BY_LINE|(NUM_SLIDERS-1)) : (uint8_t)0)

#if defined(BLUETOOTH)
#define TRAINER_LINE1_BLUETOOTH_M_ROWS    ((bluetoothDistantAddr[0] == 0 || bluetoothState == BLUETOOTH_STATE_CONNECTED) ? (uint8_t)0 : (uint8_t)1)
#define TRAINER_LINE1_ROWS                (g_model.trainerMode == TRAINER_MODE_SLAVE ? (uint8_t)1 : (g_model.trainerMode == TRAINER_MODE_MASTER_BLUETOOTH ? TRAINER_LINE1_BLUETOOTH_M_ROWS : (g_model.trainerMode == TRAINER_MODE_SLAVE_BLUETOOTH ? (uint8_t)1 : HIDDEN_ROW)))
#else
#define TRAINER_LINE1_ROWS                HIDDEN_ROW
#endif

#define TRAINER_LINE2_ROWS                (g_model.trainerMode == TRAINER_MODE_SLAVE ? (uint8_t)2 : HIDDEN_ROW)

class KeyboardWindow: public Window {
  protected:
    char * field;
    uint8_t length;
    coord_t cursorPos;
    uint8_t cursorIndex;

  public:
    KeyboardWindow() :
      Window(0, LCD_H - 270, LCD_W, 270),
      field(NULL),
      length(0),
      cursorPos(0),
      cursorIndex(0)
    {
    }

    void setField(char * field, uint8_t length = 0)
    {
      this->field = field;
      this->length = length;
    }

    void disable()
    {
      this->field = NULL;
    }

    char * getField()
    {
      return field;
    }

    coord_t getCursorPos()
    {
      return cursorPos;
    }

    void setCursorPos(coord_t x)
    {
      coord_t rest = x;
      for (cursorIndex = 0; cursorIndex < length; cursorIndex++) {
        if (field[cursorIndex] == '\0')
          break;
        char c = field[cursorIndex];
        c = idx2char(c);
        uint8_t w = getCharWidth(c, fontspecsTable[0]);
        if (rest < w)
          break;
        rest -= w;
      }
      cursorPos = x - rest;
    }

    void draw()
    {
#if 0
      drawBitmap(0, 0, keyboardBitmap);
      if (touchPressed(0, 0, width, height)) {
        coord_t x = touchState.X;
        coord_t y = touchState.Y - this->y;
        uint8_t row = y / 54;
        char c = 0;
        if (row == 0) {
          uint8_t column = (max(6, x) - 6) / 31;
          c = *("1234567890" + column);
        }
        else if (row == 1) {
          uint8_t column = (max(6, x) - 6) / 31;
          c = *("qwertyuiop" + column);
        }
        else if (row == 2) {
          uint8_t column = (max(20, x) - 20) / 31;
          c = *("asdfghjkl" + column);
        }
        else if (row == 3) {
          if (x > 270) {
            if (cursorIndex > 0) {
              // backspace
              char c = idx2char(field[cursorIndex - 1]);
              memmove(field + cursorIndex - 1, field + cursorIndex, length - cursorIndex);
              field[length - 1] = '\0';
              cursorPos -= getCharWidth(c, fontspecsTable[0]);
              cursorIndex -= 1;
            }
          }
          else {
            uint8_t column = (max(52, x) - 52) / 31;
            c = *("zxcvbnm" + column);
          }
        }
        else if (row == 4) {
          if (x > 240) {
            // enter
            disable();
          }
          else if (x > 207) {
            c = ',';
          }
          else if (x > 38) {
            c = ' ';
          }
          else {
            c = '.';
          }
        }
        if (c && zlen(field, length) < length) {
          memmove(field+cursorIndex+1, field+cursorIndex, length - cursorIndex - 1);
          field[cursorIndex++] = char2idx(c);
          cursorPos += getCharWidth(c, fontspecsTable[0]);
        }
      }
#endif
    }
};

KeyboardWindow keyboardWindow;

struct rect_t {
  coord_t x, y, w, h;
};

#define HAS_FOCUS (1<<1)

#if 1
#define CONTENT_WIDTH     140
template <class W>
bool Subtitle(W * window, rect_t & rect, const char * label)
{
  rect.h = 30;
  drawText(window, rect.x, rect.y, label, BOLD);
}

template <class W>
bool CheckBox(W * window, rect_t & rect, const char * label, uint8_t & value, LcdFlags flags=0)
{
  rect.h = 30;
  bool hasFocus = (flags & HAS_FOCUS) || window->hasFocus(rect.x, rect.y);
  drawText(window, rect.x, rect.y+4, label);
  coord_t x = rect.x + rect.w - CONTENT_WIDTH - 2;
  if (hasFocus) {
    window->drawSolidFilledRect(x-1, rect.y+8, 14, 14, TEXT_INVERTED_BGCOLOR);
    window->drawSolidFilledRect(x+1, rect.y+10, 10, 10, TEXT_BGCOLOR);
    if (value) {
      window->drawSolidFilledRect(x+2, rect.y+11, 8, 8, TEXT_INVERTED_BGCOLOR);
    }
  }
  else {
    if (value) {
      window->drawSolidFilledRect(x+2, rect.y+11, 8, 8, SCROLLBOX_COLOR);
      drawSolidRect(window, x, rect.y+9, 12, 12, 1, LINE_COLOR);
    }
    else {
      drawSolidRect(window, x, rect.y+9, 12, 12, 1, LINE_COLOR);
    }
  }
  if (window->touchPressed(rect.x, rect.y, rect.w, rect.h)) {
    value ^= 1;
    window->setFocus(rect.x, rect.y);
    return true;
  }
  else {
    return false;
  }
}

template <class W>
bool Choice(W * window, rect_t & rect, const char * label, const char * values, uint8_t & value, uint8_t vmin, uint8_t vmax, LcdFlags flags=0)
{
  rect.h = 30;
  bool hasFocus = (flags & HAS_FOCUS) || window->hasFocus(rect.x, rect.y);
  LcdFlags textColor = 0;
  LcdFlags lineColor = CURVE_AXIS_COLOR;
  if (hasFocus) {
    textColor = TEXT_INVERTED_BGCOLOR;
    lineColor = TEXT_INVERTED_BGCOLOR;
  }
  drawText(&menuBodyWindow, rect.x, rect.y, label);
  drawTextAtIndex(&menuBodyWindow, rect.x + rect.w - CONTENT_WIDTH, rect.y, values, value, textColor);
  drawSolidRect(window, rect.x + rect.w - CONTENT_WIDTH - 3, rect.y - 3, CONTENT_WIDTH + 4, rect.h, 1, lineColor);
  if (window->touchPressed(rect.x, rect.y, rect.w, rect.h)) {
    // value ^= 1;
    window->setFocus(rect.x, rect.y);
    return true;
  }
  else {
    return false;
  }
}

template <class W>
bool TextEdit(W * window, rect_t & rect, const char * label, char * value, uint8_t length, LcdFlags flags=0)
{
  rect.h = 30;
  coord_t textLeft = rect.x + rect.w - CONTENT_WIDTH;
  bool result = false;
  bool hasFocus = (flags & HAS_FOCUS) || window->hasFocus(rect.x, rect.y);
  LcdFlags textColor = 0;
  LcdFlags lineColor = CURVE_AXIS_COLOR;
  if (window->touchPressed(rect.x, rect.y, rect.w, rect.h)) {
    if (!hasFocus) {
      window->setFocus(rect.x, rect.y);
      hasFocus = true;
    }
    if (!keyboardWindow.getField()) {
      window->setHeight(LCD_H - keyboardWindow.height - window->y);
      keyboardWindow.setField(value, length);
    }
    keyboardWindow.setCursorPos(touchState.X - textLeft);
    result = true;
  }
  if (hasFocus) {
    textColor = TEXT_INVERTED_BGCOLOR;
    lineColor = TEXT_INVERTED_BGCOLOR;
  }
  else {
    if (!keyboardWindow.getField()) {
      window->setHeight(MENU_BODY_HEIGHT);
    }
  }
  drawText(&menuBodyWindow, rect.x, rect.y, label);
  if (!hasFocus && zlen(value, length) == 0)
    menuBodyWindow.drawSizedText(textLeft, rect.y, "---", length, textColor);
  else
    menuBodyWindow.drawSizedText(textLeft, rect.y, value, length, ZCHAR | textColor);
  drawSolidRect(window, textLeft - 3, rect.y - 3, CONTENT_WIDTH + 3, rect.h, 1, lineColor);
  if (hasFocus) {
    coord_t cursorPos = keyboardWindow.getCursorPos();
    window->drawSolidFilledRect(textLeft + cursorPos - 1, rect.y+1, 2, 22, TEXT_INVERTED_BGCOLOR);
//    window->drawSolidFilledRect(textLeft + cursorPos -1, rect.y+1, 1, 22, TEXT_INVERTED_BGCOLOR);
  }
  return result;
}
#else
template <class W>
bool Subtitle(W * window, rect_t & rect, const char * label)
{
  rect.h = 14;
  drawText(window, rect.x, rect.y, label, BOLD);
}

template <class W>
bool CheckBox(W * window, rect_t & rect, const char * label, uint8_t & value, LcdFlags flags=0)
{
  rect.h = 30;
  bool hasFocus = (flags & HAS_FOCUS) || window->hasFocus(rect.x, rect.y);
  drawText(window, rect.x, rect.y+4, label);
  coord_t x = rect.x + rect.w - 20;
  if (hasFocus) {
    window->drawSolidFilledRect(x-1, rect.y+8, 14, 14, TEXT_INVERTED_BGCOLOR);
    window->drawSolidFilledRect(x+1, rect.y+10, 10, 10, TEXT_BGCOLOR);
    if (value) {
      window->drawSolidFilledRect(x+2, rect.y+11, 8, 8, TEXT_INVERTED_BGCOLOR);
    }
  }
  else {
    if (value) {
      window->drawSolidFilledRect(x+2, rect.y+11, 8, 8, SCROLLBOX_COLOR);
      drawSolidRect(window, x, rect.y+9, 12, 12, 1, LINE_COLOR);
    }
    else {
      drawSolidRect(window, x, rect.y+9, 12, 12, 1, LINE_COLOR);
    }
  }
  if (window->touchPressed(rect.x, rect.y, rect.w, rect.h)) {
    value ^= 1;
    window->setFocus(rect.x, rect.y);
    return true;
  }
  else {
    return false;
  }
}


template <class W>
void drawWidgetLabel(W * window, rect_t & rect, const char * label) {
  drawText(&menuBodyWindow, rect.x, rect.y, label, TINSIZE);
}

template <class W>
void drawWidgetLine(W * window, rect_t & rect, LcdFlags color) {
  window->drawSolidFilledRect(rect.x, rect.y + rect.h, rect.w, 1, color);
}

template <class W>
bool Choice(W * window, rect_t & rect, const char * label, const char * values, uint8_t & value, uint8_t vmin, uint8_t vmax, LcdFlags flags=0)
{
  rect.h = 30;
  bool hasFocus = (flags & HAS_FOCUS) || window->hasFocus(rect.x, rect.y);
  LcdFlags textColor = 0;
  LcdFlags lineColor = CURVE_AXIS_COLOR;
  if (hasFocus) {
    textColor = TEXT_INVERTED_BGCOLOR;
    lineColor = TEXT_INVERTED_BGCOLOR;
  }
  drawTextAtIndex(&menuBodyWindow, rect.x, rect.y + 9, values, value, textColor);
  drawWidgetLine(&menuBodyWindow, rect, lineColor);
  drawWidgetLabel(&menuBodyWindow, rect, label);
  if (window->touchPressed(rect.x, rect.y, rect.w, rect.h)) {
    // value ^= 1;
    window->setFocus(rect.x, rect.y);
    return true;
  }
  else {
    return false;
  }
}


template <class W>
bool TextEdit(W * window, rect_t & rect, const char * label, char * value, uint8_t length, LcdFlags flags=0)
{
  rect.h = 30;
  bool hasFocus = (flags & HAS_FOCUS) || window->hasFocus(rect.x, rect.y);
  LcdFlags textColor = 0;
  LcdFlags lineColor = CURVE_AXIS_COLOR;
  if (hasFocus) {
    textColor = TEXT_INVERTED_BGCOLOR;
    lineColor = TEXT_INVERTED_BGCOLOR;
  }
  if (!hasFocus && zlen(value, length) == 0)
    menuBodyWindow.drawSizedText(rect.x, rect.y + 9, "---", length, textColor);
  else
    menuBodyWindow.drawSizedText(rect.x, rect.y + 9, value, length, ZCHAR | textColor);
  drawWidgetLine(&menuBodyWindow, rect, lineColor);
  drawWidgetLabel(&menuBodyWindow, rect, label);
  if (window->touchPressed(rect.x, rect.y, rect.w, rect.h)) {
    // value ^= 1;
    window->setFocus(rect.x, rect.y);
    return true;
  }
  else {
    return false;
  }
}
#endif

#define SUBTITLE(label) { rect.x = MENUS_MARGIN_LEFT; Subtitle(&menuBodyWindow, rect, label); rect.y += rect.h + 10; }
#define CHECKBOX(X, label, value) { rect.x = X; rect.w = LCD_W - (X) - 20; uint8_t _value = value; CheckBox(&menuBodyWindow, rect, label, _value); value = _value; rect.y += rect.h + 10; }
#define CHECKBOX_INVERTED(X, label, value) { rect.x = X; rect.w = LCD_W - (X) - 20; uint8_t _value = !value; CheckBox(&menuBodyWindow, rect, label, _value); value = !_value; rect.y += rect.h + 10; }
#define CHOICE(X, label, values, value, vmin, vmax) { rect.x = X; rect.w = LCD_W - (X) - 20; uint8_t _value = value; Choice(&menuBodyWindow, rect, label, values, _value, vmin, vmax); value = _value; rect.y += rect.h + 10; }
#define TEXT_EDIT(X, label, text, length) { rect.x = (X); rect.w = LCD_W - (X) - 20; TextEdit(&menuBodyWindow, rect, label, text, length); rect.y += rect.h + 10; }

void editTimerCountdown(int timerIdx, coord_t y, LcdFlags attr, event_t event)
{
  TimerData & timer = g_model.timers[timerIdx];

  rect_t rect = { MENUS_MARGIN_LEFT + INDENT_WIDTH, y, LCD_W - (MENUS_MARGIN_LEFT + INDENT_WIDTH) - 20, 30 };
//  drawWidgetLabel(&menuBodyWindow, rect, NO_INDENT(STR_BEEPCOUNTDOWN));
  drawTextAtIndex(&menuBodyWindow, MENUS_MARGIN_LEFT + INDENT_WIDTH, y + 10, STR_VBEEPCOUNTDOWN, timer.countdownBeep, (menuHorizontalPosition == 0 ? attr : 0));
//  drawWidgetLine(&menuBodyWindow, rect, CURVE_AXIS_COLOR);
  if (timer.countdownBeep != COUNTDOWN_SILENT) {
    drawNumber(&menuBodyWindow, MENUS_MARGIN_LEFT + INDENT_WIDTH, y + 10, TIMER_COUNTDOWN_START(timerIdx), (menuHorizontalPosition == 1 ? attr : 0) | LEFT, 0, NULL, "s");
  }

  if (attr && s_editMode > 0) {
    switch (menuHorizontalPosition) {
      case 0:
        CHECK_INCDEC_MODELVAR(event, timer.countdownBeep, COUNTDOWN_SILENT, COUNTDOWN_COUNT - 1);
        break;
      case 1:
        timer.countdownStart = -checkIncDecModel(event, -timer.countdownStart, -1, +2);
        break;
    }
  }
}

bool menuModelSetup(event_t event)
{
  // Switch to external antenna confirmation
  //  bool newAntennaSel;
  //  if (warningResult) {
  //    warningResult = 0;
  //    g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna = XJT_EXTERNAL_ANTENNA;
  //  }

/*  MENU(STR_MENUSETUP, MODEL_ICONS, menuTabModel, MENU_MODEL_SETUP, ITEM_MODEL_SETUP_MAX,
       { 0, 0, TIMERS_ROWS, 0, 1, 0, 0,
         LABEL(Throttle), 0, 0, 0,
         LABEL(PreflightCheck), 0, 0, SW_WARN_ITEMS(), POT_WARN_ROWS, (g_model.potsWarnMode ? POT_WARN_ITEMS() : HIDDEN_ROW), (g_model.potsWarnMode ? SLIDER_WARN_ITEMS() : HIDDEN_ROW), NAVIGATION_LINE_BY_LINE|(NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_ROTARY_ENCODERS-1), 0,
         LABEL(InternalModule),
         INTERNAL_MODULE_MODE_ROWS,
         INTERNAL_MODULE_CHANNELS_ROWS,
         IF_INTERNAL_MODULE_ON(IS_MODULE_XJT(INTERNAL_MODULE) ? (HAS_RF_PROTOCOL_MODELINDEX(g_model.moduleData[INTERNAL_MODULE].rfProtocol) ? (uint8_t)2 : (uint8_t)1) : (IS_MODULE_PPM(INTERNAL_MODULE) ? (uint8_t)1 : HIDDEN_ROW)),
         IF_INTERNAL_MODULE_ON((IS_MODULE_XJT(INTERNAL_MODULE)) ? FAILSAFE_ROWS(INTERNAL_MODULE) : HIDDEN_ROW),
         IF_INTERNAL_MODULE_ON(0),
         LABEL(ExternalModule),
         EXTERNAL_MODULE_MODE_ROWS,
         MULTIMODULE_STATUS_ROWS
         EXTERNAL_MODULE_CHANNELS_ROWS,
         ((IS_MODULE_XJT(EXTERNAL_MODULE) && !HAS_RF_PROTOCOL_MODELINDEX(g_model.moduleData[EXTERNAL_MODULE].rfProtocol)) || IS_MODULE_SBUS(EXTERNAL_MODULE)) ? (uint8_t)1 : (IS_MODULE_PPM(EXTERNAL_MODULE) || IS_MODULE_PXX(EXTERNAL_MODULE) || IS_MODULE_DSM2(EXTERNAL_MODULE) || IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) ? (uint8_t)2 : HIDDEN_ROW,
         FAILSAFE_ROWS(EXTERNAL_MODULE),
         EXTERNAL_MODULE_OPTION_ROW,
         (IS_MODULE_R9M_LBT(EXTERNAL_MODULE) ? (uint8_t)0 : HIDDEN_ROW),
         MULTIMODULE_MODULE_ROWS
         EXTERNAL_MODULE_POWER_ROW,
         LABEL(Trainer),
         0,
         TRAINER_LINE1_ROWS,
         TRAINER_LINE2_ROWS });
*/

/*  if (menuEvent) {
    moduleFlag[0] = 0;
    moduleFlag[1] = 0;
  }
*/

  MT6S_MENU(STR_MENUSETUP, MODEL_ICONS, menuTabModel);

  if (keyboardWindow.getField()) {
    keyboardWindow.draw();
  }
  else {
    menuBodyWindow.scroll();
  }

  rect_t rect = { MENUS_MARGIN_LEFT, MENUS_MARGIN_LEFT, 0, 0 };

  // Model name
  TEXT_EDIT(MENUS_MARGIN_LEFT, STR_MODELNAME, g_model.header.name, sizeof(g_model.header.name));

/*
 * case ITEM_MODEL_BITMAP: {
        rect_t rect = { MENUS_MARGIN_LEFT, y, LCD_W - MENUS_MARGIN_LEFT - 20, 30 };
        drawWidgetLabel(&menuBodyWindow, rect, STR_BITMAP);
        if (ZEXIST(g_model.header.bitmap))
          menuBodyWindow.drawSizedText(MENUS_MARGIN_LEFT, y + 10, g_model.header.bitmap, sizeof(g_model.header.bitmap), attr);
        else
          drawTextAtIndex(&menuBodyWindow, MENUS_MARGIN_LEFT, y + 10, STR_VCSWFUNC, 0, attr);
        drawWidgetLine(&menuBodyWindow, rect, CURVE_AXIS_COLOR);
        if (attr && event == EVT_KEY_BREAK(KEY_ENTER) && READ_ONLY_UNLOCKED()) {
          s_editMode = 0;
          if (sdListFiles(BITMAPS_PATH, BITMAPS_EXT, sizeof(g_model.header.bitmap) - LEN_BITMAPS_EXT,
                          g_model.header.bitmap, LIST_NONE_SD_FILE | LIST_SD_FILE_EXT)) {
            POPUP_MENU_START(onModelSetupBitmapMenu);
          } else {
            POPUP_WARNING(STR_NO_BITMAPS_ON_SD);
          }
        }
        break;
      }
*/

  for (uint8_t i=0; i<TIMERS; i++) {
    // Timer label
    char timerLabel[8];
    strAppendStringWithIndex(timerLabel, STR_TIMER, i+1);
    SUBTITLE(timerLabel);

    // editTimerMode(0, y, attr, event);

    // Timer name
    TEXT_EDIT(MENUS_MARGIN_LEFT + INDENT_WIDTH, NO_INDENT(STR_TIMER_NAME), g_model.timers[i].name, LEN_TIMER_NAME);

    // Timer minute beep
    CHECKBOX(MENUS_MARGIN_LEFT + INDENT_WIDTH, NO_INDENT(STR_MINUTEBEEP), g_model.timers[i].minuteBeep);

    // Timer countdown
    // editTimerCountdown(0, y, attr, event);

    // Timer persistent
    CHOICE(MENUS_MARGIN_LEFT + INDENT_WIDTH, NO_INDENT(STR_PERSISTENT), STR_VPERSISTENT, g_model.timers[i].persistent, 0, 2);
  }

  // Extended limits
  CHECKBOX(MENUS_MARGIN_LEFT, STR_ELIMITS, g_model.extendedLimits);

  // Extended trims
  CHECKBOX(MENUS_MARGIN_LEFT, STR_ETRIMS, g_model.extendedTrims);
/*
  drawText(&menuBodyWindow, MODEL_SETUP_2ND_COLUMN+18, y, STR_RESET_BTN, menuHorizontalPosition>0  && !NO_HIGHLIGHT() ? attr : 0);
        if (attr && menuHorizontalPosition>0) {
          s_editMode = 0;
          if (event==EVT_KEY_LONG(KEY_ENTER)) {
            START_NO_HIGHLIGHT();
            for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
              memclear(&g_model.flightModeData[i], TRIMS_ARRAY_SIZE);
            }
            storageDirty(EE_MODEL);
            AUDIO_WARNING1();
          }
        }
        */

  // Display trims
  CHOICE(MENUS_MARGIN_LEFT, STR_DISPLAY_TRIMS, "\006No\0   ChangeYes", g_model.displayTrims, 0, 2);

  // Trim step
  CHOICE(MENUS_MARGIN_LEFT, STR_TRIMINC, STR_VTRIMINC, g_model.trimInc, -2, 2);

  // Throttle parameters
  {
    SUBTITLE(STR_THROTTLE_LABEL);

    // Throttle reversed
    CHECKBOX(MENUS_MARGIN_LEFT + INDENT_WIDTH, STR_THROTTLEREVERSE, g_model.throttleReversed);

/*      case ITEM_MODEL_THROTTLE_TRACE:
      {
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_TTRACE);
        if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, g_model.thrTraceSrc, NUM_POTS+NUM_SLIDERS+MAX_OUTPUT_CHANNELS);
        uint8_t idx = g_model.thrTraceSrc + MIXSRC_Thr;
        if (idx > MIXSRC_Thr)
          idx += 1;
        if (idx >= MIXSRC_FIRST_POT+NUM_POTS+NUM_SLIDERS)
          idx += MIXSRC_CH1 - MIXSRC_FIRST_POT - NUM_POTS - NUM_SLIDERS;
        drawSource(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN, y, idx, attr);
        break;
      }*/

    // Throttle trim
    CHECKBOX(MENUS_MARGIN_LEFT + INDENT_WIDTH, STR_TTRIM, g_model.thrTrim);
  }

  // Preflight parameters
  {
    SUBTITLE(STR_PREFLIGHT);

    // Display checklist
    CHECKBOX(MENUS_MARGIN_LEFT + INDENT_WIDTH, STR_CHECKLIST, g_model.displayChecklist);

    // Throttle warning
    CHECKBOX_INVERTED(MENUS_MARGIN_LEFT + INDENT_WIDTH, STR_THROTTLEWARNING, g_model.disableThrottleWarning);

    // Switches warning
    /*
    drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, STR_SWITCHWARNING);
        if (!READ_ONLY() && attr && menuHorizontalPosition<0 && event==EVT_KEY_LONG(KEY_ENTER)) {
          killEvents(event);
          START_NO_HIGHLIGHT();
          getMovedSwitch();
          for (int i=0; i<NUM_SWITCHES; i++) {
            bool enabled = ((g_model.switchWarningState >> (3*i)) & 0x07);
            if (enabled) {
              g_model.switchWarningState &= ~(0x07 << (3*i));
              unsigned int newState = (switches_states >> (2*i) & 0x03) + 1;
              g_model.switchWarningState |= (newState << (3*i));
            }
          }
          AUDIO_WARNING1();
          storageDirty(EE_MODEL);
        }

        if (attr && menuHorizontalPosition < 0) {
          lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, (NUM_SWITCHES-1)*25+INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
        }

        unsigned int newStates = 0;
        for (int i=0, current=0; i<NUM_SWITCHES; i++) {
          if (SWITCH_WARNING_ALLOWED(i)) {
            unsigned int state = ((g_model.switchWarningState >> (3*i)) & 0x07);
            LcdFlags color = (state > 0 ? TEXT_COLOR : TEXT_DISABLE_COLOR);
            if (attr && menuHorizontalPosition < 0) {
              color |= INVERS;
            }
            char s[3];
            s[0] = 'A' + i;
            s[1] = "x\300-\301"[state];
            s[2] = '\0';
            drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN+i*25, y, s, color|(menuHorizontalPosition==current ? attr : 0));
            if (!READ_ONLY() && attr && menuHorizontalPosition==current) {
              CHECK_INCDEC_MODELVAR_ZERO_CHECK(event, state, 3, IS_CONFIG_3POS(i) ? NULL : isSwitch2POSWarningStateAvailable);
            }
            newStates |= (state << (3*i));
            ++current;
          }
        }
        g_model.switchWarningState = newStates;
        break;
      }
      */
  }

#if 0
      case ITEM_MODEL_SLIDPOT_WARNING_STATE:
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y,STR_POTWARNINGSTATE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, PSTR("\004""OFF\0""Man\0""Auto"), g_model.potsWarnMode, attr);
        if (attr) {
          CHECK_INCDEC_MODELVAR(event, g_model.potsWarnMode, POTS_WARN_OFF, POTS_WARN_AUTO);
          storageDirty(EE_MODEL);
        }
        break;

      case ITEM_MODEL_POTS_WARNING:
      {
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_POTWARNING);
        if (attr) {
          if (!READ_ONLY() && menuHorizontalPosition >= 0 && event==EVT_KEY_LONG(KEY_ENTER)) {
            killEvents(event);
            if (g_model.potsWarnMode == POTS_WARN_MANUAL) {
              SAVE_POT_POSITION(menuHorizontalPosition);
              AUDIO_WARNING1();
              storageDirty(EE_MODEL);
            }
          }

          if (!READ_ONLY() &&  menuHorizontalPosition >= 0 && s_editMode && event==EVT_KEY_BREAK(KEY_ENTER)) {
            s_editMode = 0;
            g_model.potsWarnEnabled ^= (1 << (menuHorizontalPosition));
            storageDirty(EE_MODEL);
          }
        }

        if (attr && menuHorizontalPosition < 0) {
          lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, NUM_POTS*MODEL_SETUP_SLIDPOT_SPACING+INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
        }

        if (g_model.potsWarnMode) {
          coord_t x = MODEL_SETUP_2ND_COLUMN;
          for (int i=0; i<NUM_POTS; ++i) {
            LcdFlags flags = (((menuHorizontalPosition==i) && attr) ? INVERS : 0);
            flags |= (g_model.potsWarnEnabled & (1 << i)) ? TEXT_DISABLE_COLOR : TEXT_COLOR;
            if (attr && menuHorizontalPosition < 0) {
              flags |= INVERS;
            }
            lcdDrawTextAtIndex(x, y, STR_VSRCRAW, NUM_STICKS+1+i, flags);
            x += MODEL_SETUP_SLIDPOT_SPACING;
          }
        }
        break;
      }

      case ITEM_MODEL_SLIDERS_WARNING:
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_SLIDERWARNING);
        if (attr) {
          if (!READ_ONLY() && menuHorizontalPosition+1 && event==EVT_KEY_LONG(KEY_ENTER)) {
            killEvents(event);
            if (g_model.potsWarnMode == POTS_WARN_MANUAL) {
              SAVE_POT_POSITION(menuHorizontalPosition+NUM_POTS);
              AUDIO_WARNING1();
              storageDirty(EE_MODEL);
            }
          }

          if (!READ_ONLY() && menuHorizontalPosition+1 && s_editMode && event==EVT_KEY_BREAK(KEY_ENTER)) {
            s_editMode = 0;
            g_model.potsWarnEnabled ^= (1 << (menuHorizontalPosition+NUM_POTS));
            storageDirty(EE_MODEL);
          }
        }

        if (attr && menuHorizontalPosition < 0) {
          lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, NUM_SLIDERS*MODEL_SETUP_SLIDPOT_SPACING+INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
        }

        if (g_model.potsWarnMode) {
          coord_t x = MODEL_SETUP_2ND_COLUMN;
          for (int i=NUM_POTS; i<NUM_POTS+NUM_SLIDERS; ++i) {
            LcdFlags flags = (((menuHorizontalPosition==i-NUM_POTS) && attr) ? INVERS : 0);
            flags |= (g_model.potsWarnEnabled & (1 << i)) ? TEXT_DISABLE_COLOR : TEXT_COLOR;
            if (attr && menuHorizontalPosition < 0) {
              flags |= INVERS;
            }
            lcdDrawTextAtIndex(x, y, STR_VSRCRAW, NUM_STICKS+1+i, flags);
            x += MODEL_SETUP_SLIDPOT_SPACING;
          }
        }
        break;

      case ITEM_MODEL_BEEP_CENTER:
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_BEEPCTR);
        lcdNextPos = MODEL_SETUP_2ND_COLUMN - 3;
        for (int i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) {
          LcdFlags flags = ((menuHorizontalPosition==i && attr) ? INVERS : 0);
          flags |= (g_model.beepANACenter & ((BeepANACenter)1<<i)) ? TEXT_COLOR : (TEXT_DISABLE_COLOR | NO_FONTCACHE);
          if (attr && menuHorizontalPosition < 0) flags |= INVERS;
          lcdDrawTextAtIndex(lcdNextPos+3, y, STR_RETA123, i, flags);
        }
        if (attr && CURSOR_ON_CELL) {
          if (event==EVT_KEY_BREAK(KEY_ENTER)) {
            if (READ_ONLY_UNLOCKED()) {
              s_editMode = 0;
              g_model.beepANACenter ^= ((BeepANACenter)1<<menuHorizontalPosition);
              storageDirty(EE_MODEL);
            }
          }
        }
        break;

      case ITEM_MODEL_USE_GLOBAL_FUNCTIONS:
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_USE_GLOBAL_FUNCS);
        drawCheckBox(MODEL_SETUP_2ND_COLUMN, y, !g_model.noGlobalFunctions, attr);
        if (attr) g_model.noGlobalFunctions = !checkIncDecModel(event, !g_model.noGlobalFunctions, 0, 1);
        break;

      case ITEM_MODEL_INTERNAL_MODULE_LABEL:
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, TR_INTERNALRF);
        break;

      case ITEM_MODEL_INTERNAL_MODULE_MODE:
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_XJT_PROTOCOLS, 1+g_model.moduleData[0].rfProtocol, attr);
        if (attr) {
          g_model.moduleData[INTERNAL_MODULE].rfProtocol = checkIncDec(event, g_model.moduleData[INTERNAL_MODULE].rfProtocol, RF_PROTO_OFF, RF_PROTO_LAST, EE_MODEL, isRfProtocolAvailable);
          if (checkIncDec_Ret) {
            g_model.moduleData[0].type = MODULE_TYPE_XJT;
            g_model.moduleData[0].channelsStart = 0;
            g_model.moduleData[0].channelsCount = DEFAULT_CHANNELS(INTERNAL_MODULE);
            if (g_model.moduleData[INTERNAL_MODULE].rfProtocol == RF_PROTO_OFF)
              g_model.moduleData[INTERNAL_MODULE].type = MODULE_TYPE_NONE;
          }

        }
        break;

      case ITEM_MODEL_INTERNAL_MODULE_ANTENNA:
        CHOICE(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_ANTENNASELECTION, STR_VANTENNATYPES, g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna, 0, 1);
#if 0
        if (newAntennaSel != g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna && newAntennaSel == XJT_EXTERNAL_ANTENNA) {
          POPUP_CONFIRMATION(STR_ANTENNACONFIRM1);
          const char * w = STR_ANTENNACONFIRM2;
          SET_WARNING_INFO(w, strlen(w), 0);
        }
        else {
          g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna = newAntennaSel;
        }
#endif
        break;

      case ITEM_MODEL_TRAINER_MODE:
        CHOICE(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_MODE, STR_VTRAINERMODES, g_model.trainerMode, 0, TRAINER_MODE_MAX());
#if defined(BLUETOOTH)
        if (attr && checkIncDec_Ret) {
          bluetoothState = BLUETOOTH_STATE_OFF;
          bluetoothDistantAddr[0] = 0;
        }
#endif
        break;

      case ITEM_MODEL_EXTERNAL_MODULE_LABEL:
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, TR_EXTERNALRF);
        break;

      case ITEM_MODEL_EXTERNAL_MODULE_MODE:
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_TARANIS_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].type, menuHorizontalPosition==0 ? attr : 0);
        if (IS_MODULE_XJT(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_XJT_PROTOCOLS, 1+g_model.moduleData[EXTERNAL_MODULE].rfProtocol, (menuHorizontalPosition==1 ? attr : 0));
        else if (IS_MODULE_DSM2(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_DSM_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, (menuHorizontalPosition==1 ? attr : 0));
        else if (IS_MODULE_R9M(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_R9M_MODES, g_model.moduleData[EXTERNAL_MODULE].subType, (menuHorizontalPosition==1 ? attr : 0));
#if defined(MULTIMODULE)
        else if (IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) {
          int multi_rfProto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
          if (g_model.moduleData[EXTERNAL_MODULE].multi.customProto) {
            drawText(&menuBodyWindow,MODEL_SETUP_3RD_COLUMN, y, STR_MULTI_CUSTOM, menuHorizontalPosition == 1 ? attr : 0);
            lcdDrawNumber(MODEL_SETUP_4TH_COLUMN, y, multi_rfProto, menuHorizontalPosition==2 ? attr : 0, 2);
            lcdDrawNumber(MODEL_SETUP_4TH_COLUMN + MODEL_SETUP_BIND_OFS, y, g_model.moduleData[EXTERNAL_MODULE].subType, menuHorizontalPosition==3 ? attr : 0, 2);
          }
          else {
            const mm_protocol_definition * pdef = getMultiProtocolDefinition(multi_rfProto);
            lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_MULTI_PROTOCOLS, multi_rfProto, menuHorizontalPosition == 1 ? attr : 0);
            if (pdef->subTypeString != nullptr)
              lcdDrawTextAtIndex(MODEL_SETUP_4TH_COLUMN, y, pdef->subTypeString, g_model.moduleData[EXTERNAL_MODULE].subType, menuHorizontalPosition==2 ? attr : 0);
          }
        }
#endif
        if (attr && s_editMode>0) {
          switch (menuHorizontalPosition) {
            case 0:
              g_model.moduleData[EXTERNAL_MODULE].type = checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].type, MODULE_TYPE_NONE, MODULE_TYPE_COUNT-1, EE_MODEL, isModuleAvailable);
              if (checkIncDec_Ret) {
                g_model.moduleData[EXTERNAL_MODULE].rfProtocol = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = DEFAULT_CHANNELS(EXTERNAL_MODULE);
                if (IS_MODULE_SBUS(EXTERNAL_MODULE))
                  g_model.moduleData[EXTERNAL_MODULE].sbus.refreshRate = -31;
              }
              break;
            case 1:
              if (IS_MODULE_DSM2(EXTERNAL_MODULE))
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, DSM2_PROTO_LP45, DSM2_PROTO_DSMX);
              else if (IS_MODULE_R9M(EXTERNAL_MODULE))
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, MODULE_SUBTYPE_R9M_FCC, MODULE_SUBTYPE_R9M_LBT);
#if defined(MULTIMODULE)
              else if (IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) {
                int multiRfProto = g_model.moduleData[EXTERNAL_MODULE].multi.customProto == 1 ? MM_RF_PROTO_CUSTOM : g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
                CHECK_INCDEC_MODELVAR(event, multiRfProto, MM_RF_PROTO_FIRST, MM_RF_PROTO_LAST);
                if (checkIncDec_Ret) {
                  g_model.moduleData[EXTERNAL_MODULE].multi.customProto = (multiRfProto == MM_RF_PROTO_CUSTOM);
                  if (!g_model.moduleData[EXTERNAL_MODULE].multi.customProto)
                    g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(multiRfProto);
                  g_model.moduleData[EXTERNAL_MODULE].subType = 0;
                  // Sensible default for DSM2 (same as for ppm): 7ch@22ms + Autodetect settings enabled
                  if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MM_RF_PROTO_DSM2) {
                    g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 1;
                  }
                  else {
                    g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 0;
                  }
                  g_model.moduleData[EXTERNAL_MODULE].multi.optionValue = 0;
                }
              }
#endif
              else {
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, RF_PROTO_X16, RF_PROTO_LAST);
              }
              if (checkIncDec_Ret) {
                g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = DEFAULT_CHANNELS(EXTERNAL_MODULE);
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = MAX_EXTERNAL_MODULE_CHANNELS();
              }
              break;
#if defined(MULTIMODULE)
            case 2:
              if (g_model.moduleData[EXTERNAL_MODULE].multi.customProto) {
                g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false), 0, 63, EE_MODEL));
                break;
              } else {
                const mm_protocol_definition *pdef = getMultiProtocolDefinition(g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false));
                if (pdef->maxSubtype > 0)
                  CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, pdef->maxSubtype);
              }
              break;
            case 3:
              // Custom protocol, third column is subtype
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, 7);
              break;
#endif
          }
        }
        break;

      case ITEM_MODEL_TRAINER_LABEL:
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_TRAINER);
        break;

      case ITEM_MODEL_TRAINER_LINE1:
#if defined(BLUETOOTH)
        if (g_model.trainerMode == TRAINER_MODE_MASTER_BLUETOOTH) {
          if (attr) {
            s_editMode = 0;
          }
          if (bluetoothDistantAddr[0]) {
            drawText(&menuBodyWindow,MENUS_MARGIN_LEFT + INDENT_WIDTH, y, bluetoothDistantAddr);
            if (bluetoothState != BLUETOOTH_STATE_CONNECTED) {
              drawButton(MODEL_SETUP_2ND_COLUMN, y, "Bind", menuHorizontalPosition == 0 ? attr : 0);
              drawButton(MODEL_SETUP_2ND_COLUMN+60, y, "Clear", menuHorizontalPosition == 1 ? attr : 0);
            }
            else {
              drawButton(MODEL_SETUP_2ND_COLUMN, y, "Clear", attr);
            }
            if (attr && event == EVT_KEY_FIRST(KEY_ENTER)) {
              if (bluetoothState == BLUETOOTH_STATE_CONNECTED || menuHorizontalPosition == 1) {
                bluetoothState = BLUETOOTH_STATE_OFF;
                bluetoothDistantAddr[0] = 0;
              }
              else {
                bluetoothState = BLUETOOTH_STATE_BIND_REQUESTED;
              }
            }
          }
          else {
            drawText(&menuBodyWindow,MENUS_MARGIN_LEFT + INDENT_WIDTH, y, "---");
            if (bluetoothState < BLUETOOTH_STATE_IDLE)
              drawButton(MODEL_SETUP_2ND_COLUMN, y, STR_BLUETOOTH_INIT, attr);
            else
              drawButton(MODEL_SETUP_2ND_COLUMN, y, STR_BLUETOOTH_DISC, attr);
            if (attr && event == EVT_KEY_FIRST(KEY_ENTER)) {
              if (bluetoothState < BLUETOOTH_STATE_IDLE)
                bluetoothState = BLUETOOTH_STATE_OFF;
              else
                bluetoothState = BLUETOOTH_STATE_DISCOVER_REQUESTED;
            }
          }
          break;
        }
        // no break
#endif

      case ITEM_MODEL_INTERNAL_MODULE_CHANNELS:
      case ITEM_MODEL_EXTERNAL_MODULE_CHANNELS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_CHANNELRANGE);
        if ((int8_t)PORT_CHANNELS_ROWS(moduleIdx) >= 0) {
          drawStringWithIndex(MODEL_SETUP_2ND_COLUMN, y, STR_CH, moduleData.channelsStart+1, menuHorizontalPosition==0 ? attr : 0);
          drawText(&menuBodyWindow,lcdNextPos+5, y, "-");
          drawStringWithIndex(lcdNextPos+5, y, STR_CH, moduleData.channelsStart+NUM_CHANNELS(moduleIdx), menuHorizontalPosition==1 ? attr : 0);
          if (IS_R9M_OR_XJTD16(moduleIdx)) {
            if (NUM_CHANNELS(moduleIdx) > 8)
              drawText(&menuBodyWindow,lcdNextPos + 15, y, "(18ms)");
            else
              drawText(&menuBodyWindow,lcdNextPos + 15, y, "(9ms)");
          }
          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.channelsStart, 32-8-moduleData.channelsCount);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.channelsCount, -4, min<int8_t>(MAX_CHANNELS(moduleIdx), 32-8-moduleData.channelsStart));
                if ((i == ITEM_MODEL_EXTERNAL_MODULE_CHANNELS && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_PPM)
                    || (i == ITEM_MODEL_TRAINER_LINE1)
                    )
                  SET_DEFAULT_PPM_FRAME_LENGTH(moduleIdx);
                break;
            }
          }
        }
        break;
      }

      case ITEM_MODEL_INTERNAL_MODULE_BIND:
      case ITEM_MODEL_EXTERNAL_MODULE_BIND:
      case ITEM_MODEL_TRAINER_LINE2:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        if (IS_MODULE_PPM(moduleIdx)) {
          drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_PPMFRAME);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.ppm.frameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT, 0, NULL, STR_MS);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+80, y, (moduleData.ppm.delay*50)+300, (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr|LEFT : LEFT, 0, NULL, "us");
          drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN+160, y, moduleData.ppm.pulsePol ? "+" : "-", (CURSOR_ON_LINE() || menuHorizontalPosition==2) ? attr : 0);
          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.frameLength, -20, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.delay, -4, 10);
                break;
              case 2:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.ppm.pulsePol, 1);
                break;
            }
          }
        }
        else if (IS_MODULE_SBUS(moduleIdx)) {
          drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_REFRESHRATE);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.ppm.frameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT, 0, NULL, STR_MS);
          drawText(&menuBodyWindow,MODEL_SETUP_3RD_COLUMN, y, moduleData.sbus.noninverted ? "not inverted" : "normal", (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0);

          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.frameLength, -33, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.sbus.noninverted, 1);
                break;
            }
          }
        }
        else {
          int l_posHorz = menuHorizontalPosition;
          coord_t xOffsetBind = MODEL_SETUP_BIND_OFS;
          if (IS_MODULE_XJT(moduleIdx) && IS_D8_RX(moduleIdx)) {
            xOffsetBind = 0;
            drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_RECEIVER);
            if (attr) l_posHorz += 1;
          }
          else {
            drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_RECEIVER_NUM);
          }
          if (IS_MODULE_PXX(moduleIdx) || IS_MODULE_DSM2(moduleIdx) || IS_MODULE_MULTIMODULE(moduleIdx)) {
            if (xOffsetBind)
              lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.header.modelId[moduleIdx], (l_posHorz==0 ? attr : 0) | LEADING0 | LEFT, 2);
            if (attr && l_posHorz==0 && s_editMode>0)
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.header.modelId[moduleIdx], MAX_RX_NUM(moduleIdx));
            drawButton(MODEL_SETUP_2ND_COLUMN+xOffsetBind, y, STR_MODULE_BIND, (moduleFlag[moduleIdx] == MODULE_BIND ? BUTTON_ON : BUTTON_OFF) | (l_posHorz==1 ? attr : 0));
            drawButton(MODEL_SETUP_2ND_COLUMN+MODEL_SETUP_RANGE_OFS+xOffsetBind, y, STR_MODULE_RANGE, (moduleFlag[moduleIdx] == MODULE_RANGECHECK ? BUTTON_ON : BUTTON_OFF) | (l_posHorz==2 ? attr : 0));
            uint8_t newFlag = 0;
#if defined(MULTIMODULE)
            if (multiBindStatus == MULTI_BIND_FINISHED) {
              multiBindStatus = MULTI_NORMAL_OPERATION;
              s_editMode = 0;
            }
#endif
            if (attr && l_posHorz>0) {
              if (s_editMode>0) {
                if (l_posHorz == 1) {
                  if (IS_MODULE_R9M(moduleIdx) || (IS_MODULE_XJT(moduleIdx) && g_model.moduleData[moduleIdx].rfProtocol == RF_PROTO_X16)) {
                    if (event == EVT_KEY_BREAK(KEY_ENTER)) {
                      uint8_t default_selection;
                      if (IS_MODULE_R9M_LBT(moduleIdx)) {
                        if (!IS_TELEMETRY_INTERNAL_MODULE())
                          POPUP_MENU_ADD_ITEM(STR_BINDING_25MW_CH1_8_TELEM_ON);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_25MW_CH1_8_TELEM_OFF);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_500MW_CH1_8_TELEM_OFF);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_500MW_CH9_16_TELEM_OFF);
                        default_selection = 2;
                      }
                      else {
                        if (!(IS_TELEMETRY_INTERNAL_MODULE() && moduleIdx == EXTERNAL_MODULE))
                          POPUP_MENU_ADD_ITEM(STR_BINDING_1_8_TELEM_ON);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_1_8_TELEM_OFF);
                        if (!(IS_TELEMETRY_INTERNAL_MODULE() && moduleIdx == EXTERNAL_MODULE))
                          POPUP_MENU_ADD_ITEM(STR_BINDING_9_16_TELEM_ON);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_9_16_TELEM_OFF);
                        default_selection = g_model.moduleData[moduleIdx].pxx.receiver_telem_off + (g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 << 1);
                      }
                      POPUP_MENU_SELECT_ITEM(default_selection);
                      POPUP_MENU_START(onBindMenu);
                      continue;
                    }
                    if (moduleFlag[moduleIdx] == MODULE_BIND) {
                      newFlag = MODULE_BIND;
                    }
                    else {
                      if (!popupMenuNoItems) {
                        s_editMode = 0;  // this is when popup is exited before a choice is made
                      }
                    }
                  }
                  else {
                    newFlag = MODULE_BIND;
                  }
                }
                else if (l_posHorz == 2) {
                  newFlag = MODULE_RANGECHECK;
                }
              }
            }
            moduleFlag[moduleIdx] = newFlag;
#if defined(MULTIMODULE)
            if (newFlag == MODULE_BIND)
              multiBindStatus = MULTI_BIND_INITIATED;
#endif
          }
        }
        break;
      }

      case ITEM_MODEL_INTERNAL_MODULE_FAILSAFE:
      case ITEM_MODEL_EXTERNAL_MODULE_FAILSAFE:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_FAILSAFE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VFAILSAFE, moduleData.failsafeMode, menuHorizontalPosition==0 ? attr : 0);
        if (moduleData.failsafeMode == FAILSAFE_CUSTOM) {
          drawButton(MODEL_SETUP_2ND_COLUMN + MODEL_SETUP_SET_FAILSAFE_OFS, y, STR_SET, menuHorizontalPosition==1 ? attr : 0);
        }
        if (attr) {
          if (moduleData.failsafeMode != FAILSAFE_CUSTOM)
            menuHorizontalPosition = 0;
          if (menuHorizontalPosition==0) {
            if (s_editMode>0) {
              CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.failsafeMode, FAILSAFE_LAST);
              if (checkIncDec_Ret) SEND_FAILSAFE_NOW(moduleIdx);
            }
          }
          else if (menuHorizontalPosition==1) {
            s_editMode = 0;
            if (moduleData.failsafeMode==FAILSAFE_CUSTOM && event==EVT_KEY_FIRST(KEY_ENTER)) {
              g_moduleIdx = moduleIdx;
              pushMenu(menuModelFailsafe);
            }
          }
          else {
            lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN, y, LCD_W - MODEL_SETUP_2ND_COLUMN - 2, 8, TEXT_COLOR);
          }
        }
        break;
      }

      case ITEM_MODEL_EXTERNAL_MODULE_OPTIONS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
#if defined(MULTIMODULE)
        if (IS_MODULE_MULTIMODULE(moduleIdx)) {
          int optionValue = g_model.moduleData[moduleIdx].multi.optionValue;

          const uint8_t multi_proto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true);
          const mm_protocol_definition *pdef = getMultiProtocolDefinition(multi_proto);
          if (pdef->optionsstr)
            drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, pdef->optionsstr);

          if (multi_proto == MM_RF_PROTO_FS_AFHDS2A)
            optionValue = 50 + 5 * optionValue;

          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, optionValue, LEFT | attr);
          if (attr) {
            if (multi_proto == MM_RF_PROTO_FS_AFHDS2A) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, 0, 70);
            }
            else if (multi_proto == MM_RF_PROTO_OLRS) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -1, 7);
            }
            else {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -128, 127);
            }
          }
        }
#endif
        if (IS_MODULE_R9M(moduleIdx)) {
          drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_MODULE_TELEMETRY);
          if (IS_TELEMETRY_INTERNAL_MODULE()) {
            drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN, y, STR_DISABLE_INTERNAL);
          }
          else {
            drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN, y, STR_MODULE_TELEM_ON);
          }
        }
        else if (IS_MODULE_SBUS(moduleIdx)) {
          drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_WARN_BATTVOLTAGE);
          drawValueWithUnit(MODEL_SETUP_4TH_COLUMN, y, getBatteryVoltage(), UNIT_VOLTS, attr|PREC2|LEFT);
        }
      }
      break;

      case ITEM_MODEL_EXTERNAL_MODULE_BIND_OPTIONS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);

        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT+ INDENT_WIDTH, y, "Bind mode");
        if (g_model.moduleData[moduleIdx].pxx.power == R9M_LBT_POWER_25) {
          if(g_model.moduleData[moduleIdx].pxx.receiver_telem_off == true)
            drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN, y, STR_BINDING_25MW_CH1_8_TELEM_OFF);
          else
            drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN, y, STR_BINDING_25MW_CH1_8_TELEM_ON);
        }
        else {
          if(g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 == true)
            drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN, y, STR_BINDING_500MW_CH9_16_TELEM_OFF);
          else
            drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN, y, STR_BINDING_500MW_CH1_8_TELEM_OFF);
        }
        while (menuVerticalPosition==i && menuHorizontalPosition > 0) {
          REPEAT_LAST_CURSOR_MOVE(ITEM_MODEL_SETUP_MAX, true);
        }
      }

      case ITEM_MODEL_EXTERNAL_MODULE_POWER:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
        if (IS_MODULE_R9M_FCC(moduleIdx)) {
          // Power selection is only available on R9M FCC
          drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_MULTI_RFPOWER);
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_FCC_POWER_VALUES, g_model.moduleData[moduleIdx].pxx.power, LEFT | attr);
          if (attr)
            CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].pxx.power, 0, R9M_FCC_POWER_MAX);
        }
#if defined(MULTIMODULE)
        else if (IS_MODULE_MULTIMODULE(moduleIdx)) {
          drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_MULTI_LOWPOWER);
          g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode = editCheckBox(&menuBodyWindow, g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        }
#endif
      }
      break;

#if defined(MULTIMODULE)
    case ITEM_MODEL_EXTERNAL_MODULE_AUTOBIND:
      if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MM_RF_PROTO_DSM2)
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_MULTI_DSM_AUTODTECT);
      else
        drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_MULTI_AUTOBIND);
      g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = editCheckBox(&menuBodyWindow, g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode, MODEL_SETUP_2ND_COLUMN, y, attr, event);
      break;
    case ITEM_MODEL_EXTERNAL_MODULE_STATUS: {
      drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_MODULE_STATUS);

      char statusText[64];
      multiModuleStatus.getStatusString(statusText);
      drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN, y, statusText);
      break;
    case ITEM_MODEL_EXTERNAL_MODULE_SYNCSTATUS: {
      drawText(&menuBodyWindow,MENUS_MARGIN_LEFT, y, STR_MODULE_SYNC);

      char statusText[64];
      multiSyncStatus.getRefreshString(statusText);
      drawText(&menuBodyWindow,MODEL_SETUP_2ND_COLUMN, y, statusText);
      break;
      }
    }
#endif
    }

    y += 40;
  }



  if (IS_RANGECHECK_ENABLE()) {
    theme->drawMessageBox("RSSI :", NULL, NULL, WARNING_TYPE_INFO);
    lcdDrawNumber(WARNING_LINE_X, WARNING_INFOLINE_Y, TELEMETRY_RSSI(), DBLSIZE|LEFT);
  }
#endif

  menuBodyWindow.setInnerHeight(rect.y);

  return true;
}

bool menuModelFailsafe(event_t event)
{
  uint8_t ch = 0;
  const uint8_t channelStart = g_model.moduleData[g_moduleIdx].channelsStart;
  const int lim = (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;
  const uint8_t SLIDER_W = 128;
  const uint8_t cols = NUM_CHANNELS(g_moduleIdx) > 8 ? 2 : 1;

  if (event == EVT_KEY_LONG(KEY_ENTER)) {
    killEvents(event);
    event = 0;
    if (s_editMode) {
      g_model.moduleData[g_moduleIdx].failsafeChannels[menuVerticalPosition] = channelOutputs[menuVerticalPosition+channelStart];
      storageDirty(EE_MODEL);
      AUDIO_WARNING1();
      s_editMode = 0;
      SEND_FAILSAFE_NOW(g_moduleIdx);
    }
    else {
      int16_t & failsafe = g_model.moduleData[g_moduleIdx].failsafeChannels[menuVerticalPosition];
      if (failsafe < FAILSAFE_CHANNEL_HOLD)
        failsafe = FAILSAFE_CHANNEL_HOLD;
      else if (failsafe == FAILSAFE_CHANNEL_HOLD)
        failsafe = FAILSAFE_CHANNEL_NOPULSE;
      else
        failsafe = 0;
      storageDirty(EE_MODEL);
      AUDIO_WARNING1();
      SEND_FAILSAFE_NOW(g_moduleIdx);
    }
  }

  SIMPLE_SUBMENU_WITH_OPTIONS("FAILSAFE", ICON_STATS_ANALOGS, NUM_CHANNELS(g_moduleIdx), OPTION_MENU_NO_SCROLLBAR);
  drawStringWithIndex(50, 3+FH, "Module", g_moduleIdx+1, MENU_TITLE_COLOR);

  for (uint8_t col=0; col < cols; col++) {
    for (uint8_t line=0; line < 8; line++) {
      coord_t x = col*(LCD_W/2);
      const coord_t y = MENU_CONTENT_TOP - FH + line*(FH+4);
      const int32_t channelValue = channelOutputs[ch+channelStart];
      int32_t failsafeValue = g_model.moduleData[g_moduleIdx].failsafeChannels[8*col+line];

      // Channel name if present, number if not
      if (g_model.limitData[ch+channelStart].name[0] != '\0') {
        putsChn(x+MENUS_MARGIN_LEFT, y-3, ch+1, TINSIZE);
        lcdDrawSizedText(x+MENUS_MARGIN_LEFT, y+5, g_model.limitData[ch+channelStart].name, sizeof(g_model.limitData[ch+channelStart].name), ZCHAR|SMLSIZE);
      }
      else {
        putsChn(x+MENUS_MARGIN_LEFT, y, ch+1, 0);
      }

      // Value
      LcdFlags flags = RIGHT;
      if (menuVerticalPosition == ch) {
        flags |= INVERS;
        if (s_editMode) {
          if (failsafeValue == FAILSAFE_CHANNEL_HOLD || failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
            s_editMode = 0;
          }
          else {
            flags |= BLINK;
            CHECK_INCDEC_MODELVAR(event, g_model.moduleData[g_moduleIdx].failsafeChannels[8*col+line], -lim, +lim);
          }
        }
      }

      x += (LCD_W/2)-4-MENUS_MARGIN_LEFT-SLIDER_W;

      if (failsafeValue == FAILSAFE_CHANNEL_HOLD) {
        drawText(&menuBodyWindow,x, y+2, "HOLD", flags|SMLSIZE);
        failsafeValue = 0;
      }
      else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
        drawText(&menuBodyWindow,x, y+2, "NONE", flags|SMLSIZE);
        failsafeValue = 0;
      }
      else {
#if defined(PPM_UNIT_US)
        lcdDrawNumber(x, y, PPM_CH_CENTER(ch)+failsafeValue/2, flags);
#elif defined(PPM_UNIT_PERCENT_PREC1)
        lcdDrawNumber(x, y, calcRESXto1000(failsafeValue), PREC1|flags);
#else
        lcdDrawNumber(x, y, calcRESXto1000(failsafeValue)/10, flags);
#endif
      }

      // Gauge
      x += 4;
      lcdDrawRect(x, y+3, SLIDER_W+1, 12);
      const coord_t lenChannel = limit((uint8_t)1, uint8_t((abs(channelValue) * SLIDER_W/2 + lim/2) / lim), uint8_t(SLIDER_W/2));
      const coord_t lenFailsafe = limit((uint8_t)1, uint8_t((abs(failsafeValue) * SLIDER_W/2 + lim/2) / lim), uint8_t(SLIDER_W/2));
      x += SLIDER_W/2;
      const coord_t xChannel = (channelValue>0) ? x : x+1-lenChannel;
      const coord_t xFailsafe = (failsafeValue>0) ? x : x+1-lenFailsafe;
      lcdDrawSolidFilledRect(xChannel, y+4, lenChannel, 5, TEXT_COLOR);
      lcdDrawSolidFilledRect(xFailsafe, y+9, lenFailsafe, 5, ALARM_COLOR);

      if (++ch >= NUM_CHANNELS(g_moduleIdx))
        break;

    }
  }

  return true;
}
