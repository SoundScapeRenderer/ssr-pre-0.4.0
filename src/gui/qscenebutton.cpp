/******************************************************************************
 * Copyright (c) 2006-2012 Quality & Usability Lab                            *
 *                         Deutsche Telekom Laboratories, TU Berlin           *
 *                         Ernst-Reuter-Platz 7, 10587 Berlin, Germany        *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://tu-berlin.de/?id=ssr                  SoundScapeRenderer@telekom.de *
 ******************************************************************************/

/** @file
 * TODO: add description
 *
 * $LastChangedDate: 2012-02-05 20:24:15 +0100 (Son, 05. Feb 2012) $
 * $LastChangedRevision: 1675 $
 * $LastChangedBy: geier.matthias $
 **/

#include <QPainter>

#include "qscenebutton.h"

#define MAXNUMBEROFCHARACTERS 9

QSceneButton::QSceneButton(QWidget* parent, const QString& text_in, const QString& path)
  : QPushButton(parent), path(path)
{
  // limit number of characters
  if (text_in.length() > MAXNUMBEROFCHARACTERS){
    text = text_in.left(MAXNUMBEROFCHARACTERS);
    text.append("...");
  }
  else
    text = text_in;
}

void QSceneButton::mousePressEvent(QMouseEvent *event)
{
  if ( !isChecked() )
    QPushButton::mousePressEvent(event);
  else return;

  emit signal_open_scene(path);
}

void QSceneButton::mouseMoveEvent(QMouseEvent *event)
{
  // prevend action of QPushButton
  event->accept();
}

void QSceneButton::paintEvent( QPaintEvent * event)
{
  // draw QLabel stuff
  QPushButton::paintEvent(event);

  // draw custom stuff
  QPainter painter(this);

  // enable anti-aliasing
  painter.setRenderHint(QPainter::Antialiasing);

  // black
  painter.setPen(QPen(QColor(0,0,0),1));

  // plot arrow
  if ( isChecked() ){
    // draw down-arrow
    painter.drawLine(QLineF( 80.0f,  7.0f, 80.0f, 16.0f)); // root
    painter.drawLine(QLineF( 76.0f, 13.0f, 80.0f, 16.0f)); // left branch
    painter.drawLine(QLineF( 84.0f, 13.0f, 80.0f, 16.0f)); // right branch
  }
  else {
    // draw right-arrow
    painter.drawLine(QLineF( 77.0f, 10.0f, 86.0f, 10.0f)); // root
    painter.drawLine(QLineF( 83.0f, 14.0f, 86.0f, 10.0f)); // lower branch
    painter.drawLine(QLineF( 83.0f,  6.0f, 86.0f, 10.0f)); // upper branch
  }

  // enable text anti-aliasing
  painter.setRenderHint(QPainter::TextAntialiasing);

  // display text
  painter.drawText(QPointF(10.0f, 13.0f), text);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
