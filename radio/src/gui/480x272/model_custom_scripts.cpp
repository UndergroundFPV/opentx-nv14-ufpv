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
#include "model_custom_scripts.h"
#include "opentx.h"
#include "libwindows.h"
<<<<<<< HEAD

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

#include "model_logical_switches.h"
=======

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

#include "model_custom_scripts.h"
>>>>>>> b68a4a831599a463f7499c55f21e8d72cf49a871
#include "opentx.h"
#include "libwindows.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)
<<<<<<< HEAD

void putsEdgeDelayParamTemp(coord_t x, coord_t y, LogicalSwitchData * ls)
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

class CostomScriptsEditWindow: public Page {
  public:
    CostomScriptsEditWindow(uint8_t ls):
      Page(),
      ls(ls)
=======

class CostomScriptsEditWindow: public Page {
  public:
    CostomScriptsEditWindow(uint8_t cs):
      Page(),
      cs(cs)
>>>>>>> b68a4a831599a463f7499c55f21e8d72cf49a871
    {
      buildBody(&body);
      buildHeader(&header);
    }
<<<<<<< HEAD

    bool isActive() {
      return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH+ls);
    }

    void checkEvents() override
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
      GridLayout grid;
      logicalSwitchOneWindow->clear();

      LogicalSwitchData * cs = lswAddress(ls);
      uint8_t cstate = lswFamily(cs->func);

      if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_V1);
        auto choice = new SwitchChoice(logicalSwitchOneWindow, grid.getFieldSlot(), SWSRC_FIRST_IN_LOGICAL_SWITCHES, SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v1));
        choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
        grid.nextLine();

        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_V2);
        choice = new SwitchChoice(logicalSwitchOneWindow, grid.getFieldSlot(), SWSRC_FIRST_IN_LOGICAL_SWITCHES, SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v2));
        choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
        grid.nextLine();
      }
      else if (cstate == LS_FAMILY_EDGE) {
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_V1);
        auto choice = new SwitchChoice(logicalSwitchOneWindow, grid.getFieldSlot(), SWSRC_FIRST_IN_LOGICAL_SWITCHES, SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v1));
        choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
        grid.nextLine();

        auto edit1 = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(2, 0), -129, 122, GET_DEFAULT(cs->v2));
        auto edit2 = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(2, 1), -1, 222 - cs->v2, GET_SET_DEFAULT(cs->v3));
        edit1->setSetValueHandler([=](int32_t newValue) {
          cs->v2 = newValue;
          cs->v3 = min<uint8_t>(cs->v3, 222 - cs->v2);
          SET_DIRTY();
          edit2->setMax(222 - cs->v2);
          edit2->invalidate();
        });
        edit1->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
          drawNumber(dc, 2, 2, lswTimerValue(value), flags | PREC1);
        });
        edit2->setDisplayHandler([=](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
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
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_V1);
        new SourceChoice(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MIXSRC_LAST_TELEM, GET_SET_DEFAULT(cs->v1));
        grid.nextLine();

        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_V2);
        new SourceChoice(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MIXSRC_LAST_TELEM, GET_SET_DEFAULT(cs->v2));
        grid.nextLine();
      }
      else if (cstate == LS_FAMILY_TIMER) {
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_V1);
        auto timer = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), -128, 122, GET_SET_DEFAULT(cs->v1));
        timer->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
          drawNumber(dc, 2, 2, lswTimerValue(value), flags | PREC1);
        });
        grid.nextLine();

        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_V2);
        timer = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), -128, 122, GET_SET_DEFAULT(cs->v2));
        timer->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
          drawNumber(dc, 2, 2, lswTimerValue(value), flags | PREC1);
        });
        grid.nextLine();
      }
      else {
        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_V1);
        new SourceChoice(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MIXSRC_LAST_TELEM, GET_DEFAULT(cs->v1),
                         [=](int32_t newValue) {
                           cs->v1 = newValue;
                           SET_DIRTY();
                           v2Edit->invalidate();
                         });
        grid.nextLine();

        new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_V2);
        int16_t v2_min = 0, v2_max = 0;
        getMixSrcRange(cs->v1, v2_min, v2_max);
        v2Edit = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MAX_LS_DELAY, GET_SET_DEFAULT(cs->v2));
        v2Edit->setDisplayHandler([=](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
          drawSourceCustomValue(2, 2, cs->v1, (cs->v1 <= MIXSRC_LAST_CH ? calc100toRESX(value) : value), flags);
        });
        grid.nextLine();
      }

      // AND switch
      new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_AND_SWITCH);
      auto choice = new SwitchChoice(logicalSwitchOneWindow, grid.getFieldSlot(), -MAX_LS_ANDSW, MAX_LS_ANDSW, GET_SET_DEFAULT(cs->andsw));
      choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
      grid.nextLine();

      // Duration
      new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_DURATION);
      auto edit = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MAX_LS_DURATION, GET_SET_DEFAULT(cs->duration), PREC1);
      edit->setZeroText("---");
      grid.nextLine();

      // Delay
      new StaticText(logicalSwitchOneWindow, grid.getLabelSlot(), STR_DELAY);
      if (cstate == LS_FAMILY_EDGE) {
        new StaticText(logicalSwitchOneWindow, grid.getFieldSlot(), STR_NA);
      }
      else {
        auto edit = new NumberEdit(logicalSwitchOneWindow, grid.getFieldSlot(), 0, MAX_LS_DELAY, GET_SET_DEFAULT(cs->delay), PREC1);
        edit->setZeroText("---");
=======

    void checkEvents() override
    {
      invalidate();

      if (refreshDelay) {
        if (--refreshDelay == 0) {
              body.clear();
              buildBody(&body);
        }
>>>>>>> b68a4a831599a463f7499c55f21e8d72cf49a871
      }
      grid.nextLine();
    }

    void buildHeader(Window * window) {
      new StaticText(window, { 70, 4, LCD_W - 100, 20 }, STR_MENULOGICALSWITCHES, MENU_TITLE_COLOR);
      headerSwitchName = new StaticText(window, { 70, 28, LCD_W - 100, 20 }, getSwitchString(SWSRC_SW1+ls), MENU_TITLE_COLOR);
    }

    void buildBody(Window * window) {
      LogicalSwitchData * cs = lswAddress(ls);

      GridLayout grid;
      grid.spacer(10);

      // LS Func
      new StaticText(window, grid.getLabelSlot(), STR_FUNC);
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

class CustomScriptsButton : public Button {
  public:
    CustomScriptsButton(Window * parent, const rect_t & rect, int lsIndex, std::function<uint8_t(void)> onPress):
      Button(parent, rect, onPress),
      lsIndex(lsIndex),
      active(isActive())
    {
      LogicalSwitchData * ls = lswAddress(lsIndex);
      if (ls->andsw != SWSRC_NONE || ls->duration != 0 || ls->delay != 0)
        setHeight(getHeight() + 20);
      if (lswFamily(ls->func) == LS_FAMILY_EDGE)
        setHeight(getHeight() + 20);
    }

<<<<<<< HEAD
    bool isActive()
    {
      return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + lsIndex);
    }

    void checkEvents() override
    {
      if (active != isActive()) {
        invalidate();
        active = !active;
      }
=======
  protected:
    uint8_t cs;
    bool active = false;
    uint32_t  refreshDelay = 0;
    StaticText * headerLuaName = nullptr;


    void buildHeader(Window * window) {
      char str[20];
      new StaticText(window, { 70, 4, LCD_W - 100, 20 }, STR_MENUCUSTOMSCRIPTS, MENU_TITLE_COLOR);
      strAppendUnsigned(strAppend(str, "LUA"), cs + 1);
      headerLuaName = new StaticText(window, { 70, 28, LCD_W - 100, 20 }, str, MENU_TITLE_COLOR);
    }

    void buildBody(Window * window) {
      ScriptData *sd = &(g_model.scriptsData[cs]);

      GridLayout grid;
      grid.spacer(10);


      new StaticText(window, grid.getLabelSlot(), STR_SCRIPT);
      new FileChoice(window, grid.getFieldSlot(),
                     SCRIPTS_MIXES_PATH,
                     SCRIPTS_EXT,
                     sizeof(sd->file),
                     [=]() {
                       return std::string(sd->file, ZLEN(sd->file));
                     },
                     [=](std::string newValue) {
                       memset(sd->file, 0, sizeof(sd->file));
                       strncpy(sd->file, newValue.c_str(), sizeof(sd->file));
                       SET_DIRTY();
                       if (ZEXIST(sd->file)) {
                          memset(sd->inputs, 0, sizeof(sd->inputs));
                          storageDirty(EE_MODEL);
                          LUA_LOAD_MODEL_SCRIPT(cs);
                          refreshDelay = 4;
                       }
                     });
      grid.nextLine();

      new StaticText(window, grid.getLabelSlot(), TR_NAME);
      new TextEdit(window, grid.getFieldSlot(), sd->name, sizeof(sd->name));
      grid.nextLine();

      new StaticText(window, grid.getLabelSlot(), STR_INPUTS);
      grid.nextLine();

      for (int i = 0; i < scriptInputsOutputs[cs].inputsCount; i++) {
          new StaticText(window, grid.getLabelSlot(), scriptInputsOutputs[cs].inputs[i].name);
          if (scriptInputsOutputs[cs].inputs[i].type == INPUT_TYPE_VALUE) {
              new NumberEdit(window, grid.getFieldSlot(), scriptInputsOutputs[cs].inputs[i].min-scriptInputsOutputs[cs].inputs[i].def,
                                    scriptInputsOutputs[cs].inputs[i].max-scriptInputsOutputs[cs].inputs[i].def,
                                   [=]() -> int32_t {
                                     return g_model.scriptsData[cs].inputs[i].value + scriptInputsOutputs[cs].inputs[i].def;
                                   },
                                   [=](int32_t newValue) {
                                     g_model.scriptsData[cs].inputs[i].value = newValue;
                                   });
          }
          else {
              auto swicthchoice = new SourceChoice(window, grid.getFieldSlot(), 0, MIXSRC_LAST_TELEM, GET_SET_DEFAULT(g_model.scriptsData[cs].inputs[i].source));
          }
          grid.nextLine();
      }

      new StaticText(window, grid.getLineSlot(), "---------------------------------------------------");
      grid.nextLine();

      if (scriptInputsOutputs[cs].outputsCount > 0) {
          for (int i=0; i<scriptInputsOutputs[cs].outputsCount; i++) {
             char s[16];
             getSourceString(s, MIXSRC_FIRST_LUA + (cs * MAX_SCRIPT_OUTPUTS) + i);
             new StaticText(window, grid.getLabelSlot(), s);
             new NumberEdit(window, grid.getFieldSlot(), -127, 127, GET_VALUE(calcRESXto1000(scriptInputsOutputs[cs].outputs[i].value)),NULL, PREC1);
          }
      }
    }
};

static constexpr coord_t line1 = 2;
static constexpr coord_t line2 = 22;
static constexpr coord_t line3 = 42;
static constexpr coord_t col1 = 20;
static constexpr coord_t col2 = (LCD_W - 100) / 3 + col1;
static constexpr coord_t col3 = ((LCD_W - 100) / 3) * 2 + col1;

class CustomScriptsButton : public Button {
  public:
    CustomScriptsButton(Window * parent, const rect_t & rect, int csIndex, std::function<uint8_t(void)> onPress):
      Button(parent, rect, onPress),
      csIndex(csIndex)
    {
    }

    bool isActive()
    {
      return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + csIndex);
    }

    void checkEvents() override
    {
        invalidate();
>>>>>>> b68a4a831599a463f7499c55f21e8d72cf49a871
    }

    virtual void paint(BitmapBuffer * dc) override
    {
<<<<<<< HEAD
      LogicalSwitchData * ls = lswAddress(lsIndex);
      uint8_t lsFamily = lswFamily(ls->func);

      if (active)
        dc->drawSolidFilledRect(2, 2, rect.w-4, rect.h-4, WARNING_COLOR);

      // The bounding rect
      drawSolidRect(dc, 0, 0, rect.w, rect.h, 2, hasFocus() ? SCROLLBOX_COLOR : CURVE_AXIS_COLOR);

      // CSW func
      lcdDrawTextAtIndex(col1, line1, STR_VCSWFUNC, ls->func);
#if 1
      // CSW params
      if (lsFamily == LS_FAMILY_BOOL || lsFamily == LS_FAMILY_STICKY) {
        drawSwitch(col2, line1, ls->v1);
        drawSwitch(col3, line1, ls->v2);
      }
      else if (lsFamily == LS_FAMILY_EDGE) {
        drawSwitch(col1, line2, ls->v1);
        putsEdgeDelayParamTemp(col2, line2, ls);
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
        //drawSourceCustomValue(col3, line1, ls->v1, (ls->v1 <= MIXSRC_LAST_CH ? calc100toRESX(ls->v2) : ls->v2), 0);
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
#endif
    }

  protected:
    uint8_t lsIndex;
    bool active;
};

ModelCustomScriptsPage::ModelCustomScriptsPage():
  PageTab(STR_MENUCUSTOMSCRIPTS, ICON_MODEL_LUA_SCRIPTS)
{
}

=======
      LogicalSwitchData * cs = lswAddress(csIndex);
      uint8_t lsFamily = lswFamily(cs->func);
      ScriptData &sd = g_model.scriptsData[csIndex];

      if (ZEXIST(sd.file)) {
        lcdDrawSizedText(col1, line1, sd.file, sizeof(sd.file), 0);
        lcdDrawSizedText(col2, line1, sd.name, sizeof(sd.name), ZCHAR);

        switch (scriptInternalData[csIndex].state) {
          case SCRIPT_SYNTAX_ERROR:
            lcdDrawText(col3, line1, "(error)");
            break;
          case SCRIPT_KILLED:
            lcdDrawText(col3, line1, "(killed)");
            break;
          default:
            lcdDrawNumber(col3, line1, luaGetCpuUsed(csIndex), LEFT|TEXT_COLOR, 0, NULL, "%");
            break;
        }
      }
      else {
          lcdDrawTextAtIndex(col1, line1, STR_VCSWFUNC, 0, 0);
      }
    }

  protected:
    uint8_t csIndex;
};

ModelCustomScriptsPage::ModelCustomScriptsPage():
  PageTab(STR_MENUCUSTOMSCRIPTS, ICON_MODEL_LUA_SCRIPTS)
{
}

>>>>>>> b68a4a831599a463f7499c55f21e8d72cf49a871
void ModelCustomScriptsPage::rebuild(Window * window, int8_t focusIndex)
{
  coord_t scrollPosition = window->getScrollPositionY();
  window->clear();
  build(window, focusIndex);
  window->setScrollPositionY(scrollPosition);
}

<<<<<<< HEAD
void ModelCustomScriptsPage::editLogicalSwitch(Window * window, uint8_t lsIndex)
=======
void ModelCustomScriptsPage::editCustomScript(Window * window, uint8_t lsIndex)
>>>>>>> b68a4a831599a463f7499c55f21e8d72cf49a871
{
  Window * lsWindow = new CostomScriptsEditWindow(lsIndex);
  lsWindow->setCloseHandler([=]() {
    rebuild(window, lsIndex);
  });
}

#define SCRIPTS_COLUMN_FILE  70
#define SCRIPTS_COLUMN_NAME  160
#define SCRIPTS_COLUMN_STATE 300

void ModelCustomScriptsPage::build(Window * window, int8_t focusIndex)
{
<<<<<<< HEAD
#if 0

  GridLayout grid;
  grid.spacer(8);
  grid.setLabelWidth(80);

  Choice * failSafeChoice = nullptr;


  for (uint8_t i = 0; i < MAX_SCRIPTS; i++) {
    char str[20];

    strAppendUnsigned(strAppend(str, "LUA"), i+1);

    new StaticText(window, grid.getLabelSlot(), str);

    failSafeChoice = new Choice(window, grid.getFieldSlot(3, 0), STR_VFAILSAFE, 0, FAILSAFE_LAST,
                                GET_DEFAULT(g_model.moduleData[0].failsafeMode),
                                [=](int32_t newValue) {
                                });
    failSafeChoice->setAvailableHandler([=](int8_t newValue) {
        return true;
    });

    auto button = new TextButton(window, grid.getFieldSlot(3, 1), "Setting");

    new StaticText(window, grid.getFieldSlot(3, 2), "0%");


    /*
    button->setPressHandler([button, i] {

      return 0;
    });*/

    grid.nextLine();
#endif
=======
>>>>>>> b68a4a831599a463f7499c55f21e8d72cf49a871
    GridLayout grid;
    grid.spacer(8);
    grid.setLabelWidth(70);

    for (uint8_t i=0; i<MAX_SCRIPTS; i++) {
      char str[20];
      strAppendUnsigned(strAppend(str, "LUA"), i+1);
      new TextButton(window, grid.getLabelSlot(), str);

      Button * button = new CustomScriptsButton(window, grid.getFieldSlot(), i,
                                                [=]() -> uint8_t {
                                                  Menu * menu = new Menu();
<<<<<<< HEAD
                                                  LogicalSwitchData * ls = lswAddress(i);
                                                  menu->addLine(STR_EDIT, [=]() {
                                                    editLogicalSwitch(window, i);
                                                  });
                                                  if (ls->func)
                                                    menu->addLine(STR_COPY, [=]() {
                                                      clipboard.type = CLIPBOARD_TYPE_CUSTOM_SWITCH;
                                                      clipboard.data.csw = *ls;
                                                    });
                                                  if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_SWITCH)
                                                    menu->addLine(STR_PASTE, [=]() {
                                                      *ls = clipboard.data.csw;
                                                      storageDirty(EE_MODEL);
                                                      rebuild(window, i);
                                                    });
                                                  if (ls->func || ls->v1 || ls->v2 || ls->delay || ls->duration || ls->andsw)
                                                    menu->addLine(STR_CLEAR, [=]() {
                                                      memset(ls, 0, sizeof(LogicalSwitchData));
                                                      storageDirty(EE_MODEL);
                                                      rebuild(window, i);
                                                    });
=======
                                                  ScriptData *sd = &g_model.scriptsData[i];
                                                  menu->addLine(STR_EDIT, [=]() {
                                                    editCustomScript(window, i);
                                                  });
                                                  if (ZEXIST(sd->file))
                                                    menu->addLine(STR_COPY, [=]() {
                                                      clipboard.type = CLIPBOARD_TYPE_CUSTOM_SCRIPT;
                                                      clipboard.data.csd = *sd;
                                                    });
                                                  if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_SCRIPT)
                                                    menu->addLine(STR_PASTE, [=]() {
                                                      *sd = clipboard.data.csd;
                                                      storageDirty(EE_MODEL);
                                                      LUA_LOAD_MODEL_SCRIPT(i);
                                                      rebuild(window, i);
                                                    });
                                                  menu->addLine(STR_CLEAR, [=]() {
                                                      memset(sd, 0, sizeof(ScriptData));
                                                      storageDirty(EE_MODEL);
                                                      LUA_LOAD_MODEL_SCRIPTS();
                                                      rebuild(window, i);
                                                  });
>>>>>>> b68a4a831599a463f7499c55f21e8d72cf49a871
                                                  return 0;
                                                });
      if (focusIndex == i) {
        button->setFocus();
      }

<<<<<<< HEAD
      grid.spacer(button->height() + 5);
    }



    grid.nextLine();

=======
      grid.spacer(button->height() + 15);
    }

    grid.nextLine();

>>>>>>> b68a4a831599a463f7499c55f21e8d72cf49a871
    window->setInnerHeight(grid.getWindowHeight());
}
