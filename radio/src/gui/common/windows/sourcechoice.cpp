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

class SourceMenuHeaderButton: public Button {
  public:
    SourceMenuHeaderButton(Window * window, const rect_t & rect, char picto):
      Button(window, rect, nullptr, BUTTON_CHECKED_ON_FOCUS),
      picto(picto)
    {
    }

    void paint(BitmapBuffer * dc) override
    {
      if (checked()) {
        dc->drawSolidFilledRect(11, 0, 28, 28, HEADER_BGCOLOR);
        dc->drawSizedText(rect.w / 2, (rect.h - getFontHeight(flags)) / 2, &picto, 1, CENTERED | MENU_TITLE_COLOR);
      }
      else {
        dc->drawSizedText(rect.w / 2, (rect.h - getFontHeight(flags)) / 2, &picto, 1, CENTERED | TEXT_COLOR);
      }
    }

    bool onTouchEnd(coord_t x, coord_t y) override
    {
      if (hasFocus()) {
        check(false);
        clearFocus();
      }
      else {
        setFocus();
      }
      onPress();
      return true;
    }

  protected:
    char picto;
};

class SourceMenuHeader: public Window {
  public:
    SourceMenuHeader(SourceChoice * choice, Menu * menu):
      Window(menu, { 35, 95, 50, 370 })
    {
      coord_t y = 5;
      addButton(choice, menu, MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT, char('\314'), y);
#if defined(LUA_MODEL_SCRIPTS)
      addButton(choice, menu, MIXSRC_FIRST_LUA, MIXSRC_LAST_LUA, char('\322'), y);
#endif
      addButton(choice, menu, MIXSRC_FIRST_STICK, MIXSRC_LAST_STICK, char('\307'), y);
      addButton(choice, menu, MIXSRC_FIRST_POT, MIXSRC_LAST_POT, char('\310'), y);
      addButton(choice, menu, MIXSRC_MAX, MIXSRC_MAX, char('\315'), y);
#if defined(HELI)
      addButton(choice, menu, MIXSRC_FIRST_HELI, MIXSRC_LAST_HELI, char('\316'), y);
#endif
      addButton(choice, menu, MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM, char('\313'), y);
      addButton(choice, menu, MIXSRC_FIRST_SWITCH, MIXSRC_LAST_SWITCH, char('\312'), y);
      addButton(choice, menu, MIXSRC_FIRST_TRAINER, MIXSRC_LAST_TRAINER, char('\317'), y);
      addButton(choice, menu, MIXSRC_FIRST_CH, MIXSRC_LAST_CH, char('\320'), y);
      addButton(choice, menu, MIXSRC_FIRST_GVAR, MIXSRC_LAST_GVAR, char('\311'), y);
      addButton(choice, menu, MIXSRC_FIRST_TELEM, MIXSRC_LAST_TELEM, char('\321'), y);
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->clear(CURVE_AXIS_COLOR); // TODO only after the latest button
    }

    bool onTouchEnd(coord_t x, coord_t y) override
    {
      Window::onTouchEnd(x, y);
      return true; // = don't close the menu (inverted so that click outside the menu closes it)
    }

  protected:
    void addButton(SourceChoice * choice, Menu * menu, int16_t filtermin, int16_t filtermax, char picto, coord_t & y)
    {
      int vmin = choice->vmin;
      int vmax = choice->vmax;

      if (vmin > filtermin || vmax < filtermin)
        return;

      if (choice->isValueAvailable && getFirstAvailable(filtermin, filtermax, choice->isValueAvailable) == MIXSRC_NONE)
        return;

      auto button = new SourceMenuHeaderButton(this, {0, y, 50, 30}, picto);
      button->setPressHandler([=]() {
        if (button->hasFocus()) {
          choice->fillMenu(menu, [=](int16_t index) {
            return index >= filtermin && index <= filtermax;
          });
        }
        else {
          choice->fillMenu(menu);
        }
        return 1;
      });
      y += 30;
    }
};

void SourceChoice::paint(BitmapBuffer * dc)
{
  bool hasFocus = this->hasFocus();
  unsigned value = getValue();
  LcdFlags textColor = (value == 0 ? CURVE_AXIS_COLOR : 0);
  LcdFlags lineColor = CURVE_AXIS_COLOR;
  if (hasFocus) {
    textColor = TEXT_INVERTED_BGCOLOR;
    lineColor = TEXT_INVERTED_BGCOLOR;
  }
  drawSource(dc, 3, 2, value, textColor);
  drawSolidRect(dc, 0, 0, rect.w, rect.h, 1, lineColor);
}

void SourceChoice::fillMenu(Menu * menu, std::function<bool(int16_t)> filter)
{
  auto value = getValue();
  int count = 0;
  int current = -1;

  menu->removeLines();

  for (int i = vmin; i <= vmax; ++i) {
    if (filter && !filter(i))
      continue;
    if (isValueAvailable && !isValueAvailable(i))
      continue;
    menu->addLine(getSourceString(i), [=]() {
      setValue(i);
    });
    if (value == i) {
      current = count;
    }
    ++count;
  }

  if (current >= 0) {
    menu->select(current);
  }
}

bool SourceChoice::onTouchEnd(coord_t x, coord_t y)
{
  auto menu = new Menu();
  fillMenu(menu);

  menu->setNavigationBar(new SourceMenuHeader(this, menu));

  setFocus();
  return true;
}
