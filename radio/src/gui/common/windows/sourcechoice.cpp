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

class SourceMenuHeader: public Window {
  public:
    SourceMenuHeader(Menu * menu):
      Window(menu, { 35, 95, 50, 370 })
    {
      char icon[] = " ";
      for (int i=0; i<12; i++) {
        icon[0] = '\307' + i;
        new TextButton(this, {0, 30 * i, 50, 30}, icon);
      }
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->clear(CURVE_AXIS_COLOR); // TODO only after the latest button
    }

    bool onTouchEnd(coord_t x, coord_t y) override
    {
      Window::onTouchEnd(x, y);
      return true; // = don't close the menu (inverted so that click outside the menu closes it)
    }

  protected:
    int8_t selected = -1;
};

void SourceChoice::paint(BitmapBuffer * dc)
{
  bool hasFocus = this->hasFocus();
  unsigned value = getValue();
  LcdFlags textColor = (value == 0 ? CURVE_AXIS_COLOR : 0);
  LcdFlags lineColor = CURVE_AXIS_COLOR;
  if (hasFocus) {
    textColor = TEXT_INVERTED_BGCOLOR;
    lineColor = TEXT_INVERTED_BGCOLOR;
  }
  drawSource(dc, 3, 2, value, textColor);
  drawSolidRect(dc, 0, 0, rect.w, rect.h, 1, lineColor);
}

bool SourceChoice::onTouchEnd(coord_t x, coord_t y)
{
  auto menu = new Menu();
  auto value = getValue();
  int count = 0;
  int current = -1;

  for (int i = vmin; i < vmax; ++i) {
    if (isValueAvailable && !isValueAvailable(i))
      continue;
    menu->addLine(getSourceString(i), [=]() {
      setValue(i);
    });
    if (value == i) {
      current = count;
    }
    ++count;
  }

  menu->setNavigationBar(new SourceMenuHeader(menu));

  if (current >= 0) {
    menu->select(current);
  }

  setFocus();
  return true;
}
