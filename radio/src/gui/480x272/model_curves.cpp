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
#include "model_curves.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

bool isCurveFilled(uint8_t index)
{
  CurveInfo &curve = g_model.curves[index];
  int8_t * points = curveAddress(index);
  for (int i = 0; i < 5 + curve.points; i++) {
    if (points[i] != 0) {
      return true;
    }
  }
  return false;
}

class CurveEditWindow : public Page {
  public:
    explicit CurveEditWindow(uint8_t index):
      Page(),
      index(index)
    {
      buildBody(&body);
      buildHeader(&header);
    }

  protected:
    uint8_t index;
    CurveEdit * curveEdit = nullptr;

    void buildHeader(Window * window)
    {
      new StaticText(window, {70, 4, LCD_W - 100, 20}, STR_MENUCURVE, MENU_TITLE_COLOR);
      char s[16];
      strAppendStringWithIndex(s, STR_CV, index + 1);
      new StaticText(window, {70, 28, LCD_W - 100, 20}, s, MENU_TITLE_COLOR);
    }

    void buildBody(Window * window)
    {
      GridLayout grid(*window);
      grid.setMarginLeft(20);
      grid.setMarginRight(20);
      grid.setLabelWidth(100);
      grid.spacer(20);

      CurveInfo & curve = g_model.curves[index];
      int8_t * points = curveAddress(index);

      // Curve editor
      curveEdit = new CurveEdit(window, { 20, grid.getWindowHeight(), LCD_W - 40, LCD_W - 40}, index);
      grid.spacer(curveEdit->height() + 15);

      // Name
      new StaticText(window, grid.getLabelSlot(), STR_NAME);
      new TextEdit(window, grid.getFieldSlot(), curve.name, sizeof(curve.name));
      grid.nextLine();

      // Type
      new StaticText(window, grid.getLabelSlot(), STR_TYPE);
      new Choice(window, grid.getFieldSlot(2, 0), STR_CURVE_TYPES, 0, 1, GET_DEFAULT(g_model.curves[index].type),
                 [=](int32_t newValue) {
                   CurveInfo &curve = g_model.curves[index];
                   if (newValue != curve.type) {
                     for (int i = 1; i < 4 + curve.points; i++) {
                       points[i] = calcRESXto100(applyCustomCurve(calc100toRESX(-100 + i * 200 / (4 + curve.points)), index));
                     }
                     if (moveCurve(index, newValue == CURVE_TYPE_CUSTOM ? 3 + curve.points : -3 - curve.points)) {
                       if (newValue == CURVE_TYPE_CUSTOM) {
                         resetCustomCurveX(points, 5 + curve.points);
                       }
                       curve.type = newValue;
                     }
                     SET_DIRTY();
                     curveEdit->update();
                   }
                 });

      // Points count
      auto edit = new NumberEdit(window, grid.getFieldSlot(2, 1), 2, 17, GET_DEFAULT(g_model.curves[index].points + 5),
                                 [=](int32_t newValue) {
                                   newValue -= 5;
                                   CurveInfo &curve = g_model.curves[index];
                                   int newPoints[MAX_POINTS_PER_CURVE];
                                   newPoints[0] = points[0];
                                   newPoints[4 + newValue] = points[4 + curve.points];
                                   for (int i = 1; i < 4 + newValue; i++)
                                     newPoints[i] = calcRESXto100(applyCustomCurve(-RESX + (i * 2 * RESX) / (4 + newValue), index));
                                   if (moveCurve(index, (newValue - curve.points) * (curve.type == CURVE_TYPE_CUSTOM ? 2 : 1))) {
                                     for (int i = 0; i < 5 + newValue; i++) {
                                       points[i] = newPoints[i];
                                       if (curve.type == CURVE_TYPE_CUSTOM && i != 0 && i != 4 + newValue)
                                         points[5 + newValue + i - 1] = -100 + (i * 200) / (4 + newValue);
                                     }
                                     curve.points = newValue;
                                     SET_DIRTY();
                                     curveEdit->update();
                                   }
                                 });
      edit->setSuffix(STR_PTS);
      grid.nextLine();

      // Smooth
      new StaticText(window, grid.getLabelSlot(), STR_SMOOTH);
      new CheckBox(window, grid.getFieldSlot(), GET_DEFAULT(g_model.curves[index].smooth),
                   [=](int32_t newValue) {
                     g_model.curves[index].smooth = newValue;
                     SET_DIRTY();
                     curveEdit->update();
                   });
      grid.nextLine();

    }
};

class CurveButton : public Button {
  public:
    CurveButton(Window * parent, const rect_t &rect, uint8_t index) :
      Button(parent, rect),
      index(index)
    {
      if (isCurveFilled(index)) {
        setHeight(130);
        new CurveWindow(this, {5, 5, 120, 120},
                        [=](int x) -> int {
                          return applyCustomCurve(x, index);
                        });
      }
    }

    virtual void paint(BitmapBuffer * dc) override
    {
      // bounding rect
      drawSolidRect(dc, 0, 0, rect.w, rect.h, 2, hasFocus() ? SCROLLBOX_COLOR : CURVE_AXIS_COLOR);

      // curve characteristics
      if (isCurveFilled(index)) {
        CurveInfo &curve = g_model.curves[index];
        drawNumber(dc, 130, 5, 5 + curve.points, LEFT, 0, nullptr, STR_PTS);
        drawTextAtIndex(dc, 130, 25, STR_CURVE_TYPES, curve.type);
        if (curve.smooth)
          dc->drawText(130, 45, "Smooth");
      }
    }

  protected:
    uint8_t index;
};

ModelCurvesPage::ModelCurvesPage() :
  PageTab(STR_MENUCURVES, ICON_MODEL_CURVES)
{
}

void ModelCurvesPage::rebuild(Window * window, int8_t focusIndex)
{
  coord_t scrollPosition = window->getScrollPositionY();
  window->clear();
  build(window, focusIndex);
  window->setScrollPositionY(scrollPosition);
}

void ModelCurvesPage::editCurve(Window * window, uint8_t curve)
{
  Window * editWindow = new CurveEditWindow(curve);
  editWindow->setCloseHandler([=]() {
    rebuild(window, curve);
  });
}

void ModelCurvesPage::build(Window * window, int8_t focusIndex)
{
  GridLayout grid(*window);
  grid.spacer(8);
  grid.setLabelWidth(70);

  for (uint8_t index = 0; index < MAX_CURVES; index++) {
    CurveInfo &curve = g_model.curves[index];
    int8_t * points = curveAddress(index);

    new TextButton(window, grid.getLabelSlot(), getCurveString(1 + index));

    Button * button = new CurveButton(window, grid.getFieldSlot(), index);
    button->setPressHandler([=]() -> uint8_t {
      Menu * menu = new Menu();
      menu->addLine(STR_EDIT, [=]() {
        editCurve(window, index);
      });
      menu->addLine(STR_CURVE_PRESET, [=]() {
        Menu * menu = new Menu();
        for (int angle = -45; angle <= 45; angle += 15) {
          char label[16];
          strAppend(strAppendSigned(label, angle), "@");
          menu->addLine(label, [=]() {
            int dx = 2000 / (5 + curve.points - 1);
            for (uint8_t i = 0; i < 5 + curve.points; i++) {
              int x = -1000 + i * dx;
              points[i] = div_and_round(angle * x, 450);
            }
            if (curve.type == CURVE_TYPE_CUSTOM) {
              resetCustomCurveX(points, 5 + curve.points);
            }
            storageDirty(EE_MODEL);
            rebuild(window, index);
          });
        }
      });
      if (isCurveFilled(index)) {
        menu->addLine(STR_MIRROR, [=]() {
          for (int i = 0; i < 5 + curve.points; i++)
            points[i] = -points[i];
          storageDirty(EE_MODEL);
          button->invalidate();
        });
        menu->addLine(STR_CLEAR, [=]() {
          for (int i = 0; i < 5 + curve.points; i++)
            points[i] = 0;
          if (curve.type == CURVE_TYPE_CUSTOM)
            resetCustomCurveX(points, 5 + curve.points);
          storageDirty(EE_MODEL);
          rebuild(window, index);
        });
      }
      return 0;
    });

    if (focusIndex == index) {
      button->setFocus();
    }

    grid.spacer(button->height() + 5);
  }

  window->setInnerHeight(grid.getWindowHeight());
}

# if 0
bool menuModelCurveOne(event_t event)
{
  static uint8_t pointsOfs = 0;
  CurveData & crv = g_model.curves[s_curveChan];
  int8_t * points = curveAddress(s_curveChan);

  SUBMENU_WITH_OPTIONS(STR_MENUCURVE, ICON_MODEL_CURVES, IS_COORDS1_LINE_NEEDED() ? 6 : 5, OPTION_MENU_NO_FOOTER, { 0, 0, 0, 0, uint8_t(5+crv.points-1), uint8_t(5+crv.points-1) });
  drawStringWithIndex(50, 3+FH, STR_CV, s_curveChan+1, MENU_TITLE_COLOR);
  lcdDrawSolidFilledRect(0, MENU_FOOTER_TOP, 250, MENU_FOOTER_HEIGHT, HEADER_BGCOLOR);

  if (IS_COORDS1_LINE_NEEDED() && menuVerticalPosition == ITEM_CURVE_COORDS1) {
    if (menuHorizontalPosition == 0) {
      if (CURSOR_MOVED_RIGHT(event))
        menuHorizontalPosition = 1;
      else
        menuVerticalPosition -= 1;
    }
    else if (menuHorizontalPosition == 4+crv.points) {
      if (CURSOR_MOVED_RIGHT(event))
        (menuHorizontalPosition = 0, menuVerticalPosition += 1);
      else
        menuHorizontalPosition -= 1;
    }
  }

  // Curve name
  lcdDrawText(MENUS_MARGIN_LEFT, MENU_CONTENT_TOP-FH, STR_NAME);
  editName(MODEL_CURVE_ONE_2ND_COLUMN, MENU_CONTENT_TOP-FH, crv.name, sizeof(crv.name), event, menuVerticalPosition==ITEM_CURVE_NAME);

  // Curve type
  LcdFlags attr = (menuVerticalPosition==ITEM_CURVE_TYPE ? (s_editMode>0 ? INVERS|BLINK : INVERS) : 0);
  lcdDrawText(MENUS_MARGIN_LEFT, MENU_CONTENT_TOP, "Type");
  lcdDrawTextAtIndex(MODEL_CURVE_ONE_2ND_COLUMN, MENU_CONTENT_TOP, STR_CURVE_TYPES, crv.type, attr);
  if (attr) {
    uint8_t newType = checkIncDecModelZero(event, crv.type, CURVE_TYPE_LAST);
    if (newType != crv.type) {
      for (int i = 1; i < 4 + crv.points; i++) {
        points[i] = calcRESXto100(applyCustomCurve(calc100toRESX(-100 + i * 200 / (4 + crv.points)), s_curveChan));
      }
      if (moveCurve(s_curveChan, checkIncDec_Ret > 0 ? 3 + crv.points : -3 - crv.points)) {
        if (newType == CURVE_TYPE_CUSTOM) {
          resetCustomCurveX(points, 5 + crv.points);
        }
        crv.type = newType;
      }
    }
  }

  // Curve points count
  attr = (menuVerticalPosition==ITEM_CURVE_POINTS ? (s_editMode>0 ? INVERS|BLINK : INVERS) : 0);
  lcdDrawText(MENUS_MARGIN_LEFT, MENU_CONTENT_TOP + FH, STR_COUNT);
  lcdDrawNumber(MODEL_CURVE_ONE_2ND_COLUMN, MENU_CONTENT_TOP + FH, 5+crv.points, LEFT|attr, 0, NULL, STR_PTS);
  if (attr) {
    int count = checkIncDecModel(event, crv.points, -3, 12); // 2pts - 17pts
    if (checkIncDec_Ret) {
      int newPoints[MAX_POINTS_PER_CURVE];
      newPoints[0] = points[0];
      newPoints[4+count] = points[4+crv.points];
      for (int i=1; i<4+count; i++)
        newPoints[i] = calcRESXto100(applyCustomCurve(-RESX + (i * 2 * RESX) / (4 + count), s_curveChan));
      if (moveCurve(s_curveChan, checkIncDec_Ret*(crv.type==CURVE_TYPE_CUSTOM ? 2 :1))) {
        for (int i = 0; i < 5 + count; i++) {
          points[i] = newPoints[i];
          if (crv.type == CURVE_TYPE_CUSTOM && i != 0 && i != 4 + count)
            points[5 + count + i - 1] = -100 + (i * 200) / (4 + count);
        }
        crv.points = count;
      }
    }
  }

  // Curve smooth
  lcdDrawText(MENUS_MARGIN_LEFT, MENU_CONTENT_TOP + 2*FH, STR_SMOOTH);
  drawCheckBox(MODEL_CURVE_ONE_2ND_COLUMN, MENU_CONTENT_TOP + 2*FH, crv.smooth, menuVerticalPosition==ITEM_CURVE_SMOOTH ? INVERS : 0);
  if (menuVerticalPosition==ITEM_CURVE_SMOOTH) crv.smooth = checkIncDecModel(event, crv.smooth, 0, 1);

  switch(event) {
    case EVT_ENTRY:
      pointsOfs = 0;
      break;
    case EVT_KEY_LONG(KEY_ENTER):
      if (menuVerticalPosition > ITEM_CURVE_NAME) {
        killEvents(event);
        POPUP_MENU_ADD_ITEM(STR_CURVE_PRESET);
        POPUP_MENU_ADD_ITEM(STR_MIRROR);
        POPUP_MENU_ADD_ITEM(STR_CLEAR);
        POPUP_MENU_START(onCurveOneMenu);
      }
      break;
    // TODO?
    // case EVT_KEY_LONG(KEY_MENU):
    //  pushMenu(menuChannelsView);
    //  killEvents(event);
  }

  drawCurve(CURVE_CENTER_X, CURVE_CENTER_Y, CURVE_SIDE_WIDTH);
  drawCurveHorizontalScale();
  if (menuVerticalPosition < ITEM_CURVE_COORDS1) drawCurveVerticalScale(CURVE_CENTER_X-CURVE_SIDE_WIDTH-15);

  coord_t posX = 47;
  attr = (s_editMode > 0 ? INVERS|BLINK : INVERS);
  for (int i=0; i<5+crv.points; i++) {
    point_t point = getPoint(i);
    uint8_t selectionMode = 0;
    if (menuHorizontalPosition == i) {
      if (menuVerticalPosition == ITEM_CURVE_COORDS1)
        selectionMode = (IS_COORDS1_LINE_NEEDED() ? 1 : 2);
      else if (menuVerticalPosition == ITEM_CURVE_COORDS2)
        selectionMode = 2;
    }

    int8_t x = -100 + 200*i/(5+crv.points-1);
    if (crv.type==CURVE_TYPE_CUSTOM && i>0 && i<5+crv.points-1) {
      x = points[5+crv.points+i-1];
    }

    if (i>=pointsOfs && i<pointsOfs+5) {
      lcdDrawNumber(posX, MENU_CONTENT_TOP + 5*FH-12, i+1, TEXT_DISABLE_COLOR|RIGHT);
      lcdDrawNumber(posX, MENU_CONTENT_TOP + 6*FH-10,   x, RIGHT|(selectionMode==1 ? attr : 0));
      lcdDrawNumber(posX, MENU_CONTENT_TOP + 7*FH-6, points[i], RIGHT|(selectionMode==2 ? attr : 0));
      posX += 45;
    }

    if (selectionMode > 0) {
      lcdDrawSolidFilledRect(point.x, CURVE_CENTER_Y-CURVE_SIDE_WIDTH, 2, 2*CURVE_SIDE_WIDTH+2, CURVE_CURSOR_COLOR);

      char text[5];
      strAppendSigned(text, points[i]);

      if (point.x >= CURVE_CENTER_X) {
        drawCurveVerticalScale(point.x-15);
        if (points[i-1] > points[i]) {
          drawCurveCoord(point.x+1-CURVE_COORD_WIDTH, point.y, text, selectionMode==2);
        }
        else {
          drawCurveCoord(point.x+1-CURVE_COORD_WIDTH, point.y-CURVE_COORD_HEIGHT-1, text, selectionMode==2);
        }
      }
      else {
        drawCurveVerticalScale(point.x+7);
        if (points[i+1] > points[i]) {
          drawCurveCoord(point.x+1, point.y, text, selectionMode==2);
        }
        else {
          drawCurveCoord(point.x+1, point.y-CURVE_COORD_HEIGHT-1, text, selectionMode==2);
        }
      }

      drawCurvePoint(point.x-3, point.y-4, CURVE_CURSOR_COLOR);

      strAppendSigned(text, x);
      drawCurveCoord(limit<int>(CURVE_CENTER_X-CURVE_SIDE_WIDTH-1, point.x-CURVE_COORD_WIDTH/2, CURVE_CENTER_X+CURVE_SIDE_WIDTH-CURVE_COORD_WIDTH+1), CURVE_CENTER_Y+CURVE_SIDE_WIDTH+2, text, selectionMode==1);

      if (s_editMode > 0) {
        if (selectionMode == 1)
          CHECK_INCDEC_MODELVAR(event, points[5+crv.points+i-1], i==1 ? -100 : points[5+crv.points+i-2], i==5+crv.points-2 ? 100 : points[5+crv.points+i]);  // edit X
        else if (selectionMode == 2)
          CHECK_INCDEC_MODELVAR(event, points[i], -100, 100);
      }
      if (i < pointsOfs)
        pointsOfs = i;
      else if (i > pointsOfs+5-1)
        pointsOfs = i-5+1;
    }
    else {
      drawCurvePoint(point.x-3, point.y-4, TEXT_COLOR);
    }
  }

  lcdDrawHorizontalLine(MENUS_MARGIN_LEFT, MENU_CONTENT_TOP + 6*FH - 13, SUBMENU_LINE_WIDTH, DOTTED, CURVE_AXIS_COLOR);
  lcdDrawHorizontalLine(MENUS_MARGIN_LEFT, MENU_CONTENT_TOP + 7*FH - 10, SUBMENU_LINE_WIDTH, DOTTED, CURVE_AXIS_COLOR);
  drawHorizontalScrollbar(MENUS_MARGIN_LEFT, MENU_CONTENT_TOP + 9*FH - 15, SUBMENU_LINE_WIDTH, pointsOfs, 5+crv.points, 5);

  return true;
}
#endif
