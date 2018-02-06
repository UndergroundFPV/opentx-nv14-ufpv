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

#ifndef _LCDWIDGET_H_
#define _LCDWIDGET_H_

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QTouchEvent>

#define LCD_WIDGET_REFRESH_PERIOD    16  // [ms] 16 = 62.5fps

class LcdWidget : public QWidget
{
  Q_OBJECT

  public:

    LcdWidget(QWidget * parent = 0);
    ~LcdWidget();

    void setData(unsigned char *buf, int width, int height, int depth=1);
    void setBgDefaultColor(const QColor & color);
    void setBackgroundColor(const QColor & color);
    void setAcceptTouchEvents(bool on);
    void makeScreenshot(const QString & fileName);
    void onLcdChanged(bool light);

  signals:
    void simulatorTouchEvent(QTouchEvent * event);

  protected:
    inline void doPaint(QPainter & p);
    void paintEvent(QPaintEvent*);
    bool touchEvent(QTouchEvent * event);
    bool event(QEvent * event) override;

    int lcdWidth;
    int lcdHeight;
    int lcdDepth;
    int lcdSize;

    unsigned char *lcdBuf;
    unsigned char *localBuf;

    bool lightEnable;
    bool acceptTouchEvents;
    QColor bgColor;
    QColor bgDefaultColor;
    QMutex lcdMtx;
    QElapsedTimer redrawTimer;
};

#endif // _LCDWIDGET_H_
