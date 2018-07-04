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

Keyboard::Keyboard() :
  Window(&mainWindow, {0, LCD_H - 270, LCD_W, 0})
{
}

bool Keyboard::onTouch(coord_t x, coord_t y)
{
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
        char c = idx2char(field[cursorIndex - 1]);
        memmove(field + cursorIndex - 1, field + cursorIndex, length - cursorIndex);
        field[length - 1] = '\0';
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
  if (c && zlen(field, length) < length) {
    memmove(field + cursorIndex + 1, field + cursorIndex, length - cursorIndex - 1);
    field[cursorIndex++] = char2idx(c);
    cursorPos += getCharWidth(c, fontspecsTable[0]);
  }
  return true;
}

void Keyboard::setCursorPos(coord_t x)
{
  coord_t rest = x;
  for (cursorIndex = 0; cursorIndex < length; cursorIndex++) {
    if (field[cursorIndex] == '\0')
      break;
    char c = field[cursorIndex];
    c = idx2char(c);
    uint8_t w = getCharWidth(c, fontspecsTable[0]);
    if (rest < w)
      break;
    rest -= w;
  }
  cursorPos = x - rest;
}

void Keyboard::setField(char * field, uint8_t length, Window * fieldWindow)
{
  this->field = field;
  this->length = length;
  this->setHeight(270);
  if (fieldWindow) {
    fieldWindow->setHeight(LCD_H - 270 - fieldWindow->rect.y);
    this->fieldWindow = fieldWindow;
  }
}

void Keyboard::disable()
{
  this->setHeight(0);
  this->field = nullptr;
  if (fieldWindow) {
    fieldWindow->setHeight(LCD_H - 0 - fieldWindow->rect.y);
    fieldWindow = nullptr;
  }
}

void Keyboard::paint(BitmapBuffer * dc)
{
  if (!keyboardBitmap)
    keyboardBitmap = BitmapBuffer::load(getThemePath("keyboard.png"));
  dc->drawBitmap(0, 0, keyboardBitmap);
}


Keyboard * keyboard;
