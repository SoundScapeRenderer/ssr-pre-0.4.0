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
/// Wave Field Synthesis renderer.

#ifndef SSR_WFSRENDERER_H
#define SSR_WFSRENDERER_H

#include <sndfile.hh>  // C++ bindings for libsndfile

#include "loudspeakerrenderer.h"

#include "apf/convolver.h"  // for StaticConvolver
#include "apf/blockdelayline.h"  // for NonCausalBlockDelayLine

#include "apf/stringtools.h"
using apf::str::A2S;

// TODO: make more flexible option:
#define WEIGHTING_OLD
//#define WEIGHTING_DELFT

namespace ssr
{

class WfsRenderer : public SourceToOutput<WfsRenderer, LoudspeakerRenderer>
{
  private:
    typedef SourceToOutput<WfsRenderer, LoudspeakerRenderer> _base;

  public:
    static const char* name() { return "WFS-Renderer"; }

    class Input;
    class Source;
    class SourceChannel;
    class Output;
    class RenderFunction;

    WfsRenderer(const apf::parameter_map& params)
      : _base(params)
      , _fade(this->block_size())
      , _max_delay(this->params.get("delayline_size", 0))
      , _initial_delay(this->params.get("initial_delay", 0))
    {
      // TODO: compute "ideal" initial delay?
      // TODO: check if given initial delay is sufficient?

      // TODO: make option --prefilter=none?

      // TODO: get pre-filter from reproduction setup!
      // TODO: allow alternative files for different sample rates

      std::string filename = this->params.get("prefilter_file", "");

      if (filename == "")
      {
        throw std::logic_error("No WFS pre-filter file specified!");
      }

      SndfileHandle prefilter(filename, SFM_READ);

      if (!prefilter)
      {
        throw std::logic_error("\"" + filename + "\" couldn't be loaded!");
      }

      const size_t no_of_channels = prefilter.channels();
      if (no_of_channels != 1)
      {
        throw std::logic_error(
            "WFS pre-filter must have exactly one channel, \"" + filename
            + "\" has " + A2S(no_of_channels) + "!");
      }

      const size_t sr = prefilter.samplerate();
      if (sr != this->sample_rate())
      {
        throw std::logic_error("\"" + filename
            + "\": WFS pre-filter sample rate mismatch ("
            + A2S(this->sample_rate()) + " vs. " + A2S(sr) + ")!");
      }

      size_t size = prefilter.frames();

      apf::fixed_vector<sample_type> ir(size);

      size = prefilter.readf(ir.begin(), size);

      // TODO: warning if size changed?
      // TODO: warning if size == 0?

      const size_t partitions
        = (size + this->block_size() - 1) / (this->block_size());

      _pre_filter.reset(new apf::Convolver::filter_t(
            std::make_pair(partitions, 2 * this->block_size())));

      // create local Convolver to prepare filters (1 partition is enough)
      apf::Convolver temp_convolver(this->block_size(), 1);

      temp_convolver.prepare_filter(ir.begin(), ir.end(), *_pre_filter);
    }

  private:
    apf::raised_cosine_fade<sample_type> _fade;
    std::auto_ptr<apf::Convolver::filter_t> _pre_filter;

    size_t _max_delay, _initial_delay;
};

class WfsRenderer::Input : public _base::Input
{
  public:
    friend class Source;  // give access to _delayline

    Input(const Params& p)
      : _base::Input(p)
      , _convolver(this->parent.block_size(), *this->parent._pre_filter)
      , _delayline(this->parent.block_size(), this->parent._max_delay
          , this->parent._initial_delay)
    {}

    APF_PROCESS(Input, _base::Input)
    {
      _convolver.add_input_block(this->buffer.begin());
      _delayline.write_block(_convolver.convolve_signal());
    }

  private:
    apf::StaticConvolver _convolver;
    apf::NonCausalBlockDelayLine<sample_type> _delayline;
};

class WfsRenderer::SourceChannel : public _base::SourceChannel
                                 , public apf::has_begin_and_end<
                          apf::NonCausalBlockDelayLine<sample_type>::circulator>
{
  public:
    explicit SourceChannel(const Source* s)
      : crossfade_mode(0)
      , old_weighting_factor(0)
      , weighting_factor(0)
      , source(*s)
    {}

    void update();

    int crossfade_mode;
    sample_type old_weighting_factor, weighting_factor;
    float delay, old_delay;

    const Source& source;

    // TODO: avoid making those public:
    using apf::has_begin_and_end<apf::NonCausalBlockDelayLine<sample_type>
      ::circulator>::_begin;
    using apf::has_begin_and_end<apf::NonCausalBlockDelayLine<sample_type>
      ::circulator>::_end;
};

class WfsRenderer::RenderFunction
{
  public:
    typedef sample_type result_type;

    RenderFunction(const Output& out) : _out(out) {}

    int select(SourceChannel& in);

    result_type operator()(sample_type in)
    {
      return in * _new_factor;
    }

    result_type operator()(sample_type in, apf::fade_out_tag)
    {
      return in * _old_factor;
    }

  private:
    sample_type _old_factor, _new_factor;

    const Output& _out;
};

class WfsRenderer::Output : public _base::Output
{
  public:
    friend class Source;  // to be able to see _sourcechannels

    Output(const Params& p)
      : _base::Output(p)
      , _combiner(this->sourcechannels, this->buffer, this->parent._fade)
    {}

    APF_PROCESS(Output, _base::Output)
    {
      _combiner.process(RenderFunction(*this));
    }

  private:
    apf::CombineChannelsCrossfade<apf::cast_proxy<SourceChannel
      , sourcechannels_t>, buffer_type
      , apf::raised_cosine_fade<sample_type> > _combiner;
};

class WfsRenderer::Source : public _base::Source
{
  private:
    void _process();

  public:
    Source(const Params& p)
      : _base::Source(p
          // We cannot create a std::pair of a reference, so we use a pointer:
          , std::make_pair(p.parent->get_output_list().size(), this))
      , delayline(p.input->_delayline)
    {}

    APF_PROCESS(Source, _base::Source)
    {
      _process();
    }

    bool get_output_levels(sample_type* first, sample_type* last) const
    {
      assert(size_t(std::distance(first, last)) == this->sourcechannels.size());

      apf::fixed_vector<SourceChannel>::const_iterator channel
        = this->sourcechannels.begin();

      for ( ; first != last; ++first)
      {
        *first = channel->weighting_factor;
        ++channel;
      }
      return true;
    }

    const apf::NonCausalBlockDelayLine<sample_type>& delayline;

  //private:
    bool _focused;
};

void WfsRenderer::Source::_process()
{
  if (this->model() == ::Source::plane)
  {
    // do nothing, focused-ness is irrelevant for plane waves
    _focused = false;
  }
  else
  {
    _focused = true;
    rtlist_proxy<Output> out_list = _input.parent.get_output_list();
    for (rtlist_proxy<Output>::iterator out = out_list.begin()
        ; out != out_list.end()
        ; ++out)
    {
      // subwoofers have to be ignored!
      if (out->model == Loudspeaker::subwoofer) continue;

      // TODO: calculate with inner product

      // angle (modulo) between the line connecting source<->loudspeaker
      // and the loudspeaker orientation

      // TODO: avoid getting reference 2 times (see select())
      DirectionalPoint ls = *out;
      DirectionalPoint ref(out->parent.state.reference_position()
      , out->parent.state.reference_orientation());
      ls.transform(ref);

      sample_type a = apf::math::wrap(angle(ls.position - this->position()
            , ls.orientation), 2 * apf::math::pi<sample_type>());

      sample_type halfpi = apf::math::pi<sample_type>()/2;

      if (a < halfpi || a > 3 * halfpi)
      {
        // if at least one loudspeaker "turns its back" to the source, the
        // source is considered non-focused
        _focused = false;
        break;
      }
    }
  }

  // TODO: active sources?
}

void WfsRenderer::SourceChannel::update()
{
  // TODO: avoid reading delay line if weighting_factor == 0?
  _begin = this->source.delayline.get_read_circulator(this->delay);
  _end = _begin + source.parent.block_size();
}

int WfsRenderer::RenderFunction::select(SourceChannel& in)
{
  // define a restricted area around loudspeakers to avoid division by zero:
  const float safety_radius = 0.01f; // 1 cm

  // TODO: move reference calculation to WfsRenderer::Process?
  DirectionalPoint ref_off(_out.parent.state.reference_offset_position()
      , _out.parent.state.reference_offset_orientation());
  DirectionalPoint ref(_out.parent.state.reference_position()
      , _out.parent.state.reference_orientation());
  ref_off.transform(ref);

  in.old_weighting_factor = in.weighting_factor;
  in.old_delay = in.delay;

  sample_type weighting_factor = 1;
  float delay = 0;

  Loudspeaker ls = _out;
  Position src_pos = in.source.position();

  // TODO: shortcut if in.source.weighting_factor == 0

  // Transform loudspeaker position according to reference and offset
  ls.transform(ref);

  float reference_distance = (ls.position - ref_off.position).length();

  float source_ls_distance = (ls.position - src_pos).length();

  switch (in.source.model()) // check if point source or plane wave or ...
  {
    case ::Source::point:
      if (ls.model == Loudspeaker::subwoofer)
      {
        // the delay is calculated to be correct on the reference position
        // delay can be negative!
        delay = (src_pos - ref_off.position).length()
          - reference_distance;

        if (fabs(delay) < safety_radius)
        {
          weighting_factor = 1.0f / sqrt(safety_radius);
        }
        else
        {
          weighting_factor = 1.0f / sqrt(fabs(delay));
        }
        break; // step out of switch
      }

      delay = (ls.position - src_pos).length();
      assert(delay >= 0);

      float denominator;
      if (delay < safety_radius) denominator = sqrt(safety_radius);
      else denominator = sqrt(delay);

      // TODO: does this really do the right thing?
      weighting_factor = cos(angle(ls.position - src_pos,
            ls.orientation)) / denominator;

      if (weighting_factor < 0.0f)
      {
        // negative weighting factor is only valid for focused sources
        if (in.source._focused)
        {
          // loudspeaker selection:

          // this could also be done by using the cosine function instead of
          // inner product

          // calculate inner product of those two vectors:
          // this = source
          Position lhs = ls.position - src_pos;
          Position rhs = ref_off.position - src_pos;

          // TODO: write inner product function in Position class
          if ((lhs.x * rhs.x + lhs.y * rhs.y) < 0.0f)
          {
            // if the inner product is less than zero, the source is more or
            // less between the loudspeaker and the reference
            delay = -delay;
            weighting_factor = -weighting_factor;

#if defined(WEIGHTING_OLD)
            (void)source_ls_distance;  // avoid "unused variable" warning
#elif defined(WEIGHTING_DELFT)
            // limit to a maximum of 2.0
            weighting_factor *= std::min(2.0f, std::sqrt(source_ls_distance
                / (reference_distance + source_ls_distance)));
#endif
          }
          else
          {
            // ignored focused point source
            weighting_factor = 0;
            break;
          }
        }
        else // non-focused and weighting_factor < 0
        {
          // ignored non-focused point source
          weighting_factor = 0;
          break;
        }
      }
      else if(weighting_factor > 0.0f) // positive weighting factor
      {
        if (!in.source._focused)
        {
          // non-focused point source

#if defined(WEIGHTING_OLD)
#elif defined(WEIGHTING_DELFT)
          // WARNING: division by zero is possible!
          weighting_factor *= std::sqrt(source_ls_distance
              / (reference_distance + source_ls_distance));
#endif
        }
        else // focused
        {
          // ignored focused point source
          break;
        }
      }
      else
      {
        // this should never happen: Weighting factor is 0 or +-Inf or NaN!
        break;
      }
      break;

    case ::Source::plane:
      if (ls.model == Loudspeaker::subwoofer)
      {
        weighting_factor = 1.0f; // TODO: is this correct?
        // the delay is calculated to be correct on the reference position
        // delay can be negative!
        delay = DirectionalPoint(in.source.position(), in.source.orientation())
          .plane_to_point_distance(ref_off.position) - reference_distance;
        break; // step out of switch
      }

      // weighting factor is determined by the cosine of the angle
      // difference between plane wave direction and loudspeaker direction
      weighting_factor = cos(angle(in.source.orientation(), ls.orientation));
      // check if loudspeaker is active for this source
      if (weighting_factor < 0)
      {
        // ignored plane wave
        weighting_factor = 0;
        break;
      }

      delay = DirectionalPoint(in.source.position(), in.source.orientation())
        .plane_to_point_distance(ls.position);

      if (delay < 0.0)
      {
        // "focused" plane wave
      }
      else // positive delay
      {
        // plane wave
      }
      break;

    default:
      //WARNING("Unknown source model");
      break;
  } // switch source model

  // no distance attenuation for plane waves 
  if (in.source.model() == ::Source::plane)
  {
    float ampl_ref = _out.parent.state.amplitude_reference_distance();
    assert(ampl_ref > 0);

    // 1/r:
    weighting_factor *= 0.5f / ampl_ref;
    // 1/sqrt(r)
    //weighting_factor *= 0.25f / sqrt(ampl_ref);
  }
  else
  {
#if defined(WEIGHTING_OLD)
    // consider distance attenuation
    float source_distance = (src_pos - ref_off.position).length();

    // no volume increase for sources closer than 0.5m to reference position
    source_distance = std::max(source_distance, 0.5f);

    weighting_factor *= 0.5f / source_distance; // 1/r
    // weighting_factor *= 0.25f / sqrt(source_distance); // 1/sqrt(r)
#elif defined(WEIGHTING_DELFT)
#endif
  }

  // apply the gain factor of the current source
  weighting_factor *= in.source.weighting_factor;

  // apply tapering
  weighting_factor *= ls.weight;

  assert(weighting_factor >= 0.0f);

  // delay in seconds
  delay *= c_inverse;
  // delay in samples
  delay *= _out.parent.sample_rate();

  // TODO: check for negative delay and print an error if > initial_delay

  // TODO: do proper rounding
  // TODO: enable interpolated reading from delay line.
  int int_delay = static_cast<int>(delay + 0.5f);

  if (in.old_weighting_factor != 0)
  {
    in._begin = in.source.delayline.get_read_circulator(in.old_delay);
    in._end = in._begin + _out.parent.block_size();
  }
  else
  {
    // TODO: avoid reading delay line, data is never used!
    // WARNING: if _begin/_end isn't set to something, it's a singular iterator
    in._begin = in.source.delayline.get_read_circulator(0);
    in._end = in._begin + _out.parent.block_size();
  }

  if (in.source.delayline.delay_is_valid(int_delay))
  {
    in.delay = int_delay;
    in.weighting_factor = weighting_factor;
  }
  else
  {
    // TODO: some sort of warning message?

    // TODO: avoid useless reading of delay line if weighting_factor == 0
    in.delay = 0;
    in.weighting_factor = 0;
  }

  _old_factor = in.old_weighting_factor;
  _new_factor = in.weighting_factor;

  if (_old_factor == 0 && _new_factor == 0)
  {
    return 0;
  }
  else if (_old_factor == _new_factor && in.old_delay == in.delay)
  {
    return 1;
  }
  return 2;
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
