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

#include <sndfile.hh>  // C++ bindings for libsndfile

#include "loudspeakerrenderer.h"

#include "apf/convolver.h"  // for StaticConvolver
#include "apf/blockdelayline.h"  // for NonCausalBlockDelayLine

#include "apf/stringtools.h"
using apf::str::A2S;

namespace ssr
{

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
    apf::raised_cosine_fade<sample_type> _fade;
};

struct GenericRenderer::SourceChannel : _base::SourceChannel
                                          , apf::has_begin_and_end<sample_type*>
{
  explicit SourceChannel(const Source* s) : source(*s) {}

  // out-of-class definition because of cyclic dependencies with Source
  void update();
  void convolve(sample_type weight);

  const Source& source;

  std::auto_ptr<apf::StaticConvolver> convolver;
};

class GenericRenderer::Source : public _base::Source
{
  public:
    Source(const Params& p)
      : _base::Source(p
          // We cannot create a std::pair of a reference, so we use a pointer:
          , std::make_pair(p.parent->get_output_list().size(), this))
      , _new_weighting_factor()
      , _old_weighting_factor()
    {
      std::string ir_file = p.get<std::string>("properties_file");

      SndfileHandle ir_data(ir_file, SFM_READ);

      if (!ir_data)
      {
        throw std::logic_error("\"" + ir_file + "\" couldn't be loaded!");
      }

      const size_t loudspeakers = this->parent.get_output_list().size();
      const size_t no_of_channels = ir_data.channels();
      if (no_of_channels != loudspeakers)
      {
        throw std::logic_error(
            "\"" + ir_file
            + "\" has " +  A2S(no_of_channels) + " but it needs to have "
            "as many channels as there are loudspeakers (which is "
            + A2S(loudspeakers) + ")!");
      }

      const size_t sr = ir_data.samplerate();
      if (sr != this->parent.sample_rate())
      {
        throw std::logic_error("\"" + ir_file
            + "\": sample rate mismatch ("
            + A2S(this->parent.sample_rate()) + " vs. " + A2S(sr) + ")!");
      }

      size_t size = ir_data.frames();

      apf::fixed_matrix<sample_type> ir_matrix(size, no_of_channels);

      size = ir_data.readf(ir_matrix.begin(), size);

      // TODO: warning if size == 0?

      apf::fixed_matrix<sample_type>::slices_iterator slice
        = ir_matrix.slices.begin();
      for (sourcechannels_t::iterator it = this->sourcechannels.begin()
          ; it != this->sourcechannels.end()
          ; ++it, ++slice)
      {
        it->convolver.reset(new apf::StaticConvolver(this->parent.block_size()
              , slice->begin(), slice->end()));
      }
      assert(slice == ir_matrix.slices.end());
    }

    APF_PROCESS(Source, _base::Source)
    {
      _old_weighting_factor = _new_weighting_factor;
      _new_weighting_factor = this->weighting_factor;

      for (sourcechannels_t::iterator it = this->sourcechannels.begin()
          ; it != this->sourcechannels.end()
          ; ++it)
      {
        // TODO: check if convolver == 0?
        it->convolver->add_input_block(_input.begin());
      }
    }

    sample_type _new_weighting_factor, _old_weighting_factor;
};

void GenericRenderer::SourceChannel::update()
{
  this->convolve(this->source._new_weighting_factor);
}

void GenericRenderer::SourceChannel::convolve(sample_type weight)
{
  // TODO: check if convolver == 0?
  _begin = this->convolver->convolve_signal(weight);
  _end = _begin + this->source.parent.block_size();
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
