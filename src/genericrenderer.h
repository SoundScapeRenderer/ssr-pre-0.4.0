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
  struct Params
  {
    typedef apf::fixed_matrix<sample_type>::slice_iterator iterator;

    Params(const Source& s) : source(s) {}

    size_t block_size;
    iterator first;
    iterator last;
    const Source& source;
  };

  explicit SourceChannel(const Params& p);

  // out-of-class definition because of cyclic dependencies with Source
  void update();
  void convolve(sample_type weight);

  const Source& source;

  Convolver::StaticFilter filter;
  Convolver::Output output;
};

struct GenericRenderer::_source_initializer
{
  typedef apf::fixed_matrix<sample_type> matrix_t;

  class _init_function
  {
    public:
      typedef SourceChannel::Params result_type;

      _init_function(size_t block_size, const Source& source)
        : _block_size(block_size)
        , _source(&source)
      {}

      result_type operator()(const matrix_t::Slice& slice)
      {
        SourceChannel::Params p(*_source);
        p.block_size = _block_size;
        p.first = slice.begin();
        p.last = slice.end();
        return p;
      }

    private:
      size_t _block_size;
      const Source* _source;
  };

  _source_initializer(const std::string& filename, const GenericRenderer* parent
      , const Source& source)
    // Dummy-initialization, will be overwritten later!
    : _init_begin(matrix_t::slices_iterator(matrix_t::channel_iterator(), 0, 0)
        , _init_function(0, source))
    , _init_end(_init_begin)
  {
    if (parent == 0) throw std::logic_error("Bug: parent == 0!");

    SndfileHandle ir_data = apf::load_sndfile( filename, parent->sample_rate()
        , parent->get_output_list().size());

    size_t size = ir_data.frames();

    _init_ir_data.reset(new matrix_t(size, ir_data.channels()));

    // TODO: check return value?
    ir_data.readf(_init_ir_data->begin(), size);

    _init_block_size = parent->block_size();
    _init_partitions = Convolver::min_partitions(_init_block_size, size);

    _init_begin = apf::make_transform_iterator(_init_ir_data->slices.begin()
        , _init_function(_init_block_size, source));
    _init_end = apf::make_transform_iterator(_init_ir_data->slices.end()
        , _init_function(_init_block_size, source));
  }

  // _init_ir_data may only be used in the Source constructor!
  std::auto_ptr<matrix_t> _init_ir_data;

  size_t _init_block_size;
  size_t _init_partitions;

  apf::transform_iterator<matrix_t::slices_iterator, _init_function>
    _init_begin, _init_end;
};

class GenericRenderer::Source : private _source_initializer
                              , public Convolver::Input
                              , public _base::Source
{
  public:
    Source(const Params& p)
      : _source_initializer(p.get<std::string>("properties_file"), p.parent
          , *this)
      , Convolver::Input(_init_block_size, _init_partitions)
      , _base::Source(p, _init_begin, _init_end)
      , _new_weighting_factor()
      , _old_weighting_factor()
    {
      _init_ir_data.reset();  // free memory in _source_initializer
    }

    APF_PROCESS(Source, _base::Source)
    {
      _old_weighting_factor = _new_weighting_factor;
      _new_weighting_factor = this->weighting_factor;

      this->add_block(_input.begin());
    }

    sample_type _new_weighting_factor, _old_weighting_factor;
};

GenericRenderer::SourceChannel::SourceChannel(const Params& p)
  : source(p.source)
  , filter(p.block_size, p.first, p.last)
  , output(this->source, this->filter)
{}

void GenericRenderer::SourceChannel::update()
{
  this->convolve(this->source._new_weighting_factor);
}

void GenericRenderer::SourceChannel::convolve(sample_type weight)
{
  // TODO: check if convolver == 0?
  _begin = this->output.convolve(weight);
  _end = _begin + this->source.block_size();
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
