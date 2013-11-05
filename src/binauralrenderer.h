/******************************************************************************
 * Copyright © 2012-2013 Institut für Nachrichtentechnik, Universität Rostock *
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
 * http://spatialaudio.net/ssr                           ssr@spatialaudio.net *
 ******************************************************************************/

/// @file
/// Binaural renderer.

#ifndef SSR_BINAURALRENDERER_H
#define SSR_BINAURALRENDERER_H

#include "rendererbase.h"
#include "apf/iterator.h"  // for apf::cast_proxy, apf::make_cast_proxy()
#include "apf/convolver.h"  // for apf::Convolver
#include "apf/container.h"  // for apf::fixed_matrix
#include "apf/sndfiletools.h"  // for apf::load_sndfile
#include "apf/combine_channels.h"  // for apf::raised_cosine_fade, ...

namespace ssr
{

namespace Convolver = apf::conv;

// TODO: derive from HeadphoneRenderer?
class BinauralRenderer : public SourceToOutput<BinauralRenderer, RendererBase>
{
  private:
    typedef SourceToOutput<BinauralRenderer, ssr::RendererBase> _base;

  public:
    static const char* name() { return "BinauralRenderer"; }

    class SourceChannel;
    class Source;
    class Output;
    class RenderFunction;

    BinauralRenderer(const apf::parameter_map& params)
      : _base(params)
      , _fade(this->block_size())
      , _partitions(0)
    {}

    void load_reproduction_setup();

    APF_PROCESS(BinauralRenderer, _base)
    {
      this->_process_list(_source_list);
    }

  private:
    typedef apf::fixed_vector<Convolver::Filter> hrtf_set_t;

    void _load_hrtfs(const std::string& filename, size_t size);

    static bool _cmp_abs(sample_type left, sample_type right)
    {
      return std::abs(left) < std::abs(right);
    }

    apf::raised_cosine_fade<sample_type> _fade;
    size_t _partitions;
    size_t _angles;  // Number of angles in HRIR file
    std::unique_ptr<hrtf_set_t> _hrtfs;
    std::unique_ptr<Convolver::Filter> _neutral_filter;
};

class BinauralRenderer::SourceChannel : public Convolver::Output
                                      , public apf::has_begin_and_end<float*>
{
  public:
    explicit SourceChannel(const Convolver::Input* input)
      // TODO: assert that input != 0?
      : Convolver::Output(*input)
      , temporary_hrtf(input->block_size(), input->partitions())
      , _block_size(input->block_size())
    {}

    void convolve_and_more(sample_type weight)
    {
      _begin = this->convolve(weight);
      _end = _begin + _block_size;
    }

    void update()
    {
      this->convolve_and_more(this->weight);
    }

    Convolver::Filter temporary_hrtf;

    sample_type weight;
    apf::CombineChannelsResult::type crossfade_mode;

  private:
    const size_t _block_size;
};

void
BinauralRenderer::_load_hrtfs(const std::string& filename, size_t size)
{
  SndfileHandle hrir_file = apf::load_sndfile(filename, this->sample_rate(), 0);

  const size_t no_of_channels = hrir_file.channels();

  if (no_of_channels % 2 != 0)
  {
    throw std::logic_error(
        "Number of channels in HRIR file must be a multiple of 2!");
  }

  _angles = no_of_channels / 2;

  // TODO: handle size > hrir_file.frames()

  if (size == 0) size = hrir_file.frames();

  // Deinterleave channels and transform to FFT domain

  apf::fixed_matrix<float> transpose(size, no_of_channels);

  size = hrir_file.readf(transpose.begin(), size);

  _partitions = Convolver::min_partitions(this->block_size(), size);

  Convolver::Transform temp(this->block_size());

  _hrtfs.reset(new hrtf_set_t(std::make_pair(no_of_channels
          , std::make_pair(_partitions, temp.partition_size()))));

  hrtf_set_t::iterator target = _hrtfs->begin();
  for (apf::fixed_matrix<float>::slices_iterator it = transpose.slices.begin()
      ; it != transpose.slices.end()
      ; ++it, ++target)
  {
    temp.prepare_filter(it->begin(), it->end(), *target);
  }

  // prepare neutral filter (dirac impulse) for interpolation around the head

  // get index of absolute maximum in first channel (frontal direcion, left)
  apf::fixed_matrix<sample_type>::slice_iterator maximum
    = std::max_element(transpose.slices.begin()->begin()
      , transpose.slices.begin()->end(), _cmp_abs);

  int index = std::distance(transpose.slices.begin()->begin(), maximum);

  apf::fixed_vector<sample_type> impulse(index + 1);
  impulse.back() = 1;

  _neutral_filter.reset(new Convolver::Filter(this->block_size()
        , impulse.begin(), impulse.end()));
  // Number of partitions may be different from _hrtfs!
}

class BinauralRenderer::RenderFunction
{
  public:
    RenderFunction() : _in(0) {}

    apf::CombineChannelsResult::type select(SourceChannel& in)
    {
      _in = &in;
      return in.crossfade_mode;
    }

    void update()
    {
      assert(_in);
      _in->update();
    }

  private:
    SourceChannel* _in;
};

class BinauralRenderer::Output : public _base::Output
{
  public:
    Output(const Params& p)
      : _base::Output(p)
      , _combiner(this->sourcechannels, this->buffer, this->parent._fade)
    {}

    APF_PROCESS(Output, _base::Output)
    {
      _combiner.process(RenderFunction());
    }

  private:
    apf::CombineChannelsCrossfadeCopy<apf::cast_proxy<SourceChannel
      , sourcechannels_t>, buffer_type
      , apf::raised_cosine_fade<sample_type> > _combiner;
};

void BinauralRenderer::load_reproduction_setup()
{
  // TODO: read settings from proper reproduction system

  _load_hrtfs(this->params["hrir_file"], this->params.get("hrir_size", 0));

  Output::Params params;

  const std::string prefix = this->params.get("system_output_prefix", "");

  if (prefix != "")
  {
    // TODO: read target from proper reproduction file
    params.set("connect_to", prefix + "1");
  }
  this->add(params);

  if (prefix != "")
  {
    params.set("connect_to", prefix + "2");
  }
  this->add(params);
}

class BinauralRenderer::Source : public Convolver::Input, public _base::Source
{
  private:
    void _process();

  public:
    Source(const Params& p)
      // TODO: assert that p.parent != 0?
      : Convolver::Input(p.parent->block_size(), p.parent->_partitions)
      , _base::Source(p, std::make_pair(size_t(2), this))
      , _hrtf_index(-1)
      , _interp_factor(-1)
      , _weight(0)
    {}

    APF_PROCESS(Source, _base::Source)
    {
      _process();
    }

  private:
    // Function object for interpolation between filters
    class _interpolate
    {
      public:
        _interpolate(sample_type interp_factor)
          : _interp_factor(interp_factor)
        {}

        sample_type
        operator()(sample_type one, sample_type two) const
        {
          return (1.0f - _interp_factor) * one + _interp_factor * two;
        }

      private:
        float _interp_factor;
    };

    size_t _hrtf_index, _old_hrtf_index;
    float _interp_factor, _old_interp_factor;
    float _weight, _old_weight;
};

void BinauralRenderer::Source::_process()
{
  _old_weight = _weight;
  _old_hrtf_index = _hrtf_index;
  _old_interp_factor = _interp_factor;
  _interp_factor = 0.0f;

  this->add_block(_input.begin());

  const Position& ref_pos = _input.parent.state.reference_position
    + _input.parent.state.reference_offset_position;
  const Orientation& ref_ori = _input.parent.state.reference_orientation
    + _input.parent.state.reference_offset_orientation;

  if (this->weighting_factor != 0)
  {
    _weight = 1;

    if (this->model == ::Source::plane)
    {
      // no distance attenuation for plane waves 
      // 1/r:
      _weight *= 0.5f / _input.parent.state.amplitude_reference_distance;

      // 1/sqrt(r):
      //_weight *= 0.25f / sqrt(
      //    _input.parent.state.amplitude_reference_distance());
    }
    else
    {
      float source_distance
        = (this->position - ref_pos).length();

      if (source_distance < 0.5f)
      {
        _interp_factor = 1.0f - 2 * source_distance;
      }

      // no volume increase for sources closer than 0.5m
      source_distance = std::max(source_distance, 0.5f);

      _weight *= 0.5f / source_distance; // 1/r
      // _weight *= 0.25f / sqrt(source_distance); // 1/sqrt(r)
    }

    _weight *= this->weighting_factor;
  }
  else
  {
    _weight = 0;
  }

  float angles = _input.parent._angles;

  // calculate relative orientation of sound source
  Orientation rel_ori = (this->position - ref_pos).orientation() - ref_ori;
  _hrtf_index = size_t(apf::math::wrap(
      rel_ori.azimuth * angles / 360.0f + 0.5f, angles));

  using namespace apf::CombineChannelsResult;
  type crossfade_mode;

  // Check on one channel only, filters are always changed in parallel
  bool queues_empty = this->sourcechannels[0].queues_empty();

  bool hrtf_changed = _hrtf_index != _old_hrtf_index
      || _interp_factor != _old_interp_factor;

  if (_weight == 0 && _old_weight == 0)
  {
    crossfade_mode = nothing;
  }
  else if (queues_empty
      && _weight == _old_weight
      && !hrtf_changed)
  {
    crossfade_mode = constant;
  }
  else if (_weight == 0)
  {
    crossfade_mode = fade_out;
  }
  else if (_old_weight == 0)
  {
    crossfade_mode = fade_in;
  }
  else
  {
    crossfade_mode = change;
  }

  for (size_t i = 0; i < 2; ++i)
  {
    SourceChannel& channel = this->sourcechannels[i];

    if (crossfade_mode == nothing || crossfade_mode == fade_in)
    {
      // No need to convolve
    }
    else
    {
      channel.convolve_and_more(_old_weight);
    }

    if (!queues_empty) channel.rotate_queues();

    if (hrtf_changed)
    {
      // left and right channels are interleaved
      Convolver::Filter& hrtf
        = (*_input.parent._hrtfs)[2 * _hrtf_index + i];

      if (_interp_factor == 0)
      {
        channel.set_filter(hrtf);
      }
      else
      {
        // Interpolate between selected HRTF and neutral filter (Dirac)
        apf::conv::transform_nested(hrtf
            , *_input.parent._neutral_filter, channel.temporary_hrtf
            , _interpolate(_interp_factor));
        this->sourcechannels[i].set_filter(channel.temporary_hrtf);
      }
    }

    channel.crossfade_mode = crossfade_mode;
    channel.weight = _weight;
  }
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
