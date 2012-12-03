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
 * Master scene (definition).
 *
 * $LastChangedDate: 2012-08-09 10:25:24 +0200 (Don, 09. Aug 2012) $
 * $LastChangedRevision: 1912 $
 * $LastChangedBy: geier.matthias $
 **/

#ifndef MASTERSCENE_H
#define MASTERSCENE_H

#include <stdexcept> // for std::overflow_error

#include "ssr_global.h"
#include "scene.h"
#include "apf/misc.h"

namespace ssr
{

/** Master scene.
 * Just like a normal scene, but it can additionally generate new source IDs.
 * This class is a singleton. Therefore, the constructor can not be used. You
 * can get the only instance of this class with MasterScene::get().
 **/
class MasterScene : public Scene, apf::NonCopyable
{
  public:
    static MasterScene& get(); ///< get singleton instance
    id_t get_new_source_id() throw (std::overflow_error);

    void delete_all_sources(); /// dito, plus resets the source ID

  private:
    MasterScene() :
      Scene() {} ///< private ctor.

    static MasterScene _instance;    ///< the one and only instance of the class
    static id_t _highest_source_id;  ///< last assigned source ID
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
