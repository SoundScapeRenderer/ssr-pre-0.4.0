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
 * Master scene (implementation).
 *
 * $LastChangedDate: 2012-11-21 13:29:11 +0100 (Mit, 21. Nov 2012) $
 * $LastChangedRevision: 2003 $
 * $LastChangedBy: geier.matthias $
 **/

#include <limits>       // for std::numeric_limits
#include "ssr_global.h" // for __POS__, ssr::id_t

#include "masterscene.h"

using ssr::id_t;

// initialize static variables:
id_t ssr::MasterScene::_highest_source_id = 0;
ssr::MasterScene ssr::MasterScene::_instance; // the only time the ctor is invoked

ssr::MasterScene& ssr::MasterScene::get() { return _instance; }

/** Create a new source.
 * @return Unique ID of the new source
 * @throw std::overflow_error if numeric limit of id_t is reached. This is very
 * very unlikely to ever happen!
 **/
id_t ssr::MasterScene::get_new_source_id() throw (std::overflow_error)
{
  if (_highest_source_id == std::numeric_limits<id_t>::max())
  {
    // this will actually never happen:
    throw std::overflow_error("Numeric limit reached for source ID! " __POS__);
  }
  id_t new_id = ++_highest_source_id;
  //_source_map[new_id] = Source(); // standard ctor
  return new_id;
}

void ssr::MasterScene::delete_all_sources()
{
  std::cout << "MasterScene::delete_all_sources()" << std::endl;
  Scene::delete_all_sources();
  _highest_source_id = 0;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
