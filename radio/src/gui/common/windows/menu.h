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

#ifndef _MENU_H_
#define _MENU_H_

#include <vector>
#include <functional>
#include "window.h"

class Menu : public Window {
  struct MenuLine {
    const char * text;
    std::function<void()> onPress;
  };

  public:
    Menu():
      Window(&mainWindow, {LCD_W/2 - 100, LCD_H/2, 200, 0})
    {
    }

    void addLine(const char * text, std::function<void()> onPress)
    {
      lines.push_back({text, onPress});
      coord_t h = height() + lineHeight;
      setTop((LCD_H - h) / 2);
      setHeight(h);
    }

    virtual void paint(BitmapBuffer * dc) override;

    virtual bool onTouchEnd(coord_t x, coord_t y) override;

  protected:
    std::vector<MenuLine> lines;
    static constexpr uint8_t lineHeight = 30;
};

#endif
