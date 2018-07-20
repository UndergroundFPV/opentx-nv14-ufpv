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

NumberEdit::NumberEdit(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax, int32_t step,
  std::function<int32_t()> getValue, std::function<void(int32_t)> setValue, LcdFlags flags,
  const char * prefix, const char * suffix, const char * zeroText) :
  Window(parent, rect),
  vmin(vmin),
  vmax(vmax),
  step(step),
  getValue(getValue),
  setValue(setValue),
  flags(flags),
  prefix(prefix),
  suffix(suffix),
  zeroText(zeroText)
{
}

void NumberEdit::paint(BitmapBuffer * dc)
{
  bool hasFocus = this->hasFocus();
  LcdFlags textColor = 0;
  LcdFlags lineColor = CURVE_AXIS_COLOR;
  if (hasFocus) {
    textColor = TEXT_INVERTED_BGCOLOR;
    lineColor = TEXT_INVERTED_BGCOLOR;
  }
  int32_t value = getValue();
  if (value == 0 && zeroText)
    dc->drawText(3, 2, zeroText, textColor | flags);
  else
    drawNumber(dc, 3, 2, value, textColor | flags, 0, prefix, suffix);
  drawSolidRect(dc, 0, 0, rect.w, rect.h, 1, lineColor);
}

bool NumberEdit::onTouchEnd(coord_t x, coord_t y)
{
  if (hasFocus()) {
    int32_t value = getValue() + step;
    if (value > vmax)
      value = vmin;
    setValue(value);
  }
  else {
    setFocus();
  }
  invalidate();
  return true;
}
