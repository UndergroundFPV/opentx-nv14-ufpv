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

#ifndef _FRAME_H_
#define _FRAME_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "opentx_types.h"
#include "strhelpers.h"
#include "gvars.h"
#include "translations.h"
#include "lcd_types.h"
#include "dataconstants.h"
#include "colors.h"

template <class W>
void drawText(W * window, coord_t x, coord_t y, const char * s, LcdFlags flags=0)
{
  window->drawSizedText(x, y, s, 255, flags);
}

template <class W>
void drawTextAtIndex(W * window, coord_t x, coord_t y, const char * s, uint8_t idx, LcdFlags flags=0)
{
  char length = *s++;
  window->drawSizedText(x, y, s+length*idx, length, flags & ~ZCHAR);
}

template <class W>
void drawNumber(W * window, coord_t x, coord_t y, int32_t val, LcdFlags flags=0, uint8_t len=0, const char * prefix=NULL, const char * suffix=NULL)
{
  char str[48+1]; // max=16 for the prefix, 16 chars for the number, 16 chars for the suffix
  char *s = str+32;
  *s = '\0';
  int idx = 0;
  int mode = MODE(flags);
  bool neg = false;
  if (val < 0) {
    val = -val;
    neg = true;
  }
  do {
    *--s = '0' + (val % 10);
    ++idx;
    val /= 10;
    if (mode!=0 && idx==mode) {
      mode = 0;
      *--s = '.';
      if (val==0)
        *--s = '0';
    }
  } while (val!=0 || mode>0 || (mode==MODE(LEADING0) && idx<len));
  if (neg) *--s = '-';

  // TODO needs check on all string lengths ...
  if (prefix) {
    int len = strlen(prefix);
    if (len <= 16) {
      s -= len;
      strncpy(s, prefix, len);
    }
  }
  if (suffix) {
    strncpy(&str[32], suffix, 16);
  }
  flags &= ~LEADING0;
  drawText(window, x, y, s, flags);
}

template <class W>
void drawSource(W * window, coord_t x, coord_t y, mixsrc_t idx, LcdFlags flags=0)
{
  char s[16];
  getSourceString(s, idx);
  drawText(window, x, y, s, flags);
}

template <class W>
void drawSwitch(W * window, coord_t x, coord_t y, swsrc_t idx, LcdFlags flags)
{
  char s[8];
  getSwitchString(s, idx);
  drawText(window, x, y, s, flags);
}

template <class W>
void drawStringWithIndex(W * window, coord_t x, coord_t y, const char * str, int idx, LcdFlags flags=0, const char * prefix=NULL, const char * suffix=NULL)
{
  char s[64];
  char * tmp = (prefix ? strAppend(s, prefix) : s);
  tmp = strAppend(tmp, str);
  tmp = strAppendUnsigned(tmp, abs(idx));
  if (suffix)
    strAppend(tmp, suffix);
  drawText(window, x, y, s, flags);
}

template <class W>
void drawGVarValue(W * window, coord_t x, coord_t y, gvar_t value, int16_t min, int16_t max, LcdFlags flags)
{
  uint16_t delta = GV_GET_GV1_VALUE(max);

  if (GV_IS_GV_VALUE(value, min, max)) {
    flags &= ~PREC1;

    int8_t idx = (int16_t) GV_INDEX_CALC_DELTA(value, delta);
    if (idx >= 0) ++idx;    // transform form idx=0=GV1 to idx=1=GV1 in order to handle double keys invert

    if (idx < 0) {
      drawStringWithIndex(window, x, y, STR_GV, -idx, flags, "-");
    }
    else {
      drawStringWithIndex(window, x, y, STR_GV, idx, flags);
    }
  }
  else {
    drawNumber(window, x, y, value, flags, 0, NULL, "%");
  }
}

template <class W>
void displayFlightModes(W * window, coord_t x, coord_t y, FlightModesType value, uint8_t attr)
{
  for (int i=0; i<MAX_FLIGHT_MODES; i++) {
    LcdFlags flags = TINSIZE; // ((menuHorizontalPosition==i && attr) ? INVERS : 0);
    flags |= ((value & (1<<i))) ? TEXT_DISABLE_COLOR : TEXT_COLOR;
    // if (attr && menuHorizontalPosition < 0) flags |= INVERS;
    char s[] = " ";
    s[0] = '0' + i;
    drawText(window, x, y, s, flags);
    x += 12;
  }
}

template <class W>
void drawTimerMode(W * window, coord_t x, coord_t y, int32_t mode, LcdFlags att=0)
{
  if (mode >= 0) {
    if (mode < TMRMODE_COUNT) {
      drawTextAtIndex(window, x, y, STR_VTMRMODES, mode, att);
      return;
    }
    else {
      mode -= (TMRMODE_COUNT-1);
    }
  }
  drawSwitch(window, x, y, mode, att);
}

template <class W>
void drawTimer(W * window, coord_t x, coord_t y, putstime_t tme, LcdFlags flags)
{
  char str[LEN_TIMER_STRING];
  getTimerString(str, tme, (flags & TIMEHOUR) != 0);
  drawText(window, x, y, str, flags);
}


template <class W>
void drawSolidRect(W * window, coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness=1, LcdFlags att=0)
{
  window->drawSolidFilledRect(x, y, thickness, h, att);
  window->drawSolidFilledRect(x+w-thickness, y, thickness, h, att);
  window->drawSolidFilledRect(x, y, w, thickness, att);
  window->drawSolidFilledRect(x, y+h-thickness, w, thickness, att);
}

template <class W>
int editChoice(W * window, coord_t x, coord_t y, const char * values, int value, int min, int max, LcdFlags attr, event_t event)
{
  // TODO if (attr & INVERS) value = checkIncDec(event, value, min, max, (menuVerticalPositions[0] == 0) ? EE_MODEL : EE_GENERAL);
  if (values) drawTextAtIndex(window, x, y, values, value-min, attr);
  return value;
}

extern int8_t s_editMode;
uint8_t zlen(const char *str, uint8_t size);

template <class W>
void editName(W * window, coord_t x, coord_t y, char * name, uint8_t size, event_t event, uint8_t active, LcdFlags flags=ZCHAR)
{
  if (active && s_editMode <= 0) {
    flags |= INVERS;
  }

  if (!active || s_editMode <= 0) {
    if (flags & ZCHAR) {
      uint8_t len = zlen(name, size);
      if (len == 0) {
        char tmp[] = "---";
        window->drawSizedText(x, y, tmp, size, flags-ZCHAR);
      }
      else {
        window->drawSizedText(x, y, name, len, flags);
      }
    }
    else {
      drawText(window, x, y, name, flags);
    }
  }

#if 0
  if (active) {
    if (s_editMode > 0) {
      int8_t c = name[editNameCursorPos];
      if (!(flags & ZCHAR)) {
        c = char2idx(c);
      }
      int8_t v = c;

      if (event==EVT_ROTARY_RIGHT || event==EVT_ROTARY_LEFT) {
        v = checkIncDec(event, abs(v), 0, ZCHAR_MAX, 0);
        if (c <= 0) v = -v;
      }

      switch (event) {
        case EVT_KEY_BREAK(KEY_LEFT):
          if (editNameCursorPos>0) editNameCursorPos--;
          break;

        case EVT_KEY_BREAK(KEY_RIGHT):
          if (editNameCursorPos<size-1) editNameCursorPos++;
          break;

        case EVT_KEY_BREAK(KEY_ENTER):
          if (s_editMode == EDIT_MODIFY_FIELD) {
            s_editMode = EDIT_MODIFY_STRING;
            editNameCursorPos = 0;
          }
          else if (editNameCursorPos<size-1)
            editNameCursorPos++;
          else
            s_editMode = 0;
          break;

        case EVT_KEY_LONG(KEY_ENTER):
          if (v == 0) {
            s_editMode = 0;
            killEvents(event);
            break;
          }
          // no break

        case EVT_KEY_LONG(KEY_LEFT):
        case EVT_KEY_LONG(KEY_RIGHT):
          if (v>=-26 && v<=26) {
            v = -v; // toggle case
            if (event==EVT_KEY_LONG(KEY_LEFT))
              killEvents(KEY_LEFT);
          }
          break;
      }

      if (c != v) {
        if (!(flags & ZCHAR)) {
          if (v != '\0' || name[editNameCursorPos+1] != '\0')
            v = idx2char(v);
        }
        name[editNameCursorPos] = v;
        storageDirty(menuVerticalPositions[0] == 0 ? EE_MODEL : EE_GENERAL);
      }

      lcdDrawSizedText(x, y, name, size, flags);
      coord_t left = (editNameCursorPos == 0 ? 0 : getTextWidth(name, editNameCursorPos, flags));
      char s[] = { (flags & ZCHAR) ? idx2char(name[editNameCursorPos]) : name[editNameCursorPos], '\0' };
      lcdDrawSolidFilledRect(x+left-1, y, getTextWidth(s, 1)+1, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
      lcdDrawText(x+left, y, s, TEXT_INVERTED_COLOR);
    }
    else {
      editNameCursorPos = 0;
    }
  }
#endif
}

#endif // _FRAME_H_
