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
/// %Scene player (definition).

#ifndef SSR_SCENEPLAYER_H
#define SSR_SCENEPLAYER_H

#include "subscriber.h"

namespace ssr
{

/** sceneplayer.
 * Has to inherit publicly from Subscriber because otherwise it wouldn't be
 * possible to use it e.g. in a std::list<Subscriber*>.
 **/
class ScenePlayer : public Subscriber
{
  public:
    // following functions are only for recording - not yet implemented!
    /*
    virtual void new_source(const id_t id);
    virtual bool set_source(const id_t id, const Position& position);
    virtual bool set_source(const id_t id, const Orientation& orientation);
    virtual bool set_source_gain(const id_t id, const float gain);
    virtual bool set_source_mute(const id_t id, const bool mute);
    virtual bool set_source_name(const id_t id, const std::string& name);
    virtual bool set_source_model(const id_t id, const Source::model_t model);
    virtual bool set_source_port_name(const id_t id,
        const std::string& port_name);
    virtual void set_master_volume(float volume);
    */
};

}  // namespace

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
