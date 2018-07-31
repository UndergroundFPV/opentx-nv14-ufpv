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

class Menu;

class MenuWindow: public Window {
  class MenuLine {
    friend class MenuWindow;

    public:
      MenuLine(const char * text, std::function<void()> onPress):
        text(text),
        onPress(onPress)
      {
      }

      MenuLine(MenuLine &&) = default;

    protected:
      std::string text;
      std::function<void()> onPress;

      MenuLine(MenuLine &) = delete;
  };

  public:
    MenuWindow(Menu * parent);

    void addLine(const char * text, std::function<void()> onPress)
    {
      lines.emplace_back(text, onPress);
      updatePosition();
    }

    virtual void paint(BitmapBuffer * dc) override;

    virtual bool onTouchEnd(coord_t x, coord_t y) override;

  protected:
    std::vector<MenuLine> lines;
    static constexpr uint8_t lineHeight = 40;
    void updatePosition();
};

class Menu : public Window {
  public:
    Menu() :
      Window(&mainWindow, {0, 0, LCD_W, LCD_H}),
      menuWindow(this)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "Menu";
    }
#endif

    void addLine(const char * text, std::function<void()> onPress)
    {
      menuWindow.addLine(text, onPress);
    }

    virtual bool onTouchStart(coord_t x, coord_t y) override
    {
      return true;
    }

    virtual bool onTouchEnd(coord_t x, coord_t y) override;

    virtual bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
    {
      return true;
    }

  protected:
    MenuWindow menuWindow;
};

#endif
