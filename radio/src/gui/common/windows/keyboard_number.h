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

#include "keyboard_base.h"

class NumberEdit;

class NumberKeyboard : public Keyboard<NumberEdit> {
  friend class NumberEdit;

  public:
    NumberKeyboard();

    ~NumberKeyboard();

    static NumberKeyboard * instance() {
      if (!numberKeyboard)
        numberKeyboard = new NumberKeyboard();
      return numberKeyboard;
    }

    virtual void paint(BitmapBuffer * dc) override;

  protected:
    static NumberKeyboard * numberKeyboard;
    void drawButton(BitmapBuffer * dc, const char * text, rect_t & rect);
};

#endif // _KEYBOARD_NUMBER_H_