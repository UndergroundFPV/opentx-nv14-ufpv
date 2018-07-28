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

#include <stdio.h>
#include "opentx.h"
#include "model_logical_switches.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

void putsEdgeDelayParam(coord_t x, coord_t y, LogicalSwitchData * ls)
{
  lcdDrawChar(x, y, '[');
  lcdDrawNumber(lcdNextPos+2, y, lswTimerValue(ls->v2), LEFT|PREC1);
  lcdDrawChar(lcdNextPos, y, ':');
  if (ls->v3 < 0)
    lcdDrawText(lcdNextPos+3, y, "<<");
  else if (ls->v3 == 0)
    lcdDrawText(lcdNextPos+3, y, "--");
  else
    lcdDrawNumber(lcdNextPos+3, y, lswTimerValue(ls->v2+ls->v3), LEFT|PREC1);
  lcdDrawChar(lcdNextPos, y, ']');
}

class LogicalSwitchEditWindow: public Page {
  public:
    LogicalSwitchEditWindow(uint8_t ls):
      Page(),
      ls(ls)
    {
      buildBody(&body);
      buildHeader(&header);
    }

    bool isActive() {
      return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH+ls);
    }

    virtual void checkEvents() override
    {
      if (active != isActive()) {
        headerSwitchName->setFlags(isActive() ? BOLD|WARNING_COLOR : MENU_TITLE_COLOR);
        active = !active;
      }
    }

  protected:
    uint8_t ls;
    bool active = false;
    Window * logicalSwitchOneWindow = nullptr;
    StaticText * headerSwitchName = nullptr;
    NumberEdit * v2Edit = nullptr;

    void updateLogicalSwitchOneWindow()
    {
      GridLayout grid(*logicalSwitchOneWindow);
      logicalSwitchOneWindow->clear();

      LogicalSwitchData * cs = lswAddress(ls);
      uint8_t cstate = lswFamily(cs->func);

      if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_V1);
        auto choice = new SwitchChoice(logicalSwitchOneWindow, grid.getFieldSlot(), SWSRC_FIRST_IN_LOGICAL_SWITCHES, SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v1));
        choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
        grid.nextLine();

        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_V2);
        choice = new SwitchChoice(logicalSwitchOneWindow, grid.getFieldSlot(), SWSRC_FIRST_IN_LOGICAL_SWITCHES, SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v2));
        choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
        grid.nextLine();
      }
      else if (cstate == LS_FAMILY_EDGE) {
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_V1);
        auto choice = new SwitchChoice(logicalSwitchOneWindow, grid.getFieldSlot(), SWSRC_FIRST_IN_LOGICAL_SWITCHES, SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v1));
        choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
        grid.nextLine();

        auto edit2 = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(2, 1), -1, 222 - cs->v2, GET_SET_DEFAULT(cs->v3));
        auto edit1 = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(2, 0), -129, 122,
                                    GET_DEFAULT(cs->v2),
                                    [=](int32_t newValue) -> void {
                                      cs->v2 = newValue;
                                      cs->v3 = min<uint8_t>(cs->v3, 222 - cs->v2);
                                      SET_DIRTY();
                                      edit2->setMax(222 - cs->v2);
                                      edit2->invalidate();
                                    });
        edit1->setDisplayFunction([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
          drawNumber(dc, 2, 2, lswTimerValue(value), flags | PREC1);
        });
        edit2->setDisplayFunction([=](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
          if (value < 0)
            dc->drawText(2, 2, "<<", flags);
          else if (value == 0)
            dc->drawText(2, 2, "--", flags);
          else
            drawNumber(dc, 2, 2, lswTimerValue(cs->v2 + value), flags | PREC1);
        });
        grid.nextLine();
      }
      else if (cstate == LS_FAMILY_COMP) {
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_V1);
        new SourceChoice(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MIXSRC_LAST_TELEM, GET_SET_DEFAULT(cs->v1));
        grid.nextLine();

        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_V2);
        new SourceChoice(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MIXSRC_LAST_TELEM, GET_SET_DEFAULT(cs->v2));
        grid.nextLine();
      }
      else if (cstate == LS_FAMILY_TIMER) {
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_V1);
        auto timer = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), -128, 122, GET_SET_DEFAULT(cs->v1));
        timer->setDisplayFunction([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
            drawNumber(dc, 2, 2, lswTimerValue(value), flags | PREC1);
        });
        grid.nextLine();

        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_V2);
        timer = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), -128, 122, GET_SET_DEFAULT(cs->v2));
        timer->setDisplayFunction([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
            drawNumber(dc, 2, 2, lswTimerValue(value), flags | PREC1);
        });
        grid.nextLine();
      }
      else {
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_V1);
        new SourceChoice(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MIXSRC_LAST_TELEM, GET_DEFAULT(cs->v1),
                         [=](int32_t newValue) -> void {
                           cs->v1 = newValue;
                           SET_DIRTY();
                           v2Edit->invalidate();
                         });
        grid.nextLine();

        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_V2);
        int16_t v2_min = 0, v2_max = 0;
        getMixSrcRange(cs->v1, v2_min, v2_max);
        v2Edit = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MAX_LS_DELAY, GET_SET_DEFAULT(cs->v2));
        v2Edit->setDisplayFunction([=](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
          drawSourceCustomValue(2, 2, cs->v1, (cs->v1 <= MIXSRC_LAST_CH ? calc100toRESX(value) : value), flags);
        });
        grid.nextLine();
      }

      // AND switch
      new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_AND_SWITCH);
      auto choice = new SwitchChoice(logicalSwitchOneWindow, grid.getFieldSlot(), -MAX_LS_ANDSW, MAX_LS_ANDSW, GET_SET_DEFAULT(cs->andsw));
      choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
      grid.nextLine();

      // Duration
      new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_DURATION);
      auto edit = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MAX_LS_DURATION, GET_SET_DEFAULT(cs->duration), PREC1);
      edit->setZeroText("---");
      grid.nextLine();

      // Delay
      new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(true), STR_DELAY);
      if (cstate == LS_FAMILY_EDGE) {
        new StaticText(logicalSwitchOneWindow, grid.getFieldSlot(), STR_NA);
      }
      else {
        auto edit = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MAX_LS_DELAY, GET_SET_DEFAULT(cs->delay), PREC1);
        edit->setZeroText("---");
      }
      grid.nextLine();
    }

    void buildHeader(Window * window) {
      new StaticText(window, { 70, 4, LCD_W - 100, 20 }, STR_MENULOGICALSWITCHES, MENU_TITLE_COLOR);
      headerSwitchName = new StaticText(window, { 70, 28, LCD_W - 100, 20 }, getSwitchString(SWSRC_SW1+ls), MENU_TITLE_COLOR);
    }

    // MixerOne
    void buildBody(Window * window) {
      LogicalSwitchData * cs = lswAddress(ls);

      GridLayout grid(*window);
      grid.spacer(10);

      // LS Func
      new StaticText(window, grid.getLabelSlot(true), STR_FUNC);
      Choice * funcChoice = new Choice(window, grid.getFieldSlot(), STR_VCSWFUNC, 0, LS_FUNC_MAX,
                                       GET_DEFAULT(cs->func),
                                       [=](int32_t newValue) {
                                           cs->func = newValue;
                                           if (lswFamily(cs->func) == LS_FAMILY_TIMER) {
                                             cs->v1 = cs->v2 = 0;
                                           }
                                           else if (lswFamily(cs->func)  == LS_FAMILY_EDGE) {
                                             cs->v1 = 0; cs->v2 = -129; cs->v3 = 0;
                                           }
                                           else {
                                             cs->v1 = cs->v2 = 0;
                                           }
                                           SET_DIRTY();
                                           updateLogicalSwitchOneWindow();
                                       });
      funcChoice->setAvailableHandler(isLogicalSwitchFunctionAvailable);
      grid.nextLine();

      logicalSwitchOneWindow = new Window(window, { 0, grid.getWindowHeight(), LCD_W, 0 });
      updateLogicalSwitchOneWindow();
      grid.addWindow(logicalSwitchOneWindow);
    }
};

static constexpr coord_t line1 = 2;
static constexpr coord_t line2 = 22;
static constexpr coord_t line3 = 42;
static constexpr coord_t col1 = 20;
static constexpr coord_t col2 = (LCD_W - 100) / 3 + col1;
static constexpr coord_t col3 = ((LCD_W - 100) / 3) * 2 + col1;

class LogicalSwitchButton : public Button {
  public:
    LogicalSwitchButton(Window * parent, const rect_t & rect, int lsIndex, std::function<uint8_t(void)> onPress):
      Button(parent, rect, onPress),
      lsIndex(lsIndex)
    {
      LogicalSwitchData * ls = lswAddress(lsIndex);
      if (ls->andsw != SWSRC_NONE || ls->duration != 0 || ls->delay != 0)
        setHeight(getHeight() + 20);
      if (lswFamily(ls->func) == LS_FAMILY_EDGE)
        setHeight(getHeight() + 20);
    }

    bool isActive() {
      return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH+lsIndex );
    }

    virtual void checkEvents() override
    {
      if (active != isActive()) {
        invalidate();
        active = !active;
      }
    }

    // LS box content
    virtual void paint(BitmapBuffer * dc) override {
      LogicalSwitchData * ls = lswAddress(lsIndex);
      uint8_t lsFamily = lswFamily(ls->func);

      if (active)
        dc->drawSolidFilledRect(1, 1, rect.w-2, rect.h-2, CURVE_AXIS_COLOR);

      // The bounding rect
      drawSolidRect(dc, 0, 0, rect.w, rect.h, 2, hasFocus() ? SCROLLBOX_COLOR : CURVE_AXIS_COLOR);

      // CSW func
      lcdDrawTextAtIndex(col1, line1, STR_VCSWFUNC, ls->func);

      // CSW params
      if (lsFamily == LS_FAMILY_BOOL || lsFamily == LS_FAMILY_STICKY) {
        drawSwitch(col2, line1, ls->v1);
        drawSwitch(col3, line1, ls->v2);
      }
      else if (lsFamily == LS_FAMILY_EDGE) {
        drawSwitch(col1, line2, ls->v1);
        putsEdgeDelayParam(col2, line2, ls);
      }
      else if (lsFamily == LS_FAMILY_COMP) {
        drawSource(col2, line1, ls->v1, 0);
        drawSource(col3, line1, ls->v2, 0);
      }
      else if (lsFamily == LS_FAMILY_TIMER) {
        lcdDrawNumber(col2, line1, lswTimerValue(ls->v1), LEFT|PREC1);
        lcdDrawNumber(col3, line1, lswTimerValue(ls->v2), LEFT|PREC1);
      }
      else {
        drawSource(col2, line1, ls->v1, 0);
        drawSourceCustomValue(col3, line1, ls->v1, (ls->v1 <= MIXSRC_LAST_CH ? calc100toRESX(ls->v2) : ls->v2), 0);
      }

      // AND switch
      drawSwitch(col1, (lsFamily == LS_FAMILY_EDGE) ? line3 : line2, ls->andsw, 0);

      // CSW duration
      if (ls->duration > 0) {
        drawNumber(dc, col2, (lsFamily == LS_FAMILY_EDGE) ? line3 : line2, ls->duration, PREC1 | LEFT);
      }

      // CSW delay
      if (lsFamily != LS_FAMILY_EDGE && ls->delay > 0) {
        drawNumber(dc, col3, (lsFamily == LS_FAMILY_EDGE) ? line3 : line2, ls->delay, PREC1 | LEFT);
      }
    }

  protected:
    uint8_t lsIndex;
    bool active = false;
};

ModelLogicalSwitchesPage::ModelLogicalSwitchesPage():
  PageTab(STR_MENULOGICALSWITCHES, ICON_MODEL_LOGICAL_SWITCHES)
{
}

void ModelLogicalSwitchesPage::rebuild(Window * window)
{
  coord_t scrollPosition = window->getScrollPositionY();
  window->clear();
  build(window);
  window->setScrollPositionY(scrollPosition);
}

void ModelLogicalSwitchesPage::editLogicalSwitch(Window * window, uint8_t lsIndex)
{
  Window * lsWindow = new LogicalSwitchEditWindow(lsIndex);
  lsWindow->setCloseHandler([=]() {
    rebuild(window);
  });
}

void ModelLogicalSwitchesPage::build(Window * window)
{
  GridLayout grid(*window);
  grid.spacer(8);
  grid.setLabelWidth(70);
  grid.setLabelPaddingRight(10);

  window->clear();

  for (int8_t i=0; i<MAX_OUTPUT_CHANNELS; i++) {
    new TextButton(window, grid.getLabelSlot(), getSwitchString(SWSRC_SW1+i),
                   [=]() -> uint8_t {
                     return 0;
                   });

    Button * button = new LogicalSwitchButton(window, grid.getFieldSlot(), i,
                                              [=]() -> uint8_t {
                                                Menu * menu = new Menu();
                                                LogicalSwitchData * cs = lswAddress(i);
                                                menu->addLine(STR_EDIT, [=]() {
                                                  menu->deleteLater();
                                                  editLogicalSwitch(window, i);
                                                });
                                                if (cs->func)
                                                  menu->addLine(STR_COPY, [=]() {
                                                    menu->deleteLater();
                                                    clipboard.type = CLIPBOARD_TYPE_CUSTOM_SWITCH;
                                                    clipboard.data.csw = *cs;
                                                  });
                                                if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_SWITCH)
                                                  menu->addLine(STR_PASTE, [=]() {
                                                    menu->deleteLater();
                                                    *cs = clipboard.data.csw;
                                                    storageDirty(EE_MODEL);
                                                    rebuild(window);
                                                  });
                                                if (cs->func || cs->v1 || cs->v2 || cs->delay || cs->duration || cs->andsw)
                                                  menu->addLine(STR_CLEAR, [=]() {
                                                    menu->deleteLater();
                                                    memset(cs, 0, sizeof(LogicalSwitchData));
                                                    storageDirty(EE_MODEL);
                                                  });
                                                return 0;
                                              });
    grid.spacer(button->height() + 5);
  }

  window->setInnerHeight(grid.getWindowHeight());
}

enum LogicalSwitchFields {
  LS_FIELD_FUNCTION,
  LS_FIELD_V1,
  LS_FIELD_V2,
  LS_FIELD_V3,
  LS_FIELD_ANDSW,
  LS_FIELD_DURATION,
  LS_FIELD_DELAY,
  LS_FIELD_COUNT,
  LS_FIELD_LAST = LS_FIELD_COUNT-1
};

#define CSW_1ST_COLUMN  50
#define CSW_2ND_COLUMN  120
#define CSW_3RD_COLUMN  200
#define CSW_4TH_COLUMN  315
#define CSW_5TH_COLUMN  375
#define CSW_6TH_COLUMN  425

void onLogicalSwitchesMenu(const char * result)
{
  LogicalSwitchData * cs = lswAddress(menuVerticalPosition);

  if (result == STR_COPY) {
    clipboard.type = CLIPBOARD_TYPE_CUSTOM_SWITCH;
    clipboard.data.csw = *cs;
  }
  else if (result == STR_PASTE) {
    *cs = clipboard.data.csw;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_CLEAR) {
    memset(cs, 0, sizeof(LogicalSwitchData));
    storageDirty(EE_MODEL);
  }
}

bool menuModelLogicalSwitches(event_t event)
{
  INCDEC_DECLARE_VARS(EE_MODEL);

  MENU(STR_MENULOGICALSWITCHES, MODEL_ICONS, menuTabModel, MENU_MODEL_LOGICAL_SWITCHES, MAX_LOGICAL_SWITCHES, { NAVIGATION_LINE_BY_LINE|LS_FIELD_LAST/*repeated...*/} );

  LogicalSwitchData * cs = lswAddress(menuVerticalPosition);
  uint8_t cstate = lswFamily(cs->func);
  if ((cstate == LS_FAMILY_EDGE && menuHorizontalPosition == LS_FIELD_DELAY) ||
      (cstate != LS_FAMILY_EDGE && menuHorizontalPosition == LS_FIELD_V3)) {
    REPEAT_LAST_CURSOR_MOVE(LS_FIELD_LAST, false);
  }

#if defined(MENU_TOOLTIPS)
  if (menuVerticalPosition>= 0 && menuHorizontalPosition>=0) {
    drawColumnHeader(STR_LSW_HEADERS, STR_LSW_DESCRIPTIONS, menuHorizontalPosition);
  }
#endif

  if (menuHorizontalPosition<0 && event==EVT_KEY_LONG(KEY_ENTER) && !READ_ONLY()) {
    killEvents(event);
    if (cs->func)
      POPUP_MENU_ADD_ITEM(STR_COPY);
    if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_SWITCH)
      POPUP_MENU_ADD_ITEM(STR_PASTE);
    if (cs->func || cs->v1 || cs->v2 || cs->delay || cs->duration || cs->andsw)
      POPUP_MENU_ADD_ITEM(STR_CLEAR);
    POPUP_MENU_START(onLogicalSwitchesMenu);
  }

  for (uint8_t i=0; i<NUM_BODY_LINES; ++i) {
    coord_t y = MENU_CONTENT_TOP + i*FH;
    int k = i+menuVerticalOffset;
    LcdFlags attr = (menuVerticalPosition==k ? ((s_editMode>0) ? BLINK|INVERS : INVERS)  : 0);
    LcdFlags attr1 = (menuHorizontalPosition==1 ? attr : 0);
    LcdFlags attr2 = (menuHorizontalPosition==2 ? attr : 0);
    cs = lswAddress(k);

    // CSW name
    unsigned int sw = SWSRC_SW1+k;
    drawSwitch(MENUS_MARGIN_LEFT, y, sw, (getSwitch(sw) ? BOLD : 0) | ((menuVerticalPosition==k && CURSOR_ON_LINE()) ? INVERS : 0));

    // CSW func
    lcdDrawTextAtIndex(CSW_1ST_COLUMN, y, STR_VCSWFUNC, cs->func, menuHorizontalPosition==0 ? attr : 0);

    // CSW params
    cstate = lswFamily(cs->func);
    int v1_val = cs->v1;
    int16_t v1_min = 0, v1_max = MIXSRC_LAST_TELEM;
    int16_t v2_min = 0, v2_max = MIXSRC_LAST_TELEM;
    int16_t v3_min =-1, v3_max = 100;

    if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
      drawSwitch(CSW_2ND_COLUMN, y, cs->v1, attr1);
      drawSwitch(CSW_3RD_COLUMN, y, cs->v2, attr2);
      v1_min = SWSRC_FIRST_IN_LOGICAL_SWITCHES; v1_max = SWSRC_LAST_IN_LOGICAL_SWITCHES;
      v2_min = SWSRC_FIRST_IN_LOGICAL_SWITCHES; v2_max = SWSRC_LAST_IN_LOGICAL_SWITCHES;
      INCDEC_SET_FLAG(EE_MODEL | INCDEC_SWITCH);
      INCDEC_ENABLE_CHECK(isSwitchAvailableInLogicalSwitches);
    }
    else if (cstate == LS_FAMILY_EDGE) {
      drawSwitch(CSW_2ND_COLUMN, y, cs->v1, attr1);
      // putsEdgeDelayParam(CSW_3RD_COLUMN, y, cs, attr2, (menuHorizontalPosition==LS_FIELD_V3 ? attr : 0));
      v1_min = SWSRC_FIRST_IN_LOGICAL_SWITCHES; v1_max = SWSRC_LAST_IN_LOGICAL_SWITCHES;
      v2_min=-129; v2_max = 122;
      v3_max = 222 - cs->v2;
      if (menuHorizontalPosition == 1) {
        INCDEC_SET_FLAG(EE_MODEL | INCDEC_SWITCH);
        INCDEC_ENABLE_CHECK(isSwitchAvailableInLogicalSwitches);
      }
      else {
        INCDEC_SET_FLAG(EE_MODEL);
        INCDEC_ENABLE_CHECK(NULL);
      }
    }
    else if (cstate == LS_FAMILY_COMP) {
      v1_val = cs->v1;
      drawSource(CSW_2ND_COLUMN, y, v1_val, attr1);
      drawSource(CSW_3RD_COLUMN, y, cs->v2, attr2);
      INCDEC_SET_FLAG(EE_MODEL | INCDEC_SOURCE);
      INCDEC_ENABLE_CHECK(isSourceAvailable);
    }
    else if (cstate == LS_FAMILY_TIMER) {
      lcdDrawNumber(CSW_2ND_COLUMN, y, lswTimerValue(cs->v1), LEFT|PREC1|attr1);
      lcdDrawNumber(CSW_3RD_COLUMN, y, lswTimerValue(cs->v2), LEFT|PREC1|attr2);
      v1_min = v2_min = -128;
      v1_max = v2_max = 122;
      INCDEC_SET_FLAG(EE_MODEL);
      INCDEC_ENABLE_CHECK(NULL);
    }
    else {
      v1_val = cs->v1;
      drawSource(CSW_2ND_COLUMN, y, v1_val, attr1);
      if (menuHorizontalPosition == 1) {
        INCDEC_SET_FLAG(EE_MODEL | INCDEC_SOURCE);
        INCDEC_ENABLE_CHECK(isSourceAvailableInCustomSwitches);
      }
      else {
        INCDEC_SET_FLAG(EE_MODEL);
        INCDEC_ENABLE_CHECK(NULL);
      }
      LcdFlags lf = attr2 | LEFT;
      getMixSrcRange(v1_val, v2_min, v2_max, &lf);
      drawSourceCustomValue(CSW_3RD_COLUMN, y, v1_val, (v1_val <= MIXSRC_LAST_CH ? calc100toRESX(cs->v2) : cs->v2), lf);
    }

    // CSW AND switch
    drawSwitch(CSW_4TH_COLUMN, y, cs->andsw, menuHorizontalPosition==LS_FIELD_ANDSW ? attr : 0);

    // CSW duration
    if (cs->duration > 0)
      lcdDrawNumber(CSW_5TH_COLUMN, y, cs->duration, (menuHorizontalPosition==LS_FIELD_DURATION ? attr : 0)|PREC1|LEFT);
    else
      lcdDrawMMM(CSW_5TH_COLUMN, y, menuHorizontalPosition==LS_FIELD_DURATION ? attr : 0);

    // CSW delay
    if (cstate == LS_FAMILY_EDGE) {
      lcdDrawText(CSW_6TH_COLUMN, y, STR_NA);
    }
    else if (cs->delay > 0) {
      lcdDrawNumber(CSW_6TH_COLUMN, y, cs->delay, (menuHorizontalPosition==LS_FIELD_DELAY ? attr : 0)|PREC1|LEFT);
    }
    else {
      lcdDrawMMM(CSW_6TH_COLUMN, y, menuHorizontalPosition==LS_FIELD_DELAY ? attr : 0);
    }

    if (s_editMode>0 && attr) {
      switch (menuHorizontalPosition) {
        case LS_FIELD_FUNCTION:
        {
          cs->func = checkIncDec(event, cs->func, 0, LS_FUNC_MAX, EE_MODEL, isLogicalSwitchFunctionAvailable);
          uint8_t new_cstate = lswFamily(cs->func);
          if (cstate != new_cstate) {
            unsigned int save_func = cs->func;
            memset(cs, 0, sizeof(LogicalSwitchData));
            cs->func = save_func;
            if (new_cstate == LS_FAMILY_TIMER) {
              cs->v1 = cs->v2 = -119;
            }
            else if (new_cstate == LS_FAMILY_EDGE) {
              cs->v2 = -129;
            }
          }
          break;
        }
        case LS_FIELD_V1:
          cs->v1 = CHECK_INCDEC_PARAM(event, v1_val, v1_min, v1_max);
          break;
        case LS_FIELD_V2:
          cs->v2 = CHECK_INCDEC_PARAM(event, cs->v2, v2_min, v2_max);
          if (cstate==LS_FAMILY_OFS && cs->v1!=0 && event==EVT_KEY_LONG(KEY_ENTER)) {
            killEvents(event);
            getvalue_t x = getValue(v1_val);
            if (v1_val <= MIXSRC_LAST_CH)
              cs->v2 = calcRESXto100(x);
            storageDirty(EE_MODEL);
          }
          break;
        case LS_FIELD_V3:
          cs->v3 = CHECK_INCDEC_PARAM(event, cs->v3, v3_min, v3_max);
          break;
        case LS_FIELD_ANDSW:
          INCDEC_SET_FLAG(EE_MODEL | INCDEC_SWITCH);
          INCDEC_ENABLE_CHECK(isSwitchAvailableInLogicalSwitches);
          cs->andsw = CHECK_INCDEC_PARAM(event, cs->andsw, -MAX_LS_ANDSW, MAX_LS_ANDSW);
          break;
        case LS_FIELD_DURATION:
          CHECK_INCDEC_MODELVAR_ZERO(event, cs->duration, MAX_LS_DURATION);
          break;
        case LS_FIELD_DELAY:
          CHECK_INCDEC_MODELVAR_ZERO(event, cs->delay, MAX_LS_DELAY);
          break;
      }
    }
  }

  return true;
}
