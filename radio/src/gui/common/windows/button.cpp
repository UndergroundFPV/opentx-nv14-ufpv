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

void TextButton::paint(BitmapBuffer * dc)
{
  dc->drawText(rect.w / 2, 3, label, CENTERED);
  if (state)
    drawSolidRect(dc, 0, 0, rect.w, rect.h, 2, SCROLLBOX_COLOR);
  else
    drawSolidRect(dc, 0, 0, rect.w, rect.h, 1, CURVE_AXIS_COLOR);
}

#include "alpha_button_on.lbm"
#include "alpha_button_off.lbm"

void IconButton::paint(BitmapBuffer * dc)
{
  if (0) {
    dc->drawBitmap(0, 0, &ALPHA_BUTTON_OFF);
    theme->drawIcon((68-36)/2, (68-36)/2, icon, TEXT_BGCOLOR);
  }
  else {
    dc->drawBitmap(0, 0, theme->getIconBitmap(icon, state));
  }
}
