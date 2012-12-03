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
 * %RenderSubscriber (definition).
 *
 * $LastChangedDate: 2012-11-27 19:23:02 +0100 (Die, 27. Nov 2012) $
 * $LastChangedRevision: 2009 $
 * $LastChangedBy: geier.matthias $
 **/

// TODO: the whole publish/subscribe-thing should be redesigned, so maybe the
// RenderSubscriber will be replaced by something else ...
// Also, the actual renderer supports only a subset of the current (r1509)
// Subscriber interface (no audio port names, no file names, ...).

#ifndef RENDERSUBSCRIBER_H
#define RENDERSUBSCRIBER_H

#include "subscriber.h"
#include <map>

namespace ssr
{

template<typename Renderer>
class RenderSubscriber : public Subscriber
{
  public:
    RenderSubscriber(Renderer &renderer) : _renderer(renderer) {}

    // Subscriber Interface
    virtual void set_loudspeakers(const Loudspeaker::container_t& loudspeakers);

    virtual void new_source(id_t id) { (void) id; }

    virtual void delete_source(id_t id)
    {
      _renderer.rem_source(_renderer.get_source(id));
    }

    virtual void delete_all_sources()
    {
      _renderer.rem_all_sources();
    }

    virtual bool set_source_position(id_t id, const Position& position)
    {
      // TODO: locking?
      typename Renderer::Source* src
        = static_cast<typename Renderer::Source*>(_renderer.get_source(id));
      if (!src) return false;
      src->position(position);
      return true;
    }

    virtual bool set_source_orientation(id_t id, const Orientation& orientation)
    {
      // TODO: locking?
      typename Renderer::Source* src
        = static_cast<typename Renderer::Source*>(_renderer.get_source(id));
      if (!src) return false;
      src->orientation(orientation);
      return true;
    }

    virtual bool set_source_gain(id_t id, const float& gain)
    {
      // TODO: locking?
      typename Renderer::Source* src
        = static_cast<typename Renderer::Source*>(_renderer.get_source(id));
      if (!src) return false;
      src->gain(gain);
      return true;
    }

    virtual bool set_source_mute(id_t id, const bool& mute)
    {
      // TODO: locking?
      typename Renderer::Source* src
        = static_cast<typename Renderer::Source*>(_renderer.get_source(id));
      if (!src) return false;
      src->mute(mute);
      return true;
    }

    virtual bool set_source_name(id_t id, const std::string& name)
    {
      (void) id;
      (void) name;
      return true;
    }

    virtual bool set_source_brir_file_name(id_t id, const std::string& name)
    {
      (void) id;
      (void) name;
      return true;
    }

    virtual bool set_source_model(id_t id, const Source::model_t& model)
    {
      // TODO: locking?
      typename Renderer::Source* src
        = static_cast<typename Renderer::Source*>(_renderer.get_source(id));
      if (!src) return false;
      src->model(model);
      return true;
    }

    virtual bool set_source_port_name(id_t id, const std::string& port_name)
    {
      (void) id;
      (void) port_name;
      return true;
    }

    virtual bool set_source_file_name(id_t id, const std::string& file_name)
    {
      (void) id;
      (void) file_name;
      return 1;
    }

    virtual bool set_source_file_channel(id_t id, const int& file_channel)
    {
      (void) id;
      (void) file_channel;
      return 1;
    }

    virtual bool set_source_file_length(id_t id, const long int& length)
    {
      (void) id;
      (void) length;
      return true;
    }

    virtual void set_reference_position(const Position& position)
    {
      // TODO: locking?
      _renderer.state.reference_position(position);
    }

    virtual void set_reference_orientation(const Orientation& orientation)
    {
      // TODO: locking?
      _renderer.state.reference_orientation(orientation);
    }

    virtual void set_reference_offset_position(const Position& position)
    {
      // TODO: locking?
      _renderer.state.reference_offset_position(position);
    }

    virtual void set_reference_offset_orientation(const Orientation& orientation)
    {
      // TODO: locking?
      _renderer.state.reference_offset_orientation(orientation);
    }

    virtual void set_master_volume(float volume)
    {
      // TODO: locking?
      _renderer.state.master_volume(volume);
    }

    virtual void set_source_output_levels(id_t, float*, float*) {}

    virtual void set_processing_state(bool state)
    {
      _renderer.state.processing(state);
    }

    virtual void set_transport_state(
        const std::pair<bool, jack_nframes_t>& state)
    {
      (void) state;
    }

    virtual void set_amplitude_reference_distance(float distance)
    {
      _renderer.state.amplitude_reference_distance(distance);
    }

    virtual void set_master_signal_level(float level)
    {
      (void) level;
    }

    virtual void set_cpu_load(float load)
    {
      (void) load;
    }

    virtual void set_sample_rate(int sample_rate)
    {
      (void) sample_rate;
    }

    virtual bool set_source_signal_level(const id_t id, const float& level)
    {
      (void) id;
      (void) level;
      return true;
    }

    virtual bool set_source_properties_file(ssr::id_t, const std::string&)
    {
      return 1;
    }

    virtual bool set_source_position_fixed(ssr::id_t id, const bool& fix)
    {
      (void) id;
      (void) fix;
      return true;
    }

  private:
    Renderer& _renderer;
};

template<typename Renderer>
void
RenderSubscriber<Renderer>::set_loudspeakers(
    const Loudspeaker::container_t& loudspeakers)
{
  (void)loudspeakers;

  // TODO: handle loudspeakers differently. Maybe remove them from the Scene?
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent

