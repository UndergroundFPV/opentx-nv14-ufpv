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

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <functional>
#include "window.h"

enum ButtonFlags {
  BUTTON_BACKGROUND = 1,
  BUTTON_CHECKED = 2,
  BUTTON_NOFOCUS = 4,
  BUTTON_CHECKED_ON_FOCUS = 8,
  BUTTON_DISABLED = 16,
};


class Button : public Window {
  public:
    Button(Window * parent, const rect_t & rect, std::function<uint8_t(void)> onPress, uint8_t flags=0):
      Window(parent, rect),
      onPress(onPress),
      flags(flags)
    {
    }

    void enable(bool enabled) {
      if (enabled)
        flags &= ~BUTTON_DISABLED;
      else
        flags |= BUTTON_DISABLED;
    }

    void disable() {
      enable(false);
    }

    void check(bool checked) {
      if (checked)
        flags |= BUTTON_CHECKED;
      else
        flags &= ~BUTTON_CHECKED;
    }

    bool enabled() {
      return !(flags & BUTTON_DISABLED);
    }

    bool checked() {
      if (flags & BUTTON_CHECKED_ON_FOCUS)
        return hasFocus();
      else
        return flags & BUTTON_CHECKED;
    }

    virtual bool onTouchEnd(coord_t x, coord_t y) override;

  protected:
    std::function<uint8_t(void)> onPress;
    uint8_t flags;
};

class TextButton : public Button {
  public:
    TextButton(Window * parent, const rect_t & rect, const char * text, std::function<uint8_t(void)> onPress, uint8_t flags=BUTTON_BACKGROUND):
      Button(parent, rect, onPress, flags),
      text(strdup(text))
    {
    }

    virtual ~TextButton()
    {
      free(text);
    }

    void setText(const char * text)
    {
      free(this->text);
      this->text = strdup(text);
    }

    virtual void paint(BitmapBuffer * dc) override;

  protected:
    char * text;
};

class IconButton: public Button {
  public:
    IconButton(Window * parent, const rect_t & rect, uint8_t icon, std::function<uint8_t(void)> onPress, uint8_t flags=0):
      Button(parent, rect, onPress, flags),
      icon(icon)
    {
    }

    virtual void paint(BitmapBuffer * dc) override;

  protected:
    uint8_t icon;
};

class FabIconButton: public Button {
  public:
    FabIconButton(Window * parent, coord_t x, coord_t y, uint8_t icon, std::function<uint8_t(void)> onPress, uint8_t flags=0);

    virtual void paint(BitmapBuffer * dc) override;

  protected:
    uint8_t icon;
};

#endif
