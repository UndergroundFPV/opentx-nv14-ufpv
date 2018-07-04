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
#include <touch_driver.h>
#include "bitmapbuffer.h"
#include "debug.h"

class Window {
  public:
    Window(Window * parent, const rect_t & rect):
      parent(parent),
      rect(rect),
      innerWidth(rect.w),
      innerHeight(rect.h),
      scrollPositionX(0),
      scrollPositionY(0) {
      if (parent) {
        parent->addChild(this);
      }
    }

    virtual ~Window() {
    }

    void clear()
    {
      scrollPositionX = 0;
      scrollPositionY = 0;
      innerWidth = rect.w;
      innerHeight = rect.h;

      for (auto window: children) {
        delete window;
      }
      children.clear();
    }

    bool hasFocus()
    {
      return focusWindow == this;
    }

    void clearFocus() {
      if (focusWindow)
        focusWindow->onFocusLost();
      focusWindow = NULL;
    }

    void setFocus()
    {
      clearFocus();
      focusWindow = this;
    }

    void setWidth(coord_t w) {
      rect.w = w;
    }

    void setHeight(coord_t h) {
      rect.h = h;
    }

    void setInnerWidth(coord_t w) {
      innerWidth = w;
    }

    void setInnerHeight(coord_t h) {
      innerHeight = h;
    }

    void refresh(rect_t & rect);

    void refresh() {
      refresh(rect);
    }

    virtual void paint(BitmapBuffer * dc) { }

    void drawVerticalScrollbar(BitmapBuffer * dc);

    void paintChildren(BitmapBuffer * dc);

    void fullPaint(BitmapBuffer * dc);

    bool pointInRect(coord_t x, coord_t y, rect_t & rect) {
      return (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h);
    }

    virtual void onFocusLost() { };

    virtual bool onTouch(coord_t x, coord_t y);

    virtual bool onSlide(coord_t startX, coord_t startY, coord_t slideX, coord_t slideY);

    void adjustInnerHeight();

    coord_t adjustHeight();

    void moveWindowsTop(coord_t y, coord_t delta);

    void checkEvents()
    {
      if (touchState.Event == TE_UP) {
        onTouch(touchState.startX - scrollPositionX, touchState.startY - scrollPositionY);
        touchState.Event = TE_NONE;
      }
      else if (touchState.Event == TE_SLIDE) {
        coord_t x = touchState.X - touchState.lastX;
        coord_t y = touchState.Y - touchState.lastY;
        onSlide(touchState.startX, touchState.startY, x, y);
        touchState.lastX = touchState.X;
        touchState.lastY = touchState.Y;
      }
    }

    void addChild(Window * window) {
      children.push_back(window);
    }

  public: // TODO protected later ...
    Window * parent;
    std::list<Window *> children;
    rect_t rect;
    coord_t innerWidth, innerHeight;
    coord_t scrollPositionX, scrollPositionY;
    static Window * focusWindow;
};

class MainWindow: public Window {
  public:
    MainWindow():
      Window(NULL, {0, 0, LCD_W, LCD_H}) {
    }
};

extern Window mainWindow;

#endif