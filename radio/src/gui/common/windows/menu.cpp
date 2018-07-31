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

MenuWindow::MenuWindow(Menu * parent):
  Window(parent, {LCD_W / 2 - 100, LCD_H / 2 - 20 /* to avoid redraw the menus header */, 200, 0}, OPAQUE)
{
}

bool MenuWindow::onTouchEnd(coord_t x, coord_t y)
{
  int index = y / lineHeight;
  lines[index].onPress();
  return true;
}

void MenuWindow::paint(BitmapBuffer * dc)
{
  dc->clear(HEADER_BGCOLOR);
  for (unsigned i=0; i<lines.size(); i++) {
    dc->drawText(10, i * lineHeight + (lineHeight - 22) / 2, lines[i].text.data(), MENU_TITLE_COLOR);
    if (i > 0) {
      dc->drawSolidHorizontalLine(0, i * lineHeight, 200, CURVE_AXIS_COLOR);
    }
  }
}

void MenuWindow::updatePosition()
{
  int count = min<int>(8, lines.size());
  coord_t h = count * lineHeight;
  setTop((LCD_H - h) / 2);
  setHeight(h);
  setInnerHeight(lines.size() * lineHeight);
}

bool Menu::onTouchEnd(coord_t x, coord_t y)
{
  Window::onTouchEnd(x, y);
  deleteLater();
  return true;
}

bool Menu::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  Window::onTouchSlide(x, y, startX, startY, slideX, slideY);
  return true;
}
