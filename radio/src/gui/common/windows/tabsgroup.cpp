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

TabsGroupHeader::TabsGroupHeader(TabsGroup * parent):
  Window(parent, { 0, 0, LCD_W, MENU_BODY_TOP }, OPAQUE),
  back(this, { 0, 0, TOPBAR_BUTTON_WIDTH, TOPBAR_BUTTON_WIDTH }, ICON_BACK,
       [=]() -> uint8_t {
         parent->deleteLater();
         return 1;
       }, BUTTON_NOFOCUS),
  carousel(this, parent)
{
}

void TabsGroupHeader::paint(BitmapBuffer * dc)
{
  dc->drawSolidFilledRect(0, MENU_HEADER_HEIGHT, LCD_W, MENU_TITLE_TOP - MENU_HEADER_HEIGHT, TEXT_BGCOLOR); // the white separation line
  dc->drawSolidFilledRect(0, MENU_TITLE_TOP, LCD_W, MENU_TITLE_HEIGHT, TITLE_BGCOLOR); // the title line background
  if (title) {
    lcdDrawText(MENUS_MARGIN_LEFT, MENU_TITLE_TOP, title, MENU_TITLE_COLOR);
  }
}

TabsGroupCarousel::TabsGroupCarousel(Window * parent, TabsGroup * menu):
  Window(parent, { TOPBAR_BUTTON_WIDTH, 0, LCD_W - TOPBAR_BUTTON_WIDTH, MENU_HEADER_HEIGHT }, OPAQUE),
  menu(menu)
{
}

void TabsGroupCarousel::updateInnerWidth()
{
  setInnerWidth(padding_left + TOPBAR_BUTTON_WIDTH * menu->tabs.size());
}

void TabsGroupCarousel::paint(BitmapBuffer * dc)
{
  dc->drawSolidFilledRect(0, 0, padding_left, TOPBAR_BUTTON_WIDTH, HEADER_BGCOLOR);
  for (unsigned i=0; i<menu->tabs.size(); i++) {
    dc->drawBitmap(padding_left + i*TOPBAR_BUTTON_WIDTH, 0, theme->getIconBitmap(menu->tabs[i]->icon, currentPage == i));
  }
  coord_t x = padding_left + TOPBAR_BUTTON_WIDTH * menu->tabs.size();
  coord_t w = width() - x;
  if (w > 0) {
    dc->drawSolidFilledRect(x, 0, w, TOPBAR_BUTTON_WIDTH, HEADER_BGCOLOR);
  }
}

bool TabsGroupCarousel::onTouchEnd(coord_t x, coord_t y)
{
  unsigned index = (x - padding_left) / TOPBAR_BUTTON_WIDTH;
  menu->setCurrentTab(index);
  currentPage = index;
  return true;
}

TabsGroup::TabsGroup():
  Window(&mainWindow, { 0, 0, LCD_W, LCD_H }, OPAQUE),
  header(this),
  body(this, { 0, MENU_BODY_TOP, LCD_W, MENU_BODY_HEIGHT })
{
}

void TabsGroup::addTab(PageTab * page)
{
  tabs.push_back(page);
  if (!currentTab) {
    setCurrentTab(page);
  }
  header.carousel.updateInnerWidth();
}

void TabsGroup::setCurrentTab(PageTab * tab)
{
  if (tab != currentTab) {
    body.clear();
    TextKeyboard::instance()->disable();
    NumberKeyboard::instance()->disable();
    currentTab = tab;
    tab->build(&body);
    header.setTitle(tab->title);
    invalidate();
  }
}

void TabsGroup::checkEvents()
{
  Window::checkEvents();
  if (currentTab)
    currentTab->checkEvents();
}

void TabsGroup::paint(BitmapBuffer * dc)
{
  dc->clear(TEXT_BGCOLOR);
}