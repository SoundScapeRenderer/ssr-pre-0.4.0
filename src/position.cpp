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
 * %Position class and helper functions (implementation).
 *
 * $LastChangedDate: 2012-11-19 19:58:32 +0100 (Mon, 19. Nov 2012) $
 * $LastChangedRevision: 1994 $
 * $LastChangedBy: geier.matthias $
 **/

#include <cmath> // for atan2(), sqrt()
#include <ostream>

#include "position.h"
#include "orientation.h"
#include "apf/math.h"

Position::Position(const float x, const float y) :
  x(x),
  y(y)
{}

Position Position::operator-()
{
  return Position(-this->x, -this->y);
}

Position& Position::operator+=(const Position& other)
{
  x += other.x;
  y += other.y;
  return *this;
}

Position& Position::operator-=(const Position& other)
{
  x -= other.x;
  y -= other.y;
  return *this;
}

bool Position::operator==(const Position& other)
{
  if (x == other.x && y == other.y) return true;
  else return false;
}

/** convert the orientation given by the position vector (x,y) to an
 * Orientation.
 * @return Orientation with the corresponding azimuth value
 * @warning Works only for 2D!
 **/
Orientation Position::orientation() const
{
  return Orientation(atan2(y, x) / apf::math::pi_div_180<float>());
}

float Position::length() const
{
  return sqrt(apf::math::square(x) + apf::math::square(y));
}

/** ._
 * @param angle angle in degrees.
 * @return the resulting position
 **/
Position& Position::rotate(float angle)
{
  // angle phi in radians!
  float phi = apf::math::deg2rad(this->orientation().azimuth + angle);
  float radius = this->length();
  return *this = Position(radius * cos(phi), radius * sin(phi));
}

// this is a 2D implementation!
Position& Position::rotate(const Orientation& rotation)
{
  return this->rotate(rotation.azimuth);
}

Position operator-(const Position& a, const Position& b)
{
  Position temp(a);
  return temp -= b;
}

Position operator+(const Position& a, const Position& b)
{
  Position temp(a);
  return temp += b;
}

/** _.
 * @param point
 * @param orientation
 * @return Angle in radians.
 **/
float angle(const Position& point, const Orientation& orientation)
{
  return angle(point.orientation(), orientation);
}

std::ostream& operator<<(std::ostream& stream, const Position& position)
{
  stream << "x = " << position.x << ", y = " << position.y;
  return stream;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
