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

#include "window.h"

Window * Window::focusWindow = NULL;
Window mainWindow(NULL, { 0, 0, LCD_W, LCD_H });

void Window::refresh(rect_t & rect)
{
  if (parent) {
    rect.x += this->rect.x;
    rect.y += this->rect.y;
    parent->refresh(rect);
  }
  else {
    lcd->setOffset(0, 0);
    lcd->setClippingRect(0, LCD_W, 0, LCD_H);
    lcd->clear(TEXT_BGCOLOR);
    fullPaint(lcd);
  }
}

void Window::fullPaint(BitmapBuffer * dc)
{
  paint(dc);
  drawVerticalScrollbar(dc);
  paintChildren(dc);
}

void Window::paintChildren(BitmapBuffer * dc)
{
  coord_t x = dc->getOffsetX();
  coord_t y = dc->getOffsetY();
  coord_t xmin, xmax, ymin, ymax;
  dc->getClippingRect(xmin, xmax, ymin, ymax);
  for (auto child: children) {
    dc->setOffset(x + child->rect.x + child->scrollPositionX, y + child->rect.y + child->scrollPositionY);
    dc->setClippingRect(max(xmin, x + child->rect.x),
                        min(xmax, x + child->rect.x + child->rect.w),
                        max(ymin, y + child->rect.y),
                        min(ymax, y + child->rect.y + child->rect.h));
    child->fullPaint(dc);
  }
}

bool Window::onTouch(coord_t x, coord_t y)
{
  for (auto child: children) {
    if (pointInRect(x, y, child->rect)) {
      if (child->onTouch(x - child->rect.x - child->scrollPositionX, y - child->rect.y - child->scrollPositionY)) {
        return true;
      }
    }
  }

  return false;
}

bool Window::onSlide(coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  if (slideY && innerHeight > rect.h) {
    scrollPositionY = limit<coord_t>(-innerHeight + rect.h, scrollPositionY + slideY, 0);
    return true;
  }
  else if (slideX && innerWidth > rect.w) {
    scrollPositionX = limit<coord_t>(-innerWidth + rect.w, scrollPositionX + slideX, 0);
    return true;
  }
  else {
    for (auto child: children) {
      if (pointInRect(startX, startY, child->rect)) {
        if (child->onSlide(startX - child->rect.x, startY - child->rect.y, slideX, slideY)) {
          return true;
        }
      }
    }
    return false;
  }
}

coord_t Window::adjustHeight()
{
  coord_t old = rect.h;
  rect.h = 0;
  for (auto child: children) {
    rect.h = max(rect.h, child->rect.y + child->rect.h);
  }
  return rect.h - old;
}

void Window::moveWindowsTop(coord_t y, coord_t delta)
{
  for (auto child: children) {
    if (child->rect.y > y) {
      child->rect.y += delta;
    }
  }
}

void Window::drawVerticalScrollbar(BitmapBuffer * dc)
{
  if (innerHeight > rect.h) {
    coord_t x = rect.w - 3;
    coord_t y = -scrollPositionY + 3;
    coord_t h = rect.h - 6;
    lcd->drawSolidFilledRect(x, y, 1, h, LINE_COLOR);
    coord_t yofs = (-h*scrollPositionY + innerHeight/2) / innerHeight;
    coord_t yhgt = (h*rect.h + innerHeight/2) / innerHeight;
    if (yhgt + yofs > h)
      yhgt = h - yofs;
    dc->drawSolidFilledRect(x-1, y + yofs, 3, yhgt, SCROLLBOX_COLOR);
  }
}
