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

#ifndef _STATIC_H_
#define _STATIC_H_

class StaticText : public Window {
  public:
    StaticText(Window * parent, const rect_t & rect, const char * text, LcdFlags flags = 0) :
      Window(parent, rect),
      flags(flags),
      text(strdup(text))
    {
    }

    ~StaticText()
    {
      free(text);
    }

    void paint(BitmapBuffer * dc)
    {
      dc->drawText(0, 3, text, flags);
    }

    LcdFlags flags;

  protected:
    char * text;
};

class StaticBitmap: public Window {
  public:
    StaticBitmap(Window * parent, const rect_t & rect, const char * filename):
      Window(parent, rect),
      bitmap(BitmapBuffer::load(filename))
    {
    }

    void paint(BitmapBuffer * dc)
    {
      dc->drawBitmap(0, 0, bitmap);
    }

    BitmapBuffer * bitmap;
};

#endif
