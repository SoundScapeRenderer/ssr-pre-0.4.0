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
 * Tools (helper functions) for std::auto_ptrs.
 *
 * $LastChangedDate: 2012-02-15 10:26:03 +0100 (Mit, 15. Feb 2012) $
 * $LastChangedRevision: 1693 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef PTRTOOLS_H
#define PTRTOOLS_H

#include <memory>  // for std::auto_ptr

/// helper functions for std::auto_ptrs.
namespace ptrtools
{

/// shortcut to check if a std::auto_ptr is pointing to @b NULL.
// we use a const reference to the auto_ptr to be sure it is not changed!
template <typename T> bool is_null(const std::auto_ptr<T>& pointer)
{
  return !pointer.get();
}

/// shortcut to reset a std::auto_ptr to @b NULL (destroying its object).
template <typename T> void destroy(std::auto_ptr<T>)
{
  // This function is my personal favourite, as it does nothing. At least it
  // seems like that at first sight ...
}

}  // namespace ptrtools

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
