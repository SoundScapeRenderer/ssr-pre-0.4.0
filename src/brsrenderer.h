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
/// Binaural Room Synthesis renderer.

#ifndef SSR_BRSRENDERER_H
#define SSR_BRSRENDERER_H

#include "rendererbase.h"

#include "apf/convolver.h"  // for Convolver
#include "apf/sndfiletools.h"  // for apf::load_sndfile

namespace ssr
{

namespace Convolver = apf::conv;

class BrsRenderer : public SourceToOutput<BrsRenderer, RendererBase>
{
  private:
    typedef SourceToOutput<BrsRenderer, ssr::RendererBase> _base;

  public:
    static const char* name() { return "BrsRenderer"; }

    typedef _base::DefaultInput Input;
    class Source;
    struct SourceChannel;
    class Output;
    struct RenderFunction;

    BrsRenderer(const apf::parameter_map& params)
      : _base(params)
      , _fade(this->block_size())
    {}

    void load_reproduction_setup();

  private:
    apf::raised_cosine_fade<sample_type> _fade;
};

struct BrsRenderer::SourceChannel : apf::has_begin_and_end<sample_type*>
                                  , Convolver::Output
{
  explicit SourceChannel(const Convolver::Input& in)
    : Convolver::Output(in)
  {}

  // out-of-class definition because of cyclic dependencies with Source
  void update();
  void convolve_and_more(sample_type weight);

  int crossfade_mode;
  sample_type new_weighting_factor;
};

class BrsRenderer::Source : public _base::Source
{
  public:
    Source(const Params& p)
      : _base::Source(p)
      , _new_weighting_factor(-1)
      , _old_weighting_factor(-1)
      , _brtf_index(-1)
      , _old_brtf_index(-1)
    {
      SndfileHandle ir_file
        = apf::load_sndfile(p.get<std::string>("properties_file")
            , this->parent.sample_rate(), 0);

      size_t no_of_channels = ir_file.channels();

      if (no_of_channels % 2 != 0)
      {
        throw std::logic_error(
            "Number of channels in BRIR file must be a multiple of 2!");
      }

      _angles = no_of_channels / 2;

      size_t size = ir_file.frames();

      typedef apf::fixed_matrix<sample_type> matrix_t;

      matrix_t ir_data(size, no_of_channels);

      // TODO: check return value?
      ir_file.readf(ir_data.begin(), size);

      size_t block_size = this->parent.block_size();

      Convolver::Transform temp(block_size);

      size_t partitions = Convolver::min_partitions(block_size, size);

      _brtf_set.reset(new brtf_set_t(std::make_pair(no_of_channels
              , std::make_pair(partitions, temp.partition_size()))));

      brtf_set_t::iterator target = _brtf_set->begin();
      for (matrix_t::slices_iterator it = ir_data.slices.begin()
          ; it != ir_data.slices.end()
          ; ++it, ++target)
      {
        temp.prepare_filter(it->begin(), it->end(), *target);
      }

      assert(target == _brtf_set->end());

      _convolver_input.reset(new Convolver::Input(block_size, partitions));

      this->sourcechannels.reserve(2);
      this->sourcechannels.emplace_back(*_convolver_input);
      this->sourcechannels.emplace_back(*_convolver_input);
    }

    APF_PROCESS(Source, _base::Source)
    {
      _convolver_input->add_block(_input.begin());

      _old_weighting_factor = _new_weighting_factor;
      _new_weighting_factor = this->weighting_factor;

      _old_brtf_index = _brtf_index;

      float azi = this->parent.state.reference_orientation().azimuth;

      // TODO: get reference offset!

      // get BRTF index from listener orientation
      // (source positions are NOT considered!)
      // 90 degree is in the middle of index 0
      _brtf_index = size_t(apf::math::wrap(
          (azi - 90.0f) * float(_angles) / 360.0f + 0.5f, float(_angles)));

      int crossfade_mode;

      // Check on one channel only, filters are always changed in parallel
      bool queues_empty = this->sourcechannels[0].queues_empty();

      if (queues_empty
          && _new_weighting_factor == _old_weighting_factor
          && _brtf_index == _old_brtf_index)
      {
        if (_new_weighting_factor == 0)
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
        if (crossfade_mode != 0)
        {
          this->sourcechannels[i].convolve_and_more(_old_weighting_factor);
        }

        if (!queues_empty) this->sourcechannels[i].rotate_queues();

        if (_brtf_index != _old_brtf_index)
        {
          // left and right channels are interleaved
          this->sourcechannels[i].set_filter((*_brtf_set)[2 * _brtf_index + i]);
        }

        this->sourcechannels[i].crossfade_mode = crossfade_mode;
        this->sourcechannels[i].new_weighting_factor = _new_weighting_factor;
      }
    }

  private:
    typedef apf::fixed_vector<Convolver::Filter> brtf_set_t;
    std::auto_ptr<brtf_set_t> _brtf_set;

    sample_type _new_weighting_factor, _old_weighting_factor;
    size_t _brtf_index, _old_brtf_index;

    std::auto_ptr<Convolver::Input> _convolver_input;

    size_t _angles;  // Number of angles in BRIR file
};

void BrsRenderer::SourceChannel::update()
{
  this->convolve_and_more(this->new_weighting_factor);
}

void BrsRenderer::SourceChannel::convolve_and_more(sample_type weight)
{
  _begin = this->convolve(weight);
  _end = _begin + this->block_size();
}

struct BrsRenderer::RenderFunction
{
  int select(SourceChannel& in) { return in.crossfade_mode; }
};

class BrsRenderer::Output : public _base::Output
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

void
BrsRenderer::load_reproduction_setup()
{
  // TODO: generalize this for all headphone-based renderers!

  // TODO: read settings from proper reproduction system

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

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
