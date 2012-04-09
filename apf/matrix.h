/******************************************************************************
 * Copyright (c) 2006-2012 Quality & Usability Lab                            *
 *                         Deutsche Telekom Laboratories, TU Berlin           *
 *                         Ernst-Reuter-Platz 7, 10587 Berlin, Germany        *
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
 *                                                http://tu-berlin.de/?id=apf *
 ******************************************************************************/

/// @file
/// Matrix of data for row- and column-wise access.

#ifndef APF_MATRIX_H
#define APF_MATRIX_H

#include "apf/iterator_macros.h"
#include "apf/stride_iterator.h"
#include "apf/misc.h"

// TODO: custom allocator, e.g. with fftw_malloc()

namespace apf
{

/** Two-dimensional data storage.
 * The two dimensions have following properties:
 *   -# Channel
 *     - stored in contiguous memory
 *     - Matrix can be iterated from channels.begin() to channels.end() (using
 *       Matrix::channels_iterator)
 *     - resulting channel can be iterated from .begin() to .end() (using
 *       Matrix::channel_iterator)
 *   -# Slice
 *     - stored in memory locations with constant step size
 *     - Matrix can be iterated from slices.begin() to slices.end() (using
 *       Matrix::slices_iterator)
 *     - resulting slice can be iterated from .begin() to .end() (using
 *       Matrix::slice_iterator)
 *
 * @tparam T Type of stored data
 **/
template<typename T>
class Matrix : public has_begin_and_end<T*>, NonCopyable
{
  public:
    /// Proxy class for returning one channel of the Matrix
    typedef has_begin_and_end<T*> Channel;
    /// Iterator within a Channel
    typedef typename Channel::iterator channel_iterator;

    /// Proxy class for returning one slice of the Matrix.
    typedef has_begin_and_end<stride_iterator<channel_iterator> > Slice;
    /// Iterator within a Slice
    typedef typename Slice::iterator slice_iterator;

    typedef size_t size_type;

    class channels_iterator;
    class slices_iterator;

    /** Constructor.
     * @param max_channels Number of Channels
     * @param max_slices Number of Slices
     * @throw std::bad_alloc if there is not enough memory
     **/
    Matrix(size_type max_channels, size_type max_slices)
      : _max_channels(max_channels)
      , _max_slices(max_slices)
      , _data(new T[_max_channels * _max_slices])
      , _channel_ptrs(new T*[_max_channels])
      , channels(channels_iterator(_data, _max_slices), _max_channels)
      , slices(slices_iterator(_data, _max_channels, _max_slices), _max_slices)
    {
      assert(max_channels > 0);
      assert(max_slices   > 0);
      this->_begin = _data;
      this->_end = _data + _max_channels * _max_slices;

      size_type i = 0;
      for (channels_iterator it = channels.begin(); it != channels.end(); ++it)
      {
        _channel_ptrs[i++] = it->begin();
      }
    }

    ~Matrix() { delete [] _channel_ptrs; delete [] _data; }

    /// Fill whole Matrix with a value (default @b 0)
    void reset(const T& value = T())
    {
      std::fill(this->begin(), this->end(), T());
    }

    /** Copy channels from another matrix.
     * @param ch channels (or slices) to copy from another Matrix
     * @note A plain copy may be faster with @c std::copy() from Matrix::begin()
     *   to Matrix::end().
     * @note Anyway, a plain copy of a Matrix is rarely needed, the main reason
     *   for this function is that if you use slices instead of channels, you'll
     *   get a transposed matrix.
     * @warning The dimensions of the Matrix are not changed, they must be
     *   correct beforehand!
     **/
    template<typename Ch>
    void set_channels(const Ch& ch)
    {
      assert(std::distance(ch.begin(), ch.end())
          == std::distance(this->channels.begin(), this->channels.end()));
      assert((ch.begin() == ch.end()) ? true :
          std::distance(ch.begin()->begin(), ch.begin()->end()) ==
          std::distance(this->channels.begin()->begin()
                      , this->channels.begin()->end()));

      channels_iterator target = this->channels.begin();

      for (typename Ch::iterator it = ch.begin()
          ; it != ch.end()
          ; ++it)
      {
        std::copy(it->begin(), it->end(), target->begin());
        ++target;
      }
    }

    /// Get array of pointers to the channels
    T* const* get_channel_ptrs() const { return _channel_ptrs; }

  private:
    const size_type _max_channels;
    const size_type _max_slices;
    T* const _data;
    T** _channel_ptrs;

  public:
    /// Access to Channels; use channels.begin() and channels.end()
    const has_begin_and_end<channels_iterator> channels;
    /// Access to Slices; use slices.begin() and slices.end()
    const has_begin_and_end<slices_iterator>   slices;
};

/// Iterator over Matrix::Channel%s.
template<typename T>
class Matrix<T>::channels_iterator
                : public std::iterator<std::bidirectional_iterator_tag, Channel>
{
  private:
    typedef channels_iterator self;

    /// Helper class for operator->()
    struct ChannelArrowProxy : Channel
    {
      ChannelArrowProxy(const Channel& ch) : Channel(ch) {}
      Channel* operator->() { return this; }
    };

  public:
    /// Constructor.
    channels_iterator(channel_iterator base_iterator, size_type step)
      : _base_iterator(base_iterator, step)
      , _size(step)
    {}

    /// Dereference operator.
    /// @return a proxy object of type Matrix::Channel
    Channel operator*() const
    {
      channel_iterator temp = _base_iterator.base();
      assert(apf::no_nullptr(temp));
      return Channel(temp, temp + _size);
    }

    /// Arrow operator.
    /// @return a proxy object of type Matrix::ChannelArrowProxy
    ChannelArrowProxy operator->() const
    {
      return this->operator*();
    }

    APF_ITERATOR_BIDIRECTIONAL_EQUAL(_base_iterator)
    APF_ITERATOR_BIDIRECTIONAL_PREINCREMENT(_base_iterator)
    APF_ITERATOR_BIDIRECTIONAL_PREDECREMENT(_base_iterator)
    APF_ITERATOR_BIDIRECTIONAL_POSTINCREMENT
    APF_ITERATOR_BIDIRECTIONAL_POSTDECREMENT
    APF_ITERATOR_BIDIRECTIONAL_UNEQUAL

    APF_ITERATOR_BASE(stride_iterator<channel_iterator>, _base_iterator)

  private:
    stride_iterator<channel_iterator> _base_iterator;
    const size_type _size;
};

/// Iterator over Matrix::Slice%s.
template<typename T>
class Matrix<T>::slices_iterator
                  : public std::iterator<std::bidirectional_iterator_tag, Slice>
{
  private:
    typedef slices_iterator self;

    /// Helper class for operator->()
    struct SliceArrowProxy : Slice
    {
      SliceArrowProxy(const Slice& sl) : Slice(sl) {}
      Slice* operator->() { return this; }
    };

  public:
    /// Constructor.
    slices_iterator(channel_iterator base_iterator
        , size_type max_channels, size_type max_slices)
      : _base_iterator(base_iterator)
      , _max_channels(max_channels)
      , _max_slices(max_slices)
    {}

    /// Dereference operator.
    /// @return a proxy object of type Matrix::Slice
    Slice operator*() const
    {
      assert(apf::no_nullptr(_base_iterator));
      slice_iterator temp(_base_iterator, _max_slices);
      return Slice(temp, temp + _max_channels);
    }

    /// Arrow operator.
    /// @return a proxy object of type Matrix::SliceArrowProxy
    SliceArrowProxy operator->() const
    {
      return this->operator*();
    }

    APF_ITERATOR_BIDIRECTIONAL_EQUAL(_base_iterator)
    APF_ITERATOR_BIDIRECTIONAL_PREINCREMENT(_base_iterator)
    APF_ITERATOR_BIDIRECTIONAL_PREDECREMENT(_base_iterator)
    APF_ITERATOR_BIDIRECTIONAL_POSTINCREMENT
    APF_ITERATOR_BIDIRECTIONAL_POSTDECREMENT
    APF_ITERATOR_BIDIRECTIONAL_UNEQUAL

    APF_ITERATOR_BASE(channel_iterator, _base_iterator)

  private:
    channel_iterator _base_iterator;
    const size_type _max_channels;
    const size_type _max_slices;
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
