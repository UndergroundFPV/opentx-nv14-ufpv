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
