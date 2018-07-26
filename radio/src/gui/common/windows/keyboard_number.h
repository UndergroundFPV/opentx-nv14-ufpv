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

#ifndef _KEYBOARD_NUMBER_H_
#define _KEYBOARD_NUMBER_H_

#include "window.h"
#include "lcd_types.h"

class NumberEdit;

class NumberKeyboard : public Window {
  friend class NumberEdit;

  public:
    NumberKeyboard(Window * parent);

    ~NumberKeyboard();

    void setField(NumberEdit * field);

    void disable();

    NumberEdit * getField()
    {
      return field;
    }

    virtual void paint(BitmapBuffer * dc) override;

    virtual bool onTouchEnd(coord_t x, coord_t y) override;

  protected:
    NumberEdit * field = nullptr;
};

extern NumberKeyboard * numberKeyboard; // is created in TabsGroup constructor

#endif // _KEYBOARD_NUMBER_H_