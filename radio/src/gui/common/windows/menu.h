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
#include "window.h"

class Menu;

class MenuPage {
  public:
    MenuPage(const char * title, unsigned icon):
      title(title),
      icon(icon)
    {
    }

    virtual void build(Window * window) = 0;

    const char * title;
    unsigned icon;
};

class MenuPagesCarousel: public Window {
  public:
    MenuPagesCarousel(Window * parent, Menu * menu);

    void updateInnerWidth();

    void paint(BitmapBuffer * dc);

    bool onTouch(coord_t x, coord_t y);

  protected:
    constexpr static uint8_t padding_left = 3;
    Menu * menu;
    uint8_t currentPage = 0;
};

class MenuHeaderWindow: public Window {
    friend class Menu;

  public:
    MenuHeaderWindow(Menu * menu);

    void paint(BitmapBuffer * dc);

    void setTitle(const char * title)
    {
      this->title = title;
    }

  protected:
    IconButton back;
    MenuPagesCarousel carousel;
    const char * title = nullptr;
};

class Menu: public Window {
    friend class MenuPagesCarousel;

  public:
    Menu();

    void addPage(MenuPage * page);

    void setCurrentPage(MenuPage * page);

    void setCurrentPage(unsigned index)
    {
      setCurrentPage(pages[index]);
    }

  protected:
    MenuHeaderWindow header;
    Window body;
    std::vector<MenuPage *> pages;
    MenuPage * currentPage = nullptr;
};

#endif
