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

#include "timeedit.h"
#include "keyboard_number.h"
#include "draw_functions.h"
#include "strhelpers.h"

#if 0
RGB_SPLIT(value->unsignedValue, r, g, b);

if (attr && menuHorizontalPosition < 0) {
lcdDrawSolidFilledRect(SCREENS_SETUP_2ND_COLUMN - 3, y - 1, 230, FH + 1, TEXT_INVERTED_BGCOLOR);
}

lcdSetColor(value->unsignedValue);
lcdDrawSolidFilledRect(SCREENS_SETUP_2ND_COLUMN - 1, y + 1, 42, 17, TEXT_COLOR);
lcdDrawSolidFilledRect(SCREENS_SETUP_2ND_COLUMN, y + 2, 40, 15, CUSTOM_COLOR);

r = editColorPart(SCREENS_SETUP_2ND_COLUMN + 50, y, event, 0, r, attr, i_flags);
g = editColorPart(SCREENS_SETUP_2ND_COLUMN + 110, y, event, 1, g, attr, i_flags);
b = editColorPart(SCREENS_SETUP_2ND_COLUMN + 170, y, event, 2, b, attr, i_flags);

if (attr && checkIncDec_Ret) {
value->unsignedValue = RGB_JOIN(r, g, b);
}

uint8_t editColorPart(coord_t x, coord_t y, event_t event, uint8_t part, uint8_t value, LcdFlags attr, uint32_t i_flags)
{
  const char * STR_COLOR_PARTS = "\002" "R:" "G:" "B:";
  uint8_t PART_BITS[] = { 5, 6, 5 };
  lcdDrawTextAtIndex(x, y, STR_COLOR_PARTS, part, (attr && menuHorizontalPosition < 0) ? TEXT_INVERTED_COLOR : TEXT_COLOR);
  lcdDrawNumber(x + 20, y, value << (8-PART_BITS[part]), LEFT|TEXT_COLOR|((attr && (menuHorizontalPosition < 0 || menuHorizontalPosition == part)) ? attr : TEXT_COLOR));
  if (attr && menuHorizontalPosition == part) {
    value = checkIncDec(event, value, 0, (1 << PART_BITS[part])-1, i_flags);
  }
  return value;
}
#endif

ColorEdit::ColorEdit(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax, std::function<int32_t()> getValue, std::function<void(int32_t)> setValue, LcdFlags flags):
  BaseNumberEdit(parent, rect, vmin, vmax, getValue, setValue, flags)
{
}

void ColorEdit::paint(BitmapBuffer * dc)
{
  bool hasFocus = this->hasFocus();
  LcdFlags textColor = 0;
  LcdFlags lineColor = CURVE_AXIS_COLOR;
  if (hasFocus) {
    textColor = TEXT_INVERTED_BGCOLOR;
    lineColor = TEXT_INVERTED_BGCOLOR;
  }

  dc->drawText(3, 2, getTimerString(_getValue(), (flags & TIMEHOUR) != 0), textColor);

  drawSolidRect(dc, 0, 0, rect.w, rect.h, 1, lineColor);
}

bool ColorEdit::onTouchEnd(coord_t x, coord_t y)
{
  if (!hasFocus()) {
    setFocus();
  }

  NumberKeyboard * keyboard = NumberKeyboard::instance();
  if (keyboard->getField() != this) {
    keyboard->setField(this);
  }

  return true;
}

void ColorEdit::onFocusLost()
{
  NumberKeyboard::instance()->disable();
}
