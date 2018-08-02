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
#include "radio_hardware.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

#define SWITCH_TYPE_MAX(sw)            ((MIXSRC_SF-MIXSRC_FIRST_SWITCH == sw || MIXSRC_SH-MIXSRC_FIRST_SWITCH == sw) ? SWITCH_2POS : SWITCH_3POS)

RadioHardwarePage::RadioHardwarePage():
  PageTab(STR_HARDWARE, ICON_RADIO_HARDWARE)
{
}

void RadioHardwarePage::build(Window * window)
{
  GridLayout grid(*window);
  grid.spacer(8);
  grid.setLabelWidth(80);

  // Sticks
  new Subtitle(window, grid.getLineSlot(), STR_STICKS);
  grid.nextLine();
  for(int i=0; i < NUM_STICKS; i++){
    new StaticText(window, grid.getLabelSlot(true), TEXT_AT_INDEX(STR_VSRCRAW, (i+1)));
    new TextEdit(window, grid.getFieldSlot(2,0), g_eeGeneral.anaNames[i], LEN_ANA_NAME);
    grid.nextLine();
  }

    //Pots
  new Subtitle(window, grid.getLineSlot(), STR_POTS);
  grid.nextLine();
  for(int i=0; i < NUM_POTS; i++){
    new StaticText(window, grid.getLabelSlot(true), TEXT_AT_INDEX(STR_VSRCRAW, (i+NUM_STICKS+1)));
    new TextEdit(window, grid.getFieldSlot(2,0), g_eeGeneral.anaNames[i+NUM_STICKS], LEN_ANA_NAME);
    new Choice(window, grid.getFieldSlot(2,1), STR_POTTYPES, POT_NONE, POT_WITHOUT_DETENT+1,GET_SET_BF(g_eeGeneral.potsConfig, 2 * i, 2));
    grid.nextLine();
  }

  //Switches
  new Subtitle(window, grid.getLineSlot(), STR_SWITCHES);
  grid.nextLine();
  for(int i=0; i < NUM_SWITCHES; i++) {
    new StaticText(window, grid.getLabelSlot(true),
                   TEXT_AT_INDEX(STR_VSRCRAW, (i + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1)));
    new TextEdit(window, grid.getFieldSlot(2, 0), g_eeGeneral.anaNames[i + MIXSRC_FIRST_SWITCH - MIXSRC_Rud],
                 LEN_ANA_NAME);
    new Choice(window, grid.getFieldSlot(2, 1), STR_SWTYPES, SWITCH_NONE, SWITCH_TYPE_MAX(i) + 1, GET_SET_BF(g_eeGeneral.switchConfig, 2 * i, 2));
    grid.nextLine();
  }

  grid.setLabelWidth(150);

  // ADC filter
  new StaticText(window, grid.getLabelSlot(), STR_JITTER_FILTER);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_eeGeneral.jitterFilter));
  grid.nextLine();

  // Bat calibration
  new StaticText(window, grid.getLabelSlot(), STR_BATT_CALIB);
  auto batCal = new NumberEdit(window, grid.getFieldSlot(), -127, 127, GET_SET_DEFAULT(g_eeGeneral.txVoltageCalibration));
  batCal->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
      drawNumber(dc, 2, 2, getBatteryVoltage(), flags | PREC2, 0, nullptr, "V");
  });
  grid.nextLine();

  window->setInnerHeight(grid.getWindowHeight());
}

