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

#include "model_telemetry.h"
#include "opentx.h"
#include "libwindows.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

class SensorSourceChoice : public SourceChoice {
  public:
    SensorSourceChoice(Window * window, const rect_t &rect, uint8_t * source, IsValueAvailable isValueAvailable) :
      SourceChoice(window, rect, MIXSRC_NONE, MIXSRC_LAST_TELEM,
                   GET_DEFAULT(*source ? MIXSRC_FIRST_TELEM + 3 * (*source - 1) : MIXSRC_NONE),
                   [=](uint8_t newValue) {
                     *source = newValue == MIXSRC_NONE ? 0 : (newValue - MIXSRC_FIRST_TELEM) / 3 + 1;
                   })
    {
      setAvailableHandler([=](int16_t value) {
        if (value == MIXSRC_NONE)
          return true;
        if (value < MIXSRC_FIRST_TELEM)
          return false;
        auto qr = div(value - MIXSRC_FIRST_TELEM, 3);
        return qr.rem == 0 && isValueAvailable(qr.quot + 1);
      });
    }
};

class SensorButton : public Button {
  public:
    SensorButton(Window * parent, const rect_t &rect, uint8_t index) :
      Button(parent, rect),
      index(index)
    {
    }

    static constexpr coord_t line1 = 1;
    static constexpr coord_t line2 = 22;
    static constexpr coord_t col1 = 60;
    static constexpr coord_t col2 = (LCD_W - 100) / 3 + col1;
    static constexpr coord_t col3 = ((LCD_W - 100) / 3) * 2 + col1 + 20;

    bool isActive()
    {
      // TODO
      return false;
    }

    void checkEvents() override
    {
      if (active != isActive()) {
        invalidate();
        active = !active;
      }
    }

    void paintSpecialFunctionLine(BitmapBuffer * dc)
    {
      lcdDrawNumber(2, 1, index + 1, LEFT, 0, NULL, ":");
      lcdDrawSizedText(col1, line1, g_model.telemetrySensors[index].label, TELEM_LABEL_LEN, ZCHAR);
      TelemetryItem &telemetryItem = telemetryItems[index];
      if (telemetryItem.isFresh()) {
        lcdDrawText(col2, line1, "*");
      }
      if (telemetryItem.isAvailable()) {
        LcdFlags color = telemetryItem.isOld() ? ALARM_COLOR : TEXT_COLOR;
        drawSensorCustomValue(col3, line1, index, getValue(MIXSRC_FIRST_TELEM + 3 * index), LEFT | color);
      }
      else {
        lcdDrawText(col3, line1, "---");
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
    uint8_t index;
    bool active = false;
};

class SensorEditWindow : public Page {
  public:
    SensorEditWindow(uint8_t index) :
      Page(),
      index(index)
    {
      buildBody(&body);
      buildHeader(&header);
    }

  protected:
    uint8_t index;
    Window * sensorOneWindow = nullptr;

    void buildHeader(Window * window)
    {
      new StaticText(window, {70, 4, 200, 20}, STR_SENSOR + std::to_string(index + 1), MENU_TITLE_COLOR);
      // dynamic display of sensor value ?
      //new StaticText(window, {70, 28, 100, 20}, "SF" + std::to_string(index), MENU_TITLE_COLOR);
    }

    void updateSensorOneWindow()
    {
      // Sensor variable part
      GridLayout grid(*sensorOneWindow);
      sensorOneWindow->clear();
      TelemetrySensor * sensor = &g_model.telemetrySensors[index];

      if (sensor->type == TELEM_TYPE_CALCULATED) {
        // Formula
        new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_FORMULA);
        new Choice(sensorOneWindow, grid.getFieldSlot(), STR_VFORMULAS, 0, TELEM_FORMULA_LAST, GET_DEFAULT(sensor->formula),
                   [=](uint8_t newValue) {
                     sensor->formula = newValue;
                     sensor->param = 0;
                     if (sensor->formula == TELEM_FORMULA_CELL) {
                       sensor->unit = UNIT_VOLTS;
                       sensor->prec = 2;
                     }
                     else if (sensor->formula == TELEM_FORMULA_DIST) {
                       sensor->unit = UNIT_DIST;
                       sensor->prec = 0;
                     }
                     else if (sensor->formula == TELEM_FORMULA_CONSUMPTION) {
                       sensor->unit = UNIT_MAH;
                       sensor->prec = 0;
                     }
                     SET_DIRTY();
                     telemetryItems[index].clear();
                     updateSensorOneWindow();
                   });
        grid.nextLine();
      }

      // Unit
      if ((sensor->type == TELEM_TYPE_CALCULATED && (sensor->formula == TELEM_FORMULA_DIST)) || sensor->isConfigurable()) {
        new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_UNIT);
        new Choice(sensorOneWindow, grid.getFieldSlot(), STR_VTELEMUNIT, 0, UNIT_MAX, GET_DEFAULT(sensor->unit),
                   [=](uint8_t newValue) {
                     sensor->unit = newValue;
                     if (sensor->unit == UNIT_FAHRENHEIT) {
                       sensor->prec = 0;
                     }
                     SET_DIRTY();
                     telemetryItems[index].clear();
                     updateSensorOneWindow();
                   });
        grid.nextLine();
      }

      // Precision
      if (sensor->isPrecConfigurable() && sensor->unit != UNIT_FAHRENHEIT) {
        new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_PRECISION);
        new Choice(sensorOneWindow, grid.getFieldSlot(), STR_VPREC, 0, 2, GET_DEFAULT(sensor->prec),
                   [=](uint8_t newValue) {
                     sensor->prec = newValue;
                     SET_DIRTY();
                     telemetryItems[index].clear();
                     updateSensorOneWindow();
                   });
        grid.nextLine();
      }

      // PARAM1
      if (sensor->unit < UNIT_FIRST_VIRTUAL) {
        if (sensor->type == TELEM_TYPE_CALCULATED) {
          if (sensor->formula == TELEM_FORMULA_CELL) {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_CELLSENSOR);
            new SensorSourceChoice(sensorOneWindow, grid.getFieldSlot(), &sensor->cell.source, isCellsSensor);
          }
          else if (sensor->formula == TELEM_FORMULA_DIST) {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_GPSSENSOR);
            new SensorSourceChoice(sensorOneWindow, grid.getFieldSlot(), &sensor->dist.gps, isGPSSensor);
          }
          else if (sensor->formula == TELEM_FORMULA_CONSUMPTION) {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_CURRENTSENSOR);
            new SensorSourceChoice(sensorOneWindow, grid.getFieldSlot(), &sensor->consumption.source, isSensorAvailable);
          }
          else if (sensor->formula == TELEM_FORMULA_TOTALIZE) {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_SOURCE);
            new SensorSourceChoice(sensorOneWindow, grid.getFieldSlot(), &sensor->consumption.source, isSensorAvailable);
          }
          else {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_SOURCE + std::to_string(1));
            new SensorSourceChoice(sensorOneWindow, grid.getFieldSlot(), (uint8_t *) &sensor->calc.sources[0], isSensorAvailable);
          }
        }
        else {
          if (sensor->unit == UNIT_RPMS) {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_BLADES);
            new NumberEdit(sensorOneWindow, grid.getFieldSlot(), 1, 30000, GET_SET_DEFAULT(sensor->custom.ratio));
          }
          else {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_RATIO);
            auto edit = new NumberEdit(sensorOneWindow, grid.getFieldSlot(), 0, 30000, GET_SET_DEFAULT(sensor->custom.ratio));
            edit->setZeroText("-");
          }
        }
        grid.nextLine();
      }

      //PARAM2
      if (!(sensor->unit == UNIT_GPS || sensor->unit == UNIT_DATETIME || sensor->unit == UNIT_CELLS ||
            (sensor->type == TELEM_TYPE_CALCULATED && (sensor->formula == TELEM_FORMULA_CONSUMPTION || sensor->formula == TELEM_FORMULA_TOTALIZE)))) {
        if (sensor->type == TELEM_TYPE_CALCULATED) {
          if (sensor->formula == TELEM_FORMULA_CELL) {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_CELLINDEX);
            new Choice(sensorOneWindow, grid.getFieldSlot(), STR_VCELLINDEX, 0, 8, GET_SET_DEFAULT(sensor->cell.index));
          }
          else if (sensor->formula == TELEM_FORMULA_DIST) {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_ALTSENSOR);
            new SensorSourceChoice(sensorOneWindow, grid.getFieldSlot(), &sensor->dist.alt, isAltSensor);
          }
          else {
            new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_SOURCE + std::to_string(2));
            new SensorSourceChoice(sensorOneWindow, grid.getFieldSlot(), (uint8_t *) &sensor->calc.sources[1], isSensorAvailable);
          }
        }
        else if (sensor->unit == UNIT_RPMS) {
          new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_MULTIPLIER);
          new NumberEdit(sensorOneWindow, grid.getFieldSlot(), 1, 30000, GET_SET_DEFAULT(sensor->custom.offset));
        }
        else {
          new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_OFFSET);
          new NumberEdit(sensorOneWindow, grid.getFieldSlot(), -30000, 30000, GET_SET_DEFAULT(sensor->custom.offset),
                         (sensor->prec > 0) ? (sensor->prec == 2 ? PREC2 : PREC1) : 0);
        }
        grid.nextLine();
      }

      //PARAM 3 and 4
      if ((sensor->type == TELEM_TYPE_CALCULATED && sensor->formula < TELEM_FORMULA_MULTIPLY)) {
        new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_SOURCE + std::to_string(3));
        new SensorSourceChoice(sensorOneWindow, grid.getFieldSlot(), (uint8_t *) &sensor->calc.sources[2], isSensorAvailable);
        grid.nextLine();

        new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_SOURCE + std::to_string(4));
        new SensorSourceChoice(sensorOneWindow, grid.getFieldSlot(), (uint8_t *) &sensor->calc.sources[3], isSensorAvailable);
        grid.nextLine();
      }

      // Auto Offset
      if (sensor->unit != UNIT_RPMS && sensor->isConfigurable()) {
        new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_AUTOOFFSET);
        new CheckBox(sensorOneWindow, grid.getFieldSlot(), GET_SET_DEFAULT(sensor->autoOffset));
        grid.nextLine();
      }

      if (sensor->isConfigurable()) {
        // Only positive
        new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_ONLYPOSITIVE);
        new CheckBox(sensorOneWindow, grid.getFieldSlot(), GET_SET_DEFAULT(sensor->onlyPositive));
        grid.nextLine();

        // Filter
        new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_FILTER);
        new CheckBox(sensorOneWindow, grid.getFieldSlot(), GET_SET_DEFAULT(sensor->filter));
        grid.nextLine();
      }

      if (sensor->type == TELEM_TYPE_CALCULATED) {
        new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_PERSISTENT);
        new CheckBox(sensorOneWindow, grid.getFieldSlot(), GET_DEFAULT(sensor->persistent), [=](int32_t newValue) {
          sensor->persistent = newValue;
          if (!sensor->persistent)
            sensor->persistentValue = 0;
          SET_DIRTY();
        });
        grid.nextLine();
      }

      new StaticText(sensorOneWindow, grid.getLabelSlot(), STR_LOGS);
      new CheckBox(sensorOneWindow, grid.getFieldSlot(), GET_DEFAULT(sensor->logs), [=](int32_t newValue) {
        sensor->logs = newValue;
        logsClose();
        SET_DIRTY();
      });

      coord_t delta = sensorOneWindow->adjustHeight();
      Window * parent = sensorOneWindow->getParent();
      parent->moveWindowsTop(sensorOneWindow->top(), delta);
    }

    void buildBody(Window * window)
    {
      // Sensor one
      GridLayout grid(*window);
      grid.spacer(8);

      TelemetrySensor * sensor = &g_model.telemetrySensors[index];

      // Name
      new StaticText(window, grid.getLabelSlot(), STR_NAME);
      new TextEdit(window, grid.getFieldSlot(), sensor->label, TELEM_LABEL_LEN);
      grid.nextLine();

      // Type
      new StaticText(window, grid.getLabelSlot(), STR_TYPE);
      new Choice(window, grid.getFieldSlot(), STR_VSENSORTYPES, 0, 1, GET_DEFAULT(sensor->type),
                 [=](uint8_t newValue) {
                   sensor->type = newValue;
                   sensor->instance = 0;
                   if (sensor->type == TELEM_TYPE_CALCULATED) {
                     sensor->param = 0;
                     sensor->filter = 0;
                     sensor->autoOffset = 0;
                   }
                   SET_DIRTY();
                   updateSensorOneWindow();
                 });
      grid.nextLine();

      sensorOneWindow = new Window(window, {0, grid.getWindowHeight(), LCD_W, 0});
      updateSensorOneWindow();
      grid.addWindow(sensorOneWindow);

      window->setInnerHeight(grid.getWindowHeight());
    }
};

ModelTelemetryPage::ModelTelemetryPage() :
  PageTab(STR_MENUTELEMETRY, ICON_MODEL_TELEMETRY)
{
}

void ModelTelemetryPage::rebuild(Window * window)
{
  coord_t scrollPosition = window->getScrollPositionY();
  window->clear();
  build(window);
  window->setScrollPositionY(scrollPosition);
}


void ModelTelemetryPage::editSensor(Window * window, uint8_t index)
{
  Window * editWindow = new SensorEditWindow(index);
  editWindow->setCloseHandler([=]() {
    rebuild(window);
  });
}

void ModelTelemetryPage::build(Window * window)
{
  GridLayout grid(*window);
  grid.spacer(8);
  grid.setLabelWidth(180);


  // RSSI
  if (g_model.moduleData[INTERNAL_MODULE].rfProtocol == RF_PROTO_OFF &&
      g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_MULTIMODULE &&
      g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false) == MM_RF_PROTO_FS_AFHDS2A)
    new Subtitle(window, grid.getLineSlot(), "RSNR");
  else
    new Subtitle(window, grid.getLineSlot(), "RSSI");
  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(true), STR_LOWALARM);
  auto edit = new NumberEdit(window, grid.getFieldSlot(), -30, 30, GET_SET_DEFAULT(g_model.rssiAlarms.warning));
  edit->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
    drawNumber(dc, 2, 2, g_model.rssiAlarms.getWarningRssi(), flags);
  });
  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(true), STR_CRITICALALARM);
  edit = new NumberEdit(window, grid.getFieldSlot(), -30, 30, GET_SET_DEFAULT(g_model.rssiAlarms.critical));
  edit->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
    drawNumber(dc, 2, 2, g_model.rssiAlarms.getCriticalRssi(), flags);
  });
  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(true), STR_DISABLE_ALARM);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.rssiAlarms.disabled));
  grid.nextLine();

  // Sensors
  new Subtitle(window, grid.getLineSlot(), STR_TELEMETRY_SENSORS);
  new StaticText(window, grid.getFieldSlot(2, 0), STR_VALUE);
  if (!g_model.ignoreSensorIds && !IS_SPEKTRUM_PROTOCOL())
    new StaticText(window, grid.getFieldSlot(2, 1), STR_ID);
  grid.nextLine();

  for (uint8_t idx; idx < availableTelemetryIndex(); idx++) {
    Button * button = new SensorButton(window, grid.getLineSlot(), idx);
    button->setPressHandler([=]() -> uint8_t {
      button->bringToTop();
      Menu * menu = new Menu();
      menu->addLine(STR_EDIT, [=]() {
        editSensor(window, idx);
      });
      menu->addLine(STR_COPY, [=]() {
        int newIndex = availableTelemetryIndex();
        if (newIndex >= 0) {
          TelemetrySensor &sourceSensor = g_model.telemetrySensors[idx];
          TelemetrySensor &newSensor = g_model.telemetrySensors[newIndex];
          newSensor = sourceSensor;
          TelemetryItem &sourceItem = telemetryItems[idx];
          TelemetryItem &newItem = telemetryItems[newIndex];
          newItem = sourceItem;
          SET_DIRTY();
          rebuild(window);
        }
        else {
          new Dialog(WARNING_TYPE_ALERT, "", STR_TELEMETRYFULL, nullptr);
        }
      });
      menu->addLine(STR_DELETE, [=]() {
        delTelemetryIndex(idx);
        SET_DIRTY();
        rebuild(window);
      });
      return 0;
    });
    grid.nextLine();
  }

  new TextButton(window, grid.getLineSlot(), STR_DISCOVER_SENSORS,
                 []() -> uint8_t {
                   //TODO bistable triggering disco
                   return 0;
                 });
  grid.nextLine();

  new TextButton(window, grid.getLineSlot(), STR_TELEMETRY_NEWSENSOR,
                 [=]() -> uint8_t {
                   int res = availableTelemetryIndex();
                   if (res >= 0)
                     editSensor(window, res);
                   else
                     new Dialog(WARNING_TYPE_ALERT, "", STR_TELEMETRYFULL, nullptr);
                   return 0;
                 });
  grid.nextLine();

  new TextButton(window, grid.getLineSlot(), STR_DELETE_ALL_SENSORS,
                 []() -> uint8_t {
                   new Dialog(WARNING_TYPE_CONFIRM, STR_CONFIRMDELETE, "", [=]() {
                     for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
                       delTelemetryIndex(i);
                     }
                   });
                   return 0;
                 });
  grid.nextLine();


  new StaticText(window, grid.getLabelSlot(true), STR_IGNORE_INSTANCE);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.ignoreSensorIds));
  grid.nextLine();

  // Vario
  grid.setLabelWidth(100);
  new Subtitle(window, grid.getLineSlot(), STR_VARIO);
  grid.nextLine();
  new StaticText(window, grid.getLabelSlot(true), STR_SOURCE);
  auto choice = new SourceChoice(window, grid.getFieldSlot(), MIXSRC_NONE, MIXSRC_LAST_TELEM,
                                 GET_DEFAULT(g_model.frsky.varioSource ? MIXSRC_FIRST_TELEM + 3 * (g_model.frsky.varioSource - 1) : MIXSRC_NONE),
                                 SET_VALUE(g_model.frsky.varioSource, newValue == MIXSRC_NONE ? 0 : (newValue - MIXSRC_FIRST_TELEM) / 3 + 1));
  choice->setAvailableHandler([=](int16_t value) {
    if (value == MIXSRC_NONE)
      return true;
    if (value < MIXSRC_FIRST_TELEM)
      return false;
    auto qr = div(value - MIXSRC_FIRST_TELEM, 3);
    return qr.rem == 0 && isSensorAvailable(qr.quot + 1);
  });
  grid.nextLine();
  new StaticText(window, grid.getLabelSlot(true), STR_RANGE);
  new NumberEdit(window, grid.getFieldSlot(2, 0), -7, 7, GET_SET_WITH_OFFSET(g_model.frsky.varioMin, -10));
  new NumberEdit(window, grid.getFieldSlot(2, 1), -7, 7, GET_SET_WITH_OFFSET(g_model.frsky.varioMax, 10));
  grid.nextLine();
  new StaticText(window, grid.getLabelSlot(true), STR_CENTER);
  new NumberEdit(window, grid.getFieldSlot(3, 0), -7, 7, GET_SET_WITH_OFFSET(g_model.frsky.varioCenterMin, -5), PREC1);
  new NumberEdit(window, grid.getFieldSlot(3, 1), -7, 7, GET_SET_WITH_OFFSET(g_model.frsky.varioCenterMax, 5), PREC1);
  new Choice(window, grid.getFieldSlot(3, 2), STR_VVARIOCENTER, 0, 1, GET_SET_DEFAULT(g_model.frsky.varioCenterSilent));
  grid.nextLine();

  window->setInnerHeight(grid.getWindowHeight());
}

#if 0
enum MenuModelTelemetryFrskyItems {
  ITEM_TELEMETRY_PROTOCOL_TYPE,
  ITEM_TELEMETRY_RSSI_LABEL,
  ITEM_TELEMETRY_RSSI_ALARM1,
  ITEM_TELEMETRY_RSSI_ALARM2,
  ITEM_TELEMETRY_DISABLE_ALARMS,
  ITEM_TELEMETRY_SENSORS_LABEL,
  ITEM_TELEMETRY_SENSOR1,
  ITEM_TELEMETRY_SENSOR2,
  ITEM_TELEMETRY_SENSOR3,
  ITEM_TELEMETRY_SENSOR4,
  ITEM_TELEMETRY_SENSOR5,
  ITEM_TELEMETRY_SENSOR6,
  ITEM_TELEMETRY_SENSOR7,
  ITEM_TELEMETRY_SENSOR8,
  ITEM_TELEMETRY_SENSOR9,
  ITEM_TELEMETRY_SENSOR10,
  ITEM_TELEMETRY_SENSOR11,
  ITEM_TELEMETRY_SENSOR12,
  ITEM_TELEMETRY_SENSOR13,
  ITEM_TELEMETRY_SENSOR14,
  ITEM_TELEMETRY_SENSOR15,
  ITEM_TELEMETRY_SENSOR16,
  ITEM_TELEMETRY_SENSOR17,
  ITEM_TELEMETRY_SENSOR18,
  ITEM_TELEMETRY_SENSOR19,
  ITEM_TELEMETRY_SENSOR20,
  ITEM_TELEMETRY_SENSOR21,
  ITEM_TELEMETRY_SENSOR22,
  ITEM_TELEMETRY_SENSOR23,
  ITEM_TELEMETRY_SENSOR24,
  ITEM_TELEMETRY_SENSOR25,
  ITEM_TELEMETRY_SENSOR26,
  ITEM_TELEMETRY_SENSOR27,
  ITEM_TELEMETRY_SENSOR28,
  ITEM_TELEMETRY_SENSOR29,
  ITEM_TELEMETRY_SENSOR30,
  ITEM_TELEMETRY_SENSOR31,
  ITEM_TELEMETRY_SENSOR32,
  ITEM_TELEMETRY_DISCOVER_SENSORS,
  ITEM_TELEMETRY_NEW_SENSOR,
  ITEM_TELEMETRY_DELETE_ALL_SENSORS,
  ITEM_TELEMETRY_IGNORE_SENSOR_INSTANCE,
#if defined(VARIO)
  ITEM_TELEMETRY_VARIO_LABEL,
  ITEM_TELEMETRY_VARIO_SOURCE,
  ITEM_TELEMETRY_VARIO_RANGE,
  ITEM_TELEMETRY_VARIO_CENTER,
#endif
  ITEM_TELEMETRY_MAX
};

#define TELEM_COL1                    10
#define TELEM_COL2                    200
#define TELEM_COL3                    275
#define TELEM_COL4                    350
#define TELEM_COL5                    425

#define IF_FAS_OFFSET(x)              x,
#define IS_RANGE_DEFINED(k)           (g_model.frsky.channels[k].ratio > 0)

#define SENSOR_ROWS(x)                (isTelemetryFieldAvailable(x) ? (uint8_t)0 : HIDDEN_ROW)
#define SENSORS_ROWS                  LABEL(Sensors), SENSOR_ROWS(0), SENSOR_ROWS(1), SENSOR_ROWS(2), SENSOR_ROWS(3), SENSOR_ROWS(4), SENSOR_ROWS(5), SENSOR_ROWS(6), SENSOR_ROWS(7), SENSOR_ROWS(8), SENSOR_ROWS(9), SENSOR_ROWS(10), SENSOR_ROWS(11), SENSOR_ROWS(12), SENSOR_ROWS(13), SENSOR_ROWS(14), SENSOR_ROWS(15), SENSOR_ROWS(16), SENSOR_ROWS(17), SENSOR_ROWS(18), SENSOR_ROWS(19), SENSOR_ROWS(20), SENSOR_ROWS(21), SENSOR_ROWS(22), SENSOR_ROWS(23), SENSOR_ROWS(24), SENSOR_ROWS(25), SENSOR_ROWS(26), SENSOR_ROWS(27), SENSOR_ROWS(28), SENSOR_ROWS(29), SENSOR_ROWS(30), SENSOR_ROWS(31), 0, 0, 0, 0,
#if defined(VARIO)
#define VARIO_ROWS                  LABEL(Vario), 0, 1, 2,
#else
#define VARIO_ROWS
#endif
#define RSSI_ROWS                     LABEL(RSSI), 0, 0, 0,
#define VARIO_RANGE_ROWS              3
#define TELEMETRY_TYPE_ROWS           (g_model.moduleData[INTERNAL_MODULE].rfProtocol == RF_PROTO_OFF && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_PPM) ? (uint8_t)0 : HIDDEN_ROW,

enum SensorFields {
  SENSOR_FIELD_NAME,
  SENSOR_FIELD_TYPE,
  SENSOR_FIELD_ID,
  SENSOR_FIELD_FORMULA = SENSOR_FIELD_ID,
  SENSOR_FIELD_UNIT,
  SENSOR_FIELD_PRECISION,
  SENSOR_FIELD_PARAM1,
  SENSOR_FIELD_PARAM2,
  SENSOR_FIELD_PARAM3,
  SENSOR_FIELD_PARAM4,
  SENSOR_FIELD_AUTOOFFSET,
  SENSOR_FIELD_ONLYPOSITIVE,
  SENSOR_FIELD_FILTER,
  SENSOR_FIELD_PERSISTENT,
  SENSOR_FIELD_LOGS,
  SENSOR_FIELD_MAX
};

#define SENSOR_2ND_COLUMN      140
#define SENSOR_3RD_COLUMN      280

#define SENSOR_UNIT_ROWS       ((sensor->type == TELEM_TYPE_CALCULATED && (sensor->formula == TELEM_FORMULA_DIST)) || sensor->isConfigurable() ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_PREC_ROWS       (sensor->isPrecConfigurable() && sensor->unit != UNIT_FAHRENHEIT  ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_PARAM1_ROWS     (sensor->unit >= UNIT_FIRST_VIRTUAL ? HIDDEN_ROW : (uint8_t)0)
#define SENSOR_PARAM2_ROWS     (sensor->unit == UNIT_GPS || sensor->unit == UNIT_DATETIME || sensor->unit == UNIT_CELLS || (sensor->type==TELEM_TYPE_CALCULATED && (sensor->formula==TELEM_FORMULA_CONSUMPTION || sensor->formula==TELEM_FORMULA_TOTALIZE)) ? HIDDEN_ROW : (uint8_t)0)
#define SENSOR_PARAM3_ROWS     (sensor->type == TELEM_TYPE_CALCULATED && sensor->formula < TELEM_FORMULA_MULTIPLY) ? (uint8_t)0 : HIDDEN_ROW
#define SENSOR_PARAM4_ROWS     (sensor->type == TELEM_TYPE_CALCULATED && sensor->formula < TELEM_FORMULA_MULTIPLY) ? (uint8_t)0 : HIDDEN_ROW
#define SENSOR_AUTOOFFSET_ROWS (sensor->unit != UNIT_RPMS && sensor->isConfigurable() ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_ONLYPOS_ROWS    (sensor->isConfigurable() ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_FILTER_ROWS     (sensor->isConfigurable() ? (uint8_t)0 : HIDDEN_ROW)
#define SENSOR_PERSISTENT_ROWS (sensor->type == TELEM_TYPE_CALCULATED ? (uint8_t)0 : HIDDEN_ROW)

bool menuModelSensor(event_t event)
{
  TelemetrySensor * sensor = &g_model.telemetrySensors[s_currIdx];

  SUBMENU("SENSOR", ICON_MODEL_TELEMETRY, SENSOR_FIELD_MAX, {
    0, 0, sensor->type == TELEM_TYPE_CALCULATED ? (uint8_t) 0
                                                : (uint8_t) 1, SENSOR_UNIT_ROWS, SENSOR_PREC_ROWS, SENSOR_PARAM1_ROWS, SENSOR_PARAM2_ROWS, SENSOR_PARAM3_ROWS, SENSOR_PARAM4_ROWS, SENSOR_AUTOOFFSET_ROWS, SENSOR_ONLYPOS_ROWS, SENSOR_FILTER_ROWS, SENSOR_PERSISTENT_ROWS, 0
  });
  lcdDrawNumber(lcdNextPos, 3, s_currIdx + 1, MENU_TITLE_COLOR | LEFT);
  drawSensorCustomValue(50, 3 + FH, s_currIdx, getValue(MIXSRC_FIRST_TELEM + 3 * s_currIdx), MENU_TITLE_COLOR | LEFT);

  for (uint8_t i = 0; i < NUM_BODY_LINES + 1; i++) {
    coord_t y = MENU_CONTENT_TOP - FH - 2 + i * FH;
    int k = i + menuVerticalOffset;

    for (int j = 0; j < k; j++) {
      if (mstate_tab[j + 1] == HIDDEN_ROW) {
        if (++k >= (int) DIM(mstate_tab)) {
          return true;
        }
      }
    }

    LcdFlags attr = (menuVerticalPosition == k ? (s_editMode > 0 ? BLINK | INVERS : INVERS) : 0);

    switch (k) {


  }
  return true;
}

void onSensorMenu(const char * result)
{
  uint8_t index = menuVerticalPosition - ITEM_TELEMETRY_SENSOR1;

  if (index < MAX_TELEMETRY_SENSORS) {
    if (result == STR_EDIT) {
      pushMenu(menuModelSensor);
    }
    else if (result == STR_DELETE) {
      delTelemetryIndex(index);
      index += 1;
      if (index < MAX_TELEMETRY_SENSORS && isTelemetryFieldAvailable(index))
        menuVerticalPosition += 1;
      else
        menuVerticalPosition = ITEM_TELEMETRY_NEW_SENSOR;
    }
    else if (result == STR_COPY) {
      int newIndex = availableTelemetryIndex();

      if (newIndex >= 0) {
        TelemetrySensor &sourceSensor = g_model.telemetrySensors[index];
        TelemetrySensor &newSensor = g_model.telemetrySensors[newIndex];
        newSensor = sourceSensor;
        TelemetryItem &sourceItem = telemetryItems[index];
        TelemetryItem &newItem = telemetryItems[newIndex];
        newItem = sourceItem;
        storageDirty(EE_MODEL);
      }
      else {
        POPUP_WARNING(STR_TELEMETRYFULL);
      }
    }
  }
}

bool menuModelTelemetryFrsky(event_t event)
{


  MENU(STR_MENUTELEMETRY, MODEL_ICONS, menuTabModel, MENU_MODEL_TELEMETRY_FRSKY, ITEM_TELEMETRY_MAX, {
    TELEMETRY_TYPE_ROWS
      RSSI_ROWS
    SENSORS_ROWS
      VARIO_ROWS
  });

  for (uint8_t i = 0; i < NUM_BODY_LINES; i++) {
    coord_t y = MENU_CONTENT_TOP + i * FH;
    int k = i + menuVerticalOffset;
    for (int j = 0; j <= k; j++) {
      if (mstate_tab[j] == HIDDEN_ROW)
        k++;
    }

    LcdFlags blink = ((s_editMode > 0) ? BLINK | INVERS : INVERS);
    LcdFlags attr = (menuVerticalPosition == k ? blink : 0);

    if (k >= ITEM_TELEMETRY_SENSOR1 && k < ITEM_TELEMETRY_SENSOR1 + MAX_TELEMETRY_SENSORS) {
      int index = k - ITEM_TELEMETRY_SENSOR1;
      lcdDrawNumber(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, index + 1, LEFT | attr, 0, NULL, ":");
      lcdDrawSizedText(60, y, g_model.telemetrySensors[index].label, TELEM_LABEL_LEN, ZCHAR);
      TelemetryItem &telemetryItem = telemetryItems[index];
      if (telemetryItem.isFresh()) {
        lcdDrawText(130, y, "*");
      }
      if (telemetryItem.isAvailable()) {
        LcdFlags color = telemetryItem.isOld() ? ALARM_COLOR : TEXT_COLOR;
        drawSensorCustomValue(TELEM_COL2, y, index, getValue(MIXSRC_FIRST_TELEM + 3 * index), LEFT | color);
      }
      else {
        lcdDrawText(TELEM_COL2, y, "---"); // TODO shortcut
      }
      TelemetrySensor * sensor = &g_model.telemetrySensors[index];
#if defined(MULTIMODULE)
      if (IS_SPEKTRUM_PROTOCOL()) {
        // Spektrum does not (yet?) really support multiple sensor of the same type. But a lot of
        // different sensor display the same information (e.g. voltage, capacity). Show the id
        // of the sensor in the overview to ease figuring out what sensors belong together
        lcdDrawHexNumber(TELEM_COL5, y, sensor->id, LEFT);
      }
      else
#endif
      if (sensor->type == TELEM_TYPE_CUSTOM && !g_model.ignoreSensorIds) {
        lcdDrawNumber(TELEM_COL5, y, sensor->instance, LEFT);
      }
      if (attr) {
        s_editMode = 0;
        s_currIdx = index;
        if (event == EVT_KEY_LONG(KEY_ENTER)) {
          killEvents(event);
          POPUP_MENU_ADD_ITEM(STR_EDIT);
          POPUP_MENU_ADD_ITEM(STR_COPY);
          POPUP_MENU_ADD_ITEM(STR_DELETE);
          POPUP_MENU_START(onSensorMenu);
        }
        else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
          pushMenu(menuModelSensor);
        }
      }
    }
    else

      switch (k) {
        case ITEM_TELEMETRY_PROTOCOL_TYPE:
          lcdDrawText(MENUS_MARGIN_LEFT, y, STR_TELEMETRY_TYPE);
          lcdDrawTextAtIndex(TELEM_COL2, y, STR_TELEMETRY_PROTOCOLS, g_model.telemetryProtocol, attr);
          g_model.telemetryProtocol = checkIncDec(event, g_model.telemetryProtocol, PROTOCOL_TELEMETRY_FIRST, PROTOCOL_TELEMETRY_LAST, EE_MODEL,
                                                  isTelemetryProtocolAvailable);
          break;


        case ITEM_TELEMETRY_DISCOVER_SENSORS:
          lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, allowNewSensors ? NO_INDENT(STR_STOP_DISCOVER_SENSORS) : NO_INDENT(STR_DISCOVER_SENSORS), attr);
          if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
            s_editMode = 0;
            allowNewSensors = !allowNewSensors;
          }
          break;

        case ITEM_TELEMETRY_NEW_SENSOR:
          lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, NO_INDENT(STR_TELEMETRY_NEWSENSOR), attr);
          if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
            s_editMode = 0;
            int res = availableTelemetryIndex();
            if (res >= 0) {
              s_currIdx = res;
              pushMenu(menuModelSensor);
            }
            else {
              POPUP_WARNING(STR_TELEMETRYFULL);
            }
          }
          break;


#if defined(VARIO)

        case ITEM_TELEMETRY_VARIO_SOURCE:
          lcdDrawText(MENUS_MARGIN_LEFT, y, STR_SOURCE);
          drawSource(TELEM_COL2, y, g_model.frsky.varioSource ? MIXSRC_FIRST_TELEM + 3 * (g_model.frsky.varioSource - 1) : 0, attr);
          if (attr) {
            g_model.frsky.varioSource = checkIncDec(event, g_model.frsky.varioSource, 0, MAX_TELEMETRY_SENSORS, EE_MODEL | NO_INCDEC_MARKS, isSensorAvailable);
          }
          break;
#endif
      }
  }
  return true;
}
#endif