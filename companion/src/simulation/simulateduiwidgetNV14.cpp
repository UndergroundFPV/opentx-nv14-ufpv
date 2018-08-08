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

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

#include "simulateduiwidget.h"
#include "simulatorinterface.h"
#include "ui_simulateduiwidgetNV14.h"



SimulatedUIWidgetNV14::SimulatedUIWidgetNV14(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetNV14)
{
  connect(this, &SimulatedUIWidgetNV14::touch, m_simulator, &SimulatorInterface::touchEvent);
  ui->setupUi(this);
  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 252, 30, 30), "X10/left_scrnsht.png", m_screenshotAction));
  m_backlightColors << QColor(47, 123, 227);

  setLcd(ui->lcd);
}

SimulatedUIWidgetNV14::~SimulatedUIWidgetNV14()
{
  delete ui;
}

QPoint SimulatedUIWidgetNV14::getLcdPos(QMouseEvent *event){
    QPoint point = ui->lcd->mapToGlobal(QPoint(ui->lcd->geometry().x(), ui->lcd->geometry().y()));
    QRect rect = QRect(point.x(), point.y(), ui->lcd->geometry().width(), ui->lcd->geometry().height());
    int x = rect.x() > event->globalX() ? rect.x() : event->globalX();
    if(x > rect.x() + rect.width()) x = rect.x() + rect.width();

    int y = rect.y() > event->globalY() ? rect.y() : event->globalY();
    if(y > rect.y() + rect.height()) y = rect.y() + rect.height();

    x = x - rect.x();
    y = y - rect.y();
    return QPoint(x, y);
}

void SimulatedUIWidgetNV14::mouseMoveEvent(QMouseEvent *event){
    if(lastTouchEvent != TE_DOWN && lastTouchEvent != TE_SLIDE) return;
     QPoint point = getLcdPos(event);
     lastTouchEvent = TE_SLIDE;
     emit touch(point.x(), point.y(), lastTouchEvent);
}

void SimulatedUIWidgetNV14::mousePressEvent(QMouseEvent *event){
    if(lastTouchEvent == TE_DOWN) return;
    QPoint point = getLcdPos(event);
    lastTouchEvent = TE_DOWN;
    emit touch(point.x(), point.y(), lastTouchEvent);
}
void SimulatedUIWidgetNV14::mouseReleaseEvent(QMouseEvent *event){
    if (lastTouchEvent == TE_DOWN) {
        QPoint point = getLcdPos(event);
        lastTouchEvent = TE_UP;
        emit touch(point.x(), point.y(), lastTouchEvent);
      }
      else {
        lastTouchEvent = TE_NONE;
        emit touch(0, 0, lastTouchEvent);
      }
}
