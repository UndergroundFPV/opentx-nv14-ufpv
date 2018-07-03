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


#if 1
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

    void deleteWindows()
    {
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

#else
class Window
{
  public:
    Window(coord_t x, coord_t y, coord_t width, coord_t height):
      x(x),
      y(y),
      width(width),
      height(height),
      offsetX(0),
      offsetY(0),
      innerWidth(width),
      innerHeight(height),
      focusX(-1),
      focusY(-1)
    {
    }

    coord_t x, y, width, height, offsetX, offsetY, innerWidth, innerHeight, focusX, focusY;

/*
    void drawIconButton(uint32_t x, uint32_t y, uint8_t index, LcdFlags flags)
    {
      lcd->setClippingRect(this->x, this->x + this->width, this->y, this->y + this->height);
      ::drawIconButton(x + this->x, y + this->y + this->offsetY, index, flags);
      lcd->clearClippingRect();
    }

    bool handleIconButton(uint32_t x, uint32_t y, const char * text, uint32_t flags)
    {
      drawButton(x, y, text, flags);
      if (touch(x, y, getTextWidth(text, 0, 0)+20, 21)) {
        return true;
      }
      return false;
    }
*/
    void scroll() {
      if (touchState.Event == TE_SLIDE) {
        if (innerHeight > height && touchState.startX >= this->x && touchState.startX <= this->x + this->width) {
          this->offsetY = limit<coord_t>(-innerHeight + height, this->offsetY + touchState.Y - touchState.startY, 0);
          touchState.startY = touchState.Y;
        }
        if (innerWidth > width && touchState.startY >= this->y && touchState.startY <= this->y + this->height) {
          this->offsetX = limit<coord_t>(-innerWidth + width, this->offsetX + touchState.X - touchState.startX, 0);
          touchState.startX = touchState.X;
        }
      }
    }

    bool touchPressed(coord_t x, coord_t y, coord_t w, coord_t h) {
      return ::touchPressed(this->x + this->offsetX + x, this->y + this->offsetY + y, w, h);
    }

    void setHeight(coord_t height) {
      this->height = height;
    }

    void setInnerWidth(coord_t value) {
      innerWidth = value;
    }

    void setInnerHeight(coord_t value) {
      innerHeight = value;
    }

    void setFocus(coord_t x, coord_t y) {
      focusX = x;
      focusY = y;
    }

    void clearFocus() {
      setFocus(-1, -1);
    }

    bool hasFocus(coord_t x, coord_t y) {
      return (x == focusX && y == focusY);
    }

    void drawVerticalScrollbar();

    void drawSolidFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags flags)
    {
      setClippingRect();
      lcd->drawSolidFilledRect(this->x + this->offsetX + x, this->y + this->offsetY + y, w, h, flags);
      clearLcdLimits();
    }

    template<class T>
    void drawBitmap(coord_t x, coord_t y, const T * bmp, coord_t srcx=0, coord_t srcy=0, coord_t w=0, coord_t h=0, float scale=0) {
      if (this->offsetX + x >= this->width || this->offsetY + y >= this->height) {
        return;
      }
      setClippingRect();
      lcd->drawBitmap(this->x + this->offsetX + x, this->y + this->offsetY + y, bmp, srcx, srcy, w, h, scale);
      clearLcdLimits();
    }

    inline void setClippingRect() {
      lcd->setClippingRect(this->x, this->x + this->width, this->y, this->y + this->height);
    }

    inline void clearLcdLimits() {
      lcd->clearClippingRect();
    }

    void drawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdFlags flags) {
      setClippingRect();
      lcd->drawSizedText(this->x + this->offsetX + x, this->y + this->offsetY + y, s, len, flags);
      clearLcdLimits();
    }

};

// TODO quick & dirty
extern BitmapBuffer * keyboardBitmap;

#endif
#endif