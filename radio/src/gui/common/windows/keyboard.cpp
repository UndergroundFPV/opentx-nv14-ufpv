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

#include "opentx.h"

Keyboard::Keyboard(Window * parent) :
  Window(parent, {0, parent->height() - 270, parent->width(), 0})
{
  keyboard = this;
}

Keyboard::~Keyboard()
{
  keyboard = nullptr;
}

bool Keyboard::onTouchEnd(coord_t x, coord_t y)
{
  if (!field)
    return false;

  uint8_t size = field->getMaxLength();
  char * data = field->getData();

  uint8_t row = y / 54;
  char c = 0;
  if (row == 0) {
    uint8_t column = (max<uint8_t>(6, x) - 6) / 31;
    c = *("1234567890" + column);
  }
  else if (row == 1) {
    uint8_t column = (max<uint8_t>(6, x) - 6) / 31;
    c = *("qwertyuiop" + column);
  }
  else if (row == 2) {
    uint8_t column = (max<uint8_t>(20, x) - 20) / 31;
    c = *("asdfghjkl" + column);
  }
  else if (row == 3) {
    if (x > 270) {
      if (cursorIndex > 0) {
        // backspace
        char c = idx2char(data[cursorIndex - 1]);
        memmove(data + cursorIndex - 1, data + cursorIndex, size - cursorIndex);
        data[size - 1] = '\0';
        cursorPos -= getCharWidth(c, fontspecsTable[0]);
        cursorIndex -= 1;
      }
    }
    else {
      uint8_t column = (max<uint8_t>(52, x) - 52) / 31;
      c = *("zxcvbnm" + column);
    }
  }
  else if (row == 4) {
    if (x > 240) {
      // enter
      disable();
      return true;
    }
    else if (x > 207) {
      c = ',';
    }
    else if (x > 38) {
      c = ' ';
    }
    else {
      c = '.';
    }
  }
  if (c && zlen(data, size) < size) {
    memmove(data + cursorIndex + 1, data + cursorIndex, size - cursorIndex - 1);
    data[cursorIndex++] = char2idx(c);
    cursorPos += getCharWidth(c, fontspecsTable[0]);
  }

  field->invalidate();
  return true;
}

void Keyboard::setCursorPos(coord_t x)
{
  if (!field)
    return;

  uint8_t size = field->getMaxLength();
  char * data = field->getData();
  coord_t rest = x;
  for (cursorIndex = 0; cursorIndex < size; cursorIndex++) {
    if (data[cursorIndex] == '\0')
      break;
    char c = data[cursorIndex];
    c = idx2char(c);
    uint8_t w = getCharWidth(c, fontspecsTable[0]);
    if (rest < w)
      break;
    rest -= w;
  }
  cursorPos = x - rest;
  field->invalidate();
}

void Keyboard::setField(TextEdit * field)
{
  this->field = field;
  this->setHeight(270);
  Window * w = field->getParent();
  w->setHeight(LCD_H - 270 - w->top());
  invalidate();
}

void Keyboard::disable()
{
  this->setHeight(0);
  if (field) {
    Window * w = field->getParent();
    w->setHeight(LCD_H - 0 - w->top());
    field = nullptr;
  }
}

void Keyboard::paint(BitmapBuffer * dc)
{
  if (!keyboardBitmap)
    keyboardBitmap = BitmapBuffer::load(getThemePath("keyboard.png"));
  dc->drawBitmap(0, 0, keyboardBitmap);

  dc->drawSolidFilledRect(0, 0, LCD_W, 30, CURVE_AXIS_COLOR);
  for (int i=0; i<10; i++) {
    coord_t w = (320 - 2 * 6 - 9 * 2) / 10;
    dc->drawSolidFilledRect(6 + i * (w + 2), 6, w, 30, TEXT_BGCOLOR);
    drawSolidRect(dc, 6 + i * (w + 2), 6, w, 30, TEXT_COLOR);

  }

}
