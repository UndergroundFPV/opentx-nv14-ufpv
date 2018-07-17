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
#include "model_flightmodes.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

ModelFlightModesPage::ModelFlightModesPage():
  PageTab(STR_MENUFLIGHTMODES, ICON_MODEL_FLIGHT_MODES)
{
}

FlightModesType editFlightModes(coord_t x, coord_t y, event_t event, FlightModesType value, uint8_t attr)
{
  // displayFlightModes(x, y, value, attr);

  if (attr) {
    if (s_editMode && event==EVT_KEY_BREAK(KEY_ENTER)) {
      s_editMode = 0;
      value ^= (1<<menuHorizontalPosition);
      storageDirty(EE_MODEL);
    }
  }

  return value;
}

bool isTrimModeAvailable(int mode)
{
  return (mode < 0 || (mode%2) == 0 || (mode/2) != menuVerticalPosition);
}

void ModelFlightModesPage::build(Window * window)
{
  GridLayout grid(*window);
  grid.setLabelWidth(50);
  grid.spacer();

  // TODO
  /* if (menuVerticalPosition<MAX_FLIGHT_MODES && menuHorizontalPosition>=0) {
    drawColumnHeader(STR_PHASES_HEADERS, NULL, menuHorizontalPosition);
  }*/

  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    char label[16];
    getFlightModeString(label, i+1);
    new StaticText(window, grid.getLabelSlot(), label); // TODO (getFlightMode()==k ? BOLD : 0)
    new TextEdit(window, grid.getFieldSlot(3, 0), g_model.flightModeData[i].name, LEN_FLIGHT_MODE_NAME);
    new SwitchChoice(window, grid.getFieldSlot(3, 1), MixesContext, GET_SET_DEFAULT(g_model.flightModeData[i].swtch));
    // TODOif (k == 0)
    //      lcdDrawText(FLIGHT_MODES_SWITCH_COLUMN, y, "N/A");
    grid.nextLine();
  }

  char label[32];
  sprintf(label, "Check FM%d Trims", mixerCurrentFlightMode);
  new TextButton(window, { 60, grid.getWindowHeight() + 5, LCD_W - 120, 30 }, label,
                 [&]() -> uint8_t {
                   if (trimsCheckTimer)
                     trimsCheckTimer = 0;
                   else
                     trimsCheckTimer = 200; // 2 seconds trims cancelled
                   return trimsCheckTimer;
                 });

  window->setInnerHeight(grid.getWindowHeight());
}

#if 0
bool menuModelFlightModesAll(event_t event)
{
  if (menuVerticalPosition==0 && menuHorizontalPosition==ITEM_FLIGHT_MODES_SWITCH) {
    menuHorizontalPosition += CURSOR_MOVED_LEFT(event) ? -1 : +1;
  }

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_CONTENT_TOP + i*FH;
    int k = i+menuVerticalOffset;

    FlightModeData * p = flightModeAddress(k);

    drawFlightMode(MENUS_MARGIN_LEFT, y, k+1, (getFlightMode()==k ? BOLD : 0) | ((menuVerticalPosition==k && menuHorizontalPosition<0) ? INVERS : 0));

    for (uint8_t j=0; j<ITEM_FLIGHT_MODES_COUNT; j++) {


        case ITEM_FLIGHT_MODES_TRIM_RUD:
        case ITEM_FLIGHT_MODES_TRIM_ELE:
        case ITEM_FLIGHT_MODES_TRIM_THR:
        case ITEM_FLIGHT_MODES_TRIM_AIL:
        case ITEM_FLIGHT_MODES_TRIM_T5:
        case ITEM_FLIGHT_MODES_TRIM_T6:
        {
          uint8_t t = j-ITEM_FLIGHT_MODES_TRIM_RUD;
          if (active) {
            trim_t & v = p->trim[t];
            v.mode = checkIncDec(event, v.mode==TRIM_MODE_NONE ? -1 : v.mode, -1, k==0 ? 0 : 2*MAX_FLIGHT_MODES-1, EE_MODEL, isTrimModeAvailable);
          }
          drawTrimMode(FLIGHT_MODES_TRIMS_COLUMN+j*FLIGHT_MODES_TRIM_WIDTH, y, k, t, attr);
          break;
        }

        case ITEM_FLIGHT_MODES_FADE_IN:
          if (active) p->fadeIn = checkIncDec(event, p->fadeIn, 0, DELAY_MAX, EE_MODEL|NO_INCDEC_MARKS);
          lcdDrawNumber(FLIGHT_MODES_FADEIN_COLUMN, y, (10/DELAY_STEP)*p->fadeIn, attr|PREC1|RIGHT);
          break;

        case ITEM_FLIGHT_MODES_FADE_OUT:
          if (active) p->fadeOut = checkIncDec(event, p->fadeOut, 0, DELAY_MAX, EE_MODEL|NO_INCDEC_MARKS);
          lcdDrawNumber(FLIGHT_MODES_FADEOUT_COLUMN, y, (10/DELAY_STEP)*p->fadeOut, attr|PREC1|RIGHT);
          break;

      }
    }
  }

  return true;
}
#endif