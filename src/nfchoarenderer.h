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
/// Near Field Compensated Higher Order Ambisonics renderer.

#ifndef SSR_NFCHOARENDERER_H
#define SSR_NFCHOARENDERER_H

#include "apf/math.h"  // for apf::math::linear_interpolator
#include "apf/fftwtools.h"  // for apf::fftw, apf::fftw_allocator
#include "apf/iterator.h"  // for apf::dual_iterator, apf::discard_iterator, ...
#include "apf/combine_channels.h"  // for apf::CombineChannelsInterpolation

#include "ssr_global.h"  // for ssr::c
#include "loudspeakerrenderer.h"
#include "hoacoefficients.h"

namespace ssr
{

class NfcHoaRenderer : public LoudspeakerRenderer<NfcHoaRenderer>
{
  private:
    typedef LoudspeakerRenderer<NfcHoaRenderer> _base;
    typedef HoaCoefficients<double> coeff_t;

  public:
    static const char* name() { return "NFC-HOA-Renderer"; }

    typedef apf::fixed_matrix<sample_type> matrix_t;
    typedef apf::fixed_matrix<sample_type, apf::fftw_allocator<sample_type> >
      fft_matrix_t;
    typedef apf::Cascade<apf::BiQuad<double, apf::dp::ac> > filter_type;

    class Source;
    class Mode;
    class ModePair;
    struct ModeAccumulatorBase;
    template<typename I1, typename I2> class ModeAccumulator;
    class FftProcessor;
    class RenderFunction;
    struct Output;

    NfcHoaRenderer(const apf::parameter_map& params)
      : _base(params)
      , _mode_pair_list(_fifo)
      , _mode_accumulator_list(_fifo)
      , _fft_list(_fifo)
    {}

    APF_PROCESS(NfcHoaRenderer, _base)
    {
      this->_process_list(_mode_pair_list);
      this->_process_list(_mode_accumulator_list);

      _fft_matrix.set_channels(_mode_matrix.slices);  // transpose matrix

      this->_process_list(_fft_list);
    }

    void load_reproduction_setup();
    size_t order;  // Ambisonics order
    float array_radius;

  private:
    struct _add_distance;

    matrix_t _mode_matrix;
    fft_matrix_t _fft_matrix;
    rtlist_t _mode_pair_list, _mode_accumulator_list, _fft_list;
};

class NfcHoaRenderer::Source : public _base::Source
{
  public:
    Source(const Params& p);

    void connect();
    void disconnect();

    APF_PROCESS(Source, _base::Source)
    {
      this->old_distance = this->distance;
      this->old_angle = this->angle;
      this->old_source_model = this->source_model;

      // NOTE: reference offset is not taken into account!

      // distance is only used for point sources (but is updated anyway)
      this->distance = (this->position()
          - this->parent.state.reference_position()).length();

      // TODO: distance is not correct for plane waves!
      // This is only important for the delay

      Orientation source_orientation;

      switch (this->model())
      {
        default:
        case ::Source::point:
          this->source_model = coeff_t::point_source;
          source_orientation = (this->position()
              - this->parent.state.reference_position()).orientation();
          // TODO: proper calculation of attenuation factor, this is temporary!
          {
            float distance_limit = 0.25f;
            this->weighting_factor *= std::sqrt(
                distance_limit / std::max(this->distance, distance_limit));
          }
          break;
        case ::Source::plane:
          this->source_model = coeff_t::plane_wave;
          source_orientation = this->orientation() - Orientation(180);
          // Note: no distance attenuation for plane waves!
          // TODO: constant factor using amplitude_reference_distance()?
          break;
      }

      this->angle = apf::math::deg2rad(90 + (source_orientation
            - this->parent.state.reference_orientation()).azimuth);

      // TODO: calculate delay

      // TODO: write delayed signal to a buffer?
    }

    float distance, old_distance;
    float angle, old_angle;
    coeff_t::source_t source_model, old_source_model;

  private:
    // Pointers to Mode objects for (dis-)connecting
    std::list<const Mode*> _modes;
    // Pointers to ModePair objects for removing when Source is deleted
    std::list<ModePair*> _mode_pairs;
};

class NfcHoaRenderer::Mode : public ProcessItem<Mode>
                           , public apf::fixed_vector<sample_type>
{
  private:
    class interpolate_coefficients
    {
      public:
        typedef apf::SosCoefficients<double> result_type;
        typedef const std::pair<result_type, result_type>& argument_type;

        interpolate_coefficients(double index, double block_size)
          : _index(index)
          , _block_size(block_size)
        {}

        result_type operator()(argument_type coeffs)
        {
          return coeffs.first
            + _index * (coeffs.second - coeffs.first) / _block_size;
        }

      private:
        double _index, _block_size;
    };

  public:
    Mode(size_t mode_number, const Source& s)
      : apf::fixed_vector<sample_type>(s.parent.block_size())
      , source(s)
      , rotation1(0)
      , rotation2(0)
      , old_rotation1(0)
      , old_rotation2(0)
      , _mode_number(mode_number)
      , _filter(mode_number == 0 ? 1 : (mode_number + 1) / 2)  // round up
      // Coefficients are all zeros by default
      , _coefficients(mode_number, s.parent.sample_rate()
          , s.parent.array_radius, ssr::c)
      , _old_coefficients(_coefficients)
    {}

    APF_PROCESS(Mode, ProcessItem<Mode>)
    {
      _process();
    }

    const Source& source;
    sample_type rotation1, rotation2, old_rotation1, old_rotation2;
    int interpolation_mode;

  private:
    void _process();

    sample_type _mode_number;
    filter_type _filter;
    coeff_t _coefficients, _old_coefficients;
};

void NfcHoaRenderer::Mode::_process()
{
  // IIR filtering is not done in RenderFunction because workload would be
  // distributed very un-evenly between threads!

  if (this->source.distance == this->source.old_distance
      && this->source.source_model == this->source.old_source_model)
  {
    // process filter (entire block)
    _filter.execute(this->source.begin(), this->source.end()
        , this->begin());
  }
  else
  {
    _old_coefficients.swap(_coefficients);

    // Avoid focused sources (for now ...):
    float distance = std::max(this->source.distance
        , this->source.parent.array_radius);

    // scale filter coefficients
    _coefficients.reset(distance, this->source.source_model);

    apf::dual_iterator<coeff_t::iterator>
      first_section(_old_coefficients.begin(), _coefficients.begin());
    apf::dual_iterator<coeff_t::iterator>
      last_section(_old_coefficients.end(), _coefficients.end());

    sample_type block_size = this->size();

    Source::iterator in = this->source.begin();
    iterator out = this->begin();

    // Calculate each sample separately. The first sample uses the old
    // coefficients, after the last sample the filter is updated again for the
    // next block.
    for (sample_type index = 1; index <= block_size; ++index)
    {
      // Calculate one output sample
      *out++ = _filter(*in++);

      // Create function object with current index
      interpolate_coefficients func(index, block_size);

      // Set interpolated filter coefficients
      _filter.set(apf::make_transform_iterator(first_section, func)
                , apf::make_transform_iterator(last_section, func));
    }

    assert(in == this->source.end());
    assert(out == this->end());
  }

  // Note: This must be done if angle OR weighting factor changes
  this->old_rotation1 = this->rotation1;
  this->old_rotation2 = this->rotation2;

  if (this->source.angle != this->source.old_angle)
  {
    this->rotation1
      = std::cos(-_mode_number * sample_type(this->source.angle));
    this->rotation2
      = std::sin(-_mode_number * sample_type(this->source.angle));
  }

  if (this->source.weighting_factor == 0
      && this->source.old_weighting_factor == 0)
  {
    this->interpolation_mode = 0;
  }
  else if (this->source.weighting_factor == this->source.old_weighting_factor
      && this->source.angle == this->source.old_angle
      && this->source.distance == this->source.old_distance
      && this->source.source_model == this->source.old_source_model)
  {
    this->interpolation_mode = 1;
  }
  else
  {
    this->interpolation_mode = 2;
  }
}

/** Combination of two Mode%s.
 * The only reason for this class is distribution of workload. It combines two
 * Mode%s in a way that each ModePair needs a similar amount of processing
 * power.
 **/
class NfcHoaRenderer::ModePair : public ProcessItem<ModePair>
{
  public:
    ModePair(size_t mode_number, size_t order, const Source& source)
      : _second(order - mode_number, source)
    {
      bool order_is_odd = (order % 2 == 0);

      if ((mode_number == 0) && order_is_odd)
      {
        // With an odd number of Modes, the first ModePair gets only one Mode.
        // Therefore, _first stays empty.
      }
      else
      {
        _first.reset(new Mode(mode_number - order_is_odd, source));
      }
    }

    APF_PROCESS(ModePair, ProcessItem<ModePair>)
    {
      // In C++11, std::unique_ptr has a "bool" operator to do "if (_first) ..."
      if (_first.get())
      {
        _first->process();
      }
      _second.process();
    }

    const Mode* first_ptr() const { return _first.get(); }
    const Mode* second_ptr() const { return &_second; }

  private:
    std::auto_ptr<Mode> _first;
    Mode _second;
};

NfcHoaRenderer::Source::Source(const Params& p)
  : _base::Source(p)
  // Set impossible values to force update in first cycle:
  , distance(-1)
  , angle(std::numeric_limits<float>::infinity())
  , source_model(coeff_t::source_t(-1))
{}

class NfcHoaRenderer::RenderFunction
{
  public:
    typedef std::pair<sample_type, sample_type> result_type;

    int select(const Mode& in)
    {
      // TODO: where to apply global scale factor?

      // Source/master level, mute and distance attenuation are applied here.
      // All of this is combined in in.source.weighting_factor
      // TODO: move this to Source class?

      if (in.interpolation_mode == 2)
      {
        sample_type block_size = in.size();
        _interpolator1.set(in.old_rotation1 * in.source.old_weighting_factor
            , in.rotation1 * in.source.weighting_factor, block_size);
        _interpolator2.set(in.old_rotation2 * in.source.old_weighting_factor
            , in.rotation2 * in.source.weighting_factor, block_size);
      }
      else if (in.interpolation_mode == 1)
      {
        _rotation1 = in.rotation1 * in.source.weighting_factor;
        _rotation2 = in.rotation2 * in.source.weighting_factor;
      }
      else
      {
        // do nothing
      }

      return in.interpolation_mode;
    }

    result_type operator()(sample_type in)
    {
      return std::make_pair(in * _rotation1, in * _rotation2);
    }

    result_type operator()(sample_type in, sample_type index)
    {
      return std::make_pair(in * _interpolator1(index)
                          , in * _interpolator2(index));
    }

  private:
    sample_type _rotation1, _rotation2;
    apf::math::linear_interpolator<sample_type> _interpolator1, _interpolator2;
};

// Template-free base class to be used in Source::connect()
struct NfcHoaRenderer::ModeAccumulatorBase : Item
{
  typedef std::list<const Mode*> mode_ptrs_t;

  // List of modes to be combined
  mode_ptrs_t mode_pointers;
};

// TODO: proper documentation
// First channel for positive mode, second channel for negative mode.
// Mode 0 has no negative mode, nor does the highest order if there is an even
// number of loudspeakers.
template<typename I1, typename I2>
class NfcHoaRenderer::ModeAccumulator : public ModeAccumulatorBase
{
  public:
    ModeAccumulator(I1 i1, I2 i2, size_t block_size)
      : _output_channels(i1, i2, block_size)
      , _combiner(mode_pointers, _output_channels)
    {}

    // APF_PROCESS doesn't work here because ModeAccumulatorBase cannot be a
    // class template.

    virtual void process()
    {
      // TODO: global scale factor (depends only on array size)?

      _combiner.process(RenderFunction());
    }

  private:
    class two_matrix_channels
    {
      public:
        typedef apf::dual_iterator<I1, I2> iterator;

        two_matrix_channels(I1 i1, I2 i2, size_t block_size)
          : _begin(i1, i2)
          , _end(_begin)
        {
          std::advance(_end, block_size);
        }

        iterator begin() { return _begin; }
        iterator end() { return _end; }

      private:
        iterator _begin, _end;
    };

    two_matrix_channels _output_channels;

    apf::CombineChannelsInterpolation<apf::cast_proxy_const<Mode, mode_ptrs_t>
      , two_matrix_channels> _combiner;
};

/// Helper function for automatic template type deduction
template<typename I1, typename I2>
NfcHoaRenderer::ModeAccumulator<I1, I2>*
new_mode_accumulator(I1 i1, I2 i2, size_t block_size)
{
  return new NfcHoaRenderer::ModeAccumulator<I1, I2>(i1, i2, block_size);
}

void
NfcHoaRenderer::Source::connect()
{
  size_t order = this->parent.order;

  // create ModePair objects

  for (size_t mode_number = 0; mode_number <= order / 2; ++mode_number)
  {
    _mode_pairs.push_back(new ModePair(mode_number, order, *this));
  }

  // create list of pointers to Mode objects

  typedef apf::cast_proxy<ModePair, std::list<ModePair*> > proxy_list_t;
  proxy_list_t pairs(_mode_pairs);
  for (proxy_list_t::reverse_iterator pair = pairs.rbegin()
      ; pair != pairs.rend()
      ; ++pair)
  {
    if (pair->first_ptr()) _modes.push_front(pair->first_ptr());
    _modes.push_back(pair->second_ptr());
  }

  // add _mode_pairs to _mode_pair_list

  this->parent._mode_pair_list.add(_mode_pairs.begin(), _mode_pairs.end());

  // connect modes with ModeAccumulator

  this->parent.add_to_sublist(_modes
      , apf::make_cast_proxy<ModeAccumulatorBase>(
        this->parent._mode_accumulator_list)
      , &ModeAccumulatorBase::mode_pointers);
}

void
NfcHoaRenderer::Source::disconnect()
{
  // Note: everything is done in reverse order of connect()

  this->parent.rem_from_sublist(_modes
      , apf::make_cast_proxy<ModeAccumulatorBase>(
        this->parent._mode_accumulator_list)
      , &ModeAccumulatorBase::mode_pointers);

  // The objects are actually deleted here (via the _fifo):
  this->parent._mode_pair_list.rem(_mode_pairs.begin(), _mode_pairs.end());

  _modes.clear();
  _mode_pairs.clear();
}

class NfcHoaRenderer::FftProcessor : public ProcessItem<FftProcessor>
{
  public:
    FftProcessor(size_t block_size, sample_type* first)
      : _fft_plan(apf::fftw<sample_type>::plan_r2r_1d(block_size, first, first
            , FFTW_R2HC, FFTW_PATIENT))
    {}

    ~FftProcessor() { apf::fftw<sample_type>::destroy_plan(_fft_plan); }

    APF_PROCESS(FftProcessor, ProcessItem<FftProcessor>)
    {
      // TODO: scale result?
      apf::fftw<sample_type>::execute(_fft_plan);
    }

  private:
    apf::fftw<sample_type>::plan _fft_plan;
};

struct NfcHoaRenderer::Output : _base::Output
{
  Output(const Params& p) : _base::Output(p) {}

  APF_PROCESS(Output, _base::Output)
  {
    std::copy(this->slice.begin(), this->slice.end(), this->buffer.begin());
  }

  fft_matrix_t::Slice slice;
};

struct NfcHoaRenderer::_add_distance
{
  float
  operator()(float base, const Output& out)
  {
    if (out.model == Loudspeaker::subwoofer)
    {
      throw std::logic_error("Subwoofers are currently not supported!");
    }
    return base + out.position.length();
  }
};

void
NfcHoaRenderer::load_reproduction_setup()
{
  _base::load_reproduction_setup();

  // TODO: check if reproduction setup is "reasonable"

  // TODO: check if clockwise or counterclockwise setup?

  typedef apf::cast_proxy<Output, rtlist_t> output_list_t;
  output_list_t outputs(this->get_non_const_output_list());

  // TODO: with C++11, _add_distance should be replaced by a lambda function

  float total = std::accumulate(outputs.begin(), outputs.end(), 0.0f
      , _add_distance());

  // This is only valid if there are no subwoofers:
  size_t normal_loudspeakers = outputs.size();

  this->array_radius = total / normal_loudspeakers;

  std::cout << "\nWARNING: this is a preliminary implementation of the NFC-HOA "
    "renderer!\nLoading " << normal_loudspeakers << " loudspeakers with a mean "
    "distance of " << this->array_radius << " meters.\n"
    "Assuming circular (counterclockwise) setup!\n" << std::endl;

  _mode_matrix.reset(normal_loudspeakers, this->block_size());
  _fft_matrix.reset(this->block_size(), normal_loudspeakers);

  this->order = normal_loudspeakers / 2;  // round down

  for (size_t i = 0; i <= this->order; ++i)
  {
    if (i == 0 || (i == this->order && normal_loudspeakers % 2 == 0))
    {
      _mode_accumulator_list.add(new_mode_accumulator(
            _mode_matrix.channels[i].begin()
            , apf::discard_iterator()
            , this->block_size()));
    }
    else
    {
      _mode_accumulator_list.add(new_mode_accumulator(
            _mode_matrix.channels[i].begin()
            , _mode_matrix.channels[normal_loudspeakers - i].begin()
            , this->block_size()));
    }

    // TODO: documentation, mention half-complex format of FFTW
  }

  for (fft_matrix_t::channels_iterator ch = _fft_matrix.channels.begin()
      ; ch != _fft_matrix.channels.end()
      ; ++ch)
  {
    _fft_list.add(new FftProcessor(normal_loudspeakers, ch->begin()));
  }

  assert(outputs.size() == size_t(std::distance(_fft_matrix.slices.begin()
                                              , _fft_matrix.slices.end())));

  fft_matrix_t::slices_iterator slice = _fft_matrix.slices.begin();
  for (output_list_t::iterator out = outputs.begin()
      ; out != outputs.end()
      ; ++out)
  {
    out->slice = *slice++;
  }
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
