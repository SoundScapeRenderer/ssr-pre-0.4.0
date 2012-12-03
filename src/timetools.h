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
 * Provides helper functions for calculations concerning time.
 *
 * $LastChangedDate: 2012-02-15 10:26:03 +0100 (Mit, 15. Feb 2012) $
 * $LastChangedRevision: 1693 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef TIMETOOLS_H
#define TIMETOOLS_H


/// Provides helper functions for calculations concerning time.
namespace timetools
{

/** Returns time interval in seconds between two time instances
 * @param start time instance where interval started
 * @param stop time instance where interval started
 * @return time interval in seconds 
 **/
inline float get_time_interval(struct timeval start, struct timeval stop)
{
  return static_cast<float>(stop.tv_sec - start.tv_sec
      + (stop.tv_usec - start.tv_usec) / 1000000.0);
}

inline bool is_time_stamp_valid(struct timeval time_stamp)
{
  if (time_stamp.tv_sec == 0 && time_stamp.tv_usec == 0) return false;
  else return true;
}

}  // namespace timetools

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
