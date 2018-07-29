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
#include "model_special_functions.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

class SpecialFunctionEditWindow : public Page {
public:
    SpecialFunctionEditWindow(int8_t index) :
            Page(),
            index(index)
    {
      buildBody(&body);
      buildHeader(&header);
    }

protected:
    uint8_t index;
    Window * specialFunctionOneWindow = nullptr;

    void buildHeader(Window * window)
    {
      char s[5]="SF";
      strAppendUnsigned(&s[2], index);
      new StaticText(window, {70, 4, 200, 20}, STR_MENUSPECIALFUNCS, MENU_TITLE_COLOR);
      new StaticText(window, {70, 28, 100, 20}, s, MENU_TITLE_COLOR);
    }

    void updateSpecialFunctionOneWindow()
    {
      //SF.one variable part
      GridLayout grid(*specialFunctionOneWindow);
      specialFunctionOneWindow->clear();

      CustomFunctionData * cfn = &g_model.customFn[index];
      uint8_t func = CFN_FUNC(cfn);

      // Func param
      switch(func) {
        case FUNC_OVERRIDE_CHANNEL:
          new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_CH);
          new SourceChoice(specialFunctionOneWindow, grid.getFieldSlot(), 0, MAX_OUTPUT_CHANNELS-1, GET_SET_DEFAULT(CFN_CH_INDEX(cfn)));
          grid.nextLine();

          new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VALUE);
          new NumberEdit(specialFunctionOneWindow, grid.getFieldSlot(), -100, 100, GET_SET_DEFAULT(CFN_PARAM(cfn)));
          grid.nextLine();
      }

      if (HAS_ENABLE_PARAM(func)) {
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_ENABLE);
        new CheckBox(specialFunctionOneWindow, grid.getFieldSlot(), GET_SET_DEFAULT(CFN_ACTIVE(cfn)));
        grid.nextLine();
      }
      else if (HAS_REPEAT_PARAM(func)) { // !1x 1x 1s 2s 3s ...
        // TODO
      }
    }

    void buildBody(Window * window)
    {
      //SF.one
      GridLayout grid(*window);
      grid.spacer(8);

      CustomFunctionData * cfn = &g_model.customFn[index];
      uint8_t func = CFN_FUNC(cfn);

      // SWitch
      new StaticText(window, grid.getLabelSlot(), STR_SWITCH);
      auto swicthchoice = new SwitchChoice(window, grid.getFieldSlot(), SWSRC_FIRST, SWSRC_LAST, GET_SET_DEFAULT(CFN_SWITCH(cfn)));
      swicthchoice->setAvailableHandler(isSwitchAvailableInCustomFunctions);
      grid.nextLine();

      // Function
      new StaticText(window, grid.getLabelSlot(), STR_FUNC);
      auto choice = new Choice(window, grid.getFieldSlot(), STR_VFSWFUNC, 0, FUNC_MAX-1, GET_DEFAULT(CFN_FUNC(cfn)),
                               [=](int32_t newValue) {
                                   CFN_FUNC(cfn) = newValue;
                                   SET_DIRTY();
                                   updateSpecialFunctionOneWindow();
                               });
      choice->setAvailableHandler(isAssignableFunctionAvailable);
      grid.nextLine();

      specialFunctionOneWindow = new Window(window, { 0, grid.getWindowHeight(), LCD_W, 0 });
      updateSpecialFunctionOneWindow();
      grid.addWindow(specialFunctionOneWindow);
    }
};

static constexpr coord_t line1 = 0;
static constexpr coord_t line2 = 22;
static constexpr coord_t col1 = 20;
static constexpr coord_t col2 = (LCD_W - 100) / 3 + col1;
static constexpr coord_t col3 = ((LCD_W - 100) / 3) * 2 + col1;

class SpecialFunctionButton : public Button {
public:
    SpecialFunctionButton(Window * parent, const rect_t &rect, uint8_t sfIndex) :
            Button(parent, rect),
            sfIndex(sfIndex)
    {
      const CustomFunctionData * sf = &g_model.customFn[sfIndex];
      uint8_t func = CFN_FUNC(sf);
      if (func < FUNC_FIRST_WITHOUT_ENABLE) {
        setHeight(getHeight() + 20);
      }
    }

    bool isActive()
    {
      return 0;
    }

    virtual void checkEvents() override
    {
      if (active != isActive()) {
        invalidate();
        active = !active;
      }
    }

    void paintSpecialFunctionLine(BitmapBuffer * dc)
    {
      //SF.all
      const CustomFunctionData * sf = &g_model.customFn[sfIndex];
      uint8_t func = CFN_FUNC(sf);

      drawSwitch(col1, line1, CFN_SWITCH(sf), ((modelFunctionsContext.activeSwitches & ((MASK_CFN_TYPE)1 << sfIndex)) ? BOLD : 0));
      lcdDrawTextAtIndex(col2, line1, STR_VFSWFUNC, func, 0);
      int16_t val_min = 0;
      int16_t val_max = 255;

      switch(func) {
        case FUNC_OVERRIDE_CHANNEL:
          putsChn(col1, line2, CFN_CH_INDEX(sf)+1, 0);
          getMixSrcRange(MIXSRC_FIRST_CH, val_min, val_max);
          lcdDrawNumber(col2, line2, CFN_PARAM(sf), 0);
          drawCheckBox(col3, line2, CFN_ACTIVE(sf), 0);
          break;
      }
    }

    virtual void paint(BitmapBuffer * dc) override
    {
      if (active)
        dc->drawSolidFilledRect(2, 2, rect.w - 4, rect.h - 4, WARNING_COLOR);
      paintSpecialFunctionLine(dc);
      drawSolidRect(dc, 0, 0, rect.w, rect.h, 2, hasFocus() ? SCROLLBOX_COLOR : CURVE_AXIS_COLOR);
    }

protected:
    uint8_t sfIndex;
    bool active = false;
};

ModelSpecialFunctionsPage::ModelSpecialFunctionsPage() :
        PageTab(STR_MENUSPECIALFUNCS, ICON_MODEL_SPECIAL_FUNCTIONS)
{
}

void ModelSpecialFunctionsPage::rebuild(Window * window, int8_t focusSpecialFunctionIndex)
{
  coord_t scrollPosition = window->getScrollPositionY();
  window->clear();
  build(window, focusSpecialFunctionIndex);
  window->setScrollPositionY(scrollPosition);
}

void ModelSpecialFunctionsPage::editSpecialFunction(Window * window, uint8_t index)
{
  Window * editWindow = new SpecialFunctionEditWindow(index);
  editWindow->setCloseHandler([=]() {
      rebuild(window, index);
  });
}


void ModelSpecialFunctionsPage::build(Window * window, int8_t focusIndex) {
  GridLayout grid(*window);
  grid.spacer(8);
  grid.setLabelWidth(66);

  Window::clearFocus();
  char s[5]="SF";

  for(uint8_t  i=0; i < MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData * cfn = &g_model.customFn[i];

    strAppendUnsigned(&s[2], i);
    new TextButton(window, grid.getLabelSlot(), s);
    Button * button = new SpecialFunctionButton(window, grid.getFieldSlot(), i);
    if (focusIndex == i)
      button->setFocus();
    button->setPressHandler([=]() -> uint8_t {
        button->bringToTop();
        Menu * menu = new Menu();
        menu->addLine(STR_EDIT, [=]() {
            editSpecialFunction(window, i);
        });
        return 0;
    });

    grid.spacer(button->height() + 5);
  }
  window->setInnerHeight(grid.getWindowHeight());
}

#define MODEL_SPECIAL_FUNC_1ST_COLUMN          60
#define MODEL_SPECIAL_FUNC_2ND_COLUMN          120
#define MODEL_SPECIAL_FUNC_2ND_COLUMN_EXT      (lcdNextPos + 5)
#define MODEL_SPECIAL_FUNC_3RD_COLUMN          295
#define MODEL_SPECIAL_FUNC_4TH_COLUMN          440
#define MODEL_SPECIAL_FUNC_4TH_COLUMN_ONOFF    450

void onCustomFunctionsFileSelectionMenu(const char * result)
{
  CustomFunctionData * cf;
  uint8_t eeFlags;

  if (menuHandlers[menuLevel] == menuModelSpecialFunctions) {
    cf = &g_model.customFn[menuVerticalPosition];
    eeFlags = EE_MODEL;
  }
  else {
    cf = &g_eeGeneral.customFn[menuVerticalPosition];
    eeFlags = EE_GENERAL;
  }

  uint8_t func = CFN_FUNC(cf);

  if (result == STR_UPDATE_LIST) {
    char directory[256];
    if (func == FUNC_PLAY_SCRIPT) {
      strcpy(directory, SCRIPTS_FUNCS_PATH);
    }
    else {
      strcpy(directory, SOUNDS_PATH);
      strncpy(directory+SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);
    }
    if (!sdListFiles(directory, func==FUNC_PLAY_SCRIPT ? SCRIPTS_EXT : SOUNDS_EXT, sizeof(cf->play.name), NULL)) {
      POPUP_WARNING(func==FUNC_PLAY_SCRIPT ? STR_NO_SCRIPTS_ON_SD : STR_NO_SOUNDS_ON_SD);
    }
  }
  else {
    // The user choosed a file in the list
    memcpy(cf->play.name, result, sizeof(cf->play.name));
    storageDirty(eeFlags);
    if (func == FUNC_PLAY_SCRIPT) {
      LUA_LOAD_MODEL_SCRIPTS();
    }
  }
}

void onCustomFunctionsMenu(const char * result)
{
  CustomFunctionData * cfn;
  uint8_t eeFlags;

  if (menuHandlers[menuLevel] == menuModelSpecialFunctions) {
    cfn = &g_model.customFn[menuVerticalPosition];
    eeFlags = EE_MODEL;
  }
  else {
    cfn = &g_eeGeneral.customFn[menuVerticalPosition];
    eeFlags = EE_GENERAL;
  }

  if (result == STR_COPY) {
    clipboard.type = CLIPBOARD_TYPE_CUSTOM_FUNCTION;
    clipboard.data.cfn = *cfn;
  }
  else if (result == STR_PASTE) {
    *cfn = clipboard.data.cfn;
    storageDirty(eeFlags);
  }
  else if (result == STR_CLEAR) {
    memset(cfn, 0, sizeof(CustomFunctionData));
    storageDirty(eeFlags);
  }
  else if (result == STR_INSERT) {
    memmove(cfn+1, cfn, (MAX_SPECIAL_FUNCTIONS-menuVerticalPosition-1)*sizeof(CustomFunctionData));
    memset(cfn, 0, sizeof(CustomFunctionData));
    storageDirty(eeFlags);
  }
  else if (result == STR_DELETE) {
    memmove(cfn, cfn+1, (MAX_SPECIAL_FUNCTIONS-menuVerticalPosition-1)*sizeof(CustomFunctionData));
    memset(&g_model.customFn[MAX_SPECIAL_FUNCTIONS-1], 0, sizeof(CustomFunctionData));
    storageDirty(eeFlags);
  }
}

void onAdjustGvarSourceLongEnterPress(const char * result)
{
  CustomFunctionData * cfn = &g_model.customFn[menuVerticalPosition];

  if (result == STR_CONSTANT) {
    CFN_GVAR_MODE(cfn) = FUNC_ADJUST_GVAR_CONSTANT;
    CFN_PARAM(cfn) = 0;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_MIXSOURCE) {
    CFN_GVAR_MODE(cfn) = FUNC_ADJUST_GVAR_SOURCE;
    CFN_PARAM(cfn) = 0;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_GLOBALVAR) {
    CFN_GVAR_MODE(cfn) = FUNC_ADJUST_GVAR_GVAR;
    CFN_PARAM(cfn) = 0;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_INCDEC) {
    CFN_GVAR_MODE(cfn) = FUNC_ADJUST_GVAR_INCDEC;
    CFN_PARAM(cfn) = 0;
    storageDirty(EE_MODEL);
  }
  else {
    onSourceLongEnterPress(result);
  }
}

enum SpecialFunctionsItems {
  ITEM_SPECIAL_FUNCTIONS_SWITCH,
  ITEM_SPECIAL_FUNCTIONS_FUNCTION,
  ITEM_SPECIAL_FUNCTIONS_PARAM,
  ITEM_SPECIAL_FUNCTIONS_VALUE,
  ITEM_SPECIAL_FUNCTIONS_ENABLE,
  ITEM_SPECIAL_FUNCTIONS_COUNT,
  ITEM_SPECIAL_FUNCTIONS_LAST = ITEM_SPECIAL_FUNCTIONS_COUNT-1
};

bool menuSpecialFunctions(event_t event, CustomFunctionData * functions, CustomFunctionsContext & functionsContext)
{
  uint8_t eeFlags = (functions == g_model.customFn) ? EE_MODEL : EE_GENERAL;

  if (menuHorizontalPosition<0 && event==EVT_KEY_LONG(KEY_ENTER) && !READ_ONLY()) {
    killEvents(event);
    CustomFunctionData *cfn = &functions[menuVerticalPosition];
    if (!CFN_EMPTY(cfn))
      POPUP_MENU_ADD_ITEM(STR_COPY);
    if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_FUNCTION)
      POPUP_MENU_ADD_ITEM(STR_PASTE);
    if (!CFN_EMPTY(cfn) && CFN_EMPTY(&functions[MAX_SPECIAL_FUNCTIONS-1]))
      POPUP_MENU_ADD_ITEM(STR_INSERT);
    if (!CFN_EMPTY(cfn))
      POPUP_MENU_ADD_ITEM(STR_CLEAR);
    for (int i=menuVerticalPosition+1; i<MAX_SPECIAL_FUNCTIONS; i++) {
      if (!CFN_EMPTY(&functions[i])) {
        POPUP_MENU_ADD_ITEM(STR_DELETE);
        break;
      }
    }
    POPUP_MENU_START(onCustomFunctionsMenu);
  }

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_CONTENT_TOP + i*FH;
    int k = i+menuVerticalOffset;

    drawStringWithIndex(MENUS_MARGIN_LEFT, y, functions == g_model.customFn ? STR_SF : STR_GF, k+1, (menuVerticalPosition==k && menuHorizontalPosition<0) ? INVERS : 0);

    CustomFunctionData * cfn = &functions[k];
    uint8_t func = CFN_FUNC(cfn);
    for (uint8_t j=0; j<ITEM_SPECIAL_FUNCTIONS_COUNT; j++) {
      LcdFlags attr = ((menuVerticalPosition==k && menuHorizontalPosition==j) ? ((s_editMode>0) ? BLINK|INVERS : INVERS) : 0);
      LcdFlags active = (attr && s_editMode>0);
      switch (j) {
        case ITEM_SPECIAL_FUNCTIONS_SWITCH:
          drawSwitch(MODEL_SPECIAL_FUNC_1ST_COLUMN, y, CFN_SWITCH(cfn), attr | ((functionsContext.activeSwitches & ((MASK_CFN_TYPE)1 << k)) ? BOLD : 0));
          if (active || AUTOSWITCH_ENTER_LONG()) {
            CHECK_INCDEC_SWITCH(event, CFN_SWITCH(cfn), SWSRC_FIRST, SWSRC_LAST, eeFlags, isSwitchAvailableInCustomFunctions);
          }
          if (func == FUNC_OVERRIDE_CHANNEL && functions != g_model.customFn) {
            func = CFN_FUNC(cfn) = func+1;
          }
          break;

        case ITEM_SPECIAL_FUNCTIONS_FUNCTION:
          if (CFN_SWITCH(cfn)) {
            if (active) {
              func = CFN_FUNC(cfn) = checkIncDec(event, CFN_FUNC(cfn), 0, FUNC_MAX-1, eeFlags, isAssignableFunctionAvailable);
              if (checkIncDec_Ret) CFN_RESET(cfn);
            }
            lcdDrawTextAtIndex(MODEL_SPECIAL_FUNC_2ND_COLUMN, y, STR_VFSWFUNC, func, attr);
          }
          else {
            j = ITEM_SPECIAL_FUNCTIONS_ENABLE; // skip other fields
            while (menuVerticalPosition==k && menuHorizontalPosition > 0) {
              REPEAT_LAST_CURSOR_MOVE(ITEM_SPECIAL_FUNCTIONS_LAST, true);
            }
          }
          break;

        case ITEM_SPECIAL_FUNCTIONS_PARAM:
        {
          int8_t maxParam = MAX_OUTPUT_CHANNELS-1;
#if defined(OVERRIDE_CHANNEL_FUNCTION)
          if (func == FUNC_OVERRIDE_CHANNEL) {
            putsChn(MODEL_SPECIAL_FUNC_2ND_COLUMN_EXT, y, CFN_CH_INDEX(cfn)+1, attr);
          }
          else
#endif
          if (func == FUNC_TRAINER) {
            maxParam = 4;
            drawSource(MODEL_SPECIAL_FUNC_2ND_COLUMN_EXT, y, CFN_CH_INDEX(cfn)==0 ? 0 : MIXSRC_Rud+CFN_CH_INDEX(cfn)-1, attr);
          }
#if defined(GVARS)
          else if (func == FUNC_ADJUST_GVAR) {
            maxParam = MAX_GVARS-1;
            drawStringWithIndex(MODEL_SPECIAL_FUNC_2ND_COLUMN_EXT, y, STR_GV, CFN_GVAR_INDEX(cfn)+1, attr);
            if (active) CFN_GVAR_INDEX(cfn) = checkIncDec(event, CFN_GVAR_INDEX(cfn), 0, maxParam, eeFlags);
            break;
          }
#endif
          else if (func == FUNC_SET_TIMER) {
            maxParam = TIMERS-1;
            drawStringWithIndex(MODEL_SPECIAL_FUNC_2ND_COLUMN_EXT, y, STR_TIMER, CFN_TIMER_INDEX(cfn)+1, attr);
            if (active) CFN_TIMER_INDEX(cfn) = checkIncDec(event, CFN_TIMER_INDEX(cfn), 0, maxParam, eeFlags);
            break;
          }
          else if (attr) {
            REPEAT_LAST_CURSOR_MOVE(ITEM_SPECIAL_FUNCTIONS_LAST, true);
          }
          if (active) CHECK_INCDEC_MODELVAR_ZERO(event, CFN_CH_INDEX(cfn), maxParam);
          break;
        }

        case ITEM_SPECIAL_FUNCTIONS_VALUE:
        {
          INCDEC_DECLARE_VARS(eeFlags);
          int16_t val_displayed = CFN_PARAM(cfn);
          int16_t val_min = 0;
          int16_t val_max = 255;
          if (func == FUNC_RESET) {
            val_max = FUNC_RESET_PARAM_FIRST_TELEM+lastUsedTelemetryIndex();
            int param = CFN_PARAM(cfn);
            if (param < FUNC_RESET_PARAM_FIRST_TELEM) {
              lcdDrawTextAtIndex(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, STR_VFSWRESET, param, attr);
            }
            else {
              TelemetrySensor * sensor = & g_model.telemetrySensors[param-FUNC_RESET_PARAM_FIRST_TELEM];
              lcdDrawSizedText(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, sensor->label, TELEM_LABEL_LEN, attr|ZCHAR);
            }
            if (active) INCDEC_ENABLE_CHECK(functions == g_eeGeneral.customFn ? isSourceAvailableInGlobalResetSpecialFunction : isSourceAvailableInResetSpecialFunction);
          }
#if defined(OVERRIDE_CHANNEL_FUNCTION)
          else if (func == FUNC_OVERRIDE_CHANNEL) {
            getMixSrcRange(MIXSRC_FIRST_CH, val_min, val_max);
            lcdDrawNumber(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr|LEFT);
          }
#endif
#if defined(DANGEROUS_MODULE_FUNCTIONS)
          else if (func >= FUNC_RANGECHECK && func <= FUNC_BIND) {
            val_max = NUM_MODULES-1;
            lcdDrawTextAtIndex(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, "\004Int.Ext.", CFN_PARAM(cfn), attr);
          }
#endif
          else if (func == FUNC_SET_TIMER) {
            getMixSrcRange(MIXSRC_FIRST_TIMER, val_min, val_max);
            drawTimer(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr|LEFT|TIMEHOUR);
          }
          else if (func == FUNC_PLAY_SOUND) {
            val_max = AU_SPECIAL_SOUND_LAST-AU_SPECIAL_SOUND_FIRST-1;
            lcdDrawTextAtIndex(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, STR_FUNCSOUNDS, val_displayed, attr);
          }
#if defined(HAPTIC)
          else if (func == FUNC_HAPTIC) {
            val_max = 3;
            lcdDrawNumber(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr|LEFT);
          }
#endif
#if defined(SDCARD)
          else if (func == FUNC_PLAY_TRACK || func == FUNC_BACKGND_MUSIC || func == FUNC_PLAY_SCRIPT) {
            coord_t x = MODEL_SPECIAL_FUNC_3RD_COLUMN;
            if (ZEXIST(cfn->play.name))
              lcdDrawSizedText(x, y, cfn->play.name, sizeof(cfn->play.name), attr);
            else
              lcdDrawTextAtIndex(x, y, STR_VCSWFUNC, 0, attr);
            if (attr) s_editMode = 0;
            if (attr && event==EVT_KEY_FIRST(KEY_ENTER)) {
              killEvents(KEY_ENTER);
              char directory[256];
              if (func==FUNC_PLAY_SCRIPT) {
                strcpy(directory, SCRIPTS_FUNCS_PATH);
              }
              else {
                strcpy(directory, SOUNDS_PATH);
                strncpy(directory+SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);
              }
              if (sdListFiles(directory, func==FUNC_PLAY_SCRIPT ? SCRIPTS_EXT : SOUNDS_EXT, sizeof(cfn->play.name), cfn->play.name)) {
                POPUP_MENU_START(onCustomFunctionsFileSelectionMenu);
              }
              else {
                POPUP_WARNING(func==FUNC_PLAY_SCRIPT ? STR_NO_SCRIPTS_ON_SD : STR_NO_SOUNDS_ON_SD);
              }
            }
            break;
          }
          else if (func == FUNC_PLAY_VALUE) {
            val_max = MIXSRC_LAST_TELEM;
            drawSource(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr);
            if (active) {
              INCDEC_SET_FLAG(eeFlags | INCDEC_SOURCE);
              INCDEC_ENABLE_CHECK(functions == g_eeGeneral.customFn ? isSourceAvailableInGlobalFunctions : isSourceAvailable);
            }
          }
#endif
          else if (func == FUNC_VOLUME) {
            val_max = MIXSRC_LAST_CH;
            drawSource(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr);
            if (active) {
              INCDEC_SET_FLAG(eeFlags | INCDEC_SOURCE);
              INCDEC_ENABLE_CHECK(isSourceAvailable);
            }
          }
          else if (func == FUNC_LOGS) {
            if (val_displayed) {
              lcdDrawNumber(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr|PREC1|LEFT, 0, NULL, "s");
            }
            else {
              lcdDrawMMM(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, attr);
            }
          }
#if defined(GVARS)
          else if (func == FUNC_ADJUST_GVAR) {
            switch (CFN_GVAR_MODE(cfn)) {
              case FUNC_ADJUST_GVAR_CONSTANT:
              {
                val_displayed = (int16_t)CFN_PARAM(cfn);
                getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min, val_max);
                drawGVarValue(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, CFN_GVAR_INDEX(cfn), val_displayed, attr|LEFT);
                break;
              }
              case FUNC_ADJUST_GVAR_SOURCE:
                val_max = MIXSRC_LAST_CH;
                drawSource(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr);
                if (active) {
                  INCDEC_SET_FLAG(eeFlags | INCDEC_SOURCE);
                  INCDEC_ENABLE_CHECK(isSourceAvailable);
                }
                break;
              case FUNC_ADJUST_GVAR_GVAR:
                val_max = MAX_GVARS-1;
                drawStringWithIndex(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, STR_GV, val_displayed+1, attr);
                break;
              default: // FUNC_ADJUST_GVAR_INC
                getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min, val_max);
                lcdDrawText(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, (val_displayed < 0 ? "-= " : "+= "), attr);
                drawGVarValue(lcdNextPos, y, CFN_GVAR_INDEX(cfn), abs(val_displayed), attr|LEFT);
                break;
            }
          }
#endif
          else if (attr) {
            REPEAT_LAST_CURSOR_MOVE(ITEM_SPECIAL_FUNCTIONS_LAST, true);
          }

          if (active || event==EVT_KEY_LONG(KEY_ENTER)) {
            CFN_PARAM(cfn) = CHECK_INCDEC_PARAM(event, val_displayed, val_min, val_max);
            if (func == FUNC_ADJUST_GVAR && attr && event==EVT_KEY_LONG(KEY_ENTER)) {
              killEvents(event);
              if (CFN_GVAR_MODE(cfn) != FUNC_ADJUST_GVAR_CONSTANT)
                POPUP_MENU_ADD_ITEM(STR_CONSTANT);
              if (CFN_GVAR_MODE(cfn) != FUNC_ADJUST_GVAR_SOURCE)
                POPUP_MENU_ADD_ITEM(STR_MIXSOURCE);
              if (CFN_GVAR_MODE(cfn) != FUNC_ADJUST_GVAR_GVAR)
                POPUP_MENU_ADD_ITEM(STR_GLOBALVAR);
              if (CFN_GVAR_MODE(cfn) != FUNC_ADJUST_GVAR_INCDEC)
                POPUP_MENU_ADD_ITEM(STR_INCDEC);
              POPUP_MENU_START(onAdjustGvarSourceLongEnterPress);
              s_editMode = EDIT_MODIFY_FIELD;
            }
          }
          break;
        }

        case ITEM_SPECIAL_FUNCTIONS_ENABLE:
          if (HAS_ENABLE_PARAM(func)) {
            if (active) CFN_ACTIVE(cfn) = checkIncDec(event, CFN_ACTIVE(cfn), 0, 1, eeFlags);
            drawCheckBox(MODEL_SPECIAL_FUNC_4TH_COLUMN_ONOFF, y, CFN_ACTIVE(cfn), attr);
          }
          else if (HAS_REPEAT_PARAM(func)) {
            if (active) CFN_PLAY_REPEAT(cfn) = checkIncDec(event, CFN_PLAY_REPEAT(cfn)==CFN_PLAY_REPEAT_NOSTART?-1:CFN_PLAY_REPEAT(cfn), -1, 60/CFN_PLAY_REPEAT_MUL, eeFlags);
            if (CFN_PLAY_REPEAT(cfn) == 0) {
              lcdDrawText(MODEL_SPECIAL_FUNC_4TH_COLUMN+2, y, "1x", attr);
            }
            else if (CFN_PLAY_REPEAT(cfn) == CFN_PLAY_REPEAT_NOSTART) {
              lcdDrawText(MODEL_SPECIAL_FUNC_4TH_COLUMN-1, y, "!1x", attr);
            }
            else {
              lcdDrawNumber(MODEL_SPECIAL_FUNC_4TH_COLUMN+12, y, CFN_PLAY_REPEAT(cfn)*CFN_PLAY_REPEAT_MUL, attr|RIGHT, 0, NULL, "s");
            }
          }
          else if (attr) {
            REPEAT_LAST_CURSOR_MOVE(ITEM_SPECIAL_FUNCTIONS_LAST, true);
          }
          break;
      }
    }
  }

  return true;
}

bool menuModelSpecialFunctions(event_t event)
{
  MENU(STR_MENUCUSTOMFUNC, MODEL_ICONS, menuTabModel, MENU_MODEL_SPECIAL_FUNCTIONS, MAX_SPECIAL_FUNCTIONS, { NAVIGATION_LINE_BY_LINE|4/*repeated*/ });
  return menuSpecialFunctions(event, g_model.customFn, modelFunctionsContext);
}
