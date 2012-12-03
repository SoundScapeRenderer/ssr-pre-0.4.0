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
 * %Scene player (definition).
 *
 * $LastChangedDate: 2012-02-05 20:24:15 +0100 (Son, 05. Feb 2012) $
 * $LastChangedRevision: 1675 $
 * $LastChangedBy: geier.matthias $
 **/

#include "sceneplayer.h"

// following functions are for recording functionality - not yet implemented!
/*
void ScenePlayer::new_source(const id_t id)
{}
void ScenePlayer::set_source(const id_t id, const Position& position)
{}
void ScenePlayer::set_source(const id_t id, const Orientation& orientation)
{}
void ScenePlayer::set_source_gain(const id_t id, const float gain)
{}
void ScenePlayer::set_source_mute(const id_t id, const bool mute)
{}
void ScenePlayer::set_source_name(const id_t id,
		const std::string& name)
{}
void ScenePlayer::set_source_model(const id_t id, const Source::model_t model)
{}
void ScenePlayer::set_source_port_name(const id_t id,
		const std::string& port_name)
{}
void ScenePlayer::set_master_volume(const float volume)
{}
void ScenePlayer::new_loudspeaker(const id_t id,
		const DirectionalPoint& relative_position, Loudspeaker::model_t model)
{}
*/

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
