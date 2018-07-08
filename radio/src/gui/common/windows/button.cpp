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

bool Button::onTouch(coord_t x, coord_t y)
{
  if (enabled) {
    state = onPress();
    invalidate();
  }
  return true;
}

void TextButton::paint(BitmapBuffer * dc)
{
  dc->drawText(rect.w / 2, 2, text, CENTERED | (enabled ? 0 : TEXT_DISABLE_COLOR));
  drawSolidRect(dc, 0, 0, rect.w, rect.h, 2, state ? SCROLLBOX_COLOR : CURVE_AXIS_COLOR);
}

#include "alpha_button_on.lbm"
#include "alpha_button_off.lbm"

void IconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, theme->getIconBitmap(icon, state));
}

FabIconButton::FabIconButton(Window * parent, coord_t x, coord_t y, uint8_t icon, std::function<uint8_t(void)> onPress, uint8_t state):
  Button(parent, { x - 34, y - 34, 68, 68 }, onPress, state),
  icon(icon)
{
}

void FabIconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, state ? &ALPHA_BUTTON_ON : &ALPHA_BUTTON_OFF);
  const BitmapBuffer * mask = theme->getIconMask(icon);
  dc->drawMask((68-mask->getWidth())/2, (68-mask->getHeight())/2, mask, TEXT_BGCOLOR);
}
