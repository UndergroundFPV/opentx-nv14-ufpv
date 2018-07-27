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

#define LANGUAGE_PACKS_DEFINITION

#include "opentx.h"
#include "radio_setup.h"

#define SET_DIRTY()     storageDirty(EE_GENERAL)

RadioSetupPage::RadioSetupPage():
  PageTab(STR_MENURADIOSETUP, ICON_RADIO_SETUP)
{
}

void RadioSetupPage::build(Window * window)
{
  GridLayout grid(*window);
  grid.spacer(8);

  // Date
  new StaticText(window, grid.getLabelSlot(), STR_DATE);
  new NumberEdit(window, grid.getFieldSlot(3, 0), 2018, 2100,
                 [=]() -> int32_t {
                   struct gtm t;
                   gettime(&t);
                   return TM_YEAR_BASE + t.tm_year;
                 },
                 [=](int32_t newValue) -> void {
                   struct gtm t;
                   gettime(&t);
                   t.tm_year = newValue - TM_YEAR_BASE;
                   rtcSetTime(&t);
                 });
  new NumberEdit(window, grid.getFieldSlot(3, 1), 1, 12,
                 [=]() -> int32_t {
                   struct gtm t;
                   gettime(&t);
                   return 1 + t.tm_mon;
                 },
                 [=](int32_t newValue) -> void {
                   struct gtm t;
                   gettime(&t);
                   t.tm_mon = newValue - 1;
                   rtcSetTime(&t);
                 });
  /* TODO dynamic max instead of 31 ...
  int16_t year = TM_YEAR_BASE + t.tm_year;
  int8_t dlim = (((((year%4==0) && (year%100!=0)) || (year%400==0)) && (t.tm_mon==1)) ? 1 : 0);
  static const pm_uint8_t dmon[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  dlim += pgm_read_byte(&dmon[t.tm_mon]);*/
  int8_t dlim = 31;
  new NumberEdit(window, grid.getFieldSlot(3, 2), 1, dlim,
                 [=]() -> int32_t {
                   struct gtm t;
                   gettime(&t);
                   return t.tm_mday;
                 },
                 [=](int32_t newValue) -> void {
                   struct gtm t;
                   gettime(&t);
                   t.tm_mday = newValue;
                   rtcSetTime(&t);
                 });
  grid.nextLine();

#if 0
    case ITEM_SETUP_TIME:
      {
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_TIME);
        LcdFlags flags = 0;
        if (attr && menuHorizontalPosition < 0) {
          flags |= INVERS;
        }
        for (uint8_t j=0; j<3; j++) {
          uint8_t rowattr = (menuHorizontalPosition==j ? attr : 0);
          switch (j) {
            case 0:
              if (rowattr && s_editMode>0) t.tm_hour = checkIncDec(event, t.tm_hour, 0, 23, 0);
              lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, t.tm_hour, flags|rowattr|LEADING0, 2);
              lcdDrawText(lcdNextPos+3, y, ":", flags);
              break;
            case 1:
              if (rowattr && s_editMode>0) t.tm_min = checkIncDec(event, t.tm_min, 0, 59, 0);
              lcdDrawNumber(lcdNextPos+3, y, t.tm_min, flags|rowattr|LEADING0, 2);
              lcdDrawText(lcdNextPos+3, y, ":", flags);
              break;
            case 2:
              if (rowattr && s_editMode>0) t.tm_sec = checkIncDec(event, t.tm_sec, 0, 59, 0);
              lcdDrawNumber(lcdNextPos+3, y, t.tm_sec, flags|rowattr|LEADING0, 2);
              break;
          }
        }
        if (attr && checkIncDec_Ret)
          g_rtcTime = gmktime(&t); // update local timestamp and get wday calculated
        break;
      }

      case ITEM_SETUP_BATT_RANGE:
      {
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_BATTERY_RANGE);
        LcdFlags flags = 0;
        if (attr && menuHorizontalPosition < 0) {
          flags |= INVERS;
        }
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, 90+g_eeGeneral.vBatMin, flags|(menuHorizontalPosition==0 ? attr : 0)|PREC1|LEFT);
        lcdDrawText(lcdNextPos+3, y, "-", flags);
        lcdDrawNumber(lcdNextPos+3, y, 120+g_eeGeneral.vBatMax, flags|(menuHorizontalPosition>0 ? attr : 0)|PREC1|LEFT);
        lcdDrawText(lcdNextPos+1, y, "V", flags);
        if (attr && s_editMode>0) {
          if (menuHorizontalPosition==0)
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatMin, -50, g_eeGeneral.vBatMax+29); // min=4.0V
          else
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatMax, g_eeGeneral.vBatMin-29, +40); // max=16.0V
        }
        break;
      }
#endif

  new Subtitle(window, grid.getLabelSlot(), STR_SOUND_LABEL);
  grid.nextLine();

  // Beeps mode
  new StaticText(window, grid.getLabelSlot(), STR_SPEAKER, true);
  new Choice(window, grid.getFieldSlot(), STR_VBEEPMODE, -2, 1, GET_SET_DEFAULT(g_eeGeneral.beepMode));
  grid.nextLine();

  // Main volume
  new StaticText(window, grid.getLabelSlot(), STR_SPEAKER_VOLUME, true);
  new Slider(window, grid.getFieldSlot(), -VOLUME_LEVEL_DEF, VOLUME_LEVEL_MAX-VOLUME_LEVEL_DEF, GET_SET_DEFAULT(g_eeGeneral.speakerVolume));
  grid.nextLine();

  // Beeps volume
  new StaticText(window, grid.getLabelSlot(), STR_BEEP_VOLUME, true);
  new Slider(window, grid.getFieldSlot(), -2, +2, GET_SET_DEFAULT(g_eeGeneral.beepVolume));
  grid.nextLine();

  // Wav volume
  new StaticText(window, grid.getLabelSlot(), STR_WAV_VOLUME, true);
  new Slider(window, grid.getFieldSlot(), -2, +2, GET_SET_DEFAULT(g_eeGeneral.wavVolume));
  grid.nextLine();

  // Background volume
  new StaticText(window, grid.getLabelSlot(), STR_BG_VOLUME, true);
  new Slider(window, grid.getFieldSlot(), -2, +2, GET_SET_DEFAULT(g_eeGeneral.backgroundVolume));
  grid.nextLine();

  // Beeps length
  new StaticText(window, grid.getLabelSlot(), STR_BEEP_LENGTH, true);
  new Slider(window, grid.getFieldSlot(), -2, +2, GET_SET_DEFAULT(g_eeGeneral.beepLength));
  grid.nextLine();

  // Beeps pitch
  new StaticText(window, grid.getLabelSlot(), STR_SPKRPITCH, true);
  NumberEdit * edit = new NumberEdit(window, grid.getFieldSlot(3, 0), 0, 300,
                 GET_DEFAULT(15 * g_eeGeneral.speakerPitch),
                 [=](int32_t newValue) -> void {
                   g_eeGeneral.speakerPitch = newValue / 15;
                   SET_DIRTY();
                 });
  edit->setStep(15);
  edit->setPrefix("+");
  edit->setSuffix("Hz");
  grid.nextLine();

#if defined(VARIO)
  new Subtitle(window, grid.getLabelSlot(), STR_VARIO);
  grid.nextLine();

  // Vario volume
  new StaticText(window, grid.getLabelSlot(), TR_SPEAKER_VOLUME, true);
  new Slider(window, grid.getFieldSlot(), -2, +2, GET_SET_DEFAULT(g_eeGeneral.varioVolume));
  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(), STR_PITCH_AT_ZERO, true);
  new NumberEdit(window, grid.getFieldSlot(), VARIO_FREQUENCY_ZERO-400, VARIO_FREQUENCY_ZERO+400,
                 GET_DEFAULT(VARIO_FREQUENCY_ZERO+(g_eeGeneral.varioPitch*10)),
                 [=](int8_t newValue) -> void {
                   g_eeGeneral.varioPitch = (newValue - VARIO_FREQUENCY_ZERO) / 10;
                   SET_DIRTY();
                 });
  edit->setStep(10);
  edit->setPrefix("Hz");
  grid.nextLine();

/*
      case ITEM_SETUP_VARIO_RANGE:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_PITCH_AT_MAX);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, VARIO_FREQUENCY_ZERO+(g_eeGeneral.varioPitch*10)+VARIO_FREQUENCY_RANGE+(g_eeGeneral.varioRange*10), attr|LEFT, 0, NULL, "Hz");
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.varioRange, -80, 80);
        break;
      case ITEM_SETUP_VARIO_REPEAT:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_REPEAT_AT_ZERO);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, VARIO_REPEAT_ZERO+(g_eeGeneral.varioRepeat*10), attr|LEFT, 0, NULL, STR_MS);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.varioRepeat, -30, 50);
        break;*/
#endif

#if defined(HAPTIC)
  {
    new Subtitle(window, grid.getLabelSlot(), STR_HAPTIC_LABEL);
    grid.nextLine();

    // Haptic mode
    new StaticText(window, grid.getLabelSlot(), STR_MODE, true);
    new Choice(window, grid.getFieldSlot(), STR_VBEEPMODE, -2, 1, GET_SET_DEFAULT(g_eeGeneral.hapticMode));
    grid.nextLine();

    // Haptic duration
    new StaticText(window, grid.getLabelSlot(), STR_LENGTH, true);
    new Slider(window, grid.getFieldSlot(), -2, +2, GET_SET_DEFAULT(g_eeGeneral.hapticLength));
    grid.nextLine();

    // Haptic strength
    new StaticText(window, grid.getLabelSlot(), STR_HAPTICSTRENGTH, true);
    new Slider(window, grid.getFieldSlot(), -2, +2, GET_SET_DEFAULT(g_eeGeneral.hapticStrength));
    grid.nextLine();
  }
#endif

  {
    new Subtitle(window, grid.getLabelSlot(), STR_ALARMS_LABEL);
    grid.nextLine();

    // Battery warning
    new StaticText(window, grid.getLabelSlot(), STR_BATTERYWARNING, true);
    edit = new NumberEdit(window, grid.getFieldSlot(), 40, 120, GET_SET_DEFAULT(g_eeGeneral.vBatWarn), PREC1);
    edit->setSuffix("v");
    grid.nextLine();

    // Alarms warning
    new StaticText(window, grid.getLabelSlot(), STR_ALARMWARNING, true);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_eeGeneral.disableAlarmWarning));
    grid.nextLine();

    // RSSI shutdown alarm
    new StaticText(window, grid.getLabelSlot(), STR_RSSISHUTDOWNALARM, true);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_eeGeneral.disableRssiPoweroffAlarm));
    grid.nextLine();

    // Inactivity alarm
    new StaticText(window, grid.getLabelSlot(), STR_INACTIVITYALARM, true);
    edit = new NumberEdit(window, grid.getFieldSlot(), 0, 250, GET_SET_DEFAULT(g_eeGeneral.inactivityTimer));
    edit->setSuffix("minutes");
    grid.nextLine();
  }

  new Subtitle(window, grid.getLabelSlot(), STR_BACKLIGHT_LABEL);
  grid.nextLine();

  // Backlight mode
  new StaticText(window, grid.getLabelSlot(), STR_MODE, true);
  new Choice(window, grid.getFieldSlot(), STR_VBLMODE, e_backlight_mode_off, e_backlight_mode_on, GET_SET_DEFAULT(g_eeGeneral.backlightMode));
  grid.nextLine();

  // Flash beep
  new StaticText(window, grid.getLabelSlot(), STR_ALARM);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_eeGeneral.alarmsFlash));
  grid.nextLine();

#if 0
      case ITEM_SETUP_BACKLIGHT_DELAY:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_BLDELAY);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.lightAutoOff*5, attr|LEFT, 0, NULL, "s");
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.lightAutoOff, 0, 600/5);
        break;

      case ITEM_SETUP_BRIGHTNESS:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_BLONBRIGHTNESS);
        g_eeGeneral.backlightBright = BACKLIGHT_LEVEL_MAX - editSlider(RADIO_SETUP_2ND_COLUMN, y, event, BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright, BACKLIGHT_LEVEL_MIN, BACKLIGHT_LEVEL_MAX, attr);
        break;

      case ITEM_SETUP_DIM_LEVEL:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_BLOFFBRIGHTNESS);
        g_eeGeneral.blOffBright = editSlider(RADIO_SETUP_2ND_COLUMN, y, event, g_eeGeneral.blOffBright, BACKLIGHT_LEVEL_MIN, BACKLIGHT_LEVEL_MAX, attr);
        break;
#endif

  new Subtitle(window, grid.getLabelSlot(), STR_GPS);
  grid.nextLine();

  // Timezone
  new StaticText(window, grid.getLabelSlot(), STR_TIMEZONE, true);
  new NumberEdit(window, grid.getFieldSlot(2, 0), -12, 12, GET_SET_DEFAULT(g_eeGeneral.timezone));
  grid.nextLine();

  // Adjust RTC (from telemetry)
  new StaticText(window, grid.getLabelSlot(), STR_ADJUST_RTC, true);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_eeGeneral.adjustRTC));
  grid.nextLine();

  // GPS format
  new StaticText(window, grid.getLabelSlot(), STR_GPSCOORD, true);
  new Choice(window, grid.getFieldSlot(), STR_GPSFORMAT, 0, 1, GET_SET_DEFAULT(g_eeGeneral.gpsFormat));
  grid.nextLine();

  // Country code
  new StaticText(window, grid.getLabelSlot(), STR_COUNTRYCODE, true);
  new Choice(window, grid.getFieldSlot(), STR_COUNTRYCODES, 0, 2, GET_SET_DEFAULT(g_eeGeneral.countryCode));
  grid.nextLine();

#if 0
      case ITEM_SETUP_LANGUAGE:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_VOICELANG);
        lcdDrawText(RADIO_SETUP_2ND_COLUMN, y, currentLanguagePack->name, attr);
        if (attr) {
          currentLanguagePackIdx = checkIncDec(event, currentLanguagePackIdx, 0, DIM(languagePacks)-2, EE_GENERAL);
          if (checkIncDec_Ret) {
            currentLanguagePack = languagePacks[currentLanguagePackIdx];
            strncpy(g_eeGeneral.ttsLanguage, currentLanguagePack->id, 2);
          }
        }
        break;
#endif

  // Imperial units
  new StaticText(window, grid.getLabelSlot(), STR_UNITSSYSTEM, true);
  new Choice(window, grid.getFieldSlot(), STR_VUNITSSYSTEM, 0, 1, GET_SET_DEFAULT(g_eeGeneral.imperial));
  grid.nextLine();

#if defined(FAI_CHOICE)
      case ITEM_SETUP_FAI:
        lcdDrawText(MENUS_MARGIN_LEFT, y, PSTR("FAI Mode"));
        if (g_eeGeneral.fai) {
          lcdDrawText(RADIO_SETUP_2ND_COLUMN, y, PSTR("Locked in FAI Mode"));
        }
        else {
          g_eeGeneral.fai = editCheckBox(g_eeGeneral.fai, RADIO_SETUP_2ND_COLUMN, y, attr, event);
          if (attr && checkIncDec_Ret) {
              g_eeGeneral.fai = false;
              POPUP_CONFIRMATION(PSTR("FAI mode?"));
          }
        }
        break;
#endif

#if defined(TELEMETRY_MAVLINK)
  // Imperial units
  new StaticText(window, grid.getLabelSlot(), STR_MAVLINK_BAUD_LABEL);
  new Choice(window, grid.getFieldSlot(), STR_MAVLINK_BAUDS, 0, 7, GET_SET_DEFAULT(g_eeGeneral.mavbaud));
  grid.nextLine();
#endif

#if 0
      case ITEM_SETUP_SWITCHES_DELAY:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_SWITCHES_DELAY);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, 10*SWITCHES_DELAY(), attr|LEFT, 0, NULL, STR_MS);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.switchesDelay, -15, 100-15);
        break;

      case ITEM_SETUP_USB_MODE:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_USBMODE);
        g_eeGeneral.USBMode = editChoice(RADIO_SETUP_2ND_COLUMN, y, STR_USBMODES, g_eeGeneral.USBMode, USB_UNSELECTED_MODE, USB_MAX_MODE, attr, event);
        break;

      case ITEM_SETUP_RX_CHANNEL_ORD:
      {
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_RXCHANNELORD); // RAET->AETR
        char s[5];
        for (uint8_t i=0; i<4; i++) {
          s[i] = STR_RETA123[channel_order(i+1)];
        }
        s[4] = '\0';
        lcdDrawText(RADIO_SETUP_2ND_COLUMN, y, s, attr);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.templateSetup, 0, 23);
        break;
      }

      case ITEM_SETUP_STICK_MODE:
      {
        lcdDrawText(MENUS_MARGIN_LEFT, y, NO_INDENT(STR_MODE));
        char s[2] = " ";
        s[0] = '1'+reusableBuffer.generalSettings.stickMode;
        lcdDrawText(RADIO_SETUP_2ND_COLUMN, y, s, attr);
        for (uint8_t i=0; i<4; i++) {
          drawSource(RADIO_SETUP_2ND_COLUMN + 40 + 50*i, y, MIXSRC_Rud + pgm_read_byte(modn12x3 + 4*reusableBuffer.generalSettings.stickMode + i));
        }
        if (attr && s_editMode>0) {
          CHECK_INCDEC_GENVAR(event, reusableBuffer.generalSettings.stickMode, 0, 3);
        }
        else if (reusableBuffer.generalSettings.stickMode != g_eeGeneral.stickMode) {
          pausePulses();
          g_eeGeneral.stickMode = reusableBuffer.generalSettings.stickMode;
          checkTHR();
          resumePulses();
          clearKeyEvents();
        }
        break;

#endif
  window->setInnerHeight(grid.getWindowHeight());
}
