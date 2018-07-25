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

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "window.h"
#include "lcd_types.h"

class TextEdit;

class Keyboard : public Window {
  friend class TextEdit;

  public:
    Keyboard(Window * parent);

    ~Keyboard();

    void setField(TextEdit * field);

    void disable();

    TextEdit * getField()
    {
      return field;
    }

    coord_t getCursorPos()
    {
      return cursorPos;
    }

    void setCursorPos(coord_t x);

    void paint(BitmapBuffer * dc);

    bool onTouchEnd(coord_t x, coord_t y) override;

  protected:
    TextEdit * field = nullptr;
    coord_t cursorPos = 0;
    uint8_t cursorIndex = 0;
    const char * const * layout;
};

extern Keyboard * keyboard; // is created in TabsGroup constructor

#endif