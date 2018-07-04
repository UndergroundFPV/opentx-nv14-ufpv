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

#ifndef _SWITCHCHOICE_H_
#define _SWITCHCHOICE_H_

#include <functional>
#include "window.h"

class SwitchChoice : public Window {
  public:
    SwitchChoice(Window * parent, const rect_t & rect, SwitchContext context, std::function<int16_t()> getValue, std::function<void(int16_t)> setValue):
      Window(parent, rect),
      context(context),
      getValue(getValue),
      setValue(setValue)
    {
    }

    void paint(BitmapBuffer * dc) override ;

    bool onTouch(coord_t x, coord_t y) override ;

  protected:
    const char * label;
    SwitchContext context;
    std::function<int16_t()> getValue;
    std::function<void(int16_t)> setValue;
};

#endif // _SWITCHCHOICE_H_
