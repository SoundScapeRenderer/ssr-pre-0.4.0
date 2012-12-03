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
 * %Orientation class and helper function(s) (implementation).
 *
 * $LastChangedDate: 2012-11-19 19:58:32 +0100 (Mon, 19. Nov 2012) $
 * $LastChangedRevision: 1994 $
 * $LastChangedBy: geier.matthias $
 **/

#include <ostream>

#include "orientation.h"
#include "apf/math.h"
#include "ssr_global.h"

/// ctor. @param azimuth azimuth (in degrees)
Orientation::Orientation(const float azimuth) :
  azimuth(azimuth)
{}

/** - operator.
 * @return difference of Orientations
 **/
Orientation operator-(const Orientation& lhs, const Orientation& rhs)
{
  // VBAP renderer needs wrapping 
  return Orientation(apf::math::wrap_angle(lhs.azimuth - rhs.azimuth));
}

/** + operator.
 **/
Orientation operator+(const Orientation& lhs, const Orientation& rhs)
{
  // VBAP renderer needs wrapping 
  return Orientation(apf::math::wrap_angle(lhs.azimuth + rhs.azimuth));
}

/** += operator.
 * @param other addend.
 * @return sum of Orientations
 **/
Orientation& Orientation::operator+=(const Orientation& other)
{
  azimuth += other.azimuth;
  return *this;
}

/** -= operator.
 * @param other minuend.
 * @return difference of Orientations
 **/
Orientation& Orientation::operator-=(const Orientation& other)
{
  azimuth -= other.azimuth;
  return *this;
}

/** ._
 * @param angle angle in degrees.
 * @return the resulting orientation
 **/
Orientation& Orientation::rotate(float angle)
{
  this->azimuth += angle;
  return *this;
}

// this is only a 2D implementation!
Orientation& Orientation::rotate(const Orientation& rotation)
{
  return this->rotate(rotation.azimuth);
}

/** _.
 * @param a One orientation
 * @param b Another orientation
 * @return Angle between the two orientations in radians. If the angle of @a b
 * is bigger than the angle of @a a, the result is negative.
 * @warning 2D implementation!
 **/
float angle(const Orientation& a, const Orientation& b)
{
  return apf::math::deg2rad(a.azimuth - b.azimuth);
}

/// output stream operator (<<)
std::ostream& operator<<(std::ostream& stream, const Orientation& orientation)
{
  stream << "azimuth = " << orientation.azimuth;
  return stream;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
