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
 * Some helper functions for std::map's.
 *
 * $LastChangedDate: 2012-02-15 10:26:03 +0100 (Mit, 15. Feb 2012) $
 * $LastChangedRevision: 1693 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef MAPTOOLS_H
#define MAPTOOLS_H

#include <map>

/** Some helper functions for std::maps.
 * We frequently use maps where the second element is a pointer to an object.
 * The functions in this namespace should facilitate access to and disposal of
 * those objects.
 **/
namespace maptools
{

/// @name several overloaded versions of get_item()
/// There are versions for maps of objects and for maps of pointers to
/// objects. Furthermore, there are const and non-const versions.
//@{

/** Get the pointer to an element of a map.
 * @param m the std::map we want to get the item from
 * @param id the key of the element we want to obtain
 * @return Pointer to the selected object.
 **/
template <typename idT, typename dataT>
const dataT* get_item(const std::map<idT,dataT*>& m, const idT id)
{
  typename std::map<idT,dataT*>::const_iterator iter = m.find(id);
  if (iter != m.end()) return iter->second;
  else return NULL;
}

// non-const version
template <typename idT, typename dataT>
dataT* get_item(std::map<idT,dataT*>& m, const idT id)
{
  typename std::map<idT,dataT*>::iterator iter = m.find(id);
  if (iter != m.end()) return iter->second;
  else return NULL;
}

template <typename idT, typename dataT>
const dataT* get_item(const std::map<idT,dataT>& m, const idT id)
{
  typename std::map<idT,dataT>::const_iterator iter = m.find(id);
  if (iter != m.end()) return &(iter->second);
  else return NULL;
}

// non-const version
template <typename idT, typename dataT>
dataT* get_item(std::map<idT,dataT>& m, const idT id)
{
  typename std::map<idT,dataT>::iterator iter = m.find(id);
  if (iter != m.end()) return &(iter->second);
  else return NULL;
}

//@}

/** Deletes one element of a map after deleting the object it points to.
 * @param m a std::map
 * @param id identifier of the element you want to destroy.
 * @return @b true on success. Well, to be honest, always @b true.
 **/
template<typename dataT, typename idT>
bool delete_item(std::map<idT, dataT*>& m, const idT& id)
{
  typename std::map<idT, dataT*>::iterator delinquent = m.find(id);
  if (delinquent == m.end())
  {
    //WARNING("maptools::delete_item: '" << id << "' does not exist!");
    return true; // anyway, the function kind of succeeded, didn't it?
  }
  delete delinquent->second;
  m.erase(delinquent);
  return true;
}

/** Delete all map elements and clear the map.
 * @param delinquent the map to be cleared.
 **/
template <typename idT, typename dataT>
void purge(std::map<idT,dataT*>& delinquent)
{
  for (typename std::map<idT,dataT*>::const_iterator i = delinquent.begin();
      i != delinquent.end(); ++i)
  {
    delete i->second;
  }
  delinquent.clear();
}

}  // namespace maptools

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
