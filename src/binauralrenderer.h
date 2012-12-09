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
/// Binaural renderer.

#ifndef SSR_BINAURALRENDERER_H
#define SSR_BINAURALRENDERER_H

#include <sndfile.hh>  // C++ bindings for libsndfile

#include "rendererbase.h"
#include "apf/iterator.h"  // for apf::cast_proxy, apf::make_cast_proxy()
#include "apf/convolver.h"  // for apf::Convolver
#include "apf/container.h"  // for apf::fixed_matrix

#include "apf/stringtools.h"
using apf::str::A2S;

namespace ssr
{

// TODO: derive from HeadphoneRenderer?
class BinauralRenderer : public RendererBase<BinauralRenderer>
{
  private:
    typedef RendererBase<BinauralRenderer> _base;

    apf::parameter_map _add_params(const apf::parameter_map& params)
    {
      apf::parameter_map temp(params);
      temp.set("name", params.get("name", "BinauralRenderer"));
      return temp;
    }

  public:
    class SourceChannel;
    class Source;
    class Output;
    class RenderFunction;

    BinauralRenderer(const apf::parameter_map& params)
      : _base(_add_params(params))
      , _fade(this->block_size())
      , _partitions(0)
    {}

    void load_reproduction_setup();

    // TODO: proper solution for getting the reproduction setup
    template<typename SomeListType>
    void get_loudspeakers(SomeListType&) {}

  private:
    typedef apf::fixed_vector<apf::Convolver::filter_t> hrtf_set_t;

    void _load_hrtfs(const std::string& filename, size_t size);

    static bool _cmp_abs(sample_type left, sample_type right)
    {
      return std::abs(left) < std::abs(right);
    }

    apf::raised_cosine_fade<sample_type> _fade;
    size_t _partitions;
    std::auto_ptr<hrtf_set_t> _hrtfs;
    std::auto_ptr<apf::Convolver::filter_t> _neutral_filter;
};

class BinauralRenderer::SourceChannel : public apf::has_begin_and_end<float*>
{
  public:
    // first: block size, second: number of partitions
    // A single argument must be used because of the fixed_vector constructor.
    explicit SourceChannel(std::pair<size_t, size_t> size)
      : convolver(size.first, size.second)
      , hrtf(std::make_pair(size.second, 2*size.first))
      , _block_size(size.first)
      , _old_hrtf_index(-1)
      , _old_interp_factor(-1.0f)
    {}

    void update()
    {
      convolver.rotate_queues();

      if (this->hrtf_index != _old_hrtf_index
          || this->interp_factor != _old_interp_factor)
      {
        convolver.set_filter(this->hrtf);
      }

      _begin = convolver.convolve_signal(this->gain);
      _end = _begin + _block_size;

      _old_hrtf_index = this->hrtf_index;
      _old_interp_factor = this->interp_factor;
    }

    apf::Convolver convolver;

    // (hopefully) temporary, shouldn't be public:
    using apf::has_begin_and_end<float*>::_begin;
    using apf::has_begin_and_end<float*>::_end;

    size_t hrtf_index;
    sample_type gain, interp_factor;

    apf::Convolver::filter_t hrtf;

    int crossfade_mode;

  private:
    const size_t _block_size;
    size_t _old_hrtf_index;
    sample_type _old_interp_factor;
};

void
BinauralRenderer::_load_hrtfs(const std::string& filename, size_t size)
{
  if (filename == "")
  {
    throw std::logic_error("BinauralRenderer: no HRIR filename specified!");
  }

  SndfileHandle hrir_file(filename, SFM_READ);

  if (!hrir_file)
  {
    throw std::logic_error("\"" + filename + "\" couldn't be loaded!");
  }

  const size_t sr = hrir_file.samplerate();
  if (sr != this->sample_rate())
  {
    throw std::logic_error("\"" + filename + "\": HRIR sample rate mismatch ("
        + A2S(this->sample_rate()) + " vs. " + A2S(sr) + ")!");
  }

  const size_t no_of_channels = hrir_file.channels();

  if (no_of_channels != 720)
  {
    throw std::logic_error("The file '" + filename
        + "' does not contain the required number of channels ("
        + A2S(no_of_channels) + " vs. 720).");
  }

  if (size == 0) size = hrir_file.frames();

  // Deinterleave channels and transform to FFT domain

  apf::fixed_matrix<float> transpose(size, no_of_channels);

  size = hrir_file.readf(transpose.begin(), size);

  // round up:
  _partitions = (size + this->block_size() - 1) / (this->block_size());

  const size_t partition_size = 2 * this->block_size();

  _hrtfs.reset(new hrtf_set_t(std::make_pair(no_of_channels
          , std::make_pair(_partitions, partition_size))));

  // create local Convolver to prepare filters
  // TODO: avoid some of the useless work (and memory allocations)
  apf::Convolver temp_convolver(this->block_size(), _partitions);

  hrtf_set_t::iterator target = _hrtfs->begin();
  for (apf::fixed_matrix<float>::slices_iterator it = transpose.slices.begin()
      ; it != transpose.slices.end()
      ; ++it, ++target)
  {
    temp_convolver.prepare_filter(it->begin(), it->end(), *target);
  }

  // prepare neutral filter (dirac impulse) for interpolation around the head

  // get index of absolute maximum in first channel (frontal direcion, left)
  apf::fixed_matrix<sample_type>::slice_iterator maximum
    = std::max_element(transpose.slices.begin()->begin()
      , transpose.slices.begin()->end(), _cmp_abs);

  int index = std::distance(transpose.slices.begin()->begin(), maximum);

  int partitions = (index/this->block_size()) + 1;

  // TODO: create vector with zeros and one 1

  apf::fixed_vector<sample_type> impulse(index);
  impulse.back() = 1;

  _neutral_filter.reset(new apf::Convolver::filter_t(std::make_pair(partitions
          , partition_size)));
  temp_convolver.prepare_filter(impulse.begin(), impulse.end()
      , *_neutral_filter);
}

struct BinauralRenderer::RenderFunction
{
  int select(const SourceChannel& in) { return in.crossfade_mode; }
};

class BinauralRenderer::Output : public _base::Output
{
  public:
    friend class Source;  // to be able to see _sourcechannels

    Output(const Params& p)
      : _base::Output(p)
      , _combiner(_sourcechannels, this->buffer, this->parent._fade)
    {}

    struct Process : _base::Output::Process
    {
      explicit Process(Output& p)
        : _base::Output::Process(p)
      {
        p._combiner.process(RenderFunction());
      }
    };

  private:
    std::list<SourceChannel*> _sourcechannels;

    apf::CombineChannelsCrossfadeCopy<apf::cast_proxy<SourceChannel
      , std::list<SourceChannel*> >, buffer_type
      , apf::raised_cosine_fade<sample_type> > _combiner;
};

void BinauralRenderer::load_reproduction_setup()
{
  // TODO: read settings from proper reproduction system

  _load_hrtfs(this->params["hrir_file"], this->params.get("hrir_size", 0ul));

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

class BinauralRenderer::Source : public _base::Source
{
  private:
    void _process();

  public:
    Source(apf::CommandQueue& fifo, const Input& in)
      : _base::Source(fifo, in)
      , output(std::make_pair(2
            , std::make_pair(in.parent.block_size(), in.parent._partitions)))
    {}

    struct Process : _base::Source::Process
    {
      explicit Process(Source& p)
        : _base::Source::Process(p)
      {
        p._process();
      }
    };

    void connect_to_outputs(rtlist_t& output_list)
    {
      std::list<SourceChannel*> temp;
      apf::append_pointers(this->output, temp);
      _input.parent.add_to_sublist(temp
         , apf::make_cast_proxy<Output>(output_list)
         , &Output::_sourcechannels);
    }

    void disconnect_from_outputs(rtlist_t& output_list)
    {
      std::list<SourceChannel*> temp;
      apf::append_pointers(this->output, temp);
      _input.parent.rem_from_sublist(temp
          , apf::make_cast_proxy<Output>(output_list)
          , &Output::_sourcechannels);
    }

    apf::fixed_vector<SourceChannel> output;

  private:
    // Function object for interpolation between filters
    class _interpolate
    {
      public:
        typedef sample_type result_type;
        typedef sample_type first_argument_type;
        typedef sample_type second_argument_type;

        _interpolate(sample_type interp_factor)
          : _interp_factor(interp_factor)
        {}

        result_type
        operator()(first_argument_type one, second_argument_type two) const
        {
          return (1.0f - _interp_factor) * one + _interp_factor * two;
        }

      private:
        float _interp_factor;
    };

    size_t _hrtf_index, _old_hrtf_index;
    float _interp_factor, _old_interp_factor;
    float _gain, _old_gain;
};

void BinauralRenderer::Source::_process()
{
  _old_gain = _gain;
  _old_hrtf_index = _hrtf_index;
  _old_interp_factor = _interp_factor;
  _interp_factor = 0.0f;

  const Position& ref_pos = _input.parent.state.reference_position()
    + _input.parent.state.reference_offset_position();
  const Orientation& ref_ori = _input.parent.state.reference_orientation()
    + _input.parent.state.reference_offset_orientation();

  if (this->weighting_factor != 0)
  {
    _gain = 1;

    if (this->model() == ::Source::plane)
    {
      // no distance attenuation for plane waves 
      // 1/r:
      _gain *= 0.5f / _input.parent.state.amplitude_reference_distance();

      // 1/sqrt(r):
      //_gain *= 0.25f / sqrt(
      //    _input.parent.state.amplitude_reference_distance());
    }
    else
    {
      float source_distance
        = (this->position() - ref_pos).length();

      if (source_distance < 0.5f)
      {
        _interp_factor = 1.0f - 2 * source_distance;
      }

      // no volume increase for sources closer than 0.5m
      source_distance = std::max(source_distance, 0.5f);

      _gain *= 0.5f / source_distance; // 1/r
      // _gain *= 0.25f / sqrt(source_distance); // 1/sqrt(r)
    }

    _gain *= this->weighting_factor;
  }
  else
  {
    _gain = 0;
  }

  // calculate relative orientation of sound source
  Orientation rel_ori = (this->position()-ref_pos).orientation() - ref_ori;
  _hrtf_index = apf::math::wrap(rel_ori.azimuth, 360.0f);

  typedef apf::Convolver::filter_t filter;

  for (size_t i = 0; i < 2; ++i)
  {
    if (_hrtf_index != _old_hrtf_index || _interp_factor != _old_interp_factor)
    {
      // left and right channels are interleaved
      filter& filter_data
        = (*_input.parent._hrtfs)[2 * _hrtf_index + i];

      if (_interp_factor == 0)
      {
        apf::copy_nested(filter_data, this->output[i].hrtf);
      }
      else
      {
        apf::transform_nested(filter_data, *_input.parent._neutral_filter
            , this->output[i].hrtf
            , _interpolate(_interp_factor));
      }
    }

    this->output[i].hrtf_index = _hrtf_index;
    this->output[i].interp_factor = _interp_factor;
    this->output[i].gain = _gain;
  }

  int crossfade_mode;

  if (this->output[0].convolver.queues_empty()
      && this->output[1].convolver.queues_empty()
      && _gain == _old_gain
      && _hrtf_index == _old_hrtf_index
      && _interp_factor == _old_interp_factor)
  {
    if (_gain == 0)
    {
      crossfade_mode = 0;
    }
    else
    {
      crossfade_mode = 1;
    }
  }
  else  // something changed -> crossfade
  {
    crossfade_mode = 2;
  }

  for (size_t i = 0; i < 2; ++i)
  {
    this->output[i].convolver.add_input_block(_input.begin());

    if (crossfade_mode != 0)
    {
      this->output[i]._begin
        = this->output[i].convolver.convolve_signal(_old_gain);
      this->output[i]._end
        = this->output[i]._begin + _input.parent.block_size();
    }

    this->output[i].crossfade_mode = crossfade_mode;
  }
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
