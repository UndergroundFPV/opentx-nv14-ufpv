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

constexpr coord_t KEYBOARD_HEIGHT = 90;

NumberKeyboard * numberKeyboard = nullptr;

NumberKeyboard::NumberKeyboard(Window * parent) :
  Window(parent, {0, parent->height() - KEYBOARD_HEIGHT, parent->width(), 0})
{
  numberKeyboard = this;

  new TextButton(this, { LCD_W/2 - 115, 10, 50, 30 }, "<<",
                 [=]() -> uint8_t {
                   if (field) {
                     field->setValue(field->getValue() - 10);
                   }
                   return 0;
                 }, 0);

  new TextButton(this, { LCD_W/2 - 55, 10, 50, 30 }, "-",
                 [=]() -> uint8_t {
                   if (field) {
                     field->setValue(field->getValue() - 1);
                   }
                   return 0;
                 }, 0);

  new TextButton(this, { LCD_W/2 + 5, 10, 50, 30 }, "+",
                 [=]() -> uint8_t {
                   if (field) {
                     field->setValue(field->getValue() + 1);
                   }
                   return 0;
                 }, 0);

  new TextButton(this, { LCD_W/2 + 65, 10, 50, 30 }, ">>",
                 [=]() -> uint8_t {
                   if (field) {
                     field->setValue(field->getValue() + 10);
                   }
                   return 0;
                 }, 0);

  new TextButton(this, { LCD_W/2 - 115, 50, 50, 30 }, "MIN",
                 [=]() -> uint8_t {
                   if (field) {
                     field->setValue(field->getMin());
                   }
                   return 0;
                 }, 0);

  new TextButton(this, { LCD_W/2 + 65, 50, 50, 30 }, "MAX",
                 [=]() -> uint8_t {
                   if (field) {
                     field->setValue(field->getMax());
                   }
                   return 0;
                 }, 0);

  new TextButton(this, { LCD_W/2 - 55, 50, 110, 30 }, "DEFAULT",
                 [=]() -> uint8_t {
                   if (field) {
                     field->setValue(field->getDefault());
                   }
                   return 0;
                 }, 0);
}

NumberKeyboard::~NumberKeyboard()
{
  numberKeyboard = nullptr;
}

// TODO parent class
void NumberKeyboard::setField(NumberEdit * field)
{
  this->field = field;
  this->setHeight(KEYBOARD_HEIGHT);
  Window * w = field->getParent();
  w->setHeight(LCD_H - KEYBOARD_HEIGHT - w->top());
  invalidate();
}

// TODO parent class
void NumberKeyboard::disable()
{
  this->setHeight(0);
  if (field) {
    Window * w = field->getParent();
    w->setHeight(LCD_H - 0 - w->top());
    field = nullptr;
  }
}

void NumberKeyboard::paint(BitmapBuffer * dc)
{
  lcdSetColor(RGB(0xE0, 0xE0, 0xE0));
  dc->drawSolidFilledRect(0, 0, LCD_W, height(), CUSTOM_COLOR);
}
