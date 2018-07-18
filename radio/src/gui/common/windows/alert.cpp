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

#define ALERT_FRAME_TOP           70
#define ALERT_FRAME_HEIGHT        250
#define ALERT_FRAME_PADDING       10

#define ALERT_BITMAP_PADDING      15

#define ALERT_TITLE_LEFT          135
#define ALERT_TITLE_LINE_HEIGHT   30

#define ALERT_MESSAGE_TOP         210

#define ALERT_ACTION_TOP          230

#define ALERT_BUTTON_TOP          300

Alert::Alert(uint8_t type, const char * title, const char * message, const char * action):
  Window(&mainWindow, {0, 0, LCD_W, LCD_H}),
  type(type),
  title(title ? strdup(title) : nullptr),
  message(message ? strdup(message) : nullptr),
  action(action ? strdup(action) : nullptr)
{
  new FabIconButton(this, LCD_W - 50, ALERT_BUTTON_TOP, ICON_NEXT,
                    [=]() -> uint8_t {
                      this->deleteLater();
                      return 0;
                    });
}

Alert::~Alert()
{
  free(title);
  free(message);
  free(action);
}

void Alert::paint(BitmapBuffer * dc)
{
  theme->drawBackground();

  if (type == WARNING_TYPE_ALERT || type == WARNING_TYPE_ASTERISK)
    dc->drawBitmap(ALERT_BITMAP_PADDING, ALERT_FRAME_TOP + ALERT_BITMAP_PADDING, theme->asterisk);
  else if (type == WARNING_TYPE_INFO)
    dc->drawBitmap(ALERT_BITMAP_PADDING, ALERT_FRAME_TOP + ALERT_BITMAP_PADDING, theme->busy);
  else
    dc->drawBitmap(ALERT_BITMAP_PADDING, ALERT_FRAME_TOP + ALERT_BITMAP_PADDING, theme->question);

  if (type == WARNING_TYPE_ALERT) {
#if defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_IT) || defined(TRANSLATIONS_CZ)
    dc->drawText(ALERT_TITLE_LEFT, ALERT_FRAME_TOP + ALERT_FRAME_PADDING, STR_WARNING, ALARM_COLOR|DBLSIZE);
    dc->drawText(ALERT_TITLE_LEFT, ALERT_FRAME_TOP + ALERT_FRAME_PADDING + ALERT_TITLE_LINE_HEIGHT, title, ALARM_COLOR|DBLSIZE);
#else
    dc->drawText(ALERT_TITLE_LEFT, ALERT_FRAME_TOP + ALERT_FRAME_PADDING, title, ALARM_COLOR|DBLSIZE);
    dc->drawText(ALERT_TITLE_LEFT, ALERT_FRAME_TOP + ALERT_FRAME_PADDING + ALERT_TITLE_LINE_HEIGHT, STR_WARNING, ALARM_COLOR|DBLSIZE);
#endif
  }
  else if (title) {
    dc->drawText(ALERT_TITLE_LEFT, ALERT_FRAME_TOP + ALERT_FRAME_PADDING, title, ALARM_COLOR|DBLSIZE);
  }

  if (message) {
    dc->drawText(ALERT_FRAME_PADDING+5, ALERT_MESSAGE_TOP, message, MIDSIZE);
  }

  if (action) {
   // TODO remove action? dc->drawText(ALERT_FRAME_PADDING+5, ALERT_ACTION_TOP, action);
  }
}
