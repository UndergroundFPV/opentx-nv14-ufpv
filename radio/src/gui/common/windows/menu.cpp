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

MenuHeaderWindow::MenuHeaderWindow(Menu * menu):
  Window(menu, { 0, 0, LCD_W, MENU_BODY_TOP }),
  back(this, { 0, 0, TOPBAR_BUTTON_WIDTH, TOPBAR_BUTTON_WIDTH }, ICON_BACK,
       [&]() -> uint8_t {
         // TODO pop menu
         return 1;
       }, 1),
  carousel(this, menu)
{
}

void MenuHeaderWindow::paint(BitmapBuffer * dc)
{
  dc->drawSolidFilledRect(0, MENU_HEADER_HEIGHT, LCD_W, MENU_TITLE_TOP - MENU_HEADER_HEIGHT, TEXT_BGCOLOR); // the white separation line
  dc->drawSolidFilledRect(0, MENU_TITLE_TOP, LCD_W, MENU_TITLE_HEIGHT, TITLE_BGCOLOR); // the title line background
  if (title) {
    lcdDrawText(MENUS_MARGIN_LEFT, MENU_TITLE_TOP, title, MENU_TITLE_COLOR);
  }
}

MenuPagesCarousel::MenuPagesCarousel(Window * parent, Menu * menu):
  Window(parent, { TOPBAR_BUTTON_WIDTH, 0, LCD_W - TOPBAR_BUTTON_WIDTH, MENU_HEADER_HEIGHT }),
  menu(menu)
{
}

void MenuPagesCarousel::updateInnerWidth()
{
  setInnerWidth(padding_left + TOPBAR_BUTTON_WIDTH * menu->pages.size());
}

void MenuPagesCarousel::paint(BitmapBuffer * dc)
{
  dc->drawSolidFilledRect(0, 0, padding_left, TOPBAR_BUTTON_WIDTH, HEADER_BGCOLOR);
  for (unsigned i=0; i<menu->pages.size(); i++) {
    dc->drawBitmap(padding_left + i*TOPBAR_BUTTON_WIDTH, 0, theme->getIconBitmap(menu->pages[i]->icon, currentPage == i));
  }
}

bool MenuPagesCarousel::onTouch(coord_t x, coord_t y)
{
  unsigned index = (x - padding_left) / TOPBAR_BUTTON_WIDTH;
  menu->setCurrentPage(index);
  currentPage = index;
  return true;
}
