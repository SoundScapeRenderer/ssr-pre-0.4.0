/******************************************************************************
 * Copyright © 2012      Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the Audio Processing Framework (APF).                 *
 *                                                                            *
 * The APF is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The APF is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 *                                 http://AudioProcessingFramework.github.com *
 ******************************************************************************/

/// @file
/// Convolution engine.

#ifndef APF_CONVOLVER_H
#define APF_CONVOLVER_H

#include <utility>  // for std::pair
#include <algorithm>  // for std::transform()
#include <cassert>

#include "apf/math.h"
#include "apf/fftwtools.h"  // for fftw_allocator and fftw traits
#include "apf/container.h"  // for fixed_vector, fixed_list
#include "apf/iterator.h"  // for make_*_iterator()

namespace apf
{

/** Convolution engine.
 * A convolution engine normally consists of an Input a Filter/Staticfilter and
 * an Output.
 *
 * Uses (uniformly) partitioned convolution.
 *
 * TODO: describe thread (un)safety
 **/
namespace conv
{

/// Calculate necessary number of partitions for a given filter length
static size_t min_partitions(size_t block_size, size_t filter_size)
{
  return (filter_size + block_size - 1) / block_size;
}

/// Two blocks of time-domain or FFT (half-complex) data.
struct fft_node : fixed_vector<float, fftw_allocator<float> >
{
  explicit fft_node(size_t n)
    : fixed_vector<float, fftw_allocator<float> >(n)
    , _zero(true)
  {}

  fft_node& operator=(const fft_node& rhs)
  {
    assert(this->size() == rhs.size());

    if (rhs.zero())
    {
      this->set_zero();
    }
    else
    {
      std::copy(rhs.begin(), rhs.end(), this->begin());
      this->set_non_zero();
    }
    return *this;
  }

  void set_zero() { _zero = true; }
  void set_non_zero() { _zero = false; }
  bool zero() const { return _zero; }

  // WARNING: The 'zero' flag allows saving computation power, but it also
  // raises the risk of programming errors! Handle with care!

  private:
  /// To avoid unnecessary FFTs and filling buffers with zeros.
  /// @note If _zero == true, the buffer itself is not necessarily all zeros!
  bool _zero;
};

/// Container holding a number of FFT blocks.
typedef fixed_vector<fft_node> filter_t;
/// A queue of FFT nodes
typedef fixed_list<fft_node> filter_queue_t;
/// Container for filter queues
typedef fixed_vector<filter_queue_t> filter_queues_t;

/// Forward-FFT-related functions
class TransformBase
{
  public:
    template<typename In>
    void prepare_filter(In first, In last, filter_t& c) const;

    size_t block_size() const { return _block_size; }
    size_t partition_size() const { return _partition_size; }

    template<typename In>
    In prepare_partition(In first, In last, fft_node& partition) const;

  protected:
    explicit TransformBase(size_t block_size_);

    ~TransformBase() { fftw<float>::destroy_plan(_fft_plan); }

    fftw<float>::plan _create_plan(float* array) const;

    /// In-place FFT
    void _fft(float* first) const
    {
      fftw<float>::execute_r2r(_fft_plan, first, first);
      _sort_coefficients(first);
    }

    fftw<float>::plan _fft_plan;

  private:
    void _sort_coefficients(float* first) const;

    const size_t _block_size;
    const size_t _partition_size;
};

TransformBase::TransformBase(size_t block_size_)
  : _fft_plan(0)
  , _block_size(block_size_)
  , _partition_size(2 * _block_size)
{
  if (_block_size % 8 != 0)
  {
    throw std::logic_error("Convolver: block size must be a multiple of 8!");
  }
}

fftw<float>::plan
TransformBase::_create_plan(float* array) const
{
  // TODO: planning flags as arguments?

  // Create in-place FFT plan for halfcomplex data format
  // Note: FFT plans are not re-entrant except when using FFTW_THREADSAFE!
  // Also Note: Once a plan of a certain size exists, creating further plans
  // is very fast because "wisdom" is shared (and therefore the creation of
  // plans is not thread-safe).
  // It is not necessary to re-use plans in other Convolver instances.
  return fftw<float>::plan_r2r_1d(int(_partition_size)
      , array, array, FFTW_R2HC, FFTW_PATIENT);
}

/** %Transform time-domain samples.
 * If there are too few input samples, the rest is zero-padded, if there are
 * too few blocks in the container @p c, the rest of the samples is ignored.
 * @param first Iterator to first time-domain sample
 * @param last Past-the-end iterator
 * @param c Target container
 **/
template<typename In>
void
TransformBase::prepare_filter(In first, In last, filter_t& c) const
{
  for (filter_t::iterator it = c.begin(); it != c.end(); ++it)
  {
    first = this->prepare_partition(first, last, *it);
  }
}

/** FFT of one partition.
 * If there are too few coefficients, the rest is zero-padded.
 * @param first Iterator to first coefficient
 * @param last Past-the-end iterator
 * @tparam In Forward iterator
 * @return Iterator to the first coefficient of the next block (for the next
 *   iteration, if needed)
 **/
template<typename In>
In
TransformBase::prepare_partition(In first, In last, fft_node& partition) const
{
  assert(size_t(std::distance(partition.begin(), partition.end()))
      == _partition_size);

  size_t chunk = std::min(_block_size, size_t(std::distance(first, last)));

  if (chunk == 0)
  {
    partition.set_zero();
    // No FFT has to be done (FFT of zero is also zero)
  }
  else
  {
    std::copy(first, first + chunk, partition.begin());
    std::fill(partition.begin() + chunk, partition.end(), 0.0f); // zero padding
    _fft(partition.begin());
    partition.set_non_zero();
  }
  return first + chunk;
}

/** Sort the FFT coefficients to be in proper place for the efficient 
 * multiplication of the spectra.
 **/
void
TransformBase::_sort_coefficients(float* data) const
{
  fixed_vector<float> buffer(_partition_size);

  int base = 8;

  buffer[0] = data[0];
  buffer[1] = data[1];
  buffer[2] = data[2];
  buffer[3] = data[3];
  buffer[4] = data[_block_size];
  buffer[5] = data[_partition_size - 1];
  buffer[6] = data[_partition_size - 2];
  buffer[7] = data[_partition_size - 3];

  for (size_t i = 0; i < (_partition_size / 8-1); i++)
  {
    for (int ii = 0; ii < 4; ii++)
    {
      buffer[base+ii] = data[base/2+ii];
    }

    for (int ii = 0; ii < 4; ii++)
    {
      buffer[base+4+ii] = data[_partition_size-base/2-ii];
    }

    base += 8;
  }

  std::copy(buffer.begin(), buffer.end(), data);
}

struct Transform : TransformBase
{
  Transform(size_t block_size_)
    : TransformBase(block_size_)
  {
    // Temporary memory area for FFTW planning routines
    fft_node planning_space(this->partition_size());
    _fft_plan = _create_plan(planning_space.begin());
  }
};

/** %Input stage of convolution.
 * New audio data is fed in here, further processing happens in Output.
 **/
struct Input : TransformBase
{
  /// @param block_size_ audio block size
  /// @param partitions_ maximum number of partitions
  Input(size_t block_size_, size_t partitions_)
    : TransformBase(block_size_)
    // One additional list element for preparing the upcoming partition:
    , spectra(std::make_pair(partitions_+1, this->partition_size()))
  {
    assert(partitions_ > 0);

    _fft_plan = _create_plan(spectra.front().begin());
  }

  template<typename In>
  void add_block(In first);

  size_t partitions() const { return spectra.size()-1; }

  /// List holding the spectrum of the double-frames 
  /// of the input signal to be convolved.
  /// The first element is the most recent signal chunk.
  fixed_list<fft_node> spectra;
};

/** Add a block of time-domain input samples.
 * @param first Iterator to first sample.
 * @tparam In Forward iterator
 **/
template<typename In>
void
Input::add_block(In first)
{
  In last = first;
  std::advance(last, this->block_size());

  // rotate buffers (this->spectra.size() is always at least 2)
  this->spectra.move(--this->spectra.end(), this->spectra.begin());

  fft_node& current = this->spectra.front();
  fft_node& next = this->spectra.back();

  if (math::has_only_zeros(first, last))
  {
    next.set_zero();

    if (current.zero())
    {
      // Nothing to be done, actual data is ignored
    }
    else
    {
      // If first half is not zero, second half must be filled with zeros
      std::fill(current.begin() + this->block_size(), current.end(), 0.0f);
    }
  }
  else
  {
    if (current.zero())
    {
      // First half must be actually filled with zeros
      std::fill(current.begin(), current.begin() + this->block_size(), 0.0f);
    }

    // Copy data to second half of the current partition
    std::copy(first, last, current.begin() + this->block_size());
    current.set_non_zero();
    // Copy data to first half of the upcoming partition
    std::copy(first, last, next.begin());
    next.set_non_zero();
  }

  if (current.zero())
  {
    // Nothing to be done, FFT of zero is also zero
  }
  else
  {
    _fft(current.begin());
  }
}

/// Container of filter queues (one for each partition).
class FilterBase : public TransformBase
{
  public:
    /// Constructor. Initially, all partitions are filled with zeros
    template<typename InitFunction>
    FilterBase(InitFunction init_func, size_t block_size_, size_t partitions_)
      : TransformBase(block_size_)
      , spectra(make_transform_iterator(make_index_iterator(1ul)
            , init_func)
          , make_transform_iterator(make_index_iterator(partitions_+1)
            , init_func))
    {
      assert(partitions_ > 0);

      _fft_plan = _create_plan(this->spectra.front().front().begin());
    }

    size_t partitions() const { return spectra.size(); }

    /// Internal representation of the filter
    filter_queues_t spectra;

  protected:
    template<typename In>
    void _set_filter(In first, In last)
    {
      for (filter_queues_t::iterator it = this->spectra.begin()
          ; it != this->spectra.end()
          ; ++it)
      {
        // The new filter partition starts its journey at the end of the queue
        first = this->prepare_partition(first, last, it->back());
      }
    }
};

namespace internal
{

// Create queues with different lengths: 1st list has 1 item, 2nd list 2, ...
class InitQueues
{
  public:
    typedef std::pair<size_t, size_t> result_type;
    InitQueues(size_t block_size_) : _size(2 * block_size_) {}
    result_type operator()(size_t nr) { return result_type(nr, _size); }
  private:
    const size_t _size;
};

// Disable queues, all queues have length 1.
class InitStatic
{
  public:
    typedef std::pair<size_t, size_t> result_type;
    InitStatic(size_t block_size_) : _size(2 * block_size_) {}
    result_type operator()(size_t) { return result_type(1ul, _size); }
  private:
    const size_t _size;
};

}  // namespace internal

class Filter: public FilterBase
{
  public:
    Filter(size_t block_size_, size_t partitions_)
      : FilterBase(internal::InitQueues(block_size_), block_size_, partitions_)
      // Note: In the beginning, there are no valid queue elements
      , _valid_indices(0)
      , _validity_flags((partitions_ > 2) ? partitions_ - 2 : 0)
    {}

    template<typename In>
    void set_filter(In first, In last);
    void set_filter(const filter_t& filter);

    bool queues_empty() const;
    void rotate_queues();

  private:
    void _set_last_partition_valid()
    {
      // With only 1 partition, there are never valid queues
      if (this->partitions() < 2) return;

      // With 2 partitions, _validity_flags are not used
      if (this->partitions() == 2)
      {
        _valid_indices = 1;
        return;
      }

      fixed_vector<char>::iterator first = _validity_flags.begin();
      if (_valid_indices > 0)
      {
        first += _valid_indices - 1;
        // Implicit value must be made explicit because _valid_indices changes
        *first++ = true;
      }
      // Previously unused values are set to 'false'
      std::fill(first, _validity_flags.end(), false);
      _valid_indices = this->partitions() - 1;
      // The last partition is now implicitly valid
    }

    // How many potentially valid partitions are in the (longest) queue?
    // The the last one of these is implicitly always true and its value
    // is not stored in _validity_flags!
    // All following indices are implicitly invalid.
    size_t _valid_indices;

    // List of valid partitions (1/0 = true/false). Only the first
    // '_valid_indices - 1' elements are considered.
    fixed_vector<char> _validity_flags;
};

/** Set a new filter (time domain).
 * The first filter partition is updated immediately, the later partitions are
 * updated with rotate_queues().  The length of the impulse response is
 * arbitrary. Zero padding is automatically performed if necessary.  If the
 * given IR is too long, it is trimmed.  If you already have the partitioned
 * transfer function of the filter in halfcomplex format, you should use
 * set_filter(const filter_t&) instead. 
 * @param first Iterator to first time-domain sample
 * @param last Past-the-end iterator
 **/
template<typename In>
void
Filter::set_filter(In first, In last)
{
  _set_filter(first, last);
  _set_last_partition_valid();
}

/** Set a new filter (frequency domain).
 * The first filter partition is updated immediately, the later partitions are
 * updated with rotate_queues().
 * @param filter Container holding the transfer functions of the zero padded 
 * filter partitions in halfcomplex format (see also FFTW documentation).
 * If too few partitions are given, the rest is set to zero, if too many are
 * given, they are ignored.
 **/
void
Filter::set_filter(const filter_t& filter)
{
  filter_t::const_iterator source = filter.begin();
  filter_queues_t::iterator queue = this->spectra.begin();

  for (
      ; source != filter.end() && queue != this->spectra.end()
      ; ++source, ++queue)
  {
    queue->back() = *source;
  }
  for (; queue != this->spectra.end(); ++queue)
  {
    // If less partitions are given, the rest is set to zero
    queue->back().set_zero();
  }
  // If further partitions are given, they are ignored

  _set_last_partition_valid();
}

/** Check if there are still valid partitions in the queues.
 * If this function returns @b false, rotate_queues() should be called.
 * @note This is important for crossfades: even if set_filter() wasn't used,
 *   older partitions may still change! If the queues are empty, no crossfade is
 *   necessary (except @p weight was changed in convolve()).
 **/
bool
Filter::queues_empty() const
{
  return _valid_indices == 0;
}

/** Update filter queues.
 * If queues_empty() returns @b true, calling this function unnecessary.
 * @note This can lead to artifacts, so a crossfade is recommended.
 **/
void
Filter::rotate_queues()
{
  if (_valid_indices == 0) return;

  size_t total = this->partitions();

  assert(_valid_indices <= total);

  filter_queues_t::iterator queue = &spectra[total - _valid_indices];

  // Second element of queue is valid
  queue->move(queue->begin(), queue->end());

  for (size_t i = total + 1 - _valid_indices; i < total; ++i)
  {
    queue = &spectra[i];
    if (_validity_flags[total - i] == true)
    {
      // Second element of queue is valid
      queue->move(queue->begin(), queue->end());
    }
    else
    {
      // Second element is invalid, keep current first element
      queue->move(++(queue->begin()), queue->end());
    }
  }

  // rotate 'valid' flags
  std::copy(&_validity_flags[1], &_validity_flags[_valid_indices]
      , &_validity_flags[0]);
  --_valid_indices;
}

class StaticFilter : public FilterBase
{
  public:
    /// Constructor.
    /// @param block_size_ block size
    /// @param first Iterator to first time-domain coefficient
    /// @param last past-the-end iterator
    template<typename In>
    StaticFilter(size_t block_size_, In first, In last, size_t partitions_ = 0)
      : FilterBase(internal::InitStatic(block_size_), block_size_
          , partitions_ ? partitions_
          : min_partitions(block_size_, std::distance(first, last)))
    {
      _set_filter(first, last);
    }
};

/** Convolution engine (output part).
 * @see Input, Filter, StaticFilter
 **/
class Output
{
  public:
    Output(const Input& input, const FilterBase& filter);

    ~Output() { fftw<float>::destroy_plan(_ifft_plan); }

    float* convolve(float weight = 1.0f);

  private:
    void _multiply_spectra();
    void _multiply_partition_cpp(const float* signal, const float* filter);
#ifdef __SSE__
    void _multiply_partition_simd(const float* signal, const float* filter);
#endif

    void _unsort_coefficients();

    void _ifft();

    const Input& _input;
    const FilterBase& _filter;

    const size_t _partition_size;

    fft_node _output_buffer;
    fftw<float>::plan _ifft_plan;
};

Output::Output(const Input& input, const FilterBase& filter)
  : _input(input)
  , _filter(filter)
  , _partition_size(input.partition_size())
  , _output_buffer(_partition_size)
  , _ifft_plan(fftw<float>::plan_r2r_1d(int(_partition_size)
      , _output_buffer.begin()
      , _output_buffer.begin(), FFTW_HC2R, FFTW_PATIENT))
{
  if (_input.spectra.size()-1 != _filter.spectra.size()
     || _input.spectra.front().size() != _filter.spectra.front().front().size())
  {
    throw std::logic_error("Output: input and filter must have the same size!");
  }
}

/** Fast convolution of one audio block.
 * @param weight amplitude weighting factor for current signal frame.
 * The filter has to be set in the constructor of StaticFilter or via
 * Filter::set_filter().
 * @return pointer to the first sample of the convolved (and weighted) signal
 **/
float*
Output::convolve(float weight)
{
  _multiply_spectra();

  // The first half will be discarded
  float* second_half = _output_buffer.begin() + _input.block_size();

  if (_output_buffer.zero())
  {
    // Nothing to be done, IFFT of zero is also zero.
    // _output_buffer was already reset to zero in _multiply_spectra().
  }
  else
  {
    _ifft();

    // normalize buffer (fftw3 does not do this)
    const float norm = weight / float(_partition_size);
    std::transform(second_half, _output_buffer.end(), second_half
        , std::bind1st(std::multiplies<float>(), norm));
  }
  return second_half;
}

void
Output::_multiply_partition_cpp(const float* signal, const float* filter)
{
  // see http://www.ludd.luth.se/~torger/brutefir.html#bruteconv_4

  float d1s = _output_buffer[0] + signal[0] * filter[0];
  float d2s = _output_buffer[4] + signal[4] * filter[4];

  for (size_t nn = 0; nn < _partition_size; nn += 8)
  {
    // real parts
    _output_buffer[nn+0] += signal[nn+0] * filter[nn + 0] -
                            signal[nn+4] * filter[nn + 4];
    _output_buffer[nn+1] += signal[nn+1] * filter[nn + 1] -
                            signal[nn+5] * filter[nn + 5];
    _output_buffer[nn+2] += signal[nn+2] * filter[nn + 2] -
                            signal[nn+6] * filter[nn + 6];
    _output_buffer[nn+3] += signal[nn+3] * filter[nn + 3] -
                            signal[nn+7] * filter[nn + 7];

    // imaginary parts
    _output_buffer[nn+4] += signal[nn+0] * filter[nn + 4] +
                            signal[nn+4] * filter[nn + 0];
    _output_buffer[nn+5] += signal[nn+1] * filter[nn + 5] +
                            signal[nn+5] * filter[nn + 1];
    _output_buffer[nn+6] += signal[nn+2] * filter[nn + 6] +
                            signal[nn+6] * filter[nn + 2];
    _output_buffer[nn+7] += signal[nn+3] * filter[nn + 7] +
                            signal[nn+7] * filter[nn + 3];

  } // for

  _output_buffer[0] = d1s;
  _output_buffer[4] = d2s;
}

#ifdef __SSE__
void
Output::_multiply_partition_simd(const float* signal, const float* filter)
{
  typedef float v4sf __attribute__ ((vector_size(16)));

  float dc = _output_buffer[0] + signal[0] * filter[0];
  float ny = _output_buffer[4] + signal[4] * filter[4];

  for(size_t i = 0; i < _partition_size; i += 8)
  {
    // load real and imaginary parts of signal and filter
    v4sf sigr = __builtin_ia32_loadups(signal + i);
    v4sf sigi = __builtin_ia32_loadups(signal + i + 4);
    v4sf filtr = __builtin_ia32_loadups(filter + i);
    v4sf filti = __builtin_ia32_loadups(filter + i + 4);

    // multiply and subtract
    v4sf res1 = __builtin_ia32_subps(__builtin_ia32_mulps(sigr, filtr),
                                     __builtin_ia32_mulps(sigi, filti));

    // multiply and add
    v4sf res2 = __builtin_ia32_addps(__builtin_ia32_mulps(sigr, filti),
                                     __builtin_ia32_mulps(sigi, filtr));

    // load output data for accumulation
    v4sf acc1 = __builtin_ia32_loadups(_output_buffer.begin() + i);
    v4sf acc2 = __builtin_ia32_loadups(_output_buffer.begin() + i + 4);

    // accumulate
    acc1 = __builtin_ia32_addps(acc1, res1);
    acc2 = __builtin_ia32_addps(acc2, res2);

    // store output data
    __builtin_ia32_storeups(_output_buffer.begin() + i, acc1);
    __builtin_ia32_storeups(_output_buffer.begin() + i + 4, acc2);
  }

  _output_buffer[0] = dc;
  _output_buffer[4] = ny;
}
#endif

/// This function performs the actual fast convolution.
void
Output::_multiply_spectra()
{
  assert(_input.spectra.size() == _filter.spectra.size() + 1);

  // Clear IFFT buffer
  std::fill(_output_buffer.begin(), _output_buffer.end(), 0.0f);
  _output_buffer.set_zero();

  filter_queue_t::const_iterator input = _input.spectra.begin();

  for (filter_queues_t::const_iterator queue = _filter.spectra.begin()
      ; queue != _filter.spectra.end()
      ; ++queue, ++input)
  {
    if (input->zero() || queue->front().zero()) continue;

    _output_buffer.set_non_zero();

#ifdef __SSE__
    _multiply_partition_simd(input->begin(), queue->front().begin());
#else
    _multiply_partition_cpp(input->begin(), queue->front().begin());
#endif
  }
}

void
Output::_unsort_coefficients()
{
  fixed_vector<float> buffer(_partition_size);

  int base = 8;

  buffer[0]                 = _output_buffer[0];
  buffer[1]                 = _output_buffer[1];
  buffer[2]                 = _output_buffer[2];
  buffer[3]                 = _output_buffer[3];
  buffer[_input.block_size()] = _output_buffer[4];
  buffer[_partition_size-1] = _output_buffer[5];
  buffer[_partition_size-2] = _output_buffer[6];
  buffer[_partition_size-3] = _output_buffer[7];

  for (size_t i=0; i < (_partition_size / 8-1); i++)
  {
    for (int ii = 0; ii < 4; ii++)
    {
      buffer[base/2+ii] = _output_buffer[base+ii];
    }

    for (int ii = 0; ii < 4; ii++)
    {
      buffer[_partition_size-base/2-ii] = _output_buffer[base+4+ii];
    }

    base += 8;
  }

  std::copy(buffer.begin(), buffer.end(), _output_buffer.begin());
}

void
Output::_ifft()
{
  _unsort_coefficients();
  fftw<float>::execute(_ifft_plan);
}

/** Convolver output stage with non-static filter.
 * The filter coefficients are set with set_filter().
 * @see Output, Filter
 **/
// TODO: better name!
struct FilterOutput : Filter, Output
{
  /// Constructor. Initially, all partitions are filled with zeros (i.e.
  /// the output will be silence unless the filter is changed with
  /// set_filter().
  explicit FilterOutput(const Input& input)
    : Filter(input.block_size(), input.partitions())
    , Output(input, *this)
  {}
};

/** Convolver output stage with static filter.
 * The filter coefficients are set in the constructor(s) and cannot be changed.
 * @see Output, StaticFilter
 **/
struct StaticFilterOutput : StaticFilter, Output
{
  /// Constructor.
  template<typename In>
  StaticFilterOutput(const Input& input, In first, In last)
    : StaticFilter(input.block_size(), first, last, input.partitions())
    , Output(input, *this)
  {}

  // TODO: constructor from frequency domain data?
};

/// Combination of Input and Output
struct InputOutput : Input, Output
{
  InputOutput(const FilterBase& filter)
    : Input(filter.block_size(), filter.partitions())
    , Output(*this, filter)
  {}
};

/// Combination of Input and FilterOutput
struct Convolver : Input, FilterOutput
{
  /// Constructor (see FilterOutput::FilterOutput)
  Convolver(size_t block_size_, size_t partitions_)
    : Input(block_size_, partitions_)
    // static_cast to resolve ambiguity
    , FilterOutput(*static_cast<Input*>(this))
  {}
};

/// Combination of Input and StaticFilterOutput
struct StaticConvolver : Input, StaticFilterOutput
{
  /// Constructor (see StaticFilterOutput::StaticFilterOutput)
  template<typename In>
  StaticConvolver(size_t block_size_, In first, In last, size_t partitions_ = 0)
    : Input(block_size_, partitions_ ? partitions_
        : min_partitions(block_size_, std::distance(first, last)))
    , StaticFilterOutput(*this, first, last)
  {}
};

/// Apply @c std::transform to a container of fft_node%s
template<typename BinaryFunction>
void transform_nested(const filter_t& in1, const filter_t& in2, filter_t& out
    , BinaryFunction f)
{
  filter_t::const_iterator it1 = in1.begin();
  filter_t::const_iterator it2 = in2.begin();

  for (filter_t::iterator it3 = out.begin()
      ; it3 != out.end()
      ; ++it3)
  {
    if (it1 == in1.end() || it1->zero())
    {
      if (it2 == in2.end() || it2->zero())
      {
        it3->set_zero();
      }
      else
      {
        assert(it2->size() == it3->size());
        std::transform(it2->begin(), it2->end(), it3->begin()
            , std::bind1st(f, 0));
        it3->set_non_zero();
      }
    }
    else
    {
      if (it2 == in2.end() || it2->zero())
      {
        assert(it1->size() == it3->size());
        std::transform(it1->begin(), it1->end(), it3->begin()
            , std::bind2nd(f, 0));
        it3->set_non_zero();
      }
      else
      {
        assert(it1->size() == it2->size());
        assert(it1->size() == it3->size());
        std::transform(it1->begin(), it1->end(), it2->begin(), it3->begin(), f);
        it3->set_non_zero();
      }
    }
    if (it1 != in1.end()) ++it1;
    if (it2 != in2.end()) ++it2;
  }
}

/// Copy all data from one filter_t to another
inline void copy_nested(const filter_t& in, filter_t& out)
{
  filter_t::const_iterator source = in.begin();

  for (filter_t::iterator target = out.begin()
      ; target != out.end()
      ; ++target)
  {
    if (source == in.end())
    {
      target->set_zero();
    }
    else
    {
      *target = *source++;
    }
  }
}

}  // namespace conv

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
