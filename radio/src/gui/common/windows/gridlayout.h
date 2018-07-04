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

#ifndef _GRIDLAYOUT_H_
#define _GRIDLAYOUT_H_

#define CONTENT_WIDTH     140
#define INDENT_WIDTH      12

class GridLayout {
  public:
    GridLayout(Window &window) :
      window(window)
    {
    }

    rect_t getLabelSlot(bool indent = false)
    {
      coord_t left = indent ? MENUS_MARGIN_LEFT + INDENT_WIDTH : MENUS_MARGIN_LEFT;
      return { left, currentY, LCD_W - MENUS_MARGIN_RIGHT - CONTENT_WIDTH, 30 };
    }

    rect_t getFieldSlot(uint8_t count = 1, uint8_t index = 0)
    {
      coord_t width = (CONTENT_WIDTH - (count - 1) * 10) / count;
      coord_t left = LCD_W - MENUS_MARGIN_RIGHT - CONTENT_WIDTH + (width + 10) * index;
      return {left, currentY, width, 30};
    }

    void nextLine()
    {
      currentY += 30 + 10;
    }

    void addWindow(Window * window)
    {
      window->adjustHeight();
      currentY += window->rect.h + 10;
    }

    void spacer(coord_t height=10)
    {
      currentY += height;
    }

    coord_t getWindowHeight() const
    {
      return currentY;
    }

  protected:
    Window &window;
    coord_t currentY = 0;
};

#endif
