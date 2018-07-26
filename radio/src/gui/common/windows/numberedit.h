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

#ifndef _NUMBEREDIT_H_
#define _NUMBEREDIT_H_

#include "window.h"
#include "keyboard_number.h"

class NumberEdit : public Window {
  public:
    NumberEdit(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax,
               std::function<int32_t()> getValue, std::function<void(int32_t)> setValue, LcdFlags flags = 0);

    virtual void paint(BitmapBuffer * dc) override;

    void setMin(int32_t value) {
      vmin = value;
    }

    void setMax(int32_t value) {
      vmax = value;
    }

    void setDefault(int32_t value) {
      vdefault = value;
    }

    int32_t getMin() const {
      return vmin;
    }

    int32_t getMax() const {
      return vmax;
    }

    int32_t getDefault() const {
      return vdefault;
    }

    void setStep(int32_t step) {
      this->step = step;
    }

    void setPrefix(const char * prefix) {
      this->prefix = prefix;
    }

    void setSuffix(const char * suffix) {
      this->suffix = suffix;
    }

    void setZeroText(const char * text) {
      zeroText = text;
    }

    void setValue(int32_t value);

    int32_t getValue() const {
      return _getValue();
    }

    virtual void setDisplayFunction(std::function<void(BitmapBuffer *, LcdFlags, int32_t)> function) {
      displayFunction = function;
    }

    virtual bool onTouchEnd(coord_t x, coord_t y) override;

    virtual void onFocusLost() override
    {
      numberKeyboard->disable();
    }

  protected:
    int32_t vdefault = 0;
    int32_t vmin;
    int32_t vmax;
    int32_t step = 1;
    std::function<int32_t()> _getValue;
    std::function<void(int32_t)> _setValue;
    std::function<void(BitmapBuffer *, LcdFlags, int32_t)> displayFunction;
    LcdFlags flags;
    const char * prefix = nullptr;
    const char * suffix = nullptr;
    const char * zeroText = nullptr;
};

#endif // _NUMBEREDIT_H_
