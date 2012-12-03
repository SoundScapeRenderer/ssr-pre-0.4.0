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
 * $LastChangedDate: 2009-01-16 16:30:04 +0100 (Fri, 16 Jan 2009) $
 * $LastChangedRevision: 506 $
 * $LastChangedBy: ahrens.jens $
 **/


#include "qtimeedit.h"
//#include "ssr_global.h"

QTimeEdit::QTimeEdit(QWidget* parent) : QLineEdit(parent)
{
  QString qt_style_sheet = "* { background-color: white; \n"
                               "border-radius: 0;       \n"
                               "border-width: 1px;      \n"
                               "border-color: rgb(237,237,230); }";

  this->setStyleSheet(qt_style_sheet);
}


void QTimeEdit::keyPressEvent(QKeyEvent *event)
{
  QLineEdit::keyPressEvent(event);
  
  // to avoid propagating of RETURN to main widget
  event->accept();
}
// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
