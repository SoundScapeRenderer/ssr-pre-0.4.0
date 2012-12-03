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

#include "qguiframe.h"

QGUIFrame::QGUIFrame(QWidget* parent, int top, int bottom, int left, int right)
  : QWidget(parent)
{
  // build up frame around OpenGL window
  _top = new QLabel(parent);
  //_bottom = new QClickTextLabel(parent);
  _bottom = new QLabel(parent);
  _left = new QLabel(parent);
  _right = new QLabel(parent);

  resize(top, bottom, left, right);
  //connect(_bottom, SIGNAL(clicked()), this, SLOT(_clear_text()));

  _clear_text_timer = new QTimer(this);
  _clear_text_timer->setSingleShot(true);
  connect(_clear_text_timer, SIGNAL(timeout()), this, SLOT(_clear_text()));
}

QGUIFrame::~QGUIFrame()
{
  delete _top;
  delete _bottom;
  delete _left;
  delete _right;
}

void QGUIFrame::mousePressEvent(QMouseEvent *event)
{
  event->accept();
}

void QGUIFrame::mouseMoveEvent(QMouseEvent *event)
{
  event->accept();
}

void QGUIFrame::resize(const int top, const int bottom, 
		       const int left, const int right)
{
  _top->setGeometry( 0, 0, parentWidget()->width(), top );
  _bottom->setGeometry( 0, parentWidget()->height()-bottom, 
			parentWidget()->width(), bottom );
  _left->setGeometry( 0, top, 
		      left, parentWidget()->height()-top-bottom);
  _right->setGeometry( parentWidget()->width()-right, top, 
		       right, parentWidget()->height()-top-bottom );

  _bottom->setIndent(left + 5);
}

void QGUIFrame::set_text(const QString text)
{
  _bottom->setText("<font color=\"#FF0000\">" + text + "</font>");

  if (_clear_text_timer->isActive())
  {
    _clear_text_timer->stop();
  }
      
  _clear_text_timer->start(5000); // 5 seconds
}

void QGUIFrame::_clear_text()
{
  _bottom->clear();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
