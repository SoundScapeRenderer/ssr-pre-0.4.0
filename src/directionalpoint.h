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
 * Geometry information of a point including an orientation (definition).
 *
 * $LastChangedDate: 2012-11-19 19:58:32 +0100 (Mon, 19. Nov 2012) $
 * $LastChangedRevision: 1994 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef DIRECTIONALPOINT_H
#define DIRECTIONALPOINT_H

#include <iosfwd>

#include "position.h"
#include "orientation.h"

/** Class which combines Position and Orientation.
 * Anything which <b>has a</b> position and orientation can be derived from it.
 **/
struct DirectionalPoint
{
  DirectionalPoint() {} ///< standard ctor
  DirectionalPoint(const Position& position, const Orientation& orientation);

  Position    position;    ///< position
  Orientation orientation; ///< orientation

  /// Distance between a plane (*this) and a point
  float plane_to_point_distance(const Position& point) const;

  DirectionalPoint& rotate(float angle); ///< rotate around the origin
  /// rotate around the origin
  DirectionalPoint& rotate(const Orientation& rotation);
  /// move and rotate
  DirectionalPoint& transform(const DirectionalPoint& t);

  DirectionalPoint& operator+=(const DirectionalPoint& other);
  DirectionalPoint& operator-=(const DirectionalPoint& other);

  friend DirectionalPoint operator+(const DirectionalPoint& a,
      const DirectionalPoint& b);
  friend DirectionalPoint operator-(const DirectionalPoint& a,
      const DirectionalPoint& b);
  friend std::ostream& operator<<(std::ostream& stream,
      const DirectionalPoint& point);

  /** division (/) operator.
   * @param a dividend, a DirectionalPoint.
   * @param b divisor, any numeric Type..
   * @return quotient.
   **/
  template <typename T>
  friend DirectionalPoint operator/(const DirectionalPoint& a, const T& b)
  {
    return DirectionalPoint(a.position / b, a.orientation / b);
  }
};

/// Angle between the Orientations of two DirectionalPoints.
float angle(const DirectionalPoint& a, const DirectionalPoint& b);

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
