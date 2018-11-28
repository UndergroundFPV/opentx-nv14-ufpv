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

#include "screen_lua.h"
#include "opentx.h"
#include "libwindows.h"

extern int Lua_screen_created;

class LuaBody : public Window {
  public:
    LuaBody(Window * parent, const rect_t &rect, uint8_t param) :
      Window(parent, rect),
      param(param)
    {
      build();
    }

    void checkEvents() override
    {
      invalidate();
    }

    void build()
    {

    }

    void paint(BitmapBuffer * dc) override
    {

    }

  protected:
    uint8_t param;
};

class LuaPage : public PageTab {
  public:
    LuaPage(uint8_t param) :
      PageTab("Lua screen", ICON_MODEL_LUA_SCRIPTS),
      param(param)
    {
    }

    void build(Window * window) override
    {
      new LuaBody(window, {0, 0, LCD_W, window->height() - footerHeight}, param);
    }

  protected:
    static constexpr coord_t footerHeight = 30;
    uint8_t param;
};

ScreenLua::ScreenLua(uint8_t param) :
  TabsGroup()
{
  addTab(new LuaPage(param));
}

extern int Lua_screen_exit;
extern int Lua_screen_created;

ScreenLua::~ScreenLua()
{
  Lua_screen_created = 0;
  Lua_screen_exit = 1;
  standaloneScript.state = SCRIPT_NOFILE;
  luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
}
