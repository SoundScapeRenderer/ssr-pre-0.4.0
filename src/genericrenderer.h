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
/// Generic renderer.

#ifndef SSR_GENERICRENDERER_H
#define SSR_GENERICRENDERER_H

#include "loudspeakerrenderer.h"

#include "apf/convolver.h"  // for Convolver
#include "apf/sndfiletools.h"  // for apf::load_sndfile

namespace ssr
{

namespace Convolver = apf::conv;

class GenericRenderer : public SourceToOutput<GenericRenderer
                                                          , LoudspeakerRenderer>
{
  private:
    typedef SourceToOutput<GenericRenderer, ssr::LoudspeakerRenderer> _base;

  public:
    static const char* name() { return "GenericRenderer"; }

    typedef _base::DefaultInput Input;
    class Source;
    class SourceChannel;
    class Output;
    class RenderFunction;

    GenericRenderer(const apf::parameter_map& params)
      : _base(params)
      , _fade(this->block_size())
    {}

  private:
    class _source_initializer;

    apf::raised_cosine_fade<sample_type> _fade;
};

struct GenericRenderer::SourceChannel : apf::has_begin_and_end<sample_type*>
{
  template<typename In>
  explicit SourceChannel(const Source& s, In first, In last);

  // out-of-class definition because of cyclic dependencies with Source
  void update();
  void convolve(sample_type weight);

  const Source& source;

  Convolver::StaticOutput convolver;
};

class GenericRenderer::Source : public _base::Source
{
  public:
    explicit Source(const Params& p)
      : _base::Source(p)
      , _new_weighting_factor()
      , _old_weighting_factor()
    {
      typedef apf::fixed_matrix<sample_type> matrix_t;

      size_t outputs = this->parent.get_output_list().size();

      SndfileHandle ir_file = apf::load_sndfile(
          p.get<std::string>("properties_file"), this->parent.sample_rate()
          , outputs);

      size_t size = ir_file.frames();

      matrix_t ir_data(size, outputs);

      // TODO: check return value?
      ir_file.readf(ir_data.begin(), size);

      size_t block_size = this->parent.block_size();

      _convolver.reset(new Convolver::Input(block_size
            , Convolver::min_partitions(block_size, size)));

      this->sourcechannels.reserve(outputs);

      for (matrix_t::slices_iterator slice = ir_data.slices.begin()
          ; slice != ir_data.slices.end()
          ; slice++)
      {
        this->sourcechannels.emplace_back(*this, slice->begin(), slice->end());
      }
    }

    APF_PROCESS(Source, _base::Source)
    {
      _old_weighting_factor = _new_weighting_factor;
      _new_weighting_factor = this->weighting_factor;

      _convolver->add_block(_input.begin());
    }

    sample_type _new_weighting_factor, _old_weighting_factor;

    std::auto_ptr<Convolver::Input> _convolver;
};

template<typename In>
GenericRenderer::SourceChannel::SourceChannel(const Source& s
    , In first, In last)
  : source(s)
  // TODO: assert s._convolver != 0?
  , convolver(*s._convolver, first, last)
{}

void GenericRenderer::SourceChannel::update()
{
  this->convolve(this->source._new_weighting_factor);
}

void GenericRenderer::SourceChannel::convolve(sample_type weight)
{
  // TODO: check if convolver == 0?
  _begin = this->convolver.convolve(weight);
  _end = _begin + this->convolver.block_size();
}

struct GenericRenderer::RenderFunction
{
  int select(SourceChannel& in)
  {
    sample_type old_factor = in.source._old_weighting_factor;
    sample_type new_factor = in.source._new_weighting_factor;

    if (old_factor == 0 && new_factor == 0) return 0;

    in.convolve(old_factor);

    if (old_factor == new_factor) return 1;

    return 2;
  }
};

class GenericRenderer::Output : public _base::Output
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

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
