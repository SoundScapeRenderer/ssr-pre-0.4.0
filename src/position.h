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
 * %Position class and helper functions (definition).
 *
 * $LastChangedDate: 2012-11-19 19:58:32 +0100 (Mon, 19. Nov 2012) $
 * $LastChangedRevision: 1994 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef POSITION_H
#define POSITION_H

#include "orientation.h"

/** Geometric representation of a position.
 * Stores the position of a point in space and provides some helper functions.
 * If you want to speak in design patterns, you could call this a "Messenger"
 * patter. It's the most trivial of all patterns. So maybe it's not even worth
 * mentioning. But I did it anyway ...
 * @warning For now, it only uses 2 dimensions (x,y) but a z coordinate can be 
 * added later, if needed.
 **/
struct Position
{
  /** with no arguments, all member variables are initialized to zero.
   * @param x x coordinate (in meters)
   * @param y y coordinate (in meters)
   **/
  explicit Position(const float x = 0, const float y = 0);

  float x; ///< x coordinate (in meters)
  float y; ///< y coordinate (in meters)

  /// length of the position vector
  float length() const;

  /// turn around the origin
  Position& rotate(float angle);
  Position& rotate(const Orientation& rotation);

  Orientation orientation() const;

  Position operator-();                        ///< unary minus operator
  Position& operator+=(const Position& other); ///< += operator
  Position& operator-=(const Position& other); ///< -= operator
  bool operator==(const Position& other);      ///< == operator

  // Declaring the following operators as friend is not really necessary as
  // their fields are public anyway, but it doesn't hurt either.

  /// plus (+) operator
  friend Position operator+(const Position& a, const Position& b);
  /// minus (-) operator
  friend Position operator-(const Position& a, const Position& b);
  /// output stream operator (<<)
  friend std::ostream& operator<<(std::ostream& stream,
      const Position& position);

  /** division (/) operator.
   * @param a dividend, a DirectionalPoint.
   * @param b divisor, any numeric Type..
   * @return quotient.
   **/
  template <typename T>
  friend Position operator/(const Position& a, const T& b)
  {
    return Position(a.x / b, a.y / b);
  }

};

/// Calculate the angle between the position vector of @a point and the
/// orientation @a orientation.
float angle(const Position& point, const Orientation& orientation);
// TODO: declare angle() also as friend of Position?

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
