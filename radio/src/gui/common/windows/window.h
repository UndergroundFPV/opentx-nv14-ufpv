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

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <list>
#include <functional>
#include <touch_driver.h>
#include "bitmapbuffer.h"
#include "debug.h"

class Window {
  friend class GridLayout;

  public:
    Window(Window * parent, const rect_t & rect);

    virtual ~Window();

    Window * getParent() const {
      return parent;
    }

    void setCloseHandler(std::function<void()> handler) {
      onClose = handler;
    }

    void deleteLater(bool detach=true);

    void clear();

    void deleteChildren();

    bool hasFocus() const
    {
      return focusWindow == this;
    }

    void scrollTo(Window * child);

    static void clearFocus();

    void setFocus();

    void setWidth(coord_t w)
    {
      rect.w = w;
      invalidate();
    }

    void setHeight(coord_t h)
    {
      rect.h = h;
      invalidate();
    }

    void setTop(coord_t y)
    {
      rect.y = y;
      invalidate();
    }

    coord_t top() const
    {
      return rect.y;
    }

    coord_t bottom() const
    {
      return rect.y + rect.h;
    }

    coord_t width() const
    {
      return rect.w;
    }

    coord_t height() const
    {
      return rect.h;
    }

    void setInnerWidth(coord_t w)
    {
      innerWidth = w;
    }

    void setInnerHeight(coord_t h)
    {
      innerHeight = h;
    }

    coord_t getScrollPositionY()
    {
      return scrollPositionY;
    }

    void setScrollPositionY(coord_t y)
    {
      scrollPositionY = y;
    }

    virtual void paint(BitmapBuffer * dc)
    {
    }

    void drawVerticalScrollbar(BitmapBuffer * dc);

    void paintChildren(BitmapBuffer * dc);

    void fullPaint(BitmapBuffer * dc);

    bool pointInRect(coord_t x, coord_t y, rect_t & rect)
    {
      return (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h);
    }

    virtual void onFocusLost() { invalidate(); };

    virtual bool onTouchStart(coord_t x, coord_t y);

    virtual bool onTouchEnd(coord_t x, coord_t y);

    virtual bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY);

    void adjustInnerHeight();

    coord_t adjustHeight();

    void moveWindowsTop(coord_t y, coord_t delta);

    coord_t innerWidth, innerHeight; // TODO protected later

    void invalidate()
    {
      invalidate({0, 0, rect.w, rect.h});
    }

    virtual void checkEvents();

  protected:
    Window * parent;
    std::list<Window *> children;
    rect_t rect;

    coord_t scrollPositionX = 0;
    coord_t scrollPositionY = 0;
    static Window * focusWindow;
    static std::list<Window *> trash;
    std::function<void()> onClose;

    void detach();


    void addChild(Window * window)
    {
      children.push_back(window);
    }

    void removeChild(Window * window)
    {
      children.remove(window);
      invalidate();
    }

    virtual void invalidate(const rect_t & rect);
};

class MainWindow: public Window {
  public:
    MainWindow():
      Window(nullptr, {0, 0, LCD_W, LCD_H}),
      invalidatedRect(rect)
    {
    }

    virtual void checkEvents() override;

    virtual void invalidate(const rect_t & rect) override;

    bool refresh();

    void run()
    {
      checkEvents();
      if (refresh()) {
        lcdRefresh();
      }
    }

  protected:
    void emptyTrash();
    rect_t invalidatedRect;
};

extern MainWindow mainWindow;

#endif