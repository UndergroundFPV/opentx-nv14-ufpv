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

class GridLayout {
  public:
    GridLayout(Window &window) :
      window(window)
    {
    }

    void setLabelWidth(coord_t width)
    {
      labelWidth = width;
    }

    rect_t getLabelSlot(bool indent = false)
    {
      coord_t left = indent ? lineMarginLeft + indentWidth : lineMarginLeft;
      return { left, currentY, labelWidth - left, lineHeight };
    }

    rect_t getFieldSlot(uint8_t count = 1, uint8_t index = 0)
    {
      coord_t width = (LCD_W - labelWidth - lineMarginRight - (count - 1) * lineMargin) / count;
      coord_t left = labelWidth + (width + lineMargin) * index;
      return {left, currentY, width, lineHeight};
    }

    void nextLine()
    {
      currentY += lineHeight + lineMargin;
    }

    void addWindow(Window * window)
    {
      window->adjustHeight();
      currentY += window->rect.h + lineMargin;
    }

    void spacer(coord_t height=lineMargin)
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
    coord_t labelWidth = 140;
    static constexpr uint8_t lineMargin = 7;
    static constexpr uint8_t lineMarginLeft = 6;
    static constexpr uint8_t lineMarginRight = 12;
    static constexpr uint8_t lineHeight = 25;
    static constexpr uint8_t indentWidth = 12;
};

#endif
