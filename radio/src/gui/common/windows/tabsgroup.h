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

#ifndef _TABSGROUP_H_
#define _TABSGROUP_H_

#include <vector>
#include "window.h"

class TabsGroup;

extern Keyboard * keyboard;

class PageTab {
  public:
    PageTab(const char * title, unsigned icon):
      title(title),
      icon(icon)
    {
    }

    virtual void build(Window * window) = 0;

    const char * title;
    unsigned icon;
};

class TabsGroupCarousel: public Window {
  public:
    TabsGroupCarousel(Window * parent, TabsGroup * menu);

    void updateInnerWidth();

    virtual void paint(BitmapBuffer * dc) override;

    virtual bool onTouchEnd(coord_t x, coord_t y) override;

  protected:
    constexpr static uint8_t padding_left = 3;
    TabsGroup * menu;
    uint8_t currentPage = 0;
};

class TabsGroupHeader: public Window {
    friend class TabsGroup;

  public:
    TabsGroupHeader(TabsGroup * menu);

    virtual void paint(BitmapBuffer * dc) override;

    void setTitle(const char * title)
    {
      this->title = title;
    }

  protected:
    IconButton back;
    TabsGroupCarousel carousel;
    const char * title = nullptr;
};

class TabsGroup: public Window {
    friend class TabsGroupCarousel;

  public:
    TabsGroup();

    void addTab(PageTab * page);

    void setCurrentTab(PageTab * tab);

    void setCurrentTab(unsigned index)
    {
      setCurrentTab(tabs[index]);
    }

  protected:
    TabsGroupHeader header;
    Window body;
    std::vector<PageTab *> tabs;
    PageTab * currentTab = nullptr;
};

#endif
