/******************************************************************************
 * Copyright © 2012      Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
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
 * http://SoundScapeRenderer.github.com                  ssr@spatialaudio.net *
 ******************************************************************************/

/// @file
/// Master scene (definition).

#ifndef SSR_MASTERSCENE_H
#define SSR_MASTERSCENE_H

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
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
