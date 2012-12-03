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
 * QClickTextLabel
 *
 * $LastChangedDate: 2012-02-05 20:24:15 +0100 (Son, 05. Feb 2012) $
 * $LastChangedRevision: 1675 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef QCLICKTEXTLABEL_H
#define QCLICKTEXTLABEL_H

#include <QLabel>
#include <QMouseEvent>

/// QClickTextLabel
class QClickTextLabel : public QLabel
{
  Q_OBJECT

  public:
    QClickTextLabel( QWidget* parent = 0, int ID = 0);
    QClickTextLabel( const QString& text, QWidget * parent = 0);

  protected:
    int ID;

    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

  signals:
    void clicked();
    void clicked(int);
    void signal_double_clicked();
    void signal_right_clicked(QMouseEvent *);

};

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
