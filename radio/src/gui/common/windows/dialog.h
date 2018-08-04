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

#ifndef _DIALOG_H_
#define _DIALOG_H_

#define WARNING_TYPE_ALERT     0
#define WARNING_TYPE_ASTERISK  1
#define WARNING_TYPE_CONFIRM   2
#define WARNING_TYPE_INPUT     3
#define WARNING_TYPE_INFO      4

#include "button.h"

class Dialog : public Window {
  public:
    Dialog(uint8_t type, std::string title, std::string message="", std::function<void(void)> onConfirm=nullptr);

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "Dialog";
    }
#endif

    ~Dialog()
    {
      deleteChildren();
    }

    void paint(BitmapBuffer * dc) override;

    bool onTouchEnd(coord_t x, coord_t y) override;

  protected:
    uint8_t type;
    std::string title;
    std::string message;
};

#endif // _CONFIRMATION_H_
