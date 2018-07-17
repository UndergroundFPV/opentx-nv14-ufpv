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

bool Menu::onTouchEnd(coord_t x, coord_t y)
{
  int index = y / lineHeight;
  lines[index].onPress();
  return true;
}

void Menu::paint(BitmapBuffer * dc)
{
  dc->clear(HEADER_BGCOLOR);
  for (unsigned i=0; i<lines.size(); i++) {
    dc->drawText(10, i * lineHeight + 5, lines[i].text, MENU_TITLE_COLOR);
    if (i > 0)
      dc->drawSolidHorizontalLine(0, i * lineHeight, 200, CURVE_AXIS_COLOR);
  }
}
