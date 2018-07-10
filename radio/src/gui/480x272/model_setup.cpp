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
#include "model_setup.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

ModelSetupPage::ModelSetupPage():
  MenuPage(STR_MENUSETUP, ICON_MODEL_SETUP)
{
}

uint8_t g_moduleIdx;

void onBindMenu(const char * result)
{
  uint8_t moduleIdx = 0; // TODO (menuVerticalPosition >= ITEM_MODEL_EXTERNAL_MODULE_LABEL ? EXTERNAL_MODULE : INTERNAL_MODULE);

  if (result == STR_BINDING_25MW_CH1_8_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.power = R9M_LBT_POWER_25;
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  } else if (result == STR_BINDING_25MW_CH1_8_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.power = R9M_LBT_POWER_25;
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = false;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  } else if (result == STR_BINDING_500MW_CH1_8_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.power = R9M_LBT_POWER_500;
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  } else if (result == STR_BINDING_500MW_CH9_16_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.power = R9M_LBT_POWER_500;
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = true;
  } else if (result == STR_BINDING_1_8_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = false;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  } else if (result == STR_BINDING_1_8_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = false;
  } else if (result == STR_BINDING_9_16_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = false;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = true;
  } else if (result == STR_BINDING_9_16_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.receiver_telem_off = true;
    g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 = true;
  } else {
    return;
  }

  moduleFlag[moduleIdx] = MODULE_BIND;
}

void onModelSetupBitmapMenu(const char * result)
{
  if (result == STR_UPDATE_LIST) {
    if (!sdListFiles(BITMAPS_PATH, BITMAPS_EXT, sizeof(g_model.header.bitmap) - LEN_BITMAPS_EXT, NULL)) {
      POPUP_WARNING(STR_NO_BITMAPS_ON_SD);
    }
  } else {
    // The user choosed a bmp file in the list
    copySelection(g_model.header.bitmap, result, sizeof(g_model.header.bitmap));
    storageDirty(EE_MODEL);
  }
}

void editTimerMode(int timerIdx, coord_t y, LcdFlags attr, event_t event)
{
  TimerData &timer = g_model.timers[timerIdx];
  if (attr && menuHorizontalPosition < 0) {
//    lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN - INVERT_HORZ_MARGIN, y - INVERT_VERT_MARGIN + 1,
//                           115 + 2 * INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
  }
  drawStringWithIndex(MENUS_MARGIN_LEFT, y, STR_TIMER, timerIdx + 1, BOLD);
  // drawTimerMode(window, MODEL_SETUP_2ND_COLUMN, y, timer.mode, (menuHorizontalPosition<=0 ? attr : 0));
  // drawTimer(window, MODEL_SETUP_2ND_COLUMN+50, y, timer.start, (menuHorizontalPosition!=0 ? attr|TIMEHOUR : TIMEHOUR));
  if (attr && s_editMode > 0) {
    switch (menuHorizontalPosition) {
      case 0: {
        int32_t timerMode = timer.mode;
        if (timerMode < 0) timerMode -= TMRMODE_COUNT - 1;
        CHECK_INCDEC_MODELVAR_CHECK(event, timerMode, -TMRMODE_COUNT - SWSRC_LAST + 1, TMRMODE_COUNT + SWSRC_LAST - 1,
                                    isSwitchAvailableInTimers);
        if (timerMode < 0) timerMode += TMRMODE_COUNT - 1;
        timer.mode = timerMode;
#if defined(AUTOSWITCH)
        if (s_editMode > 0) {
          int8_t val = timer.mode - (TMRMODE_COUNT - 1);
          int8_t switchVal = checkIncDecMovedSwitch(val);
          if (val != switchVal) {
            timer.mode = switchVal + (TMRMODE_COUNT - 1);
            storageDirty(EE_MODEL);
          }
        }
#endif
        break;
      }
      case 1: {
        const int stopsMinutes[] = {8, 60, 120, 180, 240, 300, 600, 900, 1200};
        timer.start = checkIncDec(event, timer.start, 0, TIMER_MAX, EE_MODEL, NULL,
                                  (const CheckIncDecStops &) stopsMinutes);
        break;
      }
    }
  }
}

int getSwitchWarningsCount()
{
  int count = 0;
  for (int i = 0; i < NUM_SWITCHES; ++i) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      ++count;
    }
  }
  return count;
}

// @3djc don't use NOINDENT anymore (this could be another task later ... remove all these INDENT from translations ...)

void resetModuleSettings(uint8_t module)
{
  g_model.moduleData[module].channelsStart = 0;
  if(IS_MODULE_PPM(module)) {
    g_model.moduleData[module].channelsCount = 0;
    SET_DEFAULT_PPM_FRAME_LENGTH(EXTERNAL_MODULE);
  }
  else
    g_model.moduleData[module].channelsCount = MAX_SENT_CHANNELS(g_model.moduleData[module].type) - 8;
  g_model.moduleData[module].rfProtocol = 0;
}

void ModelSetupPage::build(Window * window)
{
  GridLayout grid(*window);
  grid.spacer(10);

  // Model name
  new StaticText(window, grid.getLabelSlot(), STR_MODELNAME);
  new TextEdit(window, grid.getFieldSlot(), g_model.header.name, sizeof(g_model.header.name));
  grid.nextLine();

/*
 * case ITEM_MODEL_BITMAP: {
        rect_t rect = { MENUS_MARGIN_LEFT, y, LCD_W - MENUS_MARGIN_LEFT - 20, 30 };
        drawWidgetLabel(window, rect, STR_BITMAP);
        if (ZEXIST(g_model.header.bitmap))
          menuBodyWindow.drawSizedText(MENUS_MARGIN_LEFT, y + 10, g_model.header.bitmap, sizeof(g_model.header.bitmap), attr);
        else
          drawTextAtIndex(window, MENUS_MARGIN_LEFT, y + 10, STR_VCSWFUNC, 0, attr);
        drawWidgetLine(window, rect, CURVE_AXIS_COLOR);
        if (attr && event == EVT_KEY_BREAK(KEY_ENTER) && READ_ONLY_UNLOCKED()) {
          s_editMode = 0;
          if (sdListFiles(BITMAPS_PATH, BITMAPS_EXT, sizeof(g_model.header.bitmap) - LEN_BITMAPS_EXT,
                          g_model.header.bitmap, LIST_NONE_SD_FILE | LIST_SD_FILE_EXT)) {
            POPUP_MENU_START(onModelSetupBitmapMenu);
          } else {
            POPUP_WARNING(STR_NO_BITMAPS_ON_SD);
          }
        }
        break;
      }
*/

  for (uint8_t i = 0; i < TIMERS; i++) {
    // Timer label
    char timerLabel[8];
    strAppendStringWithIndex(timerLabel, STR_TIMER, i + 1);
    new Subtitle(window, grid.getLineSlot(), timerLabel);
    grid.nextLine();

    // editTimerMode(0, y, attr, event);

    // Timer name
    new StaticText(window, grid.getLabelSlot(true), STR_TIMER_NAME);
    new TextEdit(window, grid.getFieldSlot(), g_model.timers[i].name, LEN_TIMER_NAME);
    grid.nextLine();

    // Timer minute beep
    new StaticText(window, grid.getLabelSlot(true), STR_MINUTEBEEP);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.timers[i].minuteBeep));
    grid.nextLine();

    // Timer countdown
    new StaticText(window, grid.getLabelSlot(true), STR_BEEPCOUNTDOWN);
    new Choice(window, grid.getFieldSlot(2, 0), STR_VBEEPCOUNTDOWN, COUNTDOWN_SILENT, COUNTDOWN_COUNT - 1, GET_SET_DEFAULT(g_model.timers[i].countdownBeep));
    new Choice(window, grid.getFieldSlot(2, 1), STR_COUNTDOWNVALUES, 0, 3,
               GET_DEFAULT(g_model.timers[i].countdownStart + 2),
               SET_VALUE(g_model.timers[i].countdownStart, newValue - 2));
    grid.nextLine();

    // Timer persistent
    new StaticText(window, grid.getLabelSlot(true), STR_PERSISTENT);
    new Choice(window, grid.getFieldSlot(), STR_VPERSISTENT, 0, 2, GET_SET_DEFAULT(g_model.timers[i].persistent));
    grid.nextLine();
  }

  // Extended limits
  new StaticText(window, grid.getLabelSlot(), STR_ELIMITS);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.extendedLimits));
  grid.nextLine();

  // Extended trims
  new StaticText(window, grid.getLabelSlot(), STR_ETRIMS);
  new CheckBox(window, grid.getFieldSlot(2, 0), GET_SET_DEFAULT(g_model.extendedTrims));
  new TextButton(window, grid.getFieldSlot(2, 1),STR_RESET_BTN,
                 [&]() -> uint8_t {
                     START_NO_HIGHLIGHT();
                     for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
                       memclear(&g_model.flightModeData[i], TRIMS_ARRAY_SIZE);
                     }
                     storageDirty(EE_MODEL);
                     AUDIO_WARNING1();
                     return 0;
                 });
  grid.nextLine();

  // Display trims
  new StaticText(window, grid.getLabelSlot(), STR_DISPLAY_TRIMS);
  new Choice(window, grid.getFieldSlot(), "\006No\0   ChangeYes", 0, 2, GET_SET_DEFAULT(g_model.displayTrims));
  grid.nextLine();

  // Trim step
  new StaticText(window, grid.getLabelSlot(), STR_TRIMINC);
  new Choice(window, grid.getFieldSlot(), STR_VTRIMINC, -2, 2, GET_SET_DEFAULT(g_model.trimInc));
  grid.nextLine();

  // Throttle parameters
  {
    new Subtitle(window, grid.getLineSlot(), STR_THROTTLE_LABEL);
    grid.nextLine();

    // Throttle reversed
    new StaticText(window, grid.getLabelSlot(true), STR_THROTTLEREVERSE);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.throttleReversed));
    grid.nextLine();

    // Throttle source
    new StaticText(window, grid.getLabelSlot(true), STR_TTRACE);
    new SourceChoice(window, grid.getFieldSlot(), MIXSRC_LAST_CH, GET_SET_DEFAULT(g_model.thrTraceSrc));
    grid.nextLine();

    // Throttle trim
    new StaticText(window, grid.getLabelSlot(true), STR_TTRIM);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.thrTrim));
    grid.nextLine();
  }

  // Preflight parameters
  {
    new Subtitle(window, grid.getLineSlot(), STR_PREFLIGHT);
    grid.nextLine();

    // Display checklist
    new StaticText(window, grid.getLabelSlot(true), STR_CHECKLIST);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.displayChecklist));
    grid.nextLine();

    // Throttle warning
    new StaticText(window, grid.getLabelSlot(true), STR_THROTTLEWARNING);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_model.disableThrottleWarning));
    grid.nextLine();

    // Switches warning
    new StaticText(window, grid.getLabelSlot(true), STR_SWITCHWARNING);
    for (int i=0; i<NUM_SWITCHES; i++) {
      char s[3];
      if (i > 0 && (i % 3) == 0)
        grid.nextLine();

      switchWarn[i] = new TextButton(window, grid.getFieldSlot(3, i % 3), getSwitchWarningString(s, i),
                                     [=]() -> uint8_t {
                                         uint8_t newstate = BF_GET(g_model.switchWarningState, 3*i, 3);
                                         if (newstate == 1 && !IS_3POS(i))
                                           newstate = 3;
                                         else
                                           newstate = (newstate + 1) % 4;
                                         BF_SET(g_model.switchWarningState, newstate, 3*i, 3);
                                         SET_DIRTY();
                                         char s[3];
                                         switchWarn[i]->setText(getSwitchWarningString(s, i));
                                         return newstate > 0 ? 1 : 0;
                                     },
                                     (BF_GET(g_model.switchWarningState, 3*i, 3) == 0 ? 0 : 1));
    }
    grid.nextLine();
  }

  // Center beeps
  {
    new StaticText(window, grid.getLabelSlot(), STR_BEEPCTR);
    for (int i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) {
      char s[2];
      if (i > 0 && (i % 3) == 0)
        grid.nextLine();

      new TextButton(window, grid.getFieldSlot(3, i % 3), getStringAtIndex(s, STR_RETA123, i),
                     [=]() -> uint8_t {
                       BF_BIT_FLIP(g_model.beepANACenter, BF_BIT(i));
                       SET_DIRTY();
                       return BF_SINGLE_BIT_GET(g_model.beepANACenter, i);
                     },
                     BF_SINGLE_BIT_GET(g_model.beepANACenter, i));
    }
    grid.nextLine();
  }

  // Global functions
  {
    new StaticText(window, grid.getLabelSlot(), STR_USE_GLOBAL_FUNCS);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_model.noGlobalFunctions));
    grid.nextLine();
  }

  // Internal module
  {
    new Subtitle(window, grid.getLineSlot(), STR_INTERNALRF);
    grid.nextLine();
    internalModuleWindow = new Window(window, { 0, grid.getWindowHeight(), LCD_W, 0 });
    updateInternalModuleWindow();
    grid.addWindow(internalModuleWindow);
  }

  // External module
  {
    new Subtitle(window, grid.getLineSlot(), STR_EXTERNALRF);
    grid.nextLine();
    externalModuleWindow = new Window(window, { 0, grid.getWindowHeight(), LCD_W, 0 });
    updateExternalModuleWindow();
    grid.addWindow(externalModuleWindow);
  }

  window->setInnerHeight(grid.getWindowHeight());
}

void ModelSetupPage::updateInternalModuleWindow()
{
  int8_t value = g_model.moduleData[INTERNAL_MODULE].rfProtocol;

  GridLayout grid(*internalModuleWindow);

  internalModuleWindow->clear();

  new StaticText(internalModuleWindow, grid.getLabelSlot(true), STR_MODE);
  internalModuleChoice = new Choice(internalModuleWindow, grid.getFieldSlot(2, 0), STR_TARANIS_PROTOCOLS, MODULE_TYPE_NONE, MODULE_TYPE_COUNT - 1,
                                    GET_DEFAULT(1 + g_model.moduleData[INTERNAL_MODULE].rfProtocol),
                                    [=](int32_t newValue) -> void {
                                      g_model.moduleData[INTERNAL_MODULE].rfProtocol = newValue - 1;
                                      SET_DIRTY();
                                      updateInternalModuleWindow();
                                      internalModuleChoice->setFocus();
                                    });
  grid.nextLine();

  if (value != RF_PROTO_OFF) {
    new StaticText(internalModuleWindow, grid.getLabelSlot(true), STR_CHANNELRANGE);
    new NumberEdit(internalModuleWindow, grid.getFieldSlot(2, 0), 1, MAX_CHANNELS(0), 1,
                   GET_DEFAULT(1 + g_model.moduleData[INTERNAL_MODULE].channelsStart),
                   [=](int8_t newValue) -> void { g_model.moduleData[INTERNAL_MODULE].channelsStart = newValue - 1; }, 0, STR_CH);
    new NumberEdit(internalModuleWindow, grid.getFieldSlot(2, 1), 1 + g_model.moduleData[INTERNAL_MODULE].channelsStart,
                   min<int8_t>(1 + g_model.moduleData[INTERNAL_MODULE].channelsStart + MAX_CHANNELS(0) + 8, 1 + 32), 1,
                   GET_DEFAULT(g_model.moduleData[INTERNAL_MODULE].channelsCount + 8 + g_model.moduleData[INTERNAL_MODULE].channelsStart),
                   SET_VALUE(g_model.moduleData[INTERNAL_MODULE].channelsCount, newValue - 8 - g_model.moduleData[INTERNAL_MODULE].channelsStart),
                   0, STR_CH);
    grid.nextLine();

    new StaticText(internalModuleWindow, grid.getLabelSlot(true), STR_FAILSAFE);
    new Choice(internalModuleWindow, grid.getFieldSlot(), STR_VFAILSAFE, 0, FAILSAFE_LAST,
               GET_SET_DEFAULT(g_model.moduleData[INTERNAL_MODULE].failsafeMode));
    grid.nextLine();

    new StaticText(internalModuleWindow, grid.getLabelSlot(true), STR_ANTENNASELECTION);
    new Choice(internalModuleWindow, grid.getFieldSlot(), STR_VANTENNATYPES, 0, 1,
               GET_SET_DEFAULT(g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna));
  }

  coord_t delta = internalModuleWindow->adjustHeight();
  internalModuleWindow->parent->moveWindowsTop(internalModuleWindow->rect.y, delta);
  internalModuleWindow->parent->innerHeight += delta;
}

void ModelSetupPage::updateExternalModuleWindow()
{
  GridLayout grid(*externalModuleWindow);

  externalModuleWindow->clear();

  new StaticText(externalModuleWindow, grid.getLabelSlot(true), STR_MODE);
  externalModuleChoice = new Choice(externalModuleWindow, grid.getFieldSlot(2, 0), STR_TARANIS_PROTOCOLS, MODULE_TYPE_NONE, MODULE_TYPE_COUNT - 1,
                                    GET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].type),
                                    [=](int32_t newValue) -> void {
                                      g_model.moduleData[EXTERNAL_MODULE].type = newValue;
                                      SET_DIRTY();
                                      resetModuleSettings(EXTERNAL_MODULE);
                                      updateExternalModuleWindow();
                                      externalModuleChoice->setFocus();
                                    });

  if (g_model.moduleData[EXTERNAL_MODULE].type != MODULE_TYPE_NONE && !IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) {
    if (IS_MODULE_XJT(EXTERNAL_MODULE)) {
      new Choice(externalModuleWindow, grid.getFieldSlot(2, 1), STR_XJT_PROTOCOLS, 1 + RF_PROTO_X16, 1 + RF_PROTO_LAST,
                 GET_DEFAULT(1 + g_model.moduleData[EXTERNAL_MODULE].rfProtocol),
                 SET_VALUE(g_model.moduleData[EXTERNAL_MODULE].rfProtocol, newValue - 1));
    }
    else if (IS_MODULE_DSM2(EXTERNAL_MODULE)) {
      new Choice(externalModuleWindow, grid.getFieldSlot(2, 1), STR_DSM_PROTOCOLS, DSM2_PROTO_LP45, DSM2_PROTO_DSMX,
                 GET_SET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].rfProtocol));
    }
    else if (IS_MODULE_R9M(EXTERNAL_MODULE)) {
      new Choice(externalModuleWindow, grid.getFieldSlot(2, 1), STR_R9M_MODES, MODULE_SUBTYPE_R9M_FCC,
                 MODULE_SUBTYPE_R9M_LBT,
                 GET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].subType),
                 [=](int32_t newValue) -> void {
                   g_model.moduleData[EXTERNAL_MODULE].subType = newValue;
                   SET_DIRTY();
                   updateExternalModuleWindow();
                 });
    }
    grid.nextLine();

    // Channel Range
    new StaticText(externalModuleWindow, grid.getLabelSlot(true), STR_CHANNELRANGE);

    if (IS_MODULE_CROSSFIRE(EXTERNAL_MODULE)) { // CRSF has a fixed 16ch span
      // From
      new NumberEdit(externalModuleWindow, grid.getFieldSlot(2, 0), 1, 17, 1,
                     GET_DEFAULT(1 + g_model.moduleData[EXTERNAL_MODULE].channelsStart),
                     [=](int32_t newValue) -> void {
                       g_model.moduleData[EXTERNAL_MODULE].channelsStart = newValue - 1;
                       SET_DIRTY();
                       updateExternalModuleWindow();
                     }, 0, STR_CH);
      // To
      new NumberEdit(externalModuleWindow, grid.getFieldSlot(2, 1),
                     0, 32, 1,
                     GET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].channelsStart + 16),
                     SET_VALUE(g_model.moduleData[EXTERNAL_MODULE].channelsCount, 8),
                     0, STR_CH);
    }
    else {
      // From
      new NumberEdit(externalModuleWindow, grid.getFieldSlot(2, 0), 1,
                     MAX_OUTPUT_CHANNELS - g_model.moduleData[EXTERNAL_MODULE].channelsCount, 1,
                     GET_DEFAULT(1 + g_model.moduleData[EXTERNAL_MODULE].channelsStart),
                     [=](int32_t newValue) -> void {
                       g_model.moduleData[EXTERNAL_MODULE].channelsStart = newValue - 1;
                       SET_DIRTY();
                       updateExternalModuleWindow();
                     }, 0, STR_CH);
      // To
      new NumberEdit(externalModuleWindow, grid.getFieldSlot(2, 1),
                     g_model.moduleData[EXTERNAL_MODULE].channelsStart + 1,
                     g_model.moduleData[EXTERNAL_MODULE].channelsStart + MAX_CHANNELS(EXTERNAL_MODULE), 1,
                     GET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].channelsStart + 8 +
                                 g_model.moduleData[EXTERNAL_MODULE].channelsCount),
                     [=](int8_t newValue) -> void {
                       g_model.moduleData[EXTERNAL_MODULE].channelsCount = newValue - g_model.moduleData[EXTERNAL_MODULE].channelsStart - 8;
                       SET_DIRTY();
                       updateExternalModuleWindow();
                     }, 0, STR_CH);
    }
    grid.nextLine();

    // PPM modules
    if (IS_MODULE_PPM(EXTERNAL_MODULE)) {
      SET_DEFAULT_PPM_FRAME_LENGTH(EXTERNAL_MODULE);
      // PPM frame
      new StaticText(externalModuleWindow, grid.getLabelSlot(true), STR_PPMFRAME);
      // PPM frame length
      new NumberEdit(externalModuleWindow, grid.getFieldSlot(2, 0), 125, 35 * 5 + 225, 5,
                     GET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].ppm.frameLength * 5 + 225),
                     SET_VALUE(g_model.moduleData[EXTERNAL_MODULE].ppm.frameLength, (newValue - 225) / 5),
                     PREC1, NULL, STR_MS);

      // PPM frame delay
      new NumberEdit(externalModuleWindow, grid.getFieldSlot(2, 1), 100, 800, 50,
                     GET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].ppm.delay * 50 + 300),
                     SET_VALUE(g_model.moduleData[EXTERNAL_MODULE].ppm.delay, (newValue - 300) / 50),
                     0, NULL, "us");
    }

    if (IS_MODULE_PXX(EXTERNAL_MODULE) || IS_MODULE_DSM2(EXTERNAL_MODULE) || IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) {
      // Receiver
      new StaticText(externalModuleWindow, grid.getLabelSlot(true), STR_RECEIVER_NUM);
      // Receiver number
      new NumberEdit(externalModuleWindow, grid.getFieldSlot(2, 0), 0, MAX_RX_NUM(EXTERNAL_MODULE), 1,
                     GET_SET_DEFAULT(g_model.header.modelId[EXTERNAL_MODULE]), 0);
      grid.nextLine();

      // Bind and Range buttons
      // TODO use greyed button when available
      externalModuleBind = new TextButton(externalModuleWindow, grid.getFieldSlot(2, 0), STR_MODULE_BIND,
                     [=]() -> uint8_t {
                         if(moduleFlag[EXTERNAL_MODULE] == MODULE_RANGECHECK) {
                           moduleFlag[EXTERNAL_MODULE] = MODULE_BIND;
                           externalModuleRange->setState(0);
                           return 1;
                         }
                         if (moduleFlag[EXTERNAL_MODULE] == MODULE_NORMAL_MODE) {
                           moduleFlag[EXTERNAL_MODULE] = MODULE_BIND;
                           return 1;
                         }
                         else {
                           moduleFlag[EXTERNAL_MODULE] = MODULE_NORMAL_MODE;
                           return 0;
                         }
                     });

      externalModuleRange = new TextButton(externalModuleWindow, grid.getFieldSlot(2, 1), STR_MODULE_RANGE,
                     [=]() -> uint8_t {
                       if(moduleFlag[EXTERNAL_MODULE] == MODULE_BIND) {
                         moduleFlag[EXTERNAL_MODULE] = MODULE_RANGECHECK;
                         externalModuleBind->setState(0);
                         return 1;
                       }
                       if (moduleFlag[EXTERNAL_MODULE] == MODULE_NORMAL_MODE) {
                         moduleFlag[EXTERNAL_MODULE] = MODULE_RANGECHECK;
                         return 1;
                       }
                       else {
                         moduleFlag[EXTERNAL_MODULE] = MODULE_NORMAL_MODE;
                         return 0;
                       }
                     });
    }
    grid.nextLine();

    // Failsafe
    if (IS_MODULE_PXX(EXTERNAL_MODULE) || IS_MODULE_R9M(EXTERNAL_MODULE)) {
      new StaticText(externalModuleWindow, grid.getLabelSlot(true), STR_FAILSAFE);
      failSafeChoice = new Choice(externalModuleWindow, grid.getFieldSlot(2, 0), STR_VFAILSAFE, 0, FAILSAFE_LAST,
                 GET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].failsafeMode),
                 [=](int32_t newValue) -> void {
                     g_model.moduleData[EXTERNAL_MODULE].failsafeMode = newValue;
                     SET_DIRTY();
                     updateExternalModuleWindow();
                     failSafeChoice->setFocus();
                 });
      if(g_model.moduleData[EXTERNAL_MODULE].failsafeMode == FAILSAFE_CUSTOM)
      {
        new TextButton(externalModuleWindow, grid.getFieldSlot(2, 1), STR_SET,
                       [=]() -> uint8_t {
                         // TODO launch the failsafe window
                         return 1;
                       });
      }
      grid.nextLine();
    }

    // R9M Power
    if (IS_MODULE_R9M_FCC(EXTERNAL_MODULE)) {
      new StaticText(externalModuleWindow, grid.getLabelSlot(true), STR_MULTI_RFPOWER);
      new Choice(externalModuleWindow, grid.getFieldSlot(), STR_R9M_FCC_POWER_VALUES, 0, R9M_FCC_POWER_MAX,
                 GET_SET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].pxx.power));
    }
    else if (IS_MODULE_R9M_LBT(EXTERNAL_MODULE)) {
      new StaticText(externalModuleWindow, grid.getLabelSlot(true), STR_MULTI_RFPOWER);
      new Choice(externalModuleWindow, grid.getFieldSlot(), STR_R9M_LBT_POWER_VALUES, 0, R9M_LBT_POWER_MAX,
                 GET_DEFAULT(min<uint8_t>(g_model.moduleData[EXTERNAL_MODULE].pxx.power, R9M_LBT_POWER_MAX)),
                 SET_DEFAULT(g_model.moduleData[EXTERNAL_MODULE].pxx.power));
    }
  }

  coord_t delta = externalModuleWindow->adjustHeight();
  externalModuleWindow->parent->moveWindowsTop(externalModuleWindow->rect.y, delta);
  externalModuleWindow->parent->innerHeight += delta;
}

  // Switch to external antenna confirmation
  //  bool newAntennaSel;
  //  if (warningResult) {
  //    warningResult = 0;
  //    g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna = XJT_EXTERNAL_ANTENNA;
  //  }

/*  if (menuEvent) {
    moduleFlag[0] = 0;
    moduleFlag[1] = 0;
  }
*/

#if 0

      case ITEM_MODEL_SLIDPOT_WARNING_STATE:
        drawText(window,MENUS_MARGIN_LEFT, y,STR_POTWARNINGSTATE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, PSTR("\004""OFF\0""Man\0""Auto"), g_model.potsWarnMode, attr);
        if (attr) {
          CHECK_INCDEC_MODELVAR(event, g_model.potsWarnMode, POTS_WARN_OFF, POTS_WARN_AUTO);
          storageDirty(EE_MODEL);
        }
        break;

      case ITEM_MODEL_POTS_WARNING:
      {
        drawText(window,MENUS_MARGIN_LEFT, y, STR_POTWARNING);
        if (attr) {
          if (!READ_ONLY() && menuHorizontalPosition >= 0 && event==EVT_KEY_LONG(KEY_ENTER)) {
            killEvents(event);
            if (g_model.potsWarnMode == POTS_WARN_MANUAL) {
              SAVE_POT_POSITION(menuHorizontalPosition);
              AUDIO_WARNING1();
              storageDirty(EE_MODEL);
            }
          }

          if (!READ_ONLY() &&  menuHorizontalPosition >= 0 && s_editMode && event==EVT_KEY_BREAK(KEY_ENTER)) {
            s_editMode = 0;
            g_model.potsWarnEnabled ^= (1 << (menuHorizontalPosition));
            storageDirty(EE_MODEL);
          }
        }

        if (attr && menuHorizontalPosition < 0) {
          lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, NUM_POTS*MODEL_SETUP_SLIDPOT_SPACING+INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
        }

        if (g_model.potsWarnMode) {
          coord_t x = MODEL_SETUP_2ND_COLUMN;
          for (int i=0; i<NUM_POTS; ++i) {
            LcdFlags flags = (((menuHorizontalPosition==i) && attr) ? INVERS : 0);
            flags |= (g_model.potsWarnEnabled & (1 << i)) ? TEXT_DISABLE_COLOR : TEXT_COLOR;
            if (attr && menuHorizontalPosition < 0) {
              flags |= INVERS;
            }
            lcdDrawTextAtIndex(x, y, STR_VSRCRAW, NUM_STICKS+1+i, flags);
            x += MODEL_SETUP_SLIDPOT_SPACING;
          }
        }
        break;
      }

      case ITEM_MODEL_SLIDERS_WARNING:
        drawText(window,MENUS_MARGIN_LEFT, y, STR_SLIDERWARNING);
        if (attr) {
          if (!READ_ONLY() && menuHorizontalPosition+1 && event==EVT_KEY_LONG(KEY_ENTER)) {
            killEvents(event);
            if (g_model.potsWarnMode == POTS_WARN_MANUAL) {
              SAVE_POT_POSITION(menuHorizontalPosition+NUM_POTS);
              AUDIO_WARNING1();
              storageDirty(EE_MODEL);
            }
          }

          if (!READ_ONLY() && menuHorizontalPosition+1 && s_editMode && event==EVT_KEY_BREAK(KEY_ENTER)) {
            s_editMode = 0;
            g_model.potsWarnEnabled ^= (1 << (menuHorizontalPosition+NUM_POTS));
            storageDirty(EE_MODEL);
          }
        }

        if (attr && menuHorizontalPosition < 0) {
          lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, NUM_SLIDERS*MODEL_SETUP_SLIDPOT_SPACING+INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
        }

        if (g_model.potsWarnMode) {
          coord_t x = MODEL_SETUP_2ND_COLUMN;
          for (int i=NUM_POTS; i<NUM_POTS+NUM_SLIDERS; ++i) {
            LcdFlags flags = (((menuHorizontalPosition==i-NUM_POTS) && attr) ? INVERS : 0);
            flags |= (g_model.potsWarnEnabled & (1 << i)) ? TEXT_DISABLE_COLOR : TEXT_COLOR;
            if (attr && menuHorizontalPosition < 0) {
              flags |= INVERS;
            }
            lcdDrawTextAtIndex(x, y, STR_VSRCRAW, NUM_STICKS+1+i, flags);
            x += MODEL_SETUP_SLIDPOT_SPACING;
          }
        }
        break;

      case ITEM_MODEL_USE_GLOBAL_FUNCTIONS:
        drawText(window,MENUS_MARGIN_LEFT, y, STR_USE_GLOBAL_FUNCS);
        drawCheckBox(MODEL_SETUP_2ND_COLUMN, y, !g_model.noGlobalFunctions, attr);
        if (attr) g_model.noGlobalFunctions = !checkIncDecModel(event, !g_model.noGlobalFunctions, 0, 1);
        break;

      case ITEM_MODEL_INTERNAL_MODULE_LABEL:
        drawText(window,MENUS_MARGIN_LEFT, y, TR_INTERNALRF);
        break;

      case ITEM_MODEL_INTERNAL_MODULE_MODE:
        drawText(window,MENUS_MARGIN_LEFT, y, STR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_XJT_PROTOCOLS, 1+g_model.moduleData[0].rfProtocol, attr);
        if (attr) {
          g_model.moduleData[INTERNAL_MODULE].rfProtocol = checkIncDec(event, g_model.moduleData[INTERNAL_MODULE].rfProtocol, RF_PROTO_OFF, RF_PROTO_LAST, EE_MODEL, isRfProtocolAvailable);
          if (checkIncDec_Ret) {
            g_model.moduleData[0].type = MODULE_TYPE_XJT;
            g_model.moduleData[0].channelsStart = 0;
            g_model.moduleData[0].channelsCount = DEFAULT_CHANNELS(INTERNAL_MODULE);
            if (g_model.moduleData[INTERNAL_MODULE].rfProtocol == RF_PROTO_OFF)
              g_model.moduleData[INTERNAL_MODULE].type = MODULE_TYPE_NONE;
          }

        }
        break;

      case ITEM_MODEL_INTERNAL_MODULE_ANTENNA:
        CHOICE(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_ANTENNASELECTION, STR_VANTENNATYPES, g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna, 0, 1);
#if 0
        if (newAntennaSel != g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna && newAntennaSel == XJT_EXTERNAL_ANTENNA) {
          POPUP_CONFIRMATION(STR_ANTENNACONFIRM1);
          const char * w = STR_ANTENNACONFIRM2;
          SET_WARNING_INFO(w, strlen(w), 0);
        }
        else {
          g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna = newAntennaSel;
        }
#endif
        break;

      case ITEM_MODEL_TRAINER_MODE:
        CHOICE(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_MODE, STR_VTRAINERMODES, g_model.trainerMode, 0, TRAINER_MODE_MAX());
#if defined(BLUETOOTH)
        if (attr && checkIncDec_Ret) {
          bluetoothState = BLUETOOTH_STATE_OFF;
          bluetoothDistantAddr[0] = 0;
        }
#endif
        break;

      case ITEM_MODEL_EXTERNAL_MODULE_LABEL:
        drawText(window,MENUS_MARGIN_LEFT, y, TR_EXTERNALRF);
        break;

      case ITEM_MODEL_EXTERNAL_MODULE_MODE:
        drawText(window,MENUS_MARGIN_LEFT, y, STR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_TARANIS_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].type, menuHorizontalPosition==0 ? attr : 0);
        if (IS_MODULE_XJT(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_XJT_PROTOCOLS, 1+g_model.moduleData[EXTERNAL_MODULE].rfProtocol, (menuHorizontalPosition==1 ? attr : 0));
        else if (IS_MODULE_DSM2(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_DSM_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, (menuHorizontalPosition==1 ? attr : 0));
        else if (IS_MODULE_R9M(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_R9M_MODES, g_model.moduleData[EXTERNAL_MODULE].subType, (menuHorizontalPosition==1 ? attr : 0));
#if defined(MULTIMODULE)
        else if (IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) {
          int multi_rfProto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
          if (g_model.moduleData[EXTERNAL_MODULE].multi.customProto) {
            drawText(window,MODEL_SETUP_3RD_COLUMN, y, STR_MULTI_CUSTOM, menuHorizontalPosition == 1 ? attr : 0);
            lcdDrawNumber(MODEL_SETUP_4TH_COLUMN, y, multi_rfProto, menuHorizontalPosition==2 ? attr : 0, 2);
            lcdDrawNumber(MODEL_SETUP_4TH_COLUMN + MODEL_SETUP_BIND_OFS, y, g_model.moduleData[EXTERNAL_MODULE].subType, menuHorizontalPosition==3 ? attr : 0, 2);
          }
          else {
            const mm_protocol_definition * pdef = getMultiProtocolDefinition(multi_rfProto);
            lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_MULTI_PROTOCOLS, multi_rfProto, menuHorizontalPosition == 1 ? attr : 0);
            if (pdef->subTypeString != nullptr)
              lcdDrawTextAtIndex(MODEL_SETUP_4TH_COLUMN, y, pdef->subTypeString, g_model.moduleData[EXTERNAL_MODULE].subType, menuHorizontalPosition==2 ? attr : 0);
          }
        }
#endif
        if (attr && s_editMode>0) {
          switch (menuHorizontalPosition) {
            case 0:
              g_model.moduleData[EXTERNAL_MODULE].type = checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].type, MODULE_TYPE_NONE, MODULE_TYPE_COUNT-1, EE_MODEL, isModuleAvailable);
              if (checkIncDec_Ret) {
                g_model.moduleData[EXTERNAL_MODULE].rfProtocol = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = DEFAULT_CHANNELS(EXTERNAL_MODULE);
                if (IS_MODULE_SBUS(EXTERNAL_MODULE))
                  g_model.moduleData[EXTERNAL_MODULE].sbus.refreshRate = -31;
              }
              break;
            case 1:
              if (IS_MODULE_DSM2(EXTERNAL_MODULE))
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, DSM2_PROTO_LP45, DSM2_PROTO_DSMX);
              else if (IS_MODULE_R9M(EXTERNAL_MODULE))
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, MODULE_SUBTYPE_R9M_FCC, MODULE_SUBTYPE_R9M_LBT);
#if defined(MULTIMODULE)
              else if (IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) {
                int multiRfProto = g_model.moduleData[EXTERNAL_MODULE].multi.customProto == 1 ? MM_RF_PROTO_CUSTOM : g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
                CHECK_INCDEC_MODELVAR(event, multiRfProto, MM_RF_PROTO_FIRST, MM_RF_PROTO_LAST);
                if (checkIncDec_Ret) {
                  g_model.moduleData[EXTERNAL_MODULE].multi.customProto = (multiRfProto == MM_RF_PROTO_CUSTOM);
                  if (!g_model.moduleData[EXTERNAL_MODULE].multi.customProto)
                    g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(multiRfProto);
                  g_model.moduleData[EXTERNAL_MODULE].subType = 0;
                  // Sensible default for DSM2 (same as for ppm): 7ch@22ms + Autodetect settings enabled
                  if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MM_RF_PROTO_DSM2) {
                    g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 1;
                  }
                  else {
                    g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 0;
                  }
                  g_model.moduleData[EXTERNAL_MODULE].multi.optionValue = 0;
                }
              }
#endif
              else {
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, RF_PROTO_X16, RF_PROTO_LAST);
              }
              if (checkIncDec_Ret) {
                g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = DEFAULT_CHANNELS(EXTERNAL_MODULE);
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = MAX_EXTERNAL_MODULE_CHANNELS();
              }
              break;
#if defined(MULTIMODULE)
            case 2:
              if (g_model.moduleData[EXTERNAL_MODULE].multi.customProto) {
                g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false), 0, 63, EE_MODEL));
                break;
              } else {
                const mm_protocol_definition *pdef = getMultiProtocolDefinition(g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false));
                if (pdef->maxSubtype > 0)
                  CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, pdef->maxSubtype);
              }
              break;
            case 3:
              // Custom protocol, third column is subtype
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, 7);
              break;
#endif
          }
        }
        break;

      case ITEM_MODEL_TRAINER_LABEL:
        drawText(window,MENUS_MARGIN_LEFT, y, STR_TRAINER);
        break;

      case ITEM_MODEL_TRAINER_LINE1:
#if defined(BLUETOOTH)
        if (g_model.trainerMode == TRAINER_MODE_MASTER_BLUETOOTH) {
          if (attr) {
            s_editMode = 0;
          }
          if (bluetoothDistantAddr[0]) {
            drawText(window,MENUS_MARGIN_LEFT + INDENT_WIDTH, y, bluetoothDistantAddr);
            if (bluetoothState != BLUETOOTH_STATE_CONNECTED) {
              drawButton(MODEL_SETUP_2ND_COLUMN, y, "Bind", menuHorizontalPosition == 0 ? attr : 0);
              drawButton(MODEL_SETUP_2ND_COLUMN+60, y, "Clear", menuHorizontalPosition == 1 ? attr : 0);
            }
            else {
              drawButton(MODEL_SETUP_2ND_COLUMN, y, "Clear", attr);
            }
            if (attr && event == EVT_KEY_FIRST(KEY_ENTER)) {
              if (bluetoothState == BLUETOOTH_STATE_CONNECTED || menuHorizontalPosition == 1) {
                bluetoothState = BLUETOOTH_STATE_OFF;
                bluetoothDistantAddr[0] = 0;
              }
              else {
                bluetoothState = BLUETOOTH_STATE_BIND_REQUESTED;
              }
            }
          }
          else {
            drawText(window,MENUS_MARGIN_LEFT + INDENT_WIDTH, y, "---");
            if (bluetoothState < BLUETOOTH_STATE_IDLE)
              drawButton(MODEL_SETUP_2ND_COLUMN, y, STR_BLUETOOTH_INIT, attr);
            else
              drawButton(MODEL_SETUP_2ND_COLUMN, y, STR_BLUETOOTH_DISC, attr);
            if (attr && event == EVT_KEY_FIRST(KEY_ENTER)) {
              if (bluetoothState < BLUETOOTH_STATE_IDLE)
                bluetoothState = BLUETOOTH_STATE_OFF;
              else
                bluetoothState = BLUETOOTH_STATE_DISCOVER_REQUESTED;
            }
          }
          break;
        }
        // no break
#endif

      case ITEM_MODEL_INTERNAL_MODULE_CHANNELS:
      case ITEM_MODEL_EXTERNAL_MODULE_CHANNELS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        drawText(window,MENUS_MARGIN_LEFT, y, STR_CHANNELRANGE);
        if ((int8_t)PORT_CHANNELS_ROWS(moduleIdx) >= 0) {
          drawStringWithIndex(MODEL_SETUP_2ND_COLUMN, y, STR_CH, moduleData.channelsStart+1, menuHorizontalPosition==0 ? attr : 0);
          drawText(window,lcdNextPos+5, y, "-");
          drawStringWithIndex(lcdNextPos+5, y, STR_CH, moduleData.channelsStart+NUM_CHANNELS(moduleIdx), menuHorizontalPosition==1 ? attr : 0);
          if (IS_R9M_OR_XJTD16(moduleIdx)) {
            if (NUM_CHANNELS(moduleIdx) > 8)
              drawText(window,lcdNextPos + 15, y, "(18ms)");
            else
              drawText(window,lcdNextPos + 15, y, "(9ms)");
          }
          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.channelsStart, 32-8-moduleData.channelsCount);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.channelsCount, -4, min<int8_t>(MAX_CHANNELS(moduleIdx), 32-8-moduleData.channelsStart));
                if ((i == ITEM_MODEL_EXTERNAL_MODULE_CHANNELS && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_PPM)
                    || (i == ITEM_MODEL_TRAINER_LINE1)
                    )
                  SET_DEFAULT_PPM_FRAME_LENGTH(moduleIdx);
                break;
            }
          }
        }
        break;
      }

      case ITEM_MODEL_INTERNAL_MODULE_BIND:
      case ITEM_MODEL_EXTERNAL_MODULE_BIND:
      case ITEM_MODEL_TRAINER_LINE2:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        if (IS_MODULE_PPM(moduleIdx)) {
          drawText(window,MENUS_MARGIN_LEFT, y, STR_PPMFRAME);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.ppm.frameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT, 0, NULL, STR_MS);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+80, y, (moduleData.ppm.delay*50)+300, (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr|LEFT : LEFT, 0, NULL, "us");
          drawText(window,MODEL_SETUP_2ND_COLUMN+160, y, moduleData.ppm.pulsePol ? "+" : "-", (CURSOR_ON_LINE() || menuHorizontalPosition==2) ? attr : 0);
          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.frameLength, -20, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.delay, -4, 10);
                break;
              case 2:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.ppm.pulsePol, 1);
                break;
            }
          }
        }
        else if (IS_MODULE_SBUS(moduleIdx)) {
          drawText(window,MENUS_MARGIN_LEFT, y, STR_REFRESHRATE);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.ppm.frameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT, 0, NULL, STR_MS);
          drawText(window,MODEL_SETUP_3RD_COLUMN, y, moduleData.sbus.noninverted ? "not inverted" : "normal", (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0);

          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.frameLength, -33, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.sbus.noninverted, 1);
                break;
            }
          }
        }
        else {
          int l_posHorz = menuHorizontalPosition;
          coord_t xOffsetBind = MODEL_SETUP_BIND_OFS;
          if (IS_MODULE_XJT(moduleIdx) && IS_D8_RX(moduleIdx)) {
            xOffsetBind = 0;
            drawText(window,MENUS_MARGIN_LEFT, y, STR_RECEIVER);
            if (attr) l_posHorz += 1;
          }
          else {
            drawText(window,MENUS_MARGIN_LEFT, y, STR_RECEIVER_NUM);
          }
          if (IS_MODULE_PXX(moduleIdx) || IS_MODULE_DSM2(moduleIdx) || IS_MODULE_MULTIMODULE(moduleIdx)) {
            if (xOffsetBind)
              lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.header.modelId[moduleIdx], (l_posHorz==0 ? attr : 0) | LEADING0 | LEFT, 2);
            if (attr && l_posHorz==0 && s_editMode>0)
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.header.modelId[moduleIdx], MAX_RX_NUM(moduleIdx));
            drawButton(MODEL_SETUP_2ND_COLUMN+xOffsetBind, y, STR_MODULE_BIND, (moduleFlag[moduleIdx] == MODULE_BIND ? BUTTON_ON : BUTTON_OFF) | (l_posHorz==1 ? attr : 0));
            drawButton(MODEL_SETUP_2ND_COLUMN+MODEL_SETUP_RANGE_OFS+xOffsetBind, y, STR_MODULE_RANGE, (moduleFlag[moduleIdx] == MODULE_RANGECHECK ? BUTTON_ON : BUTTON_OFF) | (l_posHorz==2 ? attr : 0));
            uint8_t newFlag = 0;
#if defined(MULTIMODULE)
            if (multiBindStatus == MULTI_BIND_FINISHED) {
              multiBindStatus = MULTI_NORMAL_OPERATION;
              s_editMode = 0;
            }
#endif
            if (attr && l_posHorz>0) {
              if (s_editMode>0) {
                if (l_posHorz == 1) {
                  if (IS_MODULE_R9M(moduleIdx) || (IS_MODULE_XJT(moduleIdx) && g_model.moduleData[moduleIdx].rfProtocol == RF_PROTO_X16)) {
                    if (event == EVT_KEY_BREAK(KEY_ENTER)) {
                      uint8_t default_selection;
                      if (IS_MODULE_R9M_LBT(moduleIdx)) {
                        if (!IS_TELEMETRY_INTERNAL_MODULE())
                          POPUP_MENU_ADD_ITEM(STR_BINDING_25MW_CH1_8_TELEM_ON);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_25MW_CH1_8_TELEM_OFF);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_500MW_CH1_8_TELEM_OFF);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_500MW_CH9_16_TELEM_OFF);
                        default_selection = 2;
                      }
                      else {
                        if (!(IS_TELEMETRY_INTERNAL_MODULE() && moduleIdx == EXTERNAL_MODULE))
                          POPUP_MENU_ADD_ITEM(STR_BINDING_1_8_TELEM_ON);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_1_8_TELEM_OFF);
                        if (!(IS_TELEMETRY_INTERNAL_MODULE() && moduleIdx == EXTERNAL_MODULE))
                          POPUP_MENU_ADD_ITEM(STR_BINDING_9_16_TELEM_ON);
                        POPUP_MENU_ADD_ITEM(STR_BINDING_9_16_TELEM_OFF);
                        default_selection = g_model.moduleData[moduleIdx].pxx.receiver_telem_off + (g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 << 1);
                      }
                      POPUP_MENU_SELECT_ITEM(default_selection);
                      POPUP_MENU_START(onBindMenu);
                      continue;
                    }
                    if (moduleFlag[moduleIdx] == MODULE_BIND) {
                      newFlag = MODULE_BIND;
                    }
                    else {
                      if (!popupMenuNoItems) {
                        s_editMode = 0;  // this is when popup is exited before a choice is made
                      }
                    }
                  }
                  else {
                    newFlag = MODULE_BIND;
                  }
                }
                else if (l_posHorz == 2) {
                  newFlag = MODULE_RANGECHECK;
                }
              }
            }
            moduleFlag[moduleIdx] = newFlag;
#if defined(MULTIMODULE)
            if (newFlag == MODULE_BIND)
              multiBindStatus = MULTI_BIND_INITIATED;
#endif
          }
        }
        break;
      }

      case ITEM_MODEL_INTERNAL_MODULE_FAILSAFE:
      case ITEM_MODEL_EXTERNAL_MODULE_FAILSAFE:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        drawText(window,MENUS_MARGIN_LEFT, y, STR_FAILSAFE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VFAILSAFE, moduleData.failsafeMode, menuHorizontalPosition==0 ? attr : 0);
        if (moduleData.failsafeMode == FAILSAFE_CUSTOM) {
          drawButton(MODEL_SETUP_2ND_COLUMN + MODEL_SETUP_SET_FAILSAFE_OFS, y, STR_SET, menuHorizontalPosition==1 ? attr : 0);
        }
        if (attr) {
          if (moduleData.failsafeMode != FAILSAFE_CUSTOM)
            menuHorizontalPosition = 0;
          if (menuHorizontalPosition==0) {
            if (s_editMode>0) {
              CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.failsafeMode, FAILSAFE_LAST);
              if (checkIncDec_Ret) SEND_FAILSAFE_NOW(moduleIdx);
            }
          }
          else if (menuHorizontalPosition==1) {
            s_editMode = 0;
            if (moduleData.failsafeMode==FAILSAFE_CUSTOM && event==EVT_KEY_FIRST(KEY_ENTER)) {
              g_moduleIdx = moduleIdx;
              pushMenu(menuModelFailsafe);
            }
          }
          else {
            lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN, y, LCD_W - MODEL_SETUP_2ND_COLUMN - 2, 8, TEXT_COLOR);
          }
        }
        break;
      }

      case ITEM_MODEL_EXTERNAL_MODULE_OPTIONS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
#if defined(MULTIMODULE)
        if (IS_MODULE_MULTIMODULE(moduleIdx)) {
          int optionValue = g_model.moduleData[moduleIdx].multi.optionValue;

          const uint8_t multi_proto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true);
          const mm_protocol_definition *pdef = getMultiProtocolDefinition(multi_proto);
          if (pdef->optionsstr)
            drawText(window,MENUS_MARGIN_LEFT, y, pdef->optionsstr);

          if (multi_proto == MM_RF_PROTO_FS_AFHDS2A)
            optionValue = 50 + 5 * optionValue;

          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, optionValue, LEFT | attr);
          if (attr) {
            if (multi_proto == MM_RF_PROTO_FS_AFHDS2A) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, 0, 70);
            }
            else if (multi_proto == MM_RF_PROTO_OLRS) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -1, 7);
            }
            else {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -128, 127);
            }
          }
        }
#endif
        if (IS_MODULE_R9M(moduleIdx)) {
          drawText(window,MENUS_MARGIN_LEFT, y, STR_MODULE_TELEMETRY);
          if (IS_TELEMETRY_INTERNAL_MODULE()) {
            drawText(window,MODEL_SETUP_2ND_COLUMN, y, STR_DISABLE_INTERNAL);
          }
          else {
            drawText(window,MODEL_SETUP_2ND_COLUMN, y, STR_MODULE_TELEM_ON);
          }
        }
        else if (IS_MODULE_SBUS(moduleIdx)) {
          drawText(window,MENUS_MARGIN_LEFT, y, STR_WARN_BATTVOLTAGE);
          drawValueWithUnit(MODEL_SETUP_4TH_COLUMN, y, getBatteryVoltage(), UNIT_VOLTS, attr|PREC2|LEFT);
        }
      }
      break;

      case ITEM_MODEL_EXTERNAL_MODULE_BIND_OPTIONS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);

        drawText(window,MENUS_MARGIN_LEFT+ INDENT_WIDTH, y, "Bind mode");
        if (g_model.moduleData[moduleIdx].pxx.power == R9M_LBT_POWER_25) {
          if(g_model.moduleData[moduleIdx].pxx.receiver_telem_off == true)
            drawText(window,MODEL_SETUP_2ND_COLUMN, y, STR_BINDING_25MW_CH1_8_TELEM_OFF);
          else
            drawText(window,MODEL_SETUP_2ND_COLUMN, y, STR_BINDING_25MW_CH1_8_TELEM_ON);
        }
        else {
          if(g_model.moduleData[moduleIdx].pxx.receiver_channel_9_16 == true)
            drawText(window,MODEL_SETUP_2ND_COLUMN, y, STR_BINDING_500MW_CH9_16_TELEM_OFF);
          else
            drawText(window,MODEL_SETUP_2ND_COLUMN, y, STR_BINDING_500MW_CH1_8_TELEM_OFF);
        }
        while (menuVerticalPosition==i && menuHorizontalPosition > 0) {
          REPEAT_LAST_CURSOR_MOVE(ITEM_MODEL_SETUP_MAX, true);
        }
      }

      case ITEM_MODEL_EXTERNAL_MODULE_POWER:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(i);
        if (IS_MODULE_R9M_FCC(moduleIdx)) {
          // Power selection is only available on R9M FCC
          drawText(window,MENUS_MARGIN_LEFT, y, STR_MULTI_RFPOWER);
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_FCC_POWER_VALUES, g_model.moduleData[moduleIdx].pxx.power, LEFT | attr);
          if (attr)
            CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].pxx.power, 0, R9M_FCC_POWER_MAX);
        }
#if defined(MULTIMODULE)
        else if (IS_MODULE_MULTIMODULE(moduleIdx)) {
          drawText(window,MENUS_MARGIN_LEFT, y, STR_MULTI_LOWPOWER);
          g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode = editCheckBox(window, g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        }
#endif
      }
      break;

#if defined(MULTIMODULE)
    case ITEM_MODEL_EXTERNAL_MODULE_AUTOBIND:
      if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MM_RF_PROTO_DSM2)
        drawText(window,MENUS_MARGIN_LEFT, y, STR_MULTI_DSM_AUTODTECT);
      else
        drawText(window,MENUS_MARGIN_LEFT, y, STR_MULTI_AUTOBIND);
      g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = editCheckBox(window, g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode, MODEL_SETUP_2ND_COLUMN, y, attr, event);
      break;
    case ITEM_MODEL_EXTERNAL_MODULE_STATUS: {
      drawText(window,MENUS_MARGIN_LEFT, y, STR_MODULE_STATUS);

      char statusText[64];
      multiModuleStatus.getStatusString(statusText);
      drawText(window,MODEL_SETUP_2ND_COLUMN, y, statusText);
      break;
    case ITEM_MODEL_EXTERNAL_MODULE_SYNCSTATUS: {
      drawText(window,MENUS_MARGIN_LEFT, y, STR_MODULE_SYNC);

      char statusText[64];
      multiSyncStatus.getRefreshString(statusText);
      drawText(window,MODEL_SETUP_2ND_COLUMN, y, statusText);
      break;
      }
    }
#endif
    }

    y += 40;
  }

  if (IS_RANGECHECK_ENABLE()) {
    theme->drawMessageBox("RSSI :", NULL, NULL, WARNING_TYPE_INFO);
    lcdDrawNumber(WARNING_LINE_X, WARNING_INFOLINE_Y, TELEMETRY_RSSI(), DBLSIZE|LEFT);
  }
#endif


bool menuModelFailsafe(event_t event)
{
  uint8_t ch = 0;
  const uint8_t channelStart = g_model.moduleData[g_moduleIdx].channelsStart;
  const int lim = (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;
  const uint8_t SLIDER_W = 128;
  const uint8_t cols = NUM_CHANNELS(g_moduleIdx) > 8 ? 2 : 1;

  if (event == EVT_KEY_LONG(KEY_ENTER)) {
    killEvents(event);
    event = 0;
    if (s_editMode) {
      g_model.moduleData[g_moduleIdx].failsafeChannels[menuVerticalPosition] = channelOutputs[menuVerticalPosition +
                                                                                              channelStart];
      storageDirty(EE_MODEL);
      AUDIO_WARNING1();
      s_editMode = 0;
      SEND_FAILSAFE_NOW(g_moduleIdx);
    } else {
      int16_t &failsafe = g_model.moduleData[g_moduleIdx].failsafeChannels[menuVerticalPosition];
      if (failsafe < FAILSAFE_CHANNEL_HOLD)
        failsafe = FAILSAFE_CHANNEL_HOLD;
      else if (failsafe == FAILSAFE_CHANNEL_HOLD)
        failsafe = FAILSAFE_CHANNEL_NOPULSE;
      else
        failsafe = 0;
      storageDirty(EE_MODEL);
      AUDIO_WARNING1();
      SEND_FAILSAFE_NOW(g_moduleIdx);
    }
  }

  SIMPLE_SUBMENU_WITH_OPTIONS("FAILSAFE", ICON_STATS_ANALOGS, NUM_CHANNELS(g_moduleIdx), OPTION_MENU_NO_SCROLLBAR);
  drawStringWithIndex(50, 3 + FH, "Module", g_moduleIdx + 1, MENU_TITLE_COLOR);

  for (uint8_t col = 0; col < cols; col++) {
    for (uint8_t line = 0; line < 8; line++) {
      coord_t x = col * (LCD_W / 2);
      const coord_t y = MENU_CONTENT_TOP - FH + line * (FH + 4);
      const int32_t channelValue = channelOutputs[ch + channelStart];
      int32_t failsafeValue = g_model.moduleData[g_moduleIdx].failsafeChannels[8 * col + line];

      // Channel name if present, number if not
      if (g_model.limitData[ch + channelStart].name[0] != '\0') {
        putsChn(x + MENUS_MARGIN_LEFT, y - 3, ch + 1, TINSIZE);
        lcdDrawSizedText(x + MENUS_MARGIN_LEFT, y + 5, g_model.limitData[ch + channelStart].name,
                         sizeof(g_model.limitData[ch + channelStart].name), ZCHAR | SMLSIZE);
      } else {
        putsChn(x + MENUS_MARGIN_LEFT, y, ch + 1, 0);
      }

      // Value
      LcdFlags flags = RIGHT;
      if (menuVerticalPosition == ch) {
        flags |= INVERS;
        if (s_editMode) {
          if (failsafeValue == FAILSAFE_CHANNEL_HOLD || failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
            s_editMode = 0;
          } else {
            flags |= BLINK;
            CHECK_INCDEC_MODELVAR(event, g_model.moduleData[g_moduleIdx].failsafeChannels[8 * col + line], -lim, +lim);
          }
        }
      }

      x += (LCD_W / 2) - 4 - MENUS_MARGIN_LEFT - SLIDER_W;

      if (failsafeValue == FAILSAFE_CHANNEL_HOLD) {
        lcd->drawText(x, y + 2, "HOLD", flags | SMLSIZE);
        failsafeValue = 0;
      } else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
        lcd->drawText(x, y + 2, "NONE", flags | SMLSIZE);
        failsafeValue = 0;
      } else {
#if defined(PPM_UNIT_US)
        lcdDrawNumber(x, y, PPM_CH_CENTER(ch)+failsafeValue/2, flags);
#elif defined(PPM_UNIT_PERCENT_PREC1)
        lcdDrawNumber(x, y, calcRESXto1000(failsafeValue), PREC1 | flags);
#else
        lcdDrawNumber(x, y, calcRESXto1000(failsafeValue)/10, flags);
#endif
      }

      // Gauge
      x += 4;
      lcdDrawRect(x, y + 3, SLIDER_W + 1, 12);
      const coord_t lenChannel = limit((uint8_t) 1, uint8_t((abs(channelValue) * SLIDER_W / 2 + lim / 2) / lim),
                                       uint8_t(SLIDER_W / 2));
      const coord_t lenFailsafe = limit((uint8_t) 1, uint8_t((abs(failsafeValue) * SLIDER_W / 2 + lim / 2) / lim),
                                        uint8_t(SLIDER_W / 2));
      x += SLIDER_W / 2;
      const coord_t xChannel = (channelValue > 0) ? x : x + 1 - lenChannel;
      const coord_t xFailsafe = (failsafeValue > 0) ? x : x + 1 - lenFailsafe;
      lcdDrawSolidFilledRect(xChannel, y + 4, lenChannel, 5, TEXT_COLOR);
      lcdDrawSolidFilledRect(xFailsafe, y + 9, lenFailsafe, 5, ALARM_COLOR);

      if (++ch >= NUM_CHANNELS(g_moduleIdx))
        break;

    }
  }

  return true;
}
