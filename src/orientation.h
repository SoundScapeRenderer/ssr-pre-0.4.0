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
 * %Orientation class and helper function(s) (definition).
 *
 * $LastChangedDate: 2012-11-19 19:58:32 +0100 (Mon, 19. Nov 2012) $
 * $LastChangedRevision: 1994 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef ORIENTATION_H
#define ORIENTATION_H

#include <iosfwd>

/** Geometric representation of a orientation.
 * For now, only azimuth value is handled.
 **/
struct Orientation
{
  // the default orientation is in negative y-direction (facing the listener)
  explicit Orientation(const float azimuth = 0);

  float azimuth; ///< (=yaw) azimuth (in degrees)

  friend Orientation operator-(const Orientation& lhs, const Orientation& rhs);
  friend Orientation operator+(const Orientation& lhs, const Orientation& rhs);

  Orientation& operator+=(const Orientation& other);
  Orientation& operator-=(const Orientation& other);

  /// turn
  Orientation& rotate(float angle);
  Orientation& rotate(const Orientation& rotation);

  friend std::ostream& operator<<(std::ostream& stream,
      const Orientation& orientation);

  /** division (/) operator.
   * @param a dividend, a DirectionalPoint.
   * @param b divisor, any numeric Type..
   * @return quotient.
   **/
  template <typename T>
  friend Orientation operator/(const Orientation& a, const T& b)
  {
    return Orientation(a.azimuth / b);
  }
};

/// Angle (in radians) between two orientations.
float angle(const Orientation& a, const Orientation& b);

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
